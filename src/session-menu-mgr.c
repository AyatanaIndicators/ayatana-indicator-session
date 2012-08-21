/*
Copyright 2011 Canonical Ltd.

Authors:
    Charles Kerr <charles.kerr@canonical.com>
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

#include "config.h"

#include <sys/types.h>
#include <pwd.h> /* geteuid(), getpwuid() */

#include <glib.h>
#include <glib/gi18n.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-gtk/menuitem.h>

#include "dbus-upower.h"
#include "session-menu-mgr.h"
#include "shared-names.h"
#include "users-service-dbus.h"
#include "online-accounts-mgr.h"

#define DEBUG_SHOW_ALL FALSE

#define UPOWER_ADDRESS    "org.freedesktop.UPower"
#define UPOWER_PATH       "/org/freedesktop/UPower"

#define CMD_HELP            "yelp"
#define CMD_INFO            "gnome-control-center info"
#define CMD_SYSTEM_SETTINGS "gnome-control-center"
#ifdef HAVE_GTKLOGOUTHELPER
 #define HAVE_RESTART_CMD TRUE
 #define CMD_RESTART  LIBEXECDIR"/gtk-logout-helper --restart"
 #define CMD_LOGOUT   LIBEXECDIR"/gtk-logout-helper --logout"
 #define CMD_SHUTDOWN LIBEXECDIR"/gtk-logout-helper --shutdown"
#else
 #define HAVE_RESTART_CMD FALSE /* hmm, no gnome-session-quit --restart? */
 #define CMD_RESTART  ""
 #define CMD_LOGOUT   "gnome-session-quit --logout"
 #define CMD_SHUTDOWN "gnome-session-quit --power-off"
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

/**
 * Creates and manages the menumodel and associated actions for the
 * session menu described at <https://wiki.ubuntu.com/SystemMenu>.
 *
 * This is a pretty straightforward class: it creates the menumodel
 * and listens for events that can affect the model's properties.
 *
 * Simple event sources, such as GSettings and a UPower DBus proxy,
 * are handled here. More involved event sources are delegated to the
 * UsersServiceDBus facade class.
 */
struct _SessionMenuMgr
{
  GObject parent_instance;

  DbusmenuMenuitem * top_mi;
  DbusmenuMenuitem * screensaver_mi;
  DbusmenuMenuitem * lock_mi;
  DbusmenuMenuitem * lock_switch_mi;
  DbusmenuMenuitem * guest_mi;
  DbusmenuMenuitem * online_accounts_mi;
  DbusmenuMenuitem * online_accounts_separator;
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

  /* cached settings taken from the upower proxy */
  gboolean can_hibernate;
  gboolean can_suspend;
  gboolean allow_hibernate;
  gboolean allow_suspend;

  gboolean greeter_mode;

