/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbus-accounts.h"
#include "dbus-consolekit-seat.h"
#include "dbus-consolekit-session.h"
#include "dbus-consolekit-manager.h"
#include "dbus-user.h"

#include "users.h"

struct _IndicatorSessionUsersDbusPriv
{
  char * active_session_id;

  Accounts * accounts;

  DisplayManagerSeat * dm_seat;

  ConsoleKitSeat * seat_proxy;

  /* user's dbus object path -> AccountsUser* */
  GHashTable * path_to_user;

  /* uint32 user-id --> user's dbus object path */
  GHashTable * uid_to_user_path;

  /* uint32 user-id --> hashset of ssid strings */
  GHashTable * uid_to_sessions;

  /* ssid string --> uint32 user-id */
  GHashTable * session_to_uid;

  GCancellable * cancellable;
};

typedef IndicatorSessionUsersDbusPriv priv_t;

G_DEFINE_TYPE (IndicatorSessionUsersDbus,
               indicator_session_users_dbus,
               INDICATOR_TYPE_SESSION_USERS)

/***
****
***/

static void create_user_proxy_for_path    (IndicatorSessionUsersDbus * self,
                                           const char                * path);

static void create_session_proxy_for_ssid (IndicatorSessionUsersDbus * self,
                                           const char                * ssid);

static void
emit_user_changed_for_path (IndicatorSessionUsersDbus * self, const char * path)
{
  AccountsUser * user = g_hash_table_lookup (self->priv->path_to_user, path);

  if (user && !accounts_user_get_system_account (user))
    indicator_session_users_changed (INDICATOR_SESSION_USERS(self), path);
}

static void
emit_user_changed_for_uid (IndicatorSessionUsersDbus * self, guint uid)
{
  const char * path;

  if ((path = g_hash_table_lookup (self->priv->uid_to_user_path, GUINT_TO_POINTER(uid))))
    emit_user_changed_for_path (self, path);
}

/***
****  ACCOUNT MANAGER / USER TRACKING
***/

/* called when a user proxy gets the 'Changed' signal */
static void
on_user_changed (AccountsUser * user, gpointer gself)
{
  /* Accounts.User doesn't update properties in the standard way,
   * so create a new proxy to pull in the new properties.
   * The older proxy is freed when it's removed from our path_to_user hash */
  const char * path = g_dbus_proxy_get_object_path (G_DBUS_PROXY(user));
  create_user_proxy_for_path (gself, path);
}

static void
track_user (IndicatorSessionUsersDbus * self,
            AccountsUser              * user)
{
  priv_t * p;
  const char * path;
  gboolean already_had_user;

  p = self->priv;

  path = g_dbus_proxy_get_object_path (G_DBUS_PROXY(user));
  already_had_user = g_hash_table_contains (p->path_to_user, path);

  g_signal_connect (user, "changed", G_CALLBACK(on_user_changed), self);
  g_hash_table_insert (p->path_to_user, g_strdup(path), user);

  if (already_had_user)
    {
      emit_user_changed_for_path (self, path);
    }
  else
    {
      const guint uid = (guint) accounts_user_get_uid (user);

      g_hash_table_insert (p->uid_to_user_path,
                           GUINT_TO_POINTER(uid),
                           g_strdup(path));

      if (!accounts_user_get_system_account (user))
        indicator_session_users_added (INDICATOR_SESSION_USERS(self), path);
    }
}

static void
untrack_user (IndicatorSessionUsersDbus * self,
              const gchar               * path)
{
  g_hash_table_remove (self->priv->path_to_user, path);

  indicator_session_users_removed (INDICATOR_SESSION_USERS(self), path);
}


static void
on_user_proxy_ready (GObject       * o        G_GNUC_UNUSED,
                     GAsyncResult  * res,
                     gpointer        self)
{
  GError * err;
  AccountsUser * user;

  err = NULL;
  user = accounts_user_proxy_new_for_bus_finish (res, &err);
  if (err != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      track_user (self, user);
    }
}

static void
create_user_proxy_for_path (IndicatorSessionUsersDbus * self,
                            const char                * path)
{
  const char * name = "org.freedesktop.Accounts";
  const GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES;

  accounts_user_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                   flags, name, path,
                                   self->priv->cancellable,
                                   on_user_proxy_ready, self);
}

