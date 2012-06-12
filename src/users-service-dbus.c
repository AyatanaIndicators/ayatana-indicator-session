/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * Copyright 2009 Canonical Ltd.
 *
 * Authors:
 *     Cody Russell <crussell@canonical.com>
 *     Charles Kerr <charles.kerr@canonical.com>
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

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <glib.h>

#include <errno.h>
#include <string.h>

#include <pwd.h> /* getpwuid() */

#include "dbus-shared-names.h"
#include "dbus-accounts.h"
#include "dbus-consolekit-manager.h"
#include "dbus-consolekit-seat.h"
#include "dbus-consolekit-session.h"
#include "dbus-display-manager.h"
#include "dbus-user.h"
#include "users-service-dbus.h"

#define CK_ADDR             "org.freedesktop.ConsoleKit"
#define CK_SESSION_IFACE    "org.freedesktop.ConsoleKit.Session"

/**
***
**/

static void     init_users           (UsersServiceDbus  * self);

static gchar*   get_seat             (UsersServiceDbus  * service);

static void     on_user_added        (Accounts          * o,
                                      const gchar       * user_object_path,
                                      UsersServiceDbus  * service);

static void     on_user_deleted      (Accounts          * o,
                                      const gchar       * user_object_path,
                                      UsersServiceDbus  * service);

static void     on_session_added     (ConsoleKitSeat    * seat,
                                      const gchar       * ssid,
                                      UsersServiceDbus  * service);

static void     on_session_removed   (ConsoleKitSeat    * seat,
                                      const gchar       * ssid,
                                      UsersServiceDbus  * service);

static void     on_session_list      (ConsoleKitSeat    * seat,
                                      GAsyncResult      * result,
                                      UsersServiceDbus  * service);

/***
****  Priv Struct
***/

struct _UsersServiceDbusPrivate
{
  gchar * seat;
  gchar * guest_ssid;

  /* ssid -> AccountsUser lookup */
  GHashTable * sessions;

  /* user object path -> AccountsUser lookup */
  GHashTable * users;

  GCancellable * cancellable;
  ConsoleKitSeat * seat_proxy;
  ConsoleKitManager * ck_manager_proxy;
  Accounts * accounts_proxy;
  DisplayManagerSeat * display_proxy;
};

/***
****  GObject
***/

