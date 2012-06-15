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

#include "dbus-upower.h"
#include "device-menu-mgr.h"
#include "settings-helper.h"
#include "dbus-shared-names.h"
#include "lock-helper.h"

#define UP_ADDRESS    "org.freedesktop.UPower"
#define UP_OBJECT     "/org/freedesktop/UPower"
#define UP_INTERFACE  "org.freedesktop.UPower"

#define EXTRA_LAUNCHER_DIR "/usr/share/indicators/session/applications"

struct _DeviceMenuMgr
{
  GObject parent_instance;
  DbusmenuMenuitem * parent_mi;
  SessionDbus* session_dbus_interface;  

  GSettings *lockdown_settings;
  GSettings * keybinding_settings;

  DbusmenuMenuitem * hibernate_mi;
  DbusmenuMenuitem * suspend_mi;
  DbusmenuMenuitem * lock_mi;

  gboolean can_hibernate;
  gboolean can_suspend;
  gboolean allow_hibernate;
  gboolean allow_suspend;

  IndicatorSessionUPower * upower_proxy;
  GCancellable * cancellable;
};

static void setup_up (DeviceMenuMgr* self);
static void device_menu_mgr_rebuild_items (DeviceMenuMgr *self);
static void screensaver_keybinding_changed (GSettings*, const gchar*, gpointer);

G_DEFINE_TYPE (DeviceMenuMgr, device_menu_mgr, G_TYPE_OBJECT);

