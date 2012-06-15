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

#include "dbus-shared-names.h"
#include "dbus-upower.h"
#include "lock-helper.h"
#include "session-menu-mgr.h"
#include "settings-helper.h"
#include "users-service-dbus.h"

#define UP_ADDRESS    "org.freedesktop.UPower"
#define UP_OBJECT     "/org/freedesktop/UPower"
#define UP_INTERFACE  "org.freedesktop.UPower"

#define EXTRA_LAUNCHER_DIR "/usr/share/indicators/session/applications"

struct _SessionMenuMgr
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

  UsersServiceDbus* users_dbus_interface;
  DbusmenuMenuitem * guest_mi;
  gboolean greeter_mode;
};

static void setup_up (SessionMenuMgr* self);
static void session_menu_mgr_rebuild_items (SessionMenuMgr *self);
static void screensaver_keybinding_changed (GSettings*, const gchar*, gpointer);

static void activate_new_session (DbusmenuMenuitem * mi,
                                  guint timestamp,
                                  gpointer user_data);
static void activate_user_session (DbusmenuMenuitem *mi,
                                   guint timestamp,
                                   gpointer user_data);
static void activate_user_accounts (DbusmenuMenuitem *mi,
                                    guint timestamp,
                                    gpointer user_data);
static gint compare_users_by_username (gconstpointer a,
                                       gconstpointer b);
static void activate_user_accounts (DbusmenuMenuitem *mi,
                                    guint timestamp,
                                    gpointer user_data);                                      
static void menu_mgr_rebuild_users (SessionMenuMgr *self);
static void on_user_list_changed (UsersServiceDbus *service,
                                  SessionMenuMgr * umm);

static void on_guest_logged_in_changed (UsersServiceDbus * users_dbus,
                                        SessionMenuMgr      * self);

static void on_user_logged_in_changed (UsersServiceDbus  * users_dbus,
                                       AccountsUser      * user,
                                       SessionMenuMgr       * self);
static gboolean is_this_guest_session (void);
static void activate_guest_session (DbusmenuMenuitem * mi,
                                    guint timestamp,
                                    gpointer user_data);
                                    


G_DEFINE_TYPE (SessionMenuMgr, session_menu_mgr, G_TYPE_OBJECT);

static void
session_menu_mgr_init (SessionMenuMgr *self)
{
  self->can_hibernate = TRUE;
  self->can_suspend = TRUE;
  self->allow_hibernate = TRUE;
  self->allow_suspend = TRUE;

  self->lockdown_settings = g_settings_new (LOCKDOWN_SCHEMA);
  g_signal_connect_swapped (self->lockdown_settings, "changed::" LOCKDOWN_KEY_USER, G_CALLBACK(session_menu_mgr_rebuild_items), self);
  g_signal_connect_swapped (self->lockdown_settings, "changed::" LOCKDOWN_KEY_SCREENSAVER, G_CALLBACK(session_menu_mgr_rebuild_items), self);

  self->keybinding_settings = g_settings_new (KEYBINDING_SCHEMA);
  g_signal_connect (self->keybinding_settings, "changed::" KEY_LOCK_SCREEN, G_CALLBACK(screensaver_keybinding_changed), self);

  setup_up(self);  

  /* users */
  self->users_dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  g_signal_connect (self->users_dbus_interface, "user-list-changed",
                    G_CALLBACK (on_user_list_changed), self);
  g_signal_connect (self->users_dbus_interface, "user-logged-in-changed",
                    G_CALLBACK(on_user_logged_in_changed), self);
  g_signal_connect (self->users_dbus_interface, "guest-logged-in-changed",
                    G_CALLBACK(on_guest_logged_in_changed), self);

  g_idle_add(lock_screen_setup, NULL);  
}

static void
session_menu_mgr_dispose (GObject *object)
{
  SessionMenuMgr * self = SESSION_MENU_MGR (object);
  g_clear_object (&self->lockdown_settings);
  g_clear_object (&self->keybinding_settings);
  g_clear_object (&self->upower_proxy);
  g_clear_object (&self->cancellable);
  g_clear_object (&self->users_dbus_interface);

  G_OBJECT_CLASS (session_menu_mgr_parent_class)->finalize (object);
}

static void
session_menu_mgr_finalize (GObject *object)
{
  G_OBJECT_CLASS (session_menu_mgr_parent_class)->finalize (object);
}

static void
session_menu_mgr_class_init (SessionMenuMgrClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = session_menu_mgr_dispose;
  object_class->finalize = session_menu_mgr_finalize;
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
  update_screensaver_shortcut (SESSION_MENU_MGR(userdata)->lock_mi, settings);
}