  GCancellable * cancellable;
  DBusUPower * upower_proxy;
  SessionDbus * session_dbus;
  UsersServiceDbus * users_dbus_facade;
  OnlineAccountsMgr * online_accounts_mgr;
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
static void action_func_switch_to_user        (AccountsUser   *);
static void action_func_spawn_async           (const char * cmd);

static gboolean is_this_guest_session         (void);
static gboolean is_this_live_session          (void);

static void on_guest_logged_in_changed        (UsersServiceDbus *,
                                               SessionMenuMgr   *);

static void on_user_logged_in_changed         (UsersServiceDbus *,
                                               AccountsUser     *,
                                               SessionMenuMgr   *);

/**
***  GObject init / dispose
**/

G_DEFINE_TYPE (SessionMenuMgr, session_menu_mgr, G_TYPE_OBJECT);

static void
session_menu_mgr_init (SessionMenuMgr *mgr)
{
  mgr->top_mi = dbusmenu_menuitem_new ();

  /* Lockdown settings */
  GSettings * s = g_settings_new ("org.gnome.desktop.lockdown");
  g_signal_connect_swapped (s, "changed::disable-log-out",
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
  g_signal_connect_swapped (s, "changed::suppress-logout-menuitem",
                            G_CALLBACK(update_session_menuitems), mgr);
  g_signal_connect_swapped (s, "changed::suppress-restart-menuitem",
                            G_CALLBACK(update_session_menuitems), mgr);
  g_signal_connect_swapped (s, "changed::suppress-shutdown-menuitem",
                            G_CALLBACK(update_session_menuitems), mgr);
  mgr->indicator_settings = s;

  /* Keybinding settings */
  s = g_settings_new ("org.gnome.settings-daemon.plugins.media-keys");
  g_signal_connect_swapped (s, "changed::screensaver",
                            G_CALLBACK(update_screensaver_shortcut), mgr);
  mgr->keybinding_settings = s;

  /* listen for user events */
  mgr->users_dbus_facade = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);
  g_signal_connect_swapped (mgr->users_dbus_facade, "user-list-changed",
                            G_CALLBACK (update_user_menuitems), mgr);
  g_signal_connect (mgr->users_dbus_facade, "user-logged-in-changed",
                    G_CALLBACK(on_user_logged_in_changed), mgr);
  g_signal_connect (mgr->users_dbus_facade, "guest-logged-in-changed",
                    G_CALLBACK(on_guest_logged_in_changed), mgr);

  init_upower_proxy (mgr);

  /* Online accounts menu item */
  mgr->online_accounts_mgr = online_accounts_mgr_new ();
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
  g_clear_object (&mgr->top_mi);
  g_clear_object (&mgr->session_dbus);
  g_clear_object (&mgr->online_accounts_mgr);

  g_slist_free (mgr->user_menuitems);
  mgr->user_menuitems = NULL;

  G_OBJECT_CLASS (session_menu_mgr_parent_class)->dispose (object);
}

static void
session_menu_mgr_class_init (SessionMenuMgrClass * klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = session_menu_mgr_dispose;
}

/***
****  UPower Proxy:
****
****  1. While bootstrapping, we invoke the AllowSuspend and AllowHibernate
****     methods to find out whether or not those features are allowed.
****  2. While bootstrapping, we get the CanSuspend and CanHibernate properties
****     and also listen for property changes.
****  3. These four values are used to set suspend and hibernate's visibility.
****
***/

static void
on_upower_properties_changed (SessionMenuMgr * mgr)
{
  gboolean b;
  gboolean need_refresh = FALSE;

  /* suspend */
  b = dbus_upower_get_can_suspend (mgr->upower_proxy);
  if (mgr->can_suspend != b)
    {
      mgr->can_suspend = b;
      need_refresh = TRUE;
    }

  /* hibernate */
  b = dbus_upower_get_can_hibernate (mgr->upower_proxy);
  if (mgr->can_hibernate != b)
    {
      mgr->can_hibernate = b;
      need_refresh = TRUE;
    }

  if (need_refresh)
    {
      update_session_menuitems (mgr);
    }
}

static void
init_upower_proxy (SessionMenuMgr * mgr)
{
  /* default values */
  mgr->can_suspend = TRUE;
  mgr->can_hibernate = TRUE;
  mgr->allow_suspend = TRUE;
  mgr->allow_hibernate = TRUE;

  mgr->cancellable = g_cancellable_new ();

  GError * error = NULL;
  mgr->upower_proxy = dbus_upower_proxy_new_for_bus_sync (
                         G_BUS_TYPE_SYSTEM,
                         G_DBUS_PROXY_FLAGS_NONE,
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
  dbusmenu_menuitem_property_set_bool (mi, DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       b || DEBUG_SHOW_ALL);
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
****  Admin Menuitems
****  <https://wiki.ubuntu.com/SystemMenu#Admin_items>
***/

static void
build_admin_menuitems (SessionMenuMgr * mgr)
{
  if (!mgr->greeter_mode)
  {
    DbusmenuMenuitem * mi;
    const gboolean show_settings = !mgr->greeter_mode;

    mi = mi_new (_("About This Computer"));
    dbusmenu_menuitem_child_append (mgr->top_mi, mi);
    g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                              G_CALLBACK(action_func_spawn_async), CMD_INFO);

    mi = mi_new (_("Ubuntu Help"));
    dbusmenu_menuitem_child_append (mgr->top_mi, mi);
    g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                              G_CALLBACK(action_func_spawn_async), CMD_HELP);

    mi = mi_new_separator ();
    mi_set_visible (mi, show_settings);
    dbusmenu_menuitem_child_append (mgr->top_mi, mi);

    mi = mi_new (_("System Settings\342\200\246"));
    mi_set_visible (mi, show_settings);
    dbusmenu_menuitem_child_append (mgr->top_mi, mi);
    g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                              G_CALLBACK(action_func_spawn_async),
                              CMD_SYSTEM_SETTINGS);

    mi = mi_new_separator ();
    dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  }
}

/***
****  Session Menuitems
****  <https://wiki.ubuntu.com/SystemMenu#Session_items>
***/

