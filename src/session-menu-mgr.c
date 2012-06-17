/*
Copyright 2011 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>
    Charles Kerr <charles.kerr@canonical.com>

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

#include <sys/types.h>
#include <pwd.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-gtk/menuitem.h>

#include "dbus-shared-names.h"
#include "dbus-upower.h"
#include "session-menu-mgr.h"
#include "users-service-dbus.h"

#define UPOWER_ADDRESS    "org.freedesktop.UPower"
#define UPOWER_PATH       "/org/freedesktop/UPower"

#define CMD_HELP            "yelp"
#define CMD_INFO            "gnome-control-center info"
#define CMD_SYSTEM_SETTINGS "gnome-control-center"
#ifdef HAVE_GTKLOGOUTHELPER
 #define CMD_LOGOUT   LIBEXECDIR"/gtk-logout-helper --logout"
 #define CMD_SHUTDOWN LIBEXECDIR"/gtk-logout-helper --shutdown"
 #define CMD_RESTART  LIBEXECDIR"/gtk-logout-helper --restart"
#else
 #define CMD_LOGOUT   "gnome-session-quit --logout"
 #define CMD_SHUTDOWN "gnome-session-quit --power-off"
 #define CMD_RESTART  CMD_SHUTDOWN /* hmm, no gnome-session-quit --restart? */
#endif

/**
 * Which switch menuitem to show -- based on lockdown settings,
 * greeter mode, number of users in the system, and so on.
 * See get_switcher_mode()
 */
typedef enum
{
  SWITCHER_MODE_SCREENSAVER,
  SWITCHER_MODE_LOCK,
  SWITCHER_MODE_SWITCH,
  SWITCHER_MODE_SWITCH_OR_LOCK
}
SwitcherMode;

typedef struct
{
  SessionMenuMgr * mgr;
  AccountsUser * user;
}
ActivateUserSessionData;

struct _SessionMenuMgr
{
  GObject parent_instance;

  DbusmenuMenuitem * parent_mi;
  DbusmenuMenuitem * lock_mi;
  DbusmenuMenuitem * lock_switch_mi;
  DbusmenuMenuitem * guest_mi;
  DbusmenuMenuitem * logout_mi;
  DbusmenuMenuitem * suspend_mi;
  DbusmenuMenuitem * hibernate_mi;
  DbusmenuMenuitem * restart_mi;
  DbusmenuMenuitem * shutdown_mi;

  GSList * user_menuitems;
  gint user_menuitem_index;

  GSettings * lockdown_settings;
  GSettings * indicator_settings;
  GSettings * keybinding_settings;

  gboolean can_hibernate;
  gboolean can_suspend;
  gboolean allow_hibernate;
  gboolean allow_suspend;
  gboolean greeter_mode;

  GCancellable * cancellable;
  DBusUPower * upower_proxy;
  SessionDbus * session_dbus;  
  UsersServiceDbus * users_dbus_facade;
};

static SwitcherMode get_switcher_mode         (SessionMenuMgr *);
static void init_upower_proxy                 (SessionMenuMgr *);

static void update_screensaver_shortcut       (SessionMenuMgr *);
static void update_user_menuitems             (SessionMenuMgr *);
static void update_session_menuitems          (SessionMenuMgr *);
static void update_confirmation_labels        (SessionMenuMgr *);

static void action_func_lock                  (SessionMenuMgr *);
static void action_func_suspend               (SessionMenuMgr *);
static void action_func_hibernate             (SessionMenuMgr *);
static void action_func_switch_to_lockscreen  (SessionMenuMgr *);
static void action_func_switch_to_greeter     (SessionMenuMgr *);
static void action_func_switch_to_guest       (SessionMenuMgr *);
static void action_func_switch_to_user        (ActivateUserSessionData *);
static void action_func_spawn_async           (const char * fmt, ...);

static gboolean is_this_guest_session (void);
static gboolean is_this_live_session (void);

static void on_guest_logged_in_changed (UsersServiceDbus *,
                                        SessionMenuMgr   *);