static void
machine_sleep_from_suspend (DbusmenuMenuitem * mi        G_GNUC_UNUSED,
                            guint              timestamp G_GNUC_UNUSED,
                            gpointer           userdata)
{
  GError * error = NULL;
  SessionMenuMgr * mgr = SESSION_MENU_MGR (userdata);

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
  SessionMenuMgr * mgr = SESSION_MENU_MGR (userdata);

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
  SessionMenuMgr * mgr = SESSION_MENU_MGR (userdata);

  indicator_session_upower_call_suspend_allowed_finish (mgr->upower_proxy, &allowed, res, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }
  else if (mgr->allow_suspend != allowed)
    {
      mgr->allow_suspend = allowed;
      session_menu_mgr_rebuild_items (mgr);
    }
}

/* When allow-hibernate changes, rebuild the menus */
static void
allowed_hibernate_cb (GObject * source, GAsyncResult * res, gpointer userdata)
{
  gboolean allowed;
  GError * error = NULL;
  SessionMenuMgr * mgr = SESSION_MENU_MGR (userdata);

  indicator_session_upower_call_hibernate_allowed_finish (mgr->upower_proxy, &allowed, res, &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_error_free (error);
    }
  else if (mgr->allow_hibernate != allowed)
    {
      mgr->allow_hibernate = allowed;
      session_menu_mgr_rebuild_items (mgr);
    }
}


static void
on_upower_properties_changed (IndicatorSessionUPower * upower_proxy, SessionMenuMgr * mgr)
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
      session_menu_mgr_rebuild_items (mgr);
    }
}

/* This function goes through and sets up what we need for DKp checking.
    We're even setting up the calls for the props we need */
static void
setup_up (SessionMenuMgr* self)
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
session_menu_mgr_build_static_items (SessionMenuMgr* self, gboolean greeter_mode)
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

  mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_append (self->parent_mi, mi);

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
session_menu_mgr_rebuild_items (SessionMenuMgr* self)
{
  dbusmenu_menuitem_property_set_bool (self->hibernate_mi,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       self->can_hibernate && self->allow_hibernate);
  dbusmenu_menuitem_property_set_bool (self->suspend_mi,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       self->can_suspend && self->allow_suspend);
}

/***
****  User Menu
***/                                       

struct ActivateUserSessionData
{
  SessionMenuMgr * menu_mgr;
  AccountsUser * user;
};

/***
****
***/

static GQuark
get_menuitem_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("menuitem");
    }

  return q;
}

static DbusmenuMenuitem*
user_get_menuitem (AccountsUser * user)
{
  return g_object_get_qdata (G_OBJECT(user), get_menuitem_quark());
}

static void
user_set_menuitem (AccountsUser * user, DbusmenuMenuitem * mi)
{
  g_message ("%s %s() associating user %s with mi %p", G_STRLOC, G_STRFUNC, accounts_user_get_user_name(user), mi);
  g_object_set_qdata_full (G_OBJECT(user), get_menuitem_quark(), g_object_ref(G_OBJECT(mi)), g_object_unref);
}

static GQuark
get_name_collision_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("name-collision");
    }

  return q;
}

static gboolean
get_user_name_collision (AccountsUser * user)
{
  return g_object_get_qdata (G_OBJECT(user), get_name_collision_quark()) != NULL;
}

static void
set_user_name_collision (AccountsUser * user, gboolean b)
{
  g_object_set_qdata (G_OBJECT(user), get_name_collision_quark(), GINT_TO_POINTER(b));
}

/***
****
***/

static void
update_menuitem_icon (DbusmenuMenuitem * mi, AccountsUser * user)
{
  const gchar * str = accounts_user_get_icon_file (user);
  
  if (!str || !*str)
    {
      str = USER_ITEM_ICON_DEFAULT;
    }

  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_ICON, str);
}

static void
on_user_icon_file_changed (AccountsUser * user, GParamSpec * pspec, DbusmenuMenuitem * mi)
{
  update_menuitem_icon (mi, user);
}

