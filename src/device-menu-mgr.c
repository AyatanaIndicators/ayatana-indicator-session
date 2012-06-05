/*
Copyright 2011 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include <glib.h>
#include <gio/gio.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-gtk/menuitem.h>

#include "device-menu-mgr.h"
#include "settings-helper.h"
#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"
#include "lock-helper.h"
#include "upower-client.h"

#define UP_ADDRESS    "org.freedesktop.UPower"
#define UP_OBJECT     "/org/freedesktop/UPower"
#define UP_INTERFACE  "org.freedesktop.UPower"

#define EXTRA_LAUNCHER_DIR "/usr/share/indicators/session/applications"

struct _DeviceMenuMgr
{
  GObject parent_instance;
  DbusmenuMenuitem* root_item;
  SessionDbus* session_dbus_interface;  

  GSettings *lockdown_settings;
  GSettings * keybinding_settings;

  DbusmenuMenuitem * hibernate_mi;
  DbusmenuMenuitem * suspend_mi;
  DbusmenuMenuitem * lock_mi;

  DBusGProxy * up_main_proxy;
  DBusGProxy * up_prop_proxy;

  gboolean can_hibernate;
  gboolean can_suspend;
  gboolean allow_hibernate;
  gboolean allow_suspend;

  DBusGProxyCall * suspend_call;
  DBusGProxyCall * hibernate_call;
};

static void setup_up (DeviceMenuMgr* self);
static void device_menu_mgr_rebuild_items (DeviceMenuMgr *self);
static void machine_sleep_with_context (DeviceMenuMgr* self,
                                        const gchar* type);
static void show_system_settings (DbusmenuMenuitem * mi,
                                  guint timestamp,
                                  gpointer userdata);
static void screensaver_keybinding_changed (GSettings*, const gchar*, gpointer);
static void
machine_sleep_from_hibernate (DbusmenuMenuitem * mi,
                              guint timestamp,
                              gpointer userdata);
static void
machine_sleep_from_suspend (DbusmenuMenuitem * mi,
                            guint timestamp,
                            gpointer userdata);

G_DEFINE_TYPE (DeviceMenuMgr, device_menu_mgr, G_TYPE_OBJECT);

static void
device_menu_mgr_init (DeviceMenuMgr *self)
{
  self->root_item = dbusmenu_menuitem_new ();  

  self->can_hibernate = TRUE;
  self->can_suspend = TRUE;
  self->allow_hibernate = TRUE;
  self->allow_suspend = TRUE;

  self->lockdown_settings = g_settings_new (LOCKDOWN_SCHEMA);
  g_signal_connect_swapped (self->lockdown_settings, "changed::" LOCKDOWN_KEY_USER, G_CALLBACK(device_menu_mgr_rebuild_items), self);
  g_signal_connect_swapped (self->lockdown_settings, "changed::" LOCKDOWN_KEY_SCREENSAVER, G_CALLBACK(device_menu_mgr_rebuild_items), self);

  self->keybinding_settings = g_settings_new (KEYBINDING_SCHEMA);
  g_signal_connect (self->keybinding_settings, "changed::" KEY_LOCK_SCREEN, G_CALLBACK(screensaver_keybinding_changed), self);

  setup_up(self);  
  g_idle_add(lock_screen_setup, NULL);  
}

static void
device_menu_mgr_dispose (GObject *object)
{
  DeviceMenuMgr * self = DEVICE_MENU_MGR (object);
  g_clear_object (&self->lockdown_settings);
  g_clear_object (&self->keybinding_settings);
  g_clear_object (&self->up_main_proxy);
  g_clear_object (&self->up_prop_proxy);

  G_OBJECT_CLASS (device_menu_mgr_parent_class)->finalize (object);
}

static void
device_menu_mgr_finalize (GObject *object)
{
  G_OBJECT_CLASS (device_menu_mgr_parent_class)->finalize (object);
}

static void
device_menu_mgr_class_init (DeviceMenuMgrClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = device_menu_mgr_dispose;
  object_class->finalize = device_menu_mgr_finalize;
}

/***
****
***/

static void
update_screensaver_shortcut (DbusmenuMenuitem * menuitem, GSettings * settings)
{
  if (menuitem != NULL)
    {
      gchar * val = g_settings_get_string (settings, KEY_LOCK_SCREEN);
      g_debug ("Keybinding changed to: %s", val);
      dbusmenu_menuitem_property_set_shortcut_string (menuitem, val);
      g_free (val);
    }
}