static void
update_session_menuitems (SessionMenuMgr * mgr)
{
  gboolean v;
  GSettings * s = mgr->indicator_settings;

  v = !mgr->greeter_mode;
  mi_set_visible (mgr->online_accounts_mi, v);
  mi_set_visible (mgr->online_accounts_separator, v);

  v = !mgr->greeter_mode
   && !is_this_live_session()
   && !g_settings_get_boolean (mgr->lockdown_settings, "disable-log-out")
   && !g_settings_get_boolean (s, "suppress-logout-menuitem");
  mi_set_visible (mgr->logout_mi, v);

  v = mgr->can_suspend
   && mgr->allow_suspend;
  mi_set_visible (mgr->suspend_mi, v);

  v = mgr->can_hibernate
   && mgr->allow_hibernate;
  mi_set_visible (mgr->hibernate_mi, v);

  v = HAVE_RESTART_CMD
   && !g_settings_get_boolean (s, "suppress-restart-menuitem");
  mi_set_visible (mgr->restart_mi, v);

  v = !g_settings_get_boolean (s, "suppress-shutdown-menuitem");
  mi_set_visible (mgr->shutdown_mi, v);
}

/* Update the ellipses when the confirmation setting changes.
 *
 * <http://developer.gnome.org/hig-book/3.0/menus-design.html.en>:
 * "Label the menu item with a trailing ellipsis ("...") only if the
 * command requires further input from the user before it can be performed."
 */
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

  mi = mgr->online_accounts_mi =
    online_accounts_mgr_get_menu_item (mgr->online_accounts_mgr);
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);

  mi = mgr->online_accounts_separator = mi_new_separator ();
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);

  mi = mgr->logout_mi = mi_new (_("Log Out\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_LOGOUT);

  mi = mgr->suspend_mi = mi_new ("Suspend");
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_suspend), mgr);

  mi = mgr->hibernate_mi = mi_new (_("Hibernate"));
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_hibernate), mgr);

  mi = mgr->restart_mi = dbusmenu_menuitem_new ();
  mi_set_type (mi, RESTART_ITEM_TYPE);
  dbusmenu_menuitem_property_set (mi, RESTART_ITEM_LABEL, _("Restart\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_RESTART);

  mi = mgr->shutdown_mi = mi_new (_("Switch Off\342\200\246"));
  dbusmenu_menuitem_child_append (mgr->top_mi, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK(action_func_spawn_async), CMD_SHUTDOWN);

  update_confirmation_labels (mgr);
  update_session_menuitems (mgr);
}

/****
*****  User Menuitems
*****  https://wiki.ubuntu.com/SystemMenu#Account-switching_items
****/

/* Local extensions to AccountsUser */

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
user_clear_menuitem (AccountsUser * user)
{
  g_object_steal_qdata (G_OBJECT(user), get_menuitem_quark());
}

static void
user_set_menuitem (AccountsUser * user, DbusmenuMenuitem * mi)
{
  g_object_set_qdata (G_OBJECT(user), get_menuitem_quark(), mi);

  g_object_weak_ref (G_OBJECT(mi), (GWeakNotify)user_clear_menuitem, user);
}

/***/

static GQuark
get_mgr_quark (void)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("session-menu-mgr");
    }

  return q;
}

static SessionMenuMgr*
user_get_mgr (AccountsUser * user)
{
  return g_object_get_qdata (G_OBJECT(user), get_mgr_quark());
}

static void
user_set_mgr (AccountsUser * user, SessionMenuMgr * mgr)
{
  g_object_set_qdata (G_OBJECT(user), get_mgr_quark(), mgr);
}

/***/

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
  if (mgr->guest_mi != NULL)
    {
      mi_set_logged_in (mgr->guest_mi,
                        users_service_dbus_is_guest_logged_in (usd));
    }
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
  g_debug ("%s Screensaver shortcut changed to: '%s'", G_STRLOC, s);

  if (mgr->lock_mi != NULL)
    {
      dbusmenu_menuitem_property_set_shortcut_string (mgr->lock_mi, s);
    }

  if (mgr->lock_switch_mi != NULL)
    {
      dbusmenu_menuitem_property_set_shortcut_string (mgr->lock_switch_mi, s);
    }

  if (mgr->screensaver_mi != NULL)
    {
      dbusmenu_menuitem_property_set_shortcut_string (mgr->screensaver_mi, s);
    }

  g_free (s);
}

static void
update_user_menuitem_icon (DbusmenuMenuitem * mi, AccountsUser * user)
{
  const gchar * str = accounts_user_get_icon_file (user);

  if (!str || !*str)
    {
      str = USER_ITEM_ICON_DEFAULT;
    }

  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_ICON, str);
}