static DbusmenuMenuitem*
create_user_menuitem (SessionMenuMgr * menu_mgr, AccountsUser * user)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (mi,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  USER_ITEM_TYPE);

  /* set the name property */
  const gchar * const real_name = accounts_user_get_real_name (user);
  const gchar * const user_name = accounts_user_get_user_name (user);
  char * str = get_user_name_collision (user)
             ? g_strdup_printf ("%s (%s)", real_name, user_name)
             : g_strdup (real_name);
  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, str);
  g_free (str);

  /* set the logged-in property */
  const gboolean is_logged_in = users_service_dbus_is_user_logged_in (menu_mgr->users_dbus_interface, user);
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_LOGGED_IN,
                                       is_logged_in);

  /* set the icon property */
  update_menuitem_icon (mi, user);
  g_signal_connect (user, "notify::icon-file", G_CALLBACK(on_user_icon_file_changed), mi);

  /* set the is-current-user property */
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_IS_CURRENT_USER,
                                       !g_strcmp0 (user_name, g_get_user_name()));

  /* set the activate callback */
  struct ActivateUserSessionData * data = g_new (struct ActivateUserSessionData, 1);
  data->user = user;
  data->menu_mgr = menu_mgr;
  g_signal_connect_data (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                         G_CALLBACK (activate_user_session),
                         data, (GClosureNotify)g_free,
                         0);

  /* give this AccountsUser a hook back to this menuitem */
  user_set_menuitem (user, mi);

  /* done */
  return mi;
}

static void
on_guest_logged_in_changed (UsersServiceDbus * users_service_dbus,
                            SessionMenuMgr   * self)
{
  if (self->guest_mi != NULL)
    {
      const gboolean b = users_service_dbus_is_guest_logged_in (users_service_dbus);
      dbusmenu_menuitem_property_set_bool (self->guest_mi, USER_ITEM_PROP_LOGGED_IN, b);
    }
}

/* When a user's login state changes,
   update the corresponding menuitem's LOGGED_IN property */
static void
on_user_logged_in_changed (UsersServiceDbus  * users_service_dbus,
                           AccountsUser      * user,
                           SessionMenuMgr    * self)
{
  DbusmenuMenuitem * mi = user_get_menuitem (user);

  if (mi != NULL)
    {
      const gboolean b = users_service_dbus_is_user_logged_in (users_service_dbus, user);
      dbusmenu_menuitem_property_set_bool (mi, USER_ITEM_PROP_LOGGED_IN, b);
    }
}

/* Builds up the menu for us */
static void 
menu_mgr_rebuild_users (SessionMenuMgr *self)
{
  GList *u;
  gboolean can_activate;

  /* Check to see which menu items we're allowed to have */
  can_activate = users_service_dbus_can_activate_session (self->users_dbus_interface) &&
      !g_settings_get_boolean (self->lockdown_settings, LOCKDOWN_KEY_USER);

  /* Remove the old menu items if that makes sense */
#warning FIXME
#if 0
  GList * children = dbusmenu_menuitem_take_children (self->root_item);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);
#endif

  /* Set to NULL in case we don't end up building one */
  self->guest_mi = NULL;

  /* Build all of the user switching items */
  if (can_activate)
  {
    /* TODO: This needs to be updated once the ability to query guest session support is available */
    GList * users = users_service_dbus_get_user_list (self->users_dbus_interface);
    const gboolean guest_enabled = users_service_dbus_guest_session_enabled (self->users_dbus_interface);
    
    /* TODO we should really return here if the menu is not going to be shown. */
    
    DbusmenuMenuitem * switch_menuitem = dbusmenu_menuitem_new ();
/*
    dbusmenu_menuitem_property_set (switch_menuitem,
                                    DBUSMENU_MENUITEM_PROP_TYPE,
                                    MENU_SWITCH_TYPE);
*/
    dbusmenu_menuitem_property_set (switch_menuitem,
                                    USER_ITEM_PROP_NAME,
                                    _("Switch User Account…"));
    dbusmenu_menuitem_child_append (self->parent_mi, switch_menuitem);
g_message ("appending Switch button to %p", self->parent_mi);
    g_signal_connect (G_OBJECT (switch_menuitem),
                      DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK (activate_new_session),
                      self);
    
    if ( !is_this_guest_session () && guest_enabled)
    {
      self->guest_mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (self->guest_mi,
                                      DBUSMENU_MENUITEM_PROP_TYPE,
                                      USER_ITEM_TYPE);
      dbusmenu_menuitem_property_set (self->guest_mi,
                                      USER_ITEM_PROP_NAME,
                                      _("Guest Session"));
      on_guest_logged_in_changed (self->users_dbus_interface, self);
      dbusmenu_menuitem_child_append (self->parent_mi, self->guest_mi);
      g_signal_connect (G_OBJECT (self->guest_mi),
                        DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                        G_CALLBACK (activate_guest_session),
                        self);
    }
    else{
      session_dbus_set_users_real_name (self->session_dbus_interface,
                                        _("Guest"));      
    }
    
    

    users = g_list_sort (users, compare_users_by_username);

    for (u = users; u != NULL; u = g_list_next (u))
      {
        AccountsUser * user = u->data;

        DbusmenuMenuitem * mi = create_user_menuitem (self, user);
        dbusmenu_menuitem_child_append (self->parent_mi, mi);

        const char * const user_name = accounts_user_get_user_name (user);
        if (!g_strcmp0 (user_name, g_get_user_name()))
          {
            const char * const real_name = accounts_user_get_real_name (user);
            session_dbus_set_users_real_name (self->session_dbus_interface, real_name);
          }
      }

    g_list_free(users);
  }

  // Add the user accounts and separator
  DbusmenuMenuitem * separator1 = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (separator1,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_SEPARATOR);
  dbusmenu_menuitem_child_append (self->parent_mi, separator1);

  DbusmenuMenuitem * user_accounts_item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set (user_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_DEFAULT);
  dbusmenu_menuitem_property_set (user_accounts_item,
                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                  _("User Accounts…"));

  g_signal_connect (G_OBJECT (user_accounts_item),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_user_accounts),
                    NULL);
                                  
  dbusmenu_menuitem_child_append (self->parent_mi, user_accounts_item); 


}