static void
on_user_list_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gchar ** paths;

  err = NULL;
  paths = NULL;
  accounts_call_list_cached_users_finish (ACCOUNTS(o), &paths, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      int i;

      for (i=0; paths && paths[i]; ++i)
        create_user_proxy_for_path (gself, paths[i]);

      g_strfreev (paths);
    }
}

static void
set_account_manager (IndicatorSessionUsersDbus * self, Accounts * a)
{
  priv_t * p = self->priv;

  if (p->accounts != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->accounts, self);
      g_clear_object (&p->accounts);
    }

  if (a != NULL)
    {
      p->accounts = g_object_ref (a);

      accounts_call_list_cached_users (a,
                                       self->priv->cancellable,
                                       on_user_list_ready,
                                       self);

      g_signal_connect_swapped (a, "user-added",
                                G_CALLBACK(create_user_proxy_for_path), self);

      g_signal_connect_swapped (a, "user-deleted",
                                G_CALLBACK(untrack_user), self);
    }
}

#if 0
static void
create_accounts_proxy (IndicatorSessionUsersDbus * self)
{
  const char * name = "org.freedesktop.Accounts";
  const char * path = "/org/freedesktop/Accounts";
  GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES;

  accounts_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                              flags, name, path,
                              self->priv->cancellable,
                              on_accounts_proxy_ready, self);
}
#endif

/**
 *  SEAT / SESSION TRACKING
 *
 *  There are two simple goals here:
 *
 *   1. Keep track of how many GUI sessions each user has
 *      so that we can set the 'is_logged_in' flag correctly
 *
 *   2. Also track which is the current session,
 *      so that we can compare it to those GUI sessions to
 *      set the 'is_current_session' flag correctly.
 *
 *  Now that you know the goals, these steps may make more sense:
 *
 *   1. create a ConsoleKitManager proxy
 *   2. ask it for the current session
 *   3. create a corresponding Session proxy
 *   4. ask that Session proxy for its seat
 *   5. create a corresponding Seat proxy
 *   6. connect to that seat's session-added / session-removed signals
 *   7. ask the seat for a list of its current sessions
 *   8. create corresponding Session proxies
 *   9. of them, look for the GUI sessions by checking their X11 properties
 *   10. for each GUI session, get the corresponding uid
 *   11. use the information to update our uid <--> GUI sessions tables
 */

static void
track_session (IndicatorSessionUsersDbus * self,
               const char                * ssid,
               guint                       uid)
{
  gpointer uid_key;
  GHashTable * sessions;

  uid_key = GUINT_TO_POINTER (uid);
  sessions = g_hash_table_lookup (self->priv->uid_to_sessions, uid_key);
  if (sessions == NULL)
    {
      sessions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      g_hash_table_insert (self->priv->uid_to_sessions, uid_key, sessions);
    }

  g_hash_table_add (sessions, g_strdup (ssid));
  g_hash_table_insert (self->priv->session_to_uid, g_strdup(ssid), uid_key);

  g_debug ("%s %s now tracking ssid:%s uid:%u. uid has %u tracked ssids.",
           G_STRLOC, G_STRFUNC, ssid, uid, g_hash_table_size (sessions));

  emit_user_changed_for_uid (self, uid);
}

static void
untrack_session (IndicatorSessionUsersDbus * self,
                 const char                * ssid)
{
  gpointer uidptr;
  priv_t * p = self->priv;

  if (g_hash_table_lookup_extended (p->session_to_uid, ssid, NULL, &uidptr))
    {
      const guint uid = GPOINTER_TO_UINT (uidptr);
      GHashTable * sessions = g_hash_table_lookup (p->uid_to_sessions, uidptr);

      g_hash_table_remove (p->session_to_uid, ssid);
      g_hash_table_remove (sessions, ssid);
      g_debug ("%s %s not tracking ssid:%s uid:%u. uid has %u tracked ssids.",
               G_STRLOC, G_STRFUNC, ssid, uid,
               sessions ? g_hash_table_size (sessions) : 0);

      emit_user_changed_for_uid (self, uid);
    }
}