static void
update_user_menuitem_name (DbusmenuMenuitem * mi, AccountsUser * user)
{
  GString * gstr = g_string_new (accounts_user_get_real_name (user));

  if (user_has_name_collision (user))
    {
      g_string_append_printf (gstr, " (%s)", accounts_user_get_user_name(user));
    }

  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, gstr->str);
  g_string_free (gstr, TRUE);
}

static void
on_user_property_changed (AccountsUser      * user,
                          GParamSpec        * pspec,
                          DbusmenuMenuitem  * mi)
{
  static const char * interned_icon_file = NULL;
  static const char * interned_real_name = NULL;
  static const char * interned_user_name = NULL;

  if (G_UNLIKELY (interned_icon_file == NULL))
    {
      interned_icon_file = g_intern_static_string ("icon-file");
      interned_user_name = g_intern_static_string ("user-name");
      interned_real_name = g_intern_static_string ("real-name");
    }

  if (pspec->name == interned_icon_file)
    {
      update_user_menuitem_icon (mi, user);
    }
  else if ((pspec->name == interned_real_name)
        || (pspec->name == interned_user_name))
    {
      /* name changing can affect other menuitems too by invalidating
         the sort order or name collision flags... so let's rebuild */
      update_user_menuitems (user_get_mgr (user));
    }
}

typedef struct
{
  gpointer instance;
  gulong handler_id;
}
SignalHandlerData;

/* when a user menuitem is destroyed,
   it should stop listening for its UserAccount's property changes */
static void
on_user_menuitem_destroyed (SignalHandlerData * data)
{
  g_signal_handler_disconnect (data->instance, data->handler_id);
  g_free (data);
}

static DbusmenuMenuitem*
user_menuitem_new (AccountsUser * user, SessionMenuMgr * mgr)
{
  DbusmenuMenuitem * mi = dbusmenu_menuitem_new ();
  mi_set_type (mi, USER_ITEM_TYPE);

  /* set the name & icon and listen for property changes */
  update_user_menuitem_name (mi, user);
  update_user_menuitem_icon (mi, user);
  SignalHandlerData * hd = g_new0 (SignalHandlerData, 1);
  hd->instance = user;
  hd->handler_id = g_signal_connect (user, "notify",
                                     G_CALLBACK(on_user_property_changed), mi);
  g_object_weak_ref (G_OBJECT(mi), (GWeakNotify)on_user_menuitem_destroyed, hd);

  /* set the logged-in property */
  mi_set_logged_in (mi,
         users_service_dbus_is_user_logged_in (mgr->users_dbus_facade, user));

  /* set the is-current-user property */
  const gboolean is_current_user =
              !g_strcmp0 (g_get_user_name(), accounts_user_get_user_name(user));
  dbusmenu_menuitem_property_set_bool (mi,
                                       USER_ITEM_PROP_IS_CURRENT_USER,
                                       is_current_user);

  /* set the switch-to-user action */
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_user), user);

  /* give this AccountsUser a hook back to this menuitem */
  user_set_menuitem (user, mi);
  user_set_mgr (user, mgr);

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

/* for sorting AccountsUsers alphabetically */
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
    {
      return FALSE;
    }

  /* maybe the seat doesn't support activation */
  if (!users_service_dbus_can_activate_session (mgr->users_dbus_facade))
    {
      return FALSE;
    }

  return TRUE;
}