enum
{
  USER_ADDED,
  USER_DELETED,
  USER_LOGGED_IN_CHANGED,
  GUEST_LOGGED_IN_CHANGED,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE (UsersServiceDbus, users_service_dbus, G_TYPE_OBJECT);

static void
users_service_dbus_dispose (GObject *object)
{
  UsersServiceDbusPrivate * priv = USERS_SERVICE_DBUS(object)->priv;

  g_clear_object (&priv->accounts_proxy);
  g_clear_object (&priv->display_proxy);
  g_clear_object (&priv->seat_proxy);
  g_clear_object (&priv->ck_manager_proxy);

  if (priv->cancellable != NULL)
    {
      g_cancellable_cancel (priv->cancellable);
      g_clear_object (&priv->cancellable);
    }

  if (priv->users != NULL)
    {
      g_hash_table_destroy (priv->users);
      priv->users = NULL;
    }

  if (priv->sessions != NULL)
    {
      g_hash_table_destroy (priv->sessions);
      priv->sessions = NULL;
    }

  G_OBJECT_CLASS (users_service_dbus_parent_class)->dispose (object);
}

static void
users_service_dbus_finalize (GObject *object)
{
  UsersServiceDbusPrivate * priv = USERS_SERVICE_DBUS(object)->priv;

  g_free (priv->guest_ssid);
  g_free (priv->seat);

  G_OBJECT_CLASS (users_service_dbus_parent_class)->finalize (object);
}

static void
users_service_dbus_class_init (UsersServiceDbusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (object_class, sizeof (UsersServiceDbusPrivate));

  object_class->dispose = users_service_dbus_dispose;
  object_class->finalize = users_service_dbus_finalize;

  signals[USER_ADDED] = g_signal_new (
              "user-added",
              G_TYPE_FROM_CLASS (klass),
              G_SIGNAL_RUN_LAST,
              G_STRUCT_OFFSET (UsersServiceDbusClass, user_added),
              NULL, NULL,
              g_cclosure_marshal_VOID__OBJECT,
              G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[USER_DELETED] = g_signal_new (
              "user-deleted",
              G_TYPE_FROM_CLASS (klass),
              G_SIGNAL_RUN_LAST,
              G_STRUCT_OFFSET (UsersServiceDbusClass, user_deleted),
              NULL, NULL,
              g_cclosure_marshal_VOID__OBJECT,
              G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[USER_LOGGED_IN_CHANGED] = g_signal_new (
              "user-logged-in-changed",
              G_TYPE_FROM_CLASS (klass),
              G_SIGNAL_RUN_LAST,
              G_STRUCT_OFFSET (UsersServiceDbusClass, user_logged_in_changed),
              NULL, NULL,
              g_cclosure_marshal_VOID__OBJECT,
              G_TYPE_NONE, 1, G_TYPE_OBJECT);

  signals[GUEST_LOGGED_IN_CHANGED] = g_signal_new (
              "guest-logged-in-changed",
              G_TYPE_FROM_CLASS (klass),
              G_SIGNAL_RUN_LAST,
              G_STRUCT_OFFSET (UsersServiceDbusClass, guest_logged_in_changed),
              NULL, NULL,
              g_cclosure_marshal_VOID__VOID,
              G_TYPE_NONE, 0);
}

static void
users_service_dbus_init (UsersServiceDbus *self)
{
  GError * error = NULL;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            USERS_SERVICE_DBUS_TYPE,
                                            UsersServiceDbusPrivate);

  UsersServiceDbusPrivate * p  = self->priv;

  p->cancellable = g_cancellable_new ();

  /* ssid -> AccountsUser */
  p->sessions = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       g_object_unref);

  /* user object path -> AccountsUser */
  p->users = g_hash_table_new_full (g_str_hash,
                                    g_str_equal,
                                    g_free,
                                    g_object_unref);

  /**
  ***  create the consolekit manager proxy...
  **/

  p->ck_manager_proxy = console_kit_manager_proxy_new_for_bus_sync (
                             G_BUS_TYPE_SYSTEM,
                             G_DBUS_PROXY_FLAGS_NONE,
                             "org.freedesktop.ConsoleKit",
                             "/org/freedesktop/ConsoleKit/Manager",
                             NULL,
                             &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRLOC, error->message);
      g_clear_error (&error);
    }

  p->seat = get_seat (self);

  /**
  ***  create the consolekit seat proxy...
  **/

  if (p->seat != NULL)
    {
      ConsoleKitSeat * proxy = console_kit_seat_proxy_new_for_bus_sync (
                                 G_BUS_TYPE_SYSTEM,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 "org.freedesktop.ConsoleKit",
                                 p->seat,
                                 NULL,
                                 &error);

      if (error != NULL)
        {
          g_warning ("Failed to connect to the ConsoleKit seat: %s", error->message);
          g_clear_error (&error);
        }
      else
        {
          g_signal_connect (proxy, "session-added",
                            G_CALLBACK (on_session_added), self);
          g_signal_connect (proxy, "session-removed",
                            G_CALLBACK (on_session_removed), self);
          console_kit_seat_call_get_sessions (proxy, p->cancellable,
                            (GAsyncReadyCallback)on_session_list, self);
          p->seat_proxy = proxy;
        }
    }

  /**
  ***  create the accounts manager proxy...
  **/