static void
on_session_proxy_uid_ready (GObject       * o,
                            GAsyncResult  * res,
                            gpointer        gself)
{
  guint uid;
  GError * err;
  ConsoleKitSession * session = CONSOLE_KIT_SESSION (o);

  uid = 0;
  err = NULL;
  console_kit_session_call_get_unix_user_finish (session, &uid, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else if (uid)
    {
      const char * path = g_dbus_proxy_get_object_path (G_DBUS_PROXY(session));
      track_session (gself, path, uid);
    }

  g_object_unref (o);
}

static void
on_session_x11_display_ready (GObject       * o,
                              GAsyncResult  * res,
                              gpointer        gself)
{
  priv_t * p;
  GError * err;
  gchar * gui;
  ConsoleKitSession * session;

  p = INDICATOR_SESSION_USERS_DBUS(gself)->priv;

  err = NULL;
  gui = NULL;
  session = CONSOLE_KIT_SESSION (o);
  console_kit_session_call_get_x11_display_finish (session, &gui, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      gboolean is_gui_session;

      is_gui_session = gui && *gui;

      if (!is_gui_session)
        g_clear_object (&session);
      else
        console_kit_session_call_get_unix_user (session,
                                                p->cancellable,
                                                on_session_proxy_uid_ready,
                                                gself);

      g_free (gui);
    }
}

static void
on_session_proxy_ready (GObject * o G_GNUC_UNUSED, GAsyncResult * res, gpointer gself)
{
  GError * err;
  ConsoleKitSession * session;

  err = NULL;
  session = console_kit_session_proxy_new_finish (res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else if (session != NULL)
    {
      priv_t * p = INDICATOR_SESSION_USERS_DBUS(gself)->priv;

      console_kit_session_call_get_x11_display (session,
                                                p->cancellable,
                                                on_session_x11_display_ready,
                                                gself);
    }
}

static void
create_session_proxy_for_ssid (IndicatorSessionUsersDbus * self,
                               const char                * ssid)
{
  const char * name = "org.freedesktop.ConsoleKit";
  GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES;

  console_kit_session_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                         flags, name, ssid,
                                         self->priv->cancellable,
                                         on_session_proxy_ready, self);
}

static void
on_session_list_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gchar ** sessions;

  err = NULL;
  sessions = NULL;
  console_kit_seat_call_get_sessions_finish (CONSOLE_KIT_SEAT(o),
                                             &sessions, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else
    {
      int i;

      for (i=0; sessions && sessions[i]; i++)
        create_session_proxy_for_ssid (gself, sessions[i]);

      g_strfreev (sessions);
    }
}

static inline guint
get_uid_for_session (IndicatorSessionUsersDbus * self, const char * ssid)
{
  guint uid = 0;
  gpointer value;

  if (ssid != NULL)
    if ((value = g_hash_table_lookup (self->priv->session_to_uid, ssid)))
      uid = GPOINTER_TO_UINT (value);

  return uid;
}

/* it's a live session if username is 'ubuntu' and uid is 999 */
static gboolean
is_live_ssid (IndicatorSessionUsersDbus * self, const char * ssid)
{
  priv_t * p;
  guint uid;

  p = INDICATOR_SESSION_USERS_DBUS (self)->priv;
  uid = get_uid_for_session (self, ssid);

  if (uid == 999)
    {
      const char * path;
      AccountsUser * user = NULL;

      if ((path = g_hash_table_lookup (p->uid_to_user_path, GUINT_TO_POINTER (uid))))
        user = g_hash_table_lookup (p->path_to_user, path);

      return (user != NULL) && !g_strcmp0 (accounts_user_get_user_name(user), "ubuntu");
    }

  return FALSE;
}


static void
set_active_session (IndicatorSessionUsersDbus * self, const char * ssid)
{
  priv_t * p = self->priv;
  const guint old_uid = get_uid_for_session (self, p->active_session_id);
  const guint new_uid = get_uid_for_session (self, ssid);
  const gboolean old_live = is_live_ssid (self, p->active_session_id);
  const gboolean new_live = is_live_ssid (self, ssid);

  g_debug ("%s %s changing active_session_id from '%s' to '%s'",
           G_STRLOC, G_STRFUNC, p->active_session_id, ssid);
  g_free (p->active_session_id);
  p->active_session_id = g_strdup (ssid);

  if (old_uid != new_uid)
    {
      emit_user_changed_for_uid (self, old_uid);
      emit_user_changed_for_uid (self, new_uid);
    }

  if (old_live != new_live)
    {
      indicator_session_users_notify_is_live_session (INDICATOR_SESSION_USERS(self));
    }
}

static void
on_seat_active_session_ready (GObject * o, GAsyncResult * res, gpointer gself)
{
  GError * err;
  gchar * ssid;
  ConsoleKitSeat * seat;

  err = NULL;
  ssid = NULL;
  seat = CONSOLE_KIT_SEAT (o);
  console_kit_seat_call_get_active_session_finish (seat, &ssid, res, &err);
  if (err != NULL)
    {
      g_warning ("%s %s: %s", G_STRLOC, G_STRFUNC, err->message);
      g_error_free (err);
    }
  else if (ssid != NULL)
    {
      set_active_session (INDICATOR_SESSION_USERS_DBUS(gself), ssid);
      g_free (ssid);
    }
}

static void
set_seat (IndicatorSessionUsersDbus * self, ConsoleKitSeat * seat)
{
  priv_t * p = self->priv;

  if (p->seat_proxy != NULL)
    {
      g_signal_handlers_disconnect_by_data (p->seat_proxy, self);
      g_clear_object (&p->seat_proxy);
    }

  if (seat != NULL)
    {
      p->seat_proxy = g_object_ref (seat);

      /* ask the seat for a list of all the sessions */
      console_kit_seat_call_get_sessions (seat,
                                          p->cancellable,
                                          on_session_list_ready,
                                          self);

      /* ask the seat for the name of the active session */
      console_kit_seat_call_get_active_session (p->seat_proxy,
                                                p->cancellable,
                                                on_seat_active_session_ready,
                                                self);

      /* listen for session changes in this seat */
      g_signal_connect_swapped (seat, "session-added",
                                G_CALLBACK(create_session_proxy_for_ssid),self);
      g_signal_connect_swapped (seat, "session-removed",
                                G_CALLBACK(untrack_session), self);
      g_signal_connect_swapped (seat, "active-session-changed",
                                G_CALLBACK(set_active_session), self);
    }
}

/***
****
***/

static void
set_dm_seat (IndicatorSessionUsersDbus * self, DisplayManagerSeat * dm_seat)
{
  priv_t * p = self->priv;

  g_clear_object (&p->dm_seat);

  if (dm_seat != NULL)
    p->dm_seat = g_object_ref (dm_seat);
}

static void
activate_username (IndicatorSessionUsersDbus * self, const char * username)
{
  priv_t * p = self->priv;
  const char * session = "";

  g_return_if_fail (p->dm_seat != NULL);

  display_manager_seat_call_switch_to_user (p->dm_seat, username, session,
                                            p->cancellable, NULL, NULL);
}

/***
****
***/

static void
my_dispose (GObject * o)
{
  IndicatorSessionUsersDbus * self = INDICATOR_SESSION_USERS_DBUS (o);
  priv_t * p = self->priv;

  if (p->cancellable)
    {
      g_cancellable_cancel (p->cancellable);
      g_clear_object (&p->cancellable);
    }

  set_seat (self, NULL);
  set_dm_seat (self, NULL);
  set_account_manager (self, NULL);

  g_clear_pointer (&p->path_to_user, g_hash_table_destroy);
  g_clear_pointer (&p->session_to_uid, g_hash_table_destroy);
  g_clear_pointer (&p->uid_to_sessions, g_hash_table_destroy);
  g_clear_pointer (&p->uid_to_user_path, g_hash_table_destroy);

  G_OBJECT_CLASS (indicator_session_users_dbus_parent_class)->dispose (o);
}

static void
my_finalize (GObject * o)
{
  IndicatorSessionUsersDbus * u = INDICATOR_SESSION_USERS_DBUS (o);

  g_free (u->priv->active_session_id);

  G_OBJECT_CLASS (indicator_session_users_dbus_parent_class)->finalize (o);
}

static void
my_activate_user (IndicatorSessionUsers * users, const char * key)
{
  priv_t * p;
  const char * username = 0;

  p = INDICATOR_SESSION_USERS_DBUS (users)->priv;
  if (p != 0)
    {
      AccountsUser * au = g_hash_table_lookup (p->path_to_user, key);

      if (au != NULL)
        username = accounts_user_get_user_name (au);
    }

  if (username != 0)
    activate_username (INDICATOR_SESSION_USERS_DBUS(users), username);
  else
    g_warning ("%s %s can't find user for '%s'", G_STRLOC, G_STRFUNC, key);
}

static gboolean
my_is_live_session (IndicatorSessionUsers * users)
{
  IndicatorSessionUsersDbus * self = INDICATOR_SESSION_USERS_DBUS(users);

  return is_live_ssid (self, self->priv->active_session_id);
}

static GStrv
my_get_keys (IndicatorSessionUsers * users)
{
  int i;
  priv_t * p;
  gchar ** keys;
  GHashTableIter iter;
  gpointer path;
  gpointer user;

  g_return_val_if_fail (INDICATOR_IS_SESSION_USERS_DBUS(users), NULL);
  p = INDICATOR_SESSION_USERS_DBUS (users)->priv;

  i = 0;
  keys = g_new (gchar*, g_hash_table_size(p->path_to_user)+1);
  g_hash_table_iter_init (&iter, p->path_to_user);
  while (g_hash_table_iter_next (&iter, &path, &user))
    if (!accounts_user_get_system_account (user))
      keys[i++] = g_strdup (path);
  keys[i] = NULL;

  return keys;
}

static IndicatorSessionUser *
my_get_user (IndicatorSessionUsers * users, const gchar * key)
{
  priv_t * p;
  AccountsUser * au;
  IndicatorSessionUser * ret = NULL;

  p = INDICATOR_SESSION_USERS_DBUS (users)->priv;

  au = g_hash_table_lookup (p->path_to_user, key);
  if (au && !accounts_user_get_system_account(au))
    {
      const guint uid = (guint) accounts_user_get_uid (au);
      GHashTable * s;

      ret = g_new0 (IndicatorSessionUser, 1);

      s = g_hash_table_lookup (p->uid_to_sessions, GUINT_TO_POINTER(uid));
      if (s == NULL)
        {
          ret->is_logged_in = FALSE;
          ret->is_current_user = FALSE;
        }
      else
        {
          ret->is_logged_in = g_hash_table_size (s) > 0;
          ret->is_current_user = g_hash_table_contains (s, p->active_session_id);
        }

      ret->uid = uid;
      ret->user_name = g_strdup (accounts_user_get_user_name (au));
      ret->real_name = g_strdup (accounts_user_get_real_name (au));
      ret->icon_file = g_strdup (accounts_user_get_icon_file (au));
      ret->login_frequency = accounts_user_get_login_frequency (au);
    }

  return ret;
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_users_dbus_class_init (IndicatorSessionUsersDbusClass * klass)
{
  GObjectClass * object_class;
  IndicatorSessionUsersClass * users_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = my_dispose;
  object_class->finalize = my_finalize;

  users_class = INDICATOR_SESSION_USERS_CLASS (klass);
  users_class->is_live_session = my_is_live_session;
  users_class->get_keys = my_get_keys;
  users_class->get_user = my_get_user;
  users_class->activate_user = my_activate_user;

  g_type_class_add_private (klass, sizeof (IndicatorSessionUsersDbusPriv));
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_users_dbus_init (IndicatorSessionUsersDbus * self)
{
  priv_t * p;

  p = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                   INDICATOR_TYPE_SESSION_USERS_DBUS,
                                   IndicatorSessionUsersDbusPriv);
  self->priv = p;
  p->cancellable = g_cancellable_new ();

  p->path_to_user = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, g_object_unref);

  p->uid_to_user_path = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                               NULL, g_free);

  p->session_to_uid = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, NULL);

  p->uid_to_sessions = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                              NULL,
                                              (GDestroyNotify)g_hash_table_destroy);

#if 0
  console_kit_manager_proxy_new_for_bus (
                               G_BUS_TYPE_SYSTEM,
                               G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                               "org.freedesktop.ConsoleKit",
                               "/org/freedesktop/ConsoleKit/Manager",
                               p->cancellable,
                               on_console_kit_manager_proxy_ready,
                               self);
#endif
}

/***
****  Public
***/

IndicatorSessionUsers *
indicator_session_users_dbus_new (void)
{
  gpointer o = g_object_new (INDICATOR_TYPE_SESSION_USERS_DBUS, NULL);

  return INDICATOR_SESSION_USERS (o);
}

void
indicator_session_users_dbus_set_proxies (IndicatorSessionUsersDbus * self,
                                          Accounts                  * accounts,
                                          DisplayManagerSeat        * dm_seat,
                                          ConsoleKitSeat            * seat)
{
  g_return_if_fail (INDICATOR_IS_SESSION_USERS_DBUS (self));

  set_account_manager (self, accounts);
  set_seat (self, seat);
  set_dm_seat (self, dm_seat);
}