static void
device_menu_mgr_init (DeviceMenuMgr *self)
{
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
  g_clear_object (&self->upower_proxy);
  g_clear_object (&self->cancellable);

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
  GError * error = NULL;
  DeviceMenuMgr * mgr = DEVICE_MENU_MGR (userdata);

  indicator_session_upower_call_suspend_sync (mgr->upower_proxy, mgr->cancellable, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
}

static void
machine_sleep_from_hibernate (DbusmenuMenuitem * mi        G_GNUC_UNUSED,
                              guint              timestamp G_GNUC_UNUSED,
                              gpointer           userdata)
{
  GError * error = NULL;
  DeviceMenuMgr * mgr = DEVICE_MENU_MGR (userdata);

  indicator_session_upower_call_hibernate_sync (mgr->upower_proxy, mgr->cancellable, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
}

/***
****
***/

/* When allow-suspend changes, rebuild the menus */
static void
allowed_suspend_cb (GObject * source, GAsyncResult * res, gpointer userdata)
{
  gboolean allowed;
  GError * error = NULL;
  DeviceMenuMgr * mgr = DEVICE_MENU_MGR (userdata);

  indicator_session_upower_call_suspend_allowed_finish (mgr->upower_proxy, &allowed, res, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }
  else if (mgr->allow_suspend != allowed)
    {
      mgr->allow_suspend = allowed;
      device_menu_mgr_rebuild_items (mgr);
    }
}

/* When allow-hibernate changes, rebuild the menus */
static void
allowed_hibernate_cb (GObject * source, GAsyncResult * res, gpointer userdata)
{
  gboolean allowed;
  GError * error = NULL;
  DeviceMenuMgr * mgr = DEVICE_MENU_MGR (userdata);

  indicator_session_upower_call_hibernate_allowed_finish (mgr->upower_proxy, &allowed, res, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }
  else if (mgr->allow_hibernate != allowed)
    {
      mgr->allow_hibernate = allowed;
      device_menu_mgr_rebuild_items (mgr);
    }
}


static void
on_upower_properties_changed (IndicatorSessionUPower * upower_proxy, DeviceMenuMgr * mgr)
{
  gboolean b;
  gboolean refresh = FALSE;

  /* suspend */
  b = indicator_session_upower_get_can_suspend (upower_proxy);
  if (mgr->can_suspend != b)
    {
      mgr->can_suspend = b;
      refresh = TRUE;
    }

  /* hibernate */
  b = indicator_session_upower_get_can_hibernate (upower_proxy);
  if (mgr->can_hibernate != b)
    {
      mgr->can_hibernate = b;
      refresh = TRUE;
    }

  if (refresh)
    {
      device_menu_mgr_rebuild_items (mgr);
    }
}

/* This function goes through and sets up what we need for DKp checking.
    We're even setting up the calls for the props we need */
static void
setup_up (DeviceMenuMgr* self)
{
  self->cancellable = g_cancellable_new ();

  GError * error = NULL;
  self->upower_proxy = indicator_session_upower_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                                        0,
                                                                        UP_ADDRESS,
                                                                        UP_OBJECT,
                                                                        NULL,
                                                                        &error);
  if (error != NULL)
    {
      g_warning ("Error creating cups notify handler: %s", error->message);
      g_error_free (error);
    }
  else
    {
      /* Check to see if these are getting blocked by PolicyKit */
      indicator_session_upower_call_suspend_allowed (self->upower_proxy, self->cancellable, allowed_suspend_cb, self);
      indicator_session_upower_call_hibernate_allowed (self->upower_proxy, self->cancellable, allowed_hibernate_cb, self);

      g_signal_connect (self->upower_proxy, "changed", G_CALLBACK(on_upower_properties_changed), self);

      /* trigger an initial "changed" event */
      on_upower_properties_changed (self->upower_proxy, self);
    }


}

static void
spawn_command_line_async (const char * fmt, ...)
{
  va_list marker;
  va_start (marker, fmt);
  gchar * cmd = g_strdup_vprintf (fmt, marker);
  va_end (marker);
  
  GError * error = NULL; 
  if (!g_spawn_command_line_async (cmd, &error))
    {
      g_warning ("Unable to show \"%s\": %s", cmd, error->message);
    }

  g_clear_error (&error);
  g_free (cmd);
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
  spawn_command_line_async ("%s --%s", helper, type);
  g_free (helper);
}

static void
device_menu_mgr_build_static_items (DeviceMenuMgr* self, gboolean greeter_mode)
{
  const char * name;
  DbusmenuMenuitem * mi;
  DbusmenuMenuitem * logout_mi = NULL;
  DbusmenuMenuitem * shutdown_mi = NULL;

  /***
  ****  Admin items
  ***/

  name = _("About This Computer");
  mi = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
  dbusmenu_menuitem_child_append (self->parent_mi, mi);
g_message ("appending About This Computer to %p", self->parent_mi);

  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(spawn_command_line_async), "gnome-control-center info");

  name = _("Ubuntu Help");
  mi = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
  dbusmenu_menuitem_child_append (self->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(spawn_command_line_async), "yelp");
 
  if (!greeter_mode)
    {
      mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
      dbusmenu_menuitem_child_append (self->parent_mi, mi);

      name = _("System Settings…");
      mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
      dbusmenu_menuitem_child_append (self->parent_mi, mi);
      g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                                G_CALLBACK(spawn_command_line_async), "gnome-control-center");
    }

  /***
  ****  Account-switching items
  ***/

  /* TODO: FIXME */

  const gboolean can_lockscreen = !g_settings_get_boolean (self->lockdown_settings, LOCKDOWN_KEY_SCREENSAVER);
  if (can_lockscreen)
    {
      name = _("Lock Screen");
      self->lock_mi = mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
      update_screensaver_shortcut (mi, self->keybinding_settings);
      dbusmenu_menuitem_child_append (self->parent_mi, mi);
      g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK(lock_screen), NULL);
    }

  /***
  ****  Session Items
  ***/

  mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_append (self->parent_mi, mi);

  if (!greeter_mode)
    {
      name = supress_confirmations() ? _("Log Out") : _("Log Out\342\200\246");
      logout_mi = mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
      dbusmenu_menuitem_property_set_bool (mi, DBUSMENU_MENUITEM_PROP_VISIBLE, show_logout());
      dbusmenu_menuitem_child_append(self->parent_mi, mi);
      g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK(show_dialog), "logout");
    }

  if (self->can_suspend && self->allow_suspend)
    {
      name = _("Suspend");
      self->suspend_mi = mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
      dbusmenu_menuitem_child_append (self->parent_mi, mi);
      g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK(machine_sleep_from_suspend), self);
    }

  if (self->can_hibernate && self->allow_hibernate)
    {
      name = _("Hibernate");
      self->hibernate_mi = mi = dbusmenu_menuitem_new();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
      dbusmenu_menuitem_child_append(self->parent_mi, mi);
      g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK(machine_sleep_from_hibernate), self);
    }

  name = _("Restart");
  mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
  dbusmenu_menuitem_child_append (self->parent_mi, mi);
  /* FIXME: not implemented */
 
  name = supress_confirmations() ? _("Shut Down") : _("Shut Down\342\200\246");
  shutdown_mi = mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, name);
  dbusmenu_menuitem_property_set_bool (mi, DBUSMENU_MENUITEM_PROP_VISIBLE, show_shutdown());
  dbusmenu_menuitem_child_append (self->parent_mi, mi);
  g_signal_connect (G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
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

/*
 * Clean Entry Point 
 */
DeviceMenuMgr* device_menu_mgr_new (DbusmenuMenuitem  * parent_mi,
                                    SessionDbus       * session_dbus,
                                    gboolean            greeter_mode)
{
  DeviceMenuMgr* device_mgr = g_object_new (DEVICE_TYPE_MENU_MGR, NULL);
  device_mgr->parent_mi = parent_mi;
  device_mgr->session_dbus_interface = session_dbus;
  device_menu_mgr_build_static_items (device_mgr, greeter_mode);
  return device_mgr;
}