/* Check to see if the lockdown key is protecting from
   locking the screen.  If not, lock it. */
static void
lock_if_possible (SessionMenuMgr * menu_mgr)
{
  if (!g_settings_get_boolean (menu_mgr->lockdown_settings, LOCKDOWN_KEY_SCREENSAVER))
    {
      lock_screen(NULL, 0, NULL);
    }
}

/* Starts a new generic session */
static void
activate_new_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
  SessionMenuMgr * menu_mgr = SESSION_MENU_MGR (user_data);
  g_return_if_fail (menu_mgr != NULL);

  lock_if_possible (menu_mgr);
  users_service_dbus_show_greeter (menu_mgr->users_dbus_interface);
}

/* Activates a session for a particular user. */
static void
activate_user_session (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  struct ActivateUserSessionData * data = user_data;

  lock_if_possible (data->menu_mgr);
  users_service_dbus_activate_user_session (data->menu_mgr->users_dbus_interface, data->user);
}

/* Comparison function to look into the UserData struct
   to compare by using the username value */
static gint
compare_users_by_username (gconstpointer ga, gconstpointer gb)
{
  AccountsUser * a = ACCOUNTS_USER(ga);
  AccountsUser * b = ACCOUNTS_USER(gb);

  const int ret = g_strcmp0 (accounts_user_get_real_name (a),
                             accounts_user_get_real_name (b));

  if (!ret) /* names are the same, so both have a name collision */
    {
      set_user_name_collision (a, TRUE);
      set_user_name_collision (b, TRUE);
    }

  return ret;
}

static void
activate_user_accounts (DbusmenuMenuitem *mi,
                          guint timestamp,
                          gpointer user_data)
{
  GError * error = NULL;
  if (!g_spawn_command_line_async("gnome-control-center user-accounts", &error))
  {
    g_warning("Unable to show control centre: %s", error->message);
    g_error_free(error);
  }
}

/* Signal called when a user is added.
   It updates the count and rebuilds the menu */
static void
on_user_list_changed (UsersServiceDbus * service   G_GNUC_UNUSED, 
                      SessionMenuMgr   * menu_mgr)
{
  g_return_if_fail (IS_SESSION_MENU_MGR(menu_mgr));

  menu_mgr_rebuild_users (menu_mgr);
}


/* Checks to see if we should show the guest session item.
   System users shouldn't have guest account shown.
   Mostly this would be the case of the guest user itself. */
static gboolean
is_this_guest_session (void)
{
  return geteuid() < 500;
}

/* Called when someone clicks on the guest session item. */
static void
activate_guest_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
  SessionMenuMgr * menu_mgr = SESSION_MENU_MGR (user_data);
  g_return_if_fail (menu_mgr != NULL);

  lock_if_possible (menu_mgr);
  users_service_dbus_activate_guest_session (menu_mgr->users_dbus_interface);
}

/***
****
***/

SessionMenuMgr* session_menu_mgr_new (DbusmenuMenuitem  * parent_mi,
                                      SessionDbus       * session_dbus,
                                      gboolean            greeter_mode)
{
  SessionMenuMgr* menu_mgr = g_object_new (SESSION_TYPE_MENU_MGR, NULL);
  menu_mgr->parent_mi = parent_mi;
  menu_mgr->greeter_mode = greeter_mode;
  menu_mgr->session_dbus_interface = session_dbus;
  session_menu_mgr_build_static_items (menu_mgr, greeter_mode);
  menu_mgr_rebuild_users (menu_mgr);
  return menu_mgr;
}