static void on_user_logged_in_changed (UsersServiceDbus *,
                                       AccountsUser     *,
                                       SessionMenuMgr  *);

/**
***  GObject init / dispose / finalize
**/

G_DEFINE_TYPE (SessionMenuMgr, session_menu_mgr, G_TYPE_OBJECT);

static void
session_menu_mgr_init (SessionMenuMgr *mgr)
{
  mgr->can_hibernate = TRUE;
  mgr->can_suspend = TRUE;
  mgr->allow_hibernate = TRUE;
  mgr->allow_suspend = TRUE;

  /* Lockdown settings */
  GSettings * s = g_settings_new ("org.gnome.desktop.lockdown");
  g_signal_connect_swapped (s, "changed",
                            G_CALLBACK(update_session_menuitems), mgr);
  g_signal_connect_swapped (s, "changed::disable-lock-screen",
                            G_CALLBACK(update_user_menuitems), mgr);
  g_signal_connect_swapped (s, "changed::disable-user-switching",
                            G_CALLBACK(update_user_menuitems), mgr);
  mgr->lockdown_settings = s;

  /* Indicator settings */
  s = g_settings_new ("com.canonical.indicator.session");
  g_signal_connect_swapped (s, "changed::suppress-logout-restart-shutdown",
                            G_CALLBACK(update_confirmation_labels), mgr);
  g_signal_connect (s, "changed", G_CALLBACK(update_session_menuitems), mgr);
  mgr->indicator_settings = s;

  /* Keybinding settings */
  s = g_settings_new ("org.gnome.settings-daemon.plugins.media-keys");
  g_signal_connect_swapped (s, "changed::screensaver",
                            G_CALLBACK(update_screensaver_shortcut), mgr);
  mgr->keybinding_settings = s;

  /* listen for users who appear or log in or log out */
  mgr->users_dbus_facade = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  g_signal_connect_swapped (mgr->users_dbus_facade, "user-list-changed",
                            G_CALLBACK (update_user_menuitems), mgr);
  g_signal_connect (mgr->users_dbus_facade, "user-logged-in-changed",
                    G_CALLBACK(on_user_logged_in_changed), mgr);
  g_signal_connect (mgr->users_dbus_facade, "guest-logged-in-changed",
                    G_CALLBACK(on_guest_logged_in_changed), mgr);

  init_upower_proxy (mgr);  
}

static void
session_menu_mgr_dispose (GObject *object)
{
  SessionMenuMgr * mgr = SESSION_MENU_MGR (object);

  if (mgr->cancellable != NULL)
    {
      g_cancellable_cancel (mgr->cancellable);
      g_clear_object (&mgr->cancellable);
     }

  g_clear_object (&mgr->indicator_settings);
  g_clear_object (&mgr->lockdown_settings);
  g_clear_object (&mgr->keybinding_settings);
  g_clear_object (&mgr->upower_proxy);
  g_clear_object (&mgr->users_dbus_facade);

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
****  Menuitem Helpers
***/

static inline void
mi_set_label (DbusmenuMenuitem * mi, const char * str)
{
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, str);
}

static inline void
mi_set_type (DbusmenuMenuitem * mi, const char * str)
{
  dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, str);
}

static inline void
mi_set_visible (DbusmenuMenuitem * mi, gboolean b)
{
  dbusmenu_menuitem_property_set_bool (mi, DBUSMENU_MENUITEM_PROP_VISIBLE, b);
}

static inline void
mi_set_logged_in (DbusmenuMenuitem * mi, gboolean b)
{
  dbusmenu_menuitem_property_set_bool (mi, USER_ITEM_PROP_LOGGED_IN, b);
}

static DbusmenuMenuitem*
mi_new_separator (void)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  mi_set_type (mi, DBUSMENU_CLIENT_TYPES_SEPARATOR);
  return mi;
}

static DbusmenuMenuitem*
mi_new (const char * label)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  mi_set_label (mi, label);
  return mi;
}

/***
****  UPower Proxy:
****
****  1. While bootstrapping, we invoke the AllowSuspend and AllowHibernate
****     methods to find out whether or not those functions are allowed.
****  2. While bootstrapping, we get the CanSuspend and CanHibernate properties
****     and also listen for property changes.
****  3. These four values are used to set suspend and hibernate's visibility
****
***/