  Accounts * proxy = accounts_proxy_new_for_bus_sync (
                       G_BUS_TYPE_SYSTEM,
                       G_DBUS_PROXY_FLAGS_NONE,
                       "org.freedesktop.Accounts",
                       "/org/freedesktop/Accounts",
                       NULL,
                       &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
  else
    {
      g_signal_connect (proxy, "user-added", G_CALLBACK(on_user_added), self);
      g_signal_connect (proxy, "user-deleted", G_CALLBACK(on_user_deleted), self);
      p->accounts_proxy = proxy;
      init_users (self);
    }
}

/***
****
***/

static void
emit_user_added (UsersServiceDbus * self, AccountsUser * user)
{
  g_signal_emit (self, signals[USER_ADDED], 0, user);
}

static void
emit_user_deleted (UsersServiceDbus * self, AccountsUser * user)
{
  g_signal_emit (self, signals[USER_DELETED], 0, user);
}

static void
emit_user_login_changed (UsersServiceDbus * self, AccountsUser * user)
{
  g_signal_emit (self, signals[USER_LOGGED_IN_CHANGED], 0, user);
}

static void
emit_guest_login_changed (UsersServiceDbus * self)
{
  g_signal_emit (self, signals[GUEST_LOGGED_IN_CHANGED], 0);
}

/***
****
***/

static ConsoleKitSession*
create_consolekit_session_proxy (const char * ssid)
{
  GError * error = NULL;

  ConsoleKitSession * p = console_kit_session_proxy_new_for_bus_sync (
                            G_BUS_TYPE_SYSTEM,
                            G_DBUS_PROXY_FLAGS_NONE,
                            CK_ADDR,
                            ssid,
                            NULL,
                            &error);

  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }

  return p;
}

static gchar *
get_seat_from_session_proxy (ConsoleKitSession * session_proxy)
{
  gchar * seat = NULL;

  GError * error = NULL;
  console_kit_session_call_get_seat_id_sync (session_proxy,
                                             &seat,
                                             NULL,
                                             &error);
  if (error != NULL)
    {
      g_debug ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }

  return seat;
}

static gchar *
get_seat (UsersServiceDbus *service)
{
  gchar * seat = NULL;
  gchar * ssid = NULL;
  GError * error = NULL;
  UsersServiceDbusPrivate * priv = service->priv;

  console_kit_manager_call_get_current_session_sync (priv->ck_manager_proxy,
                                                     &ssid,
                                                     NULL,
                                                     &error);

  if (error != NULL)
    {
      g_debug ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }
  else
    {
      ConsoleKitSession * session = create_consolekit_session_proxy (ssid);

      if (session != NULL)
        {
          seat = get_seat_from_session_proxy (session);
          g_object_unref (session);
        }
    }

  return seat;
}

/* lazy-create the display manager proxy */
static DisplayManagerSeat *
get_display_proxy (UsersServiceDbus * self)
{
  UsersServiceDbusPrivate * priv = self->priv;

  if (priv->display_proxy == NULL)
    {
      const gchar * const seat = g_getenv ("XDG_SEAT_PATH");
      g_debug ("%s lazy-creating the DisplayManager proxy for seat %s", G_STRLOC, seat);

      GError * error = NULL;
      DisplayManagerSeat * p = display_manager_seat_proxy_new_for_bus_sync (
                                 G_BUS_TYPE_SYSTEM,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 "org.freedesktop.DisplayManager",
                                 seat,
                                 NULL,
                                 &error);

      if (error == NULL)
        {
          priv->display_proxy = p;
        }
      else
        {
          g_warning ("%s: %s", G_STRLOC, error->message);
          g_error_free (error);
        }
    }

  return priv->display_proxy;
}

/***
****  AccountsUser add-ons for tracking sessions
***/

static GHashTable*
user_get_sessions_hashset (AccountsUser * user)
{
  static GQuark q = 0;

  if (G_UNLIKELY(!q))
    {
      q = g_quark_from_static_string ("sessions");
    }

  GObject * o = G_OBJECT (user);
  GHashTable * h = g_object_get_qdata (o, q);
  if (h == NULL)
    {
      h = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      g_object_set_qdata_full (o, q, h, (GDestroyNotify)g_hash_table_destroy);
    }

  return h;
}

static void
user_add_session (AccountsUser * user, const char * ssid)
{
  g_hash_table_add (user_get_sessions_hashset(user), g_strdup(ssid));
}

static void
user_remove_session (AccountsUser * user, const char * ssid)
{
  g_hash_table_remove (user_get_sessions_hashset(user), ssid);
}

static guint
user_count_sessions (AccountsUser * user)
{
  return g_hash_table_size (user_get_sessions_hashset(user));
}

/***
****  Users
***/