static void
screensaver_keybinding_changed (GSettings   * settings,
                                const gchar * key  G_GNUC_UNUSED,
                                gpointer      userdata)
{
  update_screensaver_shortcut (DEVICE_MENU_MGR(userdata)->lock_mi, settings);
}

static void
machine_sleep_from_suspend (DbusmenuMenuitem * mi        G_GNUC_UNUSED,
                            guint              timestamp G_GNUC_UNUSED,
                            gpointer           userdata)
{
  machine_sleep_with_context (DEVICE_MENU_MGR(userdata), "Suspend");
}

static void
machine_sleep_from_hibernate (DbusmenuMenuitem * mi        G_GNUC_UNUSED,
                              guint              timestamp G_GNUC_UNUSED,
                              gpointer           userdata)
{
  machine_sleep_with_context (DEVICE_MENU_MGR(userdata), "Hibernate");
}

/* Let's put this machine to sleep, with some info on how
   it should sleep.  */
static void
machine_sleep_with_context (DeviceMenuMgr* self, const gchar* type)
{
	if (self->up_main_proxy == NULL) {
		g_warning("Can not %s as no upower proxy", type);
	}

	dbus_g_proxy_begin_call(self->up_main_proxy,
	                        type,
	                        NULL,
	                        NULL,
	                        NULL,
	                        G_TYPE_INVALID);

	return;
}

/* A response to getting the suspend property */
static void
suspend_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
  DeviceMenuMgr* self = DEVICE_MENU_MGR (userdata);

  self->suspend_call = NULL;
  
	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check suspend: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Suspend: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	gboolean local_can_suspend = g_value_get_boolean(&candoit);
	if (local_can_suspend != self->can_suspend) {
		self->can_suspend = local_can_suspend;
    // TODO figure out what needs updating on the menu
    // And add or remove it but just don't rebuild the whole menu
    // a waste
		device_menu_mgr_rebuild_items(self);
	}
	return;
}

/* Response to getting the hibernate property */
static void
hibernate_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
  DeviceMenuMgr* self = DEVICE_MENU_MGR (userdata);

  self->hibernate_call = NULL;

	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check hibernate: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Hibernate: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	gboolean local_can_hibernate = g_value_get_boolean(&candoit);
	if (local_can_hibernate != self->can_hibernate) {
		self->can_hibernate = local_can_hibernate;
		device_menu_mgr_rebuild_items(self);
	}
}

/* A signal that we need to recheck to ensure we can still
   hibernate and/or suspend */
static void
up_changed_cb (DBusGProxy * proxy, gpointer user_data)
{
	DeviceMenuMgr * self = DEVICE_MENU_MGR(user_data);

	/* Start Async call to see if we can hibernate */
	if (self->suspend_call == NULL) {
		self->suspend_call = dbus_g_proxy_begin_call(self->up_prop_proxy,
		                                       "Get",
		                                       suspend_prop_cb,
		                                       user_data,
		                                       NULL,
		                                       G_TYPE_STRING,
		                                       UP_INTERFACE,
		                                       G_TYPE_STRING,
		                                       "CanSuspend",
		                                       G_TYPE_INVALID,
		                                       G_TYPE_VALUE,
		                                       G_TYPE_INVALID);
	}

	/* Start Async call to see if we can suspend */
	if (self->hibernate_call == NULL) {
		self->hibernate_call = dbus_g_proxy_begin_call(self->up_prop_proxy,
		                                         "Get",
		                                         hibernate_prop_cb,
		                                         user_data,
		                                         NULL,
		                                         G_TYPE_STRING,
		                                         UP_INTERFACE,
		                                         G_TYPE_STRING,
		                                         "CanHibernate",
		                                         G_TYPE_INVALID,
		                                         G_TYPE_VALUE,
		                                         G_TYPE_INVALID);
	}
}

/* If the allow-suspend value changed, rebuild the menus */
static void
allowed_suspend_cb (DBusGProxy * proxy G_GNUC_UNUSED,
                    gboolean     allow_suspend,
                    GError     * error,
                    gpointer     userdata)
{
  DeviceMenuMgr * self = DEVICE_MENU_MGR (userdata);

  if (error != NULL)
    {
      g_warning("Unable to get information on what is allowed from UPower: %s", error->message);
    }
  else if (self->allow_suspend != allow_suspend)
    {
      self->allow_suspend = allow_suspend;
      device_menu_mgr_rebuild_items (self);
    }
}