static void
on_upower_properties_changed (SessionMenuMgr * mgr)
{
  gboolean b;
  gboolean refresh = FALSE;

  /* suspend */
  b = dbus_upower_get_can_suspend (mgr->upower_proxy);
  if (mgr->can_suspend != b)
    {
      mgr->can_suspend = b;
      refresh = TRUE;
    }

  /* hibernate */
  b = dbus_upower_get_can_hibernate (mgr->upower_proxy);
  if (mgr->can_hibernate != b)
    {
      mgr->can_hibernate = b;
      refresh = TRUE;
    }

  if (refresh)
    {
      update_session_menuitems (mgr);
    }
}

static void
init_upower_proxy (SessionMenuMgr * mgr)
{
  mgr->cancellable = g_cancellable_new ();

  GError * error = NULL;
  mgr->upower_proxy = dbus_upower_proxy_new_for_bus_sync (
                         G_BUS_TYPE_SYSTEM,
                         0,
                         UPOWER_ADDRESS,
                         UPOWER_PATH,
                         NULL,
                         &error);
  if (error != NULL)
    {
      g_warning ("Error creating upower proxy: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      dbus_upower_call_suspend_allowed_sync (mgr->upower_proxy,
                                             &mgr->allow_suspend,
                                             NULL,
                                             &error);
      if (error != NULL)
        {
          g_warning ("%s: %s", G_STRFUNC, error->message);
          g_clear_error (&error);
        }

      dbus_upower_call_hibernate_allowed_sync (mgr->upower_proxy,
                                               &mgr->allow_hibernate,
                                               NULL,
                                               &error);
      if (error != NULL)
        {
          g_warning ("%s: %s", G_STRFUNC, error->message);
          g_clear_error (&error);
        }

      on_upower_properties_changed (mgr);
      g_signal_connect_swapped (mgr->upower_proxy, "changed",
                                G_CALLBACK(on_upower_properties_changed), mgr);
    }
}

/***
****  Admin Menuitems
***/

#define DEBUG_SHOW_ALL 1

static void
build_admin_menuitems (SessionMenuMgr * mgr)
{
  DbusmenuMenuitem * mi;
  const gboolean show_settings = DEBUG_SHOW_ALL || !mgr->greeter_mode;

  mi = mi_new (_("About This Computer"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_INFO);

  mi = mi_new (_("Ubuntu Help"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_HELP);

  mi = mi_new_separator ();
  mi_set_visible (mi, show_settings);
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);

  mi = mi_new (_("System Settings\342\200\246"));
  mi_set_visible (mi, show_settings);
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async),
                            CMD_SYSTEM_SETTINGS);

  mi = mi_new_separator ();
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
}

/***
****  Session Menuitems
***/

static void
update_session_menuitems (SessionMenuMgr * mgr)
{
  gboolean v;
  GSettings * s = mgr->indicator_settings;

  v = !mgr->greeter_mode
   && !is_this_live_session()
   && !g_settings_get_boolean (mgr->lockdown_settings, "disable-logout")
   && !g_settings_get_boolean (s, "suppress-logout-menuitem");
  mi_set_visible (mgr->logout_mi, v);

  v = mgr->can_suspend
   && mgr->allow_suspend;
  mi_set_visible (mgr->suspend_mi, v);

  v = mgr->can_hibernate
   && mgr->allow_hibernate;
  mi_set_visible (mgr->hibernate_mi, v);

  v = !g_settings_get_boolean (s, "suppress-restart-menuitem");
  mi_set_visible (mgr->restart_mi, v);

  v = !g_settings_get_boolean (s, "suppress-shutdown-menuitem");
  mi_set_visible (mgr->shutdown_mi, v);
}

/* if confirmation is enabled,
   add ellipsis to the labels of items whose actions need confirmation */