/* adds this user session to the user's and service's session tables */
static void
add_user_session (UsersServiceDbus  * service,
                  AccountsUser      * user,
                  const gchar       * ssid)
{
  ConsoleKitSession * session_proxy = create_consolekit_session_proxy (ssid);
  if (session_proxy != NULL)
    {
      UsersServiceDbusPrivate * priv = service->priv;
      gchar * seat = get_seat_from_session_proxy (session_proxy);

      /* is this session in our seat? */
      if (seat && priv->seat && !g_strcmp0 (seat, priv->seat))
        {
          /* does this session have a display? */
          gchar * display = NULL;
          console_kit_session_call_get_x11_display_sync (session_proxy,
                                                         &display,
                                                         NULL, NULL);
          const gboolean has_display = display && *display;
          g_free (display);

          if (has_display)
            {
              const gchar * username = accounts_user_get_user_name (user);
              g_debug ("%s adding %s's session '%s' to our tables",
                       G_STRLOC, username, ssid);

              g_hash_table_insert (priv->sessions,
                                   g_strdup (ssid),
                                   g_object_ref (user));

              user_add_session (user, ssid);
            }
        }

      g_free (seat);
      g_object_unref (session_proxy);
    }
}

/* calls add_user_session() for each of this user's sessions */
static void
add_user (UsersServiceDbus *self, AccountsUser * user)
{
  const guint64 uid = accounts_user_get_uid (user);
  const char * username = accounts_user_get_user_name (user);
  g_debug ("%s adding %s (%i)", G_STRLOC, username, (int)uid);

  GError * error = NULL;
  gchar ** sessions = NULL;
  console_kit_manager_call_get_sessions_for_unix_user_sync (
                                              self->priv->ck_manager_proxy,
                                              uid,
                                              &sessions,
                                              NULL,
                                              &error);

  if (error != NULL)
    {
      g_debug ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }
  else if (sessions != NULL)
    {
      int i;

      for (i=0; sessions[i]; i++)
        {
          const char * const ssid = sessions[i];
          g_debug ("%s adding %s's session %s", G_STRLOC, username, ssid);
          add_user_session (self, user, ssid);
        }

      g_strfreev (sessions);
    }
}

static void
add_user_from_object_path (UsersServiceDbus  * self,
                           const char        * user_object_path)
{
  GError * error = NULL;
  AccountsUser * user = accounts_user_proxy_new_for_bus_sync (
                          G_BUS_TYPE_SYSTEM,
                          G_DBUS_PROXY_FLAGS_NONE,
                          "org.freedesktop.Accounts",
                          user_object_path,
                          NULL,
                          &error);

  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRLOC, error->message);
      g_clear_error (&error);
    }
  else
    {
      g_debug ("%s adding user %s from object path %s", G_STRLOC,
               accounts_user_get_user_name(user),
               user_object_path);
      g_hash_table_insert (self->priv->users, g_strdup(user_object_path), user);
      add_user (self, user);
    }
}


/* calls add_user_from_object_path() on a list of user object paths */
static void
init_users (UsersServiceDbus *self)
{
  g_return_if_fail(IS_USERS_SERVICE_DBUS(self));

  GError * error = NULL;
  char ** object_paths = NULL;
  UsersServiceDbusPrivate * priv = self->priv;
  g_debug ("%s bootstrapping the user list", G_STRLOC);

  accounts_call_list_cached_users_sync (priv->accounts_proxy,
                                        &object_paths,
                                        NULL,
                                        &error);

  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_clear_error (&error);
    }
  else if (object_paths != NULL)
    {
      gint i;

      for (i=0; object_paths[i] != NULL; ++i)
        {
          add_user_from_object_path (self, object_paths[i]);
        }

      g_strfreev (object_paths);
    }

  g_debug ("%s finished bootstrapping the user list", G_STRLOC);
}

static void
on_user_added (Accounts          * o                    G_GNUC_UNUSED,
               const gchar       * user_path,
               UsersServiceDbus  * service)
{
  add_user_from_object_path (service, user_path);

  AccountsUser * user = g_hash_table_lookup (service->priv->users, user_path);
  emit_user_added (service, user);
}