/* If the allow-hibernate value changed, rebuild the menus */
static void
allowed_hibernate_cb (DBusGProxy * proxy,
                      gboolean     allow_hibernate,
                      GError     * error,
                      gpointer     userdata)
{
  DeviceMenuMgr * self = DEVICE_MENU_MGR (userdata);

  if (error != NULL)
    {
      g_warning("Unable to get information on what is allowed from UPower: %s", error->message);
    }
  else if (self->allow_hibernate != allow_hibernate)
    {
      self->allow_hibernate = allow_hibernate;
      device_menu_mgr_rebuild_items (self);
    }
}

/* This function goes through and sets up what we need for
   DKp checking.  We're even setting up the calls for the props
   we need */
static void
setup_up (DeviceMenuMgr* self) {
	DBusGConnection * bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	g_return_if_fail(bus != NULL);

	if (self->up_main_proxy == NULL) {
		self->up_main_proxy = dbus_g_proxy_new_for_name(bus,
		                                           UP_ADDRESS,
		                                           UP_OBJECT,
		                                           UP_INTERFACE);
	}
	g_return_if_fail(self->up_main_proxy != NULL);

	if (self->up_prop_proxy == NULL) {
		self->up_prop_proxy = dbus_g_proxy_new_for_name(bus,
                                              UP_ADDRESS,
                                              UP_OBJECT,
                                              DBUS_INTERFACE_PROPERTIES);
		/* Connect to changed signal */
		dbus_g_proxy_add_signal(self->up_main_proxy,
		                        "Changed",
		                        G_TYPE_INVALID);

		dbus_g_proxy_connect_signal(self->up_main_proxy,
		                            "Changed",
		                            G_CALLBACK(up_changed_cb),
		                            self,
		                            NULL);
	}
	g_return_if_fail(self->up_prop_proxy != NULL);


	/* Force an original "changed" event */
	up_changed_cb(self->up_main_proxy, self);

	/* Check to see if these are getting blocked by PolicyKit */
	org_freedesktop_UPower_suspend_allowed_async(self->up_main_proxy,
	                                             allowed_suspend_cb,
	                                             self);
	org_freedesktop_UPower_hibernate_allowed_async(self->up_main_proxy,
	                                               allowed_hibernate_cb,
	                                               self);

	return;
}

/* This is the function to show a dialog on actions that
   can destroy data.  Currently it just calls the GTK version
   but it seems that in the future it should figure out
   what's going on and something better. */
static void
show_dialog (DbusmenuMenuitem * mi, guint timestamp, gchar * type)
{

#ifdef HAVE_GTKLOGOUTHELPER
	gchar * helper = g_build_filename(LIBEXECDIR, "gtk-logout-helper", NULL);
#else
	gchar * helper = g_build_filename("gnome-session-quit", NULL);
#endif  /* HAVE_GTKLOGOUTHELPER */
	gchar * dialog_line = g_strdup_printf("%s --%s", helper, type);
	g_free(helper);

	g_debug("Showing dialog '%s'", dialog_line);

	GError * error = NULL;
	if (!g_spawn_command_line_async(dialog_line, &error)) {
		g_warning("Unable to show dialog: %s", error->message);
		g_error_free(error);
	}
	g_free(dialog_line);  
}

static void
show_system_settings (DbusmenuMenuitem  * mi         G_GNUC_UNUSED,
                      guint               timestamp  G_GNUC_UNUSED,
                      gpointer            user_data  G_GNUC_UNUSED)
{
  const char * const cmd = "gnome-control-center";

  GError * error = NULL;
  if (!g_spawn_command_line_async (cmd, &error))
  {
    g_warning("Unable to show dialog: %s", error->message);
    g_error_free(error);
  }
}

static void
device_menu_mgr_build_settings_items (DeviceMenuMgr* self)
{
  DbusmenuMenuitem * mi;

  /* system settings... */
  mi = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, _("System Settings…"));
  dbusmenu_menuitem_child_add_position(self->root_item, mi, 0);
  g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK(show_system_settings), NULL);
 
  /* separator */ 
  mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_add_position (self->root_item, mi, 1);
}