static void
update_confirmation_labels (SessionMenuMgr * mgr)
{
  const gboolean confirm_needed = !g_settings_get_boolean (
                                       mgr->indicator_settings,
                                       "suppress-logout-restart-shutdown");

  mi_set_label (mgr->logout_mi, confirm_needed ? _("Log Out\342\200\246")
                                               : _("Log Out"));

  mi_set_label (mgr->shutdown_mi, confirm_needed ? _("Switch Off\342\200\246")
                                                 : _("Switch Off"));

  dbusmenu_menuitem_property_set (mgr->restart_mi, RESTART_ITEM_LABEL,
                                  confirm_needed ? _("Restart\342\200\246")
                                                 : _("Restart"));
}

static void
build_session_menuitems (SessionMenuMgr* mgr)
{
  DbusmenuMenuitem * mi;

  mi = mi_new_separator ();
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);

  mi = mgr->logout_mi = mi_new (_("Log Out\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_LOGOUT);

  mi = mgr->suspend_mi = mi_new ("Suspend");
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_suspend), mgr);

  mi = mgr->hibernate_mi = mi_new (_("Hibernate"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_hibernate), mgr);

  mi = mgr->restart_mi = mi_new (_("Restart\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_RESTART);
 
  mi = mgr->shutdown_mi = mi_new (_("Switch Off\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->parent_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_SHUTDOWN);

  update_confirmation_labels (mgr);
}

/****
*****  User Menuitems
****/

/* Local Extensions to AccountsUser */

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
  g_message ("%s %s() associating user %s with mi %p",
             G_STRLOC, G_STRFUNC, accounts_user_get_user_name(user), mi);
  g_object_set_qdata_full (G_OBJECT(user), get_menuitem_quark(),
                           g_object_ref(G_OBJECT(mi)), g_object_unref);
}

static GQuark
get_collision_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("name-collision");
    }

  return q;
}

static gboolean
user_has_name_collision (AccountsUser * u)
{
  return g_object_get_qdata (G_OBJECT(u), get_collision_quark()) != NULL;
}

static void
user_set_name_collision (AccountsUser * u, gboolean b)
{
  g_object_set_qdata (G_OBJECT(u), get_collision_quark(), GINT_TO_POINTER(b));
}

/***
****
***/

static void
on_guest_logged_in_changed (UsersServiceDbus * usd,
                            SessionMenuMgr   * mgr)
{
  mi_set_logged_in (mgr->guest_mi,
                    users_service_dbus_is_guest_logged_in (usd));
}

/* When a user's login state changes,
   update the corresponding menuitem's LOGGED_IN property */
static void
on_user_logged_in_changed (UsersServiceDbus  * usd,
                           AccountsUser      * user,
                           SessionMenuMgr    * mgr)
{
  DbusmenuMenuitem * mi = user_get_menuitem (user);

  if (mi != NULL)
    {
      mi_set_logged_in (mi, users_service_dbus_is_user_logged_in (usd, user));
    }
}

static void
update_screensaver_shortcut (SessionMenuMgr * mgr)
{
  gchar * s = g_settings_get_string (mgr->keybinding_settings, "screensaver");
  g_debug ("Keybinding changed to: %s", s);
  dbusmenu_menuitem_property_set_shortcut_string (mgr->lock_mi, s);
  dbusmenu_menuitem_property_set_shortcut_string (mgr->lock_switch_mi, s);
  g_free (s);
}

static void
on_user_icon_file_changed (AccountsUser      * user,
                           GParamSpec        * pspec G_GNUC_UNUSED,
                           DbusmenuMenuitem  * mi)
{
  const gchar * str = accounts_user_get_icon_file (user);
  
  if (!str || !*str)
    {
      str = USER_ITEM_ICON_DEFAULT;
    }

  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_ICON, str);
}

typedef struct
{
  AccountsUser * user;
  gulong handler_id;
}
UserChangeListenerData;

/* when the menuitem is destroyed,
   it should stop listening for changes to the UserAccount properties :) */
static void
on_user_menuitem_destroyed (UserChangeListenerData * data)
{
  g_signal_handler_disconnect (data->user, data->handler_id);
  g_free (data);
}