static void
on_user_deleted (Accounts          * o                  G_GNUC_UNUSED,
                 const gchar       * user_path,
                 UsersServiceDbus  * service)
{
  AccountsUser * user = g_hash_table_lookup (service->priv->users, user_path);

  if (user != NULL)
    {
      GObject * o = g_object_ref (G_OBJECT(user));
      g_hash_table_remove (service->priv->users, user_path);
      emit_user_deleted (service, user);
      g_object_unref (o);
    }
}

static AccountsUser *
find_user_from_username (UsersServiceDbus  * self,
                         const gchar       * username)
{
  AccountsUser * match = NULL;

  g_return_val_if_fail (IS_USERS_SERVICE_DBUS(self), match);

  gpointer user;
  GHashTableIter iter;
  g_hash_table_iter_init (&iter, self->priv->users);
  while (!match && g_hash_table_iter_next (&iter, NULL, &user))
    {
      if (!g_strcmp0 (username, accounts_user_get_user_name (user)))
        {
          match = user;
        }
    }

  return match;
}

/***
****  Sessions
***/

static void
on_session_removed (ConsoleKitSeat   * seat_proxy,
                    const gchar      * ssid,
                    UsersServiceDbus * service)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS (service));

  UsersServiceDbusPrivate * priv = service->priv;
  g_debug ("%s %s() session removed %s", G_STRLOC, G_STRFUNC, ssid);

  if (!g_strcmp0 (ssid, priv->guest_ssid))
    {
      g_debug ("%s removing guest session %s", G_STRLOC, ssid);
      g_clear_pointer (&priv->guest_ssid, g_free);
      emit_guest_login_changed (service);
    }
  else
    {
      AccountsUser * user = g_hash_table_lookup (priv->sessions, ssid);
      if (user == NULL)
        {
          g_debug ("%s we're not tracking ssid %s", G_STRLOC, ssid);
        }
      else
        {
          GObject * o = g_object_ref (G_OBJECT(user));
          g_hash_table_remove (service->priv->users, ssid);
          user_remove_session (user, ssid);
          emit_user_login_changed (service, user);
          g_object_unref (o);
        }
    }
}

static gchar*
get_unix_username_from_ssid (UsersServiceDbus * self,
                             const gchar      * ssid)
{
  gchar * username = NULL;

  ConsoleKitSession * session_proxy = create_consolekit_session_proxy (ssid);
  if (session_proxy != NULL)
    {
      guint uid = 0;
      GError * error = NULL;
      console_kit_session_call_get_unix_user_sync (session_proxy,
                                                   &uid,
                                                   NULL, &error);
      if (error != NULL)
        {
          g_warning ("%s: %s", G_STRLOC, error->message);
          g_clear_error (&error);
        }
      else
        {
          errno = 0;
          const struct passwd * pwent = getpwuid (uid);
          if (pwent == NULL)
            {
              g_warning ("Failed to lookup user id %d: %s", (int)uid, g_strerror(errno));
            }
          else
            { 
              username = g_strdup (pwent->pw_name);
            }
        }

      g_object_unref (session_proxy);
    }

  return username;
}

static gboolean
is_guest_username (const char * username)
{
  if (!g_strcmp0 (username, "guest"))
    return TRUE;

  if ((strlen(username)==12) && !memcmp(username,"guest-",6))
    return TRUE;

  return FALSE;
}

/* If the new session belongs to 'guest', update our guest_ssid.
   Otherwise, call add_user_session() to update our session tables */
static void
on_session_added (ConsoleKitSeat   * seat_proxy G_GNUC_UNUSED,
                  const gchar      * ssid,
                  UsersServiceDbus * service)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS(service));

  gchar * username = get_unix_username_from_ssid (service, ssid);
  g_debug ("%s %s() username %s has new session %s", G_STRLOC, G_STRFUNC, username, ssid);

  if (is_guest_username (username))
    {
      /* handle guest as a special case -- it's not in the GDM
         user tables and there isn't be an AccountsUser for it */
      g_debug("Found guest session: %s", ssid);
      g_free (service->priv->guest_ssid);
      service->priv->guest_ssid = g_strdup (ssid);
      emit_guest_login_changed (service);
    }
  else
    {
      AccountsUser * user = find_user_from_username (service, username);

      if (user != NULL)
        {
          add_user_session (service, user, ssid);
          emit_user_login_changed (service, user);
        }
    }

  g_free (username);
}