static void
device_menu_mgr_build_devices_items (DeviceMenuMgr* self)
{
  DbusmenuMenuitem * device_heading = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (device_heading,
                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                  _("Attached Devices"));
  dbusmenu_menuitem_property_set_bool (device_heading,
                                       DBUSMENU_MENUITEM_PROP_ENABLED,
                                       FALSE);
  dbusmenu_menuitem_child_add_position (self->root_item,
                                        device_heading,
                                        5);

  DbusmenuMenuitem * separator3 = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (separator3,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_add_position (self->root_item, separator3, 6);
}

static void
device_menu_mgr_build_static_items (DeviceMenuMgr* self, gboolean greeter_mode)
{
  DbusmenuMenuitem * logout_mi = NULL;
  DbusmenuMenuitem * shutdown_mi = NULL;

  // Static Setting items
  if (!greeter_mode) {
    device_menu_mgr_build_settings_items (self);
  }

  // Devices control
  if (!greeter_mode) {
    device_menu_mgr_build_devices_items (self);
  }

  // Session control  
  if (!greeter_mode) {
    gboolean can_lockscreen;

    /* Make sure we have a valid GConf client, and build one
       if needed */
    can_lockscreen = !g_settings_get_boolean (self->lockdown_settings,
                                              LOCKDOWN_KEY_SCREENSAVER);
    /* Lock screen item */
    if (can_lockscreen) {
      self->lock_mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (self->lock_mi,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      _("Lock Screen"));

      update_screensaver_shortcut (self->lock_mi, self->keybinding_settings);

      g_signal_connect (G_OBJECT(self->lock_mi),
                        DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK(lock_screen), NULL);
      dbusmenu_menuitem_child_append(self->root_item, self->lock_mi);
    }

    logout_mi = dbusmenu_menuitem_new();

    if (supress_confirmations()) {
      dbusmenu_menuitem_property_set (logout_mi,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      _("Log Out"));
    }
    else {
      dbusmenu_menuitem_property_set (logout_mi,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      _("Log Out\342\200\246"));
    }
    dbusmenu_menuitem_property_set_bool (logout_mi,
                                         DBUSMENU_MENUITEM_PROP_VISIBLE,
                                         show_logout());
    dbusmenu_menuitem_child_append(self->root_item, logout_mi);
    g_signal_connect( G_OBJECT(logout_mi),
                      DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK(show_dialog), "logout");
  }

	if (self->can_suspend && self->allow_suspend) {
		self->suspend_mi = dbusmenu_menuitem_new();
		dbusmenu_menuitem_property_set (self->suspend_mi,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Suspend"));
		dbusmenu_menuitem_child_append (self->root_item, self->suspend_mi);
		g_signal_connect( G_OBJECT(self->suspend_mi),
                      DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK(machine_sleep_from_suspend),
                      self);
	}

	if (self->can_hibernate && self->allow_hibernate) {
		self->hibernate_mi = dbusmenu_menuitem_new();
		dbusmenu_menuitem_property_set (self->hibernate_mi,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Hibernate"));
		dbusmenu_menuitem_child_append(self->root_item, self->hibernate_mi);
		g_signal_connect (G_OBJECT(self->hibernate_mi),
                      DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK(machine_sleep_from_hibernate), self);
	}
  
	shutdown_mi = dbusmenu_menuitem_new();

	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set (shutdown_mi,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Shut Down"));
	}
  else {
		dbusmenu_menuitem_property_set (shutdown_mi,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Shut Down\342\200\246"));
	}
	dbusmenu_menuitem_property_set_bool (shutdown_mi,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       show_shutdown());
	dbusmenu_menuitem_child_append (self->root_item, shutdown_mi);
	g_signal_connect (G_OBJECT(shutdown_mi),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
#ifdef HAVE_GTKLOGOUTHELPER
                    G_CALLBACK(show_dialog), "shutdown");
#else
                    G_CALLBACK(show_dialog), "power-off");
#endif  /* HAVE_GTKLOGOUTHELPER */

	RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi = g_new0 (RestartShutdownLogoutMenuItems, 1);
	restart_shutdown_logout_mi->logout_mi = logout_mi;
	restart_shutdown_logout_mi->shutdown_mi = shutdown_mi;

	update_menu_entries(restart_shutdown_logout_mi);
}

static void
device_menu_mgr_rebuild_items (DeviceMenuMgr* self)
{
  dbusmenu_menuitem_property_set_bool (self->hibernate_mi,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       self->can_hibernate && self->allow_hibernate);
  dbusmenu_menuitem_property_set_bool (self->suspend_mi,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       self->can_suspend && self->allow_suspend);
}                                       

DbusmenuMenuitem*
device_mgr_get_root_item (DeviceMenuMgr* self)
{
  return self->root_item;
}

/*
 * Clean Entry Point 
 */
DeviceMenuMgr* device_menu_mgr_new (SessionDbus* session_dbus, gboolean greeter_mode)
{
  DeviceMenuMgr* device_mgr = g_object_new (DEVICE_TYPE_MENU_MGR, NULL);
  device_mgr->session_dbus_interface = session_dbus;
  device_menu_mgr_build_static_items (device_mgr, greeter_mode);
  return device_mgr;
}