static DbusmenuMenuitem*
user_menuitem_new (AccountsUser * user, SessionMenuMgr * mgr)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  mi_set_type (mi, USER_ITEM_TYPE);

  /* set the name property */
  GString * gstr = g_string_new (accounts_user_get_real_name (user));
  if (user_has_name_collision (user))
    {
      g_string_append_printf (gstr, " (%s)", accounts_user_get_user_name(user));
    }
  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, gstr->str);
  g_string_free (gstr, TRUE);

  /* set the logged-in property */
  mi_set_logged_in (mi,
         users_service_dbus_is_user_logged_in (mgr->users_dbus_facade, user));

  /* set the icon property & listen for changes */
  UserChangeListenerData * cd = g_new0 (UserChangeListenerData, 1);
  cd->user = user;
  cd->handler_id = g_signal_connect (user, "notify::icon-file",
                                     G_CALLBACK(on_user_icon_file_changed), mi);
  g_object_weak_ref (G_OBJECT(mi), (GWeakNotify)on_user_menuitem_destroyed, cd);
  on_user_icon_file_changed (user, NULL, mi);

  /* set the is-current-user property */
  const gboolean is_current_user =
              !g_strcmp0 (g_get_user_name(), accounts_user_get_user_name(user));
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_IS_CURRENT_USER,
                                       is_current_user);

  /* set the activate callback */
  ActivateUserSessionData * data = g_new (ActivateUserSessionData, 1);
  data->user = user;
  data->mgr = mgr;
  g_signal_connect_data (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                         G_CALLBACK (action_func_switch_to_user),
                         data, (GClosureNotify)g_free,
                         G_CONNECT_SWAPPED);

  /* give this AccountsUser a hook back to this menuitem */
  user_set_menuitem (user, mi);

  return mi;
}

/* for sorting AccountsUsers from most to least frequently used */
static gint
compare_users_by_login_frequency (gconstpointer a, gconstpointer b)
{
  const guint64 a_freq = accounts_user_get_login_frequency (ACCOUNTS_USER(a));
  const guint64 b_freq = accounts_user_get_login_frequency (ACCOUNTS_USER(b));
  if (a_freq > b_freq) return -1;
  if (a_freq < b_freq) return  1;
  return 0;
}

/* for sorting AccountsUsers by name */
static gint
compare_users_by_username (gconstpointer ga, gconstpointer gb)
{
  AccountsUser * a = ACCOUNTS_USER(ga);
  AccountsUser * b = ACCOUNTS_USER(gb);

  const int ret = g_strcmp0 (accounts_user_get_real_name (a),
                             accounts_user_get_real_name (b));

  if (!ret) /* names are the same, so both have a name collision */
    {
      user_set_name_collision (a, TRUE);
      user_set_name_collision (b, TRUE);
    }

  return ret;
}

static gboolean
is_user_switching_allowed (SessionMenuMgr * mgr)
{
  /* maybe it's locked down */
  if (g_settings_get_boolean (mgr->lockdown_settings, "disable-user-switching"))
    return FALSE;

  /* maybe the seat doesn't support activation */
  if (!users_service_dbus_can_activate_session (mgr->users_dbus_facade))
    return FALSE;

  return TRUE;
}