/* Receives a list of sessions and calls on_session_added() for each of them */
static void
on_session_list (ConsoleKitSeat   * seat_proxy,
                 GAsyncResult     * result,
                 UsersServiceDbus * self)
{
  GError * error = NULL;
  gchar ** sessions = NULL;
  g_debug ("%s bootstrapping the session list", G_STRLOC);

  console_kit_seat_call_get_sessions_finish (seat_proxy,
                                             &sessions,
                                             result,
                                             &error);

  if (error != NULL)
    {
      g_debug ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }
  else if (sessions != NULL)
    {
      int i;

      for (i=0; sessions[i]; i++)
        {
          g_debug ("%s adding initial session '%s'", G_STRLOC, sessions[i]);
          on_session_added (seat_proxy, sessions[i], self);
        }

      g_strfreev (sessions);
    }

  g_debug ("%s done bootstrapping the session list", G_STRLOC);
}

/***
****  Public API
***/

/**
 * users_service_dbus_get_user_list:
 *
 * Returns: (transfer container): a list of AccountsUser objects
 */
GList *
users_service_dbus_get_user_list (UsersServiceDbus * self)
{
  g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), NULL);

  return g_hash_table_get_values (self->priv->users);
}

/**
 * users_service_dbus_show_greeter:
 *
 * Ask the Display Mnaager to switch to the greeter screen.
 */
void
users_service_dbus_show_greeter (UsersServiceDbus * self)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS(self));

  display_manager_seat_call_switch_to_greeter_sync (get_display_proxy(self),
                                                    NULL,
                                                    NULL);
}

/**
 * users_service_dbus_activate_guest_session:
 *
 * Activates the guest account.
 */
void
users_service_dbus_activate_guest_session (UsersServiceDbus * self)
{
  g_return_if_fail(IS_USERS_SERVICE_DBUS(self));

  display_manager_seat_call_switch_to_guest_sync (get_display_proxy(self),
                                                  "",
                                                  NULL,
                                                  NULL);
}

/**
 * users_service_dbus_activate_user_session:
 *
 * Activates a specific user.
 */
void
users_service_dbus_activate_user_session (UsersServiceDbus * self,
                                          AccountsUser     * user)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS(self));

  const char * const username = accounts_user_get_user_name (user);
  display_manager_seat_call_switch_to_user_sync (get_display_proxy(self),
                                                 username,
                                                 "",
                                                 NULL,
                                                 NULL);
}

/**
 * users_service_dbus_guest_session_enabled:
 *
 * Tells whether or not guest sessions are allowed.
 */
gboolean
users_service_dbus_guest_session_enabled (UsersServiceDbus * self)
{
  g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);

  return display_manager_seat_get_has_guest_account (get_display_proxy(self));
}

gboolean
users_service_dbus_can_activate_session (UsersServiceDbus * self)
{
  gboolean can_activate = FALSE;

  g_return_val_if_fail (IS_USERS_SERVICE_DBUS(self), can_activate);

  GError * error = NULL;
  console_kit_seat_call_can_activate_sessions_sync (self->priv->seat_proxy,
                                                    &can_activate,
                                                    NULL,
                                                    &error);
  if (error != NULL)
    {
      g_warning ("%s: %s", G_STRLOC, error->message);
      g_error_free (error);
    }

  return can_activate;
}

gboolean
users_service_dbus_is_guest_logged_in (UsersServiceDbus * self)
{
  g_return_val_if_fail (IS_USERS_SERVICE_DBUS(self), FALSE);

  return self->priv->guest_ssid != NULL;
}

gboolean
users_service_dbus_is_user_logged_in (UsersServiceDbus  * self,
                                      AccountsUser      * user)
{
  g_return_val_if_fail (IS_USERS_SERVICE_DBUS(self), FALSE);
  g_return_val_if_fail (IS_ACCOUNTS_USER(user), FALSE);

  return user_count_sessions (user) > 0;
}