static void
build_user_menuitems (SessionMenuMgr * mgr)
{
  g_return_if_fail (!mgr->greeter_mode);

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

  const SwitcherMode mode = get_switcher_mode (mgr);

  mi = mgr->screensaver_mi = mi_new (_("Start Screen Saver"));
  mi_set_visible (mi, mode == SWITCHER_MODE_SCREENSAVER);
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_lock), mgr);

  mi = mi_new (_("Switch User Account\342\200\246"));
  mi_set_visible (mi, mode == SWITCHER_MODE_SWITCH);
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_greeter), mgr);

  mi = mgr->lock_mi = mi_new (_("Lock"));
  mi_set_visible (mi, mode == SWITCHER_MODE_LOCK);
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_lockscreen), mgr);

  mi = mgr->lock_switch_mi = mi_new (_("Lock/Switch Account\342\200\246"));
  mi_set_visible (mi, mode == SWITCHER_MODE_SWITCH_OR_LOCK);
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_lockscreen), mgr);

  const gboolean is_guest = is_this_guest_session ();
  const gboolean guest_allowed =
              users_service_dbus_guest_session_enabled (mgr->users_dbus_facade);
  mi = mgr->guest_mi = dbusmenu_menuitem_new ();
  mi_set_type (mi, USER_ITEM_TYPE);
  mi_set_visible (mi, !is_guest && guest_allowed);
  dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, _("Guest Session"));
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
  on_guest_logged_in_changed (mgr->users_dbus_facade, mgr);
  items = g_slist_prepend (items, mi);
  g_signal_connect_swapped (mi, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                            G_CALLBACK (action_func_switch_to_guest), mgr);

  if (guest_allowed && is_guest)
    {
      current_real_name = _("Guest");
    }

  /***
  ****  Users
  ***/

  /* if we can switch to another user account, show them here */
  const char * const username = g_get_user_name();
  GList * users = users_service_dbus_get_user_list (mgr->users_dbus_facade);

  /* since we're building (or rebuilding) from scratch,
     clear the name collision flags */
  GList * u;
  for (u=users; u!=NULL; u=u->next)
    {
      AccountsUser * user = ACCOUNTS_USER(u->data);

      user_set_name_collision (user, FALSE);

      if (!g_strcmp0 (username, accounts_user_get_user_name(user)))
        {
          current_real_name = accounts_user_get_real_name (user);
        }
    }

  if (is_user_switching_allowed (mgr))
    {
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
      for (i=0, u=users; i<MAX_USERS && u!=NULL; u=u->next, i++)
        {
          AccountsUser * user = u->data;
          DbusmenuMenuitem * mi = user_menuitem_new (user, mgr);
          dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
          items = g_slist_prepend (items, mi);
        }
    }

  g_list_free (users);

  /* separator */
  mi = mi_new_separator ();
  dbusmenu_menuitem_child_add_position (mgr->top_mi, mi, pos++);
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
      dbusmenu_menuitem_child_delete (mgr->top_mi, l->data);
    }
  g_slist_free (mgr->user_menuitems);
  mgr->user_menuitems = NULL;

  /* add fresh user menuitems */
  if (!mgr->greeter_mode)
    {
      build_user_menuitems (mgr);
    }
}

/***
****  Actions!
***/

static void
action_func_spawn_async (const char * cmd)
{
  GError * error = NULL;

  g_debug ("%s calling \"%s\"", G_STRFUNC, cmd);
  g_spawn_command_line_async (cmd, &error);

  if (error != NULL)
    {
      g_warning ("Unable to execute \"%s\": %s", cmd, error->message);
      g_clear_error (&error);
    }
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
action_func_switch_to_user (AccountsUser * user)
{
  SessionMenuMgr * mgr = user_get_mgr (user);
  g_return_if_fail (mgr != NULL);
  action_func_lock (mgr);
  users_service_dbus_activate_user_session (mgr->users_dbus_facade, user);
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
  /* FIXME: this test has been here awhile and seems to work,
     but seems brittle to me */
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
  else /* both locking & switching are allowed */
    {
      GList * l = users_service_dbus_get_user_list (mgr->users_dbus_facade);
      const size_t user_count = g_list_length (l);
      g_list_free (l);

      /* only show switch mode if we have users to switch to */
      mode = user_count > (is_this_guest_session() ? 0 : 1)
           ? SWITCHER_MODE_SWITCH_OR_LOCK
           : SWITCHER_MODE_LOCK;
    }

  return mode;
}


/***
****
***/

SessionMenuMgr*
session_menu_mgr_new (SessionDbus  * session_dbus,
                      gboolean       greeter_mode)
{
  SessionMenuMgr* mgr = g_object_new (SESSION_TYPE_MENU_MGR, NULL);
  mgr->greeter_mode = greeter_mode;
  mgr->session_dbus = g_object_ref (session_dbus);
  build_admin_menuitems (mgr);
  const guint n = g_list_length (dbusmenu_menuitem_get_children (mgr->top_mi));
  mgr->user_menuitem_index = n;
  update_user_menuitems (mgr);
  build_session_menuitems (mgr);
  return mgr;
}

/**
 * session_menu_mgr_get_menu:
 *
 * Returns: (transfer none): the manager's menu.
 */
DbusmenuMenuitem *
session_menu_mgr_get_menu (SessionMenuMgr * mgr)
{
  g_return_val_if_fail (IS_SESSION_MENU_MGR(mgr), NULL);

  return mgr->top_mi;
}