static void
build_user_menuitems (SessionMenuMgr * mgr)
{
  DbusmenuMenuitem * mi;
  GSList * items = NULL;
  gint pos = mgr->user_menuitem_index;
  const char * current_real_name = NULL;

  /**
  ***  Start Screen Saver
  ***  Switch Account...
  ***  Lock
  ***  Lock / Switch Account...
  **/

  const gboolean show_all = DEBUG_SHOW_ALL;
  const SwitcherMode mode = get_switcher_mode (mgr);

  mi = mi_new (_("Start Screen Saver"));
  mi_set_visible (mi, show_all || (mode == SWITCHER_MODE_SCREENSAVER));
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_lock), mgr);

  mi = mi_new (_("Switch User Account\342\200\246"));
  mi_set_visible (mi, show_all || (mode == SWITCHER_MODE_SWITCH));
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_greeter), mgr);

  mi = mgr->lock_mi = mi_new (_("Lock"));
  mi_set_visible (mi, show_all || (mode == SWITCHER_MODE_LOCK));
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_lockscreen), mgr);

  mi = mgr->lock_switch_mi = mi_new (_("Lock/Switch Account\342\200\246"));
  mi_set_visible (mi, show_all || (mode == SWITCHER_MODE_SWITCH_OR_LOCK));
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_lockscreen), mgr);

  const gboolean guest_allowed =
           users_service_dbus_guest_session_enabled (mgr->users_dbus_facade);
  const gboolean is_guest = is_this_guest_session();
  mi = mi_new (_("Guest Session"));
  mi_set_type (mi, USER_ITEM_TYPE);
  mi_set_visible (mi, guest_allowed && !is_guest);
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  on_guest_logged_in_changed (mgr->users_dbus_facade, mgr);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_guest), mgr);
  mgr->guest_mi = mi;
  if (guest_allowed && is_guest)
    {
      current_real_name = _("Guest");
    }
  
  /***
  ****  Users
  ***/

  /* if we can switch to another user account, show them here */
  if (is_user_switching_allowed (mgr))
    {
      GList * users = users_service_dbus_get_user_list (mgr->users_dbus_facade);

      /* pick the most frequently used accounts */
      const int MAX_USERS = 12; /* this limit comes from the spec */
      if (g_list_length(users) > MAX_USERS)
        {
          users = g_list_sort (users, compare_users_by_login_frequency);
          GList * last = g_list_nth (users, MAX_USERS-1);
          GList * remainder = last->next;
          last->next = NULL;
          remainder->prev = NULL;
          g_list_free (remainder);
        }

      /* Sort the users by name for display */
      users = g_list_sort (users, compare_users_by_username);

      /* Create menuitems for them */
      int i;
      GList * u;
      const char * const username = g_get_user_name();
      for (i=0, u=users; i<MAX_USERS && u!=NULL; u=u->next, i++)
        {
          AccountsUser * user = u->data;
          DbusmenuMenuitem * mi = user_menuitem_new (user, mgr);
          dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
          items = g_slist_prepend (items, mi);

          if (!g_strcmp0 (username, accounts_user_get_user_name(user)))
            {
              current_real_name = accounts_user_get_real_name (user);
            }
        }
      g_list_free(users);
    }

  /* separator */
  mi = mi_new_separator ();
  dbusmenu_menuitem_child_add_position (mgr->parent_mi, mi, pos++);
  items = g_slist_prepend (items, mi);

  if (current_real_name != NULL)
    {
      session_dbus_set_users_real_name (mgr->session_dbus,
                                        current_real_name);
    }

  update_screensaver_shortcut (mgr);
  mgr->user_menuitems = items;
}

static void
update_user_menuitems (SessionMenuMgr * mgr)
{
  /* remove any previous user menuitems */
  GSList * l;
  for (l=mgr->user_menuitems; l!=NULL; l=l->next)
    {
      dbusmenu_menuitem_child_delete (mgr->parent_mi, l->data);
    }
  g_slist_free (mgr->user_menuitems);
  mgr->user_menuitems = NULL;

  /* add fresh user menuitems */
  build_user_menuitems (mgr);
}

/***
****
***/

static void
action_func_spawn_async (const char * fmt, ...)
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

/* Calling "Lock" locks the screen & goes to black.
   Calling "SimulateUserActivity" afterwards shows the Lock Screen. */
static void
lock_helper (SessionMenuMgr * mgr, gboolean show_lock_screen)
{
  if (!g_settings_get_boolean (mgr->lockdown_settings, "disable-lock-screen"))
    {
      GError * error = NULL;
      GDBusProxy * proxy = g_dbus_proxy_new_for_bus_sync (
                             G_BUS_TYPE_SESSION,
                             G_DBUS_PROXY_FLAGS_NONE,
                             NULL,
                             "org.gnome.ScreenSaver",
                             "/org/gnome/ScreenSaver",
                             "org.gnome.ScreenSaver",
                             NULL,
                             &error);

      if (error == NULL)
        {
          g_dbus_proxy_call_sync (proxy, "Lock",
                                  NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                  &error);
        }

      if ((error == NULL) && show_lock_screen)
        {
          g_dbus_proxy_call_sync (proxy, "SimulateUserActivity",
                                  NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                                  &error);
        }

      if (error != NULL)
        {
          g_warning ("Error locking screen: %s", error->message);
        }

      g_clear_object (&proxy);
      g_clear_error (&error);
    }
}

static void
action_func_lock (SessionMenuMgr * mgr)
{
  lock_helper (mgr, FALSE);
}

static void
action_func_switch_to_lockscreen (SessionMenuMgr * mgr)
{
  lock_helper (mgr, TRUE);
}

static void
action_func_switch_to_greeter (SessionMenuMgr * mgr)
{
  action_func_lock (mgr);
  users_service_dbus_show_greeter (mgr->users_dbus_facade);
}

static void
action_func_switch_to_user (ActivateUserSessionData * data)
{
  action_func_lock (data->mgr);
  users_service_dbus_activate_user_session (data->mgr->users_dbus_facade,
                                            data->user);
}

static void
action_func_switch_to_guest (SessionMenuMgr * mgr)
{
  action_func_lock (mgr);
  users_service_dbus_activate_guest_session (mgr->users_dbus_facade);
}

static void
action_func_suspend (SessionMenuMgr * mgr)
{
  GError * error = NULL;

  dbus_upower_call_suspend_sync (mgr->upower_proxy,
                                 mgr->cancellable,
                                 &error);

  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
}

static void
action_func_hibernate (SessionMenuMgr * mgr)
{
  GError * error = NULL;

  dbus_upower_call_hibernate_sync (mgr->upower_proxy,
                                   mgr->cancellable,
                                   &error);

  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
}

/***
****
***/

static gboolean
is_this_guest_session (void)
{
  return geteuid() < 500;
}

static gboolean
is_this_live_session (void)
{
  const struct passwd * const pw = getpwuid (geteuid());
  return (pw->pw_uid==999) && !g_strcmp0("ubuntu",pw->pw_name);
}

static SwitcherMode
get_switcher_mode (SessionMenuMgr * mgr)
{
  SwitcherMode mode;

  const gboolean can_lock = !g_settings_get_boolean (mgr->lockdown_settings,
                                                     "disable-lock-screen");
  const gboolean can_switch = is_user_switching_allowed (mgr);

  if (!can_lock && !can_switch) /* hmm, quite an extreme lockdown */
    {
      mode = SWITCHER_MODE_SCREENSAVER;
    }
  else if (is_this_live_session()) /* live sessions can't lock or switch */
    {
      mode = SWITCHER_MODE_SCREENSAVER;
    }
  else if (!can_switch) /* switching's locked down */
    {
      mode = SWITCHER_MODE_LOCK;
    }
  else if (is_this_guest_session ()) /* guest sessions can't lock */
    {
      mode = SWITCHER_MODE_SWITCH;
    }
  else
    {
      GList * l = users_service_dbus_get_user_list (mgr->users_dbus_facade);
      const size_t user_count = g_list_length (l);
      g_list_free (l);

      mode = user_count < 2
           ? SWITCHER_MODE_LOCK /* you can't switch if no other users */
           : SWITCHER_MODE_SWITCH_OR_LOCK;
    }

  return mode;
}


/***
****
***/

SessionMenuMgr* session_menu_mgr_new (DbusmenuMenuitem  * parent_mi,
                                      SessionDbus       * session_dbus,
                                      gboolean            greeter_mode)
{
  SessionMenuMgr* mgr = g_object_new (SESSION_TYPE_MENU_MGR, NULL);
  mgr->parent_mi = parent_mi;
  mgr->greeter_mode = greeter_mode;
  mgr->session_dbus = session_dbus;
  build_admin_menuitems (mgr);
  const guint n = g_list_length (dbusmenu_menuitem_get_children (parent_mi));
  mgr->user_menuitem_index = n;
  build_user_menuitems (mgr);
  build_session_menuitems (mgr);
  return mgr;
}
