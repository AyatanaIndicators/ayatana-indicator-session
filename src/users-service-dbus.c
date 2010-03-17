/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * Copyright 2009 Canonical Ltd.
 *
 * Authors:
 *     Cody Russell <crussell@canonical.com>
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

#include <string.h>
#include <errno.h>
#include <pwd.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "dbus-shared-names.h"
#include "gdm-local-display-factory-client.h"
#include "users-service-dbus.h"
#include "users-service-client.h"
#include "users-service-marshal.h"
#include "consolekit-manager-client.h"

static void     users_service_dbus_class_init         (UsersServiceDbusClass *klass);
static void     users_service_dbus_init               (UsersServiceDbus  *self);
static void     users_service_dbus_dispose            (GObject           *object);
static void     users_service_dbus_finalize           (GObject           *object);
static void     create_gdm_proxy                       (UsersServiceDbus  *self);
static void     create_seat_proxy                      (UsersServiceDbus  *self);
static void     create_ck_proxy                        (UsersServiceDbus *self);
static void     create_cksession_proxy                 (UsersServiceDbus *self);
static gchar   *get_seat                               (UsersServiceDbus *service);
static void     users_loaded                           (DBusGProxy        *proxy,
                                                        gpointer           user_data);
static void     user_added                             (DBusGProxy        *proxy,
                                                        gint64             uid,
                                                        gpointer           user_data);
static void     user_removed                           (DBusGProxy        *proxy,
                                                        gint64             uid,
                                                        gpointer           user_data);
static void     user_updated                           (DBusGProxy        *proxy,
                                                        guint              uid,
                                                        gpointer           user_data);
static void     seat_proxy_session_added               (DBusGProxy        *seat_proxy,
                                                        const gchar       *session_id,
                                                        UsersServiceDbus  *service);
static void     seat_proxy_session_removed             (DBusGProxy        *seat_proxy,
                                                        const gchar       *session_id,
                                                        UsersServiceDbus  *service);
static gboolean do_add_session                         (UsersServiceDbus  *service,
                                                        UserData          *user,
                                                        const gchar       *ssid);
static gchar *  get_seat_internal                      (UsersServiceDbus  *self);

/* Private */
typedef struct _UsersServiceDbusPrivate UsersServiceDbusPrivate;

struct _UsersServiceDbusPrivate
{
  GHashTable *users;
  gint        count;
  gchar      *seat;
  gchar      *ssid;

  DBusGConnection *system_bus;

  DBusGProxy *gdm_proxy;
  DBusGProxy *gdm_local_proxy;
  DBusGProxy *ck_proxy;
  DBusGProxy *seat_proxy;
  DBusGProxy *session_proxy;

  GHashTable *exclusions;
  GHashTable *sessions;

  DbusmenuMenuitem * guest_item;
  gchar * guest_session_id;
};

#define USERS_SERVICE_DBUS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), USERS_SERVICE_DBUS_TYPE, UsersServiceDbusPrivate))

/* Signals */
enum {
  USERS_LOADED,
  USER_ADDED,
  USER_REMOVED,
  USER_UPDATED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GObject Boilerplate */
G_DEFINE_TYPE (UsersServiceDbus, users_service_dbus, G_TYPE_OBJECT);

static void
users_service_dbus_class_init (UsersServiceDbusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (object_class, sizeof (UsersServiceDbusPrivate));

  object_class->dispose = users_service_dbus_dispose;
  object_class->finalize = users_service_dbus_finalize;

  signals[USERS_LOADED] = g_signal_new ("users-loaded",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (UsersServiceDbusClass, users_loaded),
                                        NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);

  signals[USER_ADDED] = g_signal_new ("user-added",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_LAST,
                                      G_STRUCT_OFFSET (UsersServiceDbusClass, user_added),
                                      NULL, NULL,
                                      _users_service_marshal_VOID__INT64,
                                      G_TYPE_NONE, 1, G_TYPE_INT64);

  signals[USER_REMOVED] = g_signal_new ("user-removed",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (UsersServiceDbusClass, user_removed),
                                        NULL, NULL,
                                        _users_service_marshal_VOID__INT64,
                                        G_TYPE_NONE, 1, G_TYPE_INT64);

  signals[USER_UPDATED] = g_signal_new ("user-updated",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (UsersServiceDbusClass, user_updated),
                                        NULL, NULL,
                                        _users_service_marshal_VOID__INT64,
                                        G_TYPE_NONE, 1, G_TYPE_INT64);
}

static void
users_service_dbus_init (UsersServiceDbus *self)
{
  GError *error = NULL;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  priv->users = NULL;
  priv->count = 0;
  priv->guest_item = NULL;
  priv->guest_session_id = NULL;

  /* Get the system bus */
  priv->system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (error != NULL)
    {
      g_error ("Unable to get system bus: %s", error->message);
      g_error_free(error);

      return;
    }

  priv->sessions = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_free);

  priv->users = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       NULL);

  dbus_g_object_register_marshaller (_users_service_marshal_VOID__INT64,
                                     G_TYPE_NONE,
                                     G_TYPE_INT64,
                                     G_TYPE_INVALID);

  create_gdm_proxy (self);
  create_ck_proxy (self);
  create_seat_proxy (self);

  if (priv->gdm_proxy)
    users_loaded (priv->gdm_proxy, self);
}

static void
users_service_dbus_dispose (GObject *object)
{
  G_OBJECT_CLASS (users_service_dbus_parent_class)->dispose (object);
}

static void
users_service_dbus_finalize (GObject *object)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (object);

  if (priv->guest_session_id != NULL) {
    g_free(priv->guest_session_id);
    priv->guest_session_id = NULL;
  }
  
  G_OBJECT_CLASS (users_service_dbus_parent_class)->finalize (object);
}

static void
create_gdm_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;

  priv->gdm_proxy = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                     "org.gnome.DisplayManager",
                                                     "/org/gnome/DisplayManager/UserManager",
                                                     "org.gnome.DisplayManager.UserManager",
                                                     &error);

  if (!priv->gdm_proxy)
    {
      if (error != NULL)
        {
          g_warning ("Unable to get DisplayManager proxy on system bus: %s", error->message);
          g_error_free (error);
        }

      return;
    }

  dbus_g_proxy_add_signal (priv->gdm_proxy,
                           "UsersLoaded",
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->gdm_proxy,
                           "UserAdded",
                           G_TYPE_INT64,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->gdm_proxy,
                           "UserRemoved",
                           G_TYPE_INT64,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->gdm_proxy,
                           "UserUpdated",
                           G_TYPE_INT64,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->gdm_proxy,
                               "UsersLoaded",
                               G_CALLBACK (users_loaded),
                               self,
                               NULL);

  dbus_g_proxy_connect_signal (priv->gdm_proxy,
                               "UserAdded",
                               G_CALLBACK (user_added),
                               self,
                               NULL);

  dbus_g_proxy_connect_signal (priv->gdm_proxy,
                               "UserRemoved",
                               G_CALLBACK (user_removed),
                               self,
                               NULL);

  dbus_g_proxy_connect_signal (priv->gdm_proxy,
                               "UserUpdated",
                               G_CALLBACK (user_updated),
                               self,
                               NULL);

  priv->gdm_local_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                     "org.gnome.DisplayManager",
                                                     "/org/gnome/DisplayManager/LocalDisplayFactory",
                                                     "org.gnome.DisplayManager.LocalDisplayFactory");
}

static void
create_ck_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  priv->ck_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                              "org.freedesktop.ConsoleKit",
                                              "/org/freedesktop/ConsoleKit/Manager",
                                              "org.freedesktop.ConsoleKit.Manager");

  if (!priv->ck_proxy)
    {
      g_warning ("Failed to get ConsoleKit proxy.");
      return;
    }
}

static void
create_seat_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;

  priv->seat = get_seat (self);
  if (priv->seat == NULL)
    {
      return;
    }

  priv->seat_proxy = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                      "org.freedesktop.ConsoleKit",
                                                      priv->seat,
                                                      "org.freedesktop.ConsoleKit.Seat",
                                                      &error);

  if (!priv->seat_proxy)
    {
      if (error != NULL)
        {
          g_warning ("Failed to connect to the ConsoleKit seat: %s", error->message);
          g_error_free (error);
        }

      return;
    }

  dbus_g_proxy_add_signal (priv->seat_proxy,
                           "SessionAdded",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);
  dbus_g_proxy_add_signal (priv->seat_proxy,
                           "SessionRemoved",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->seat_proxy,
                               "SessionAdded",
                               G_CALLBACK (seat_proxy_session_added),
                               self,
                               NULL);
  dbus_g_proxy_connect_signal (priv->seat_proxy,
                               "SessionRemoved",
                               G_CALLBACK (seat_proxy_session_removed),
                               self,
                               NULL);
}

static void
create_cksession_proxy (UsersServiceDbus *service)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);

  priv->session_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                   "org.freedesktop.ConsoleKit",
                                                   priv->ssid,
                                                   "org.freedesktop.ConsoleKit.Session");

  if (!priv->session_proxy)
    {
      g_warning ("Failed to get ConsoleKit session proxy");
      return;
    }
}

static gchar *
get_seat (UsersServiceDbus *service)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GError     *error = NULL;
  gchar      *ssid = NULL;
  gchar      *seat;

  if (!dbus_g_proxy_call (priv->ck_proxy,
                          "GetCurrentSession",
                          &error,
                          G_TYPE_INVALID,
                          DBUS_TYPE_G_OBJECT_PATH,
                          &ssid,
                          G_TYPE_INVALID))
    {
      if (error)
        {
          g_debug ("Failed to call GetCurrentSession: %s", error->message);
          g_error_free (error);
        }

      if (ssid)
        g_free (ssid);

      return NULL;
    }

  priv->ssid = ssid;
  create_cksession_proxy (service);

  seat = get_seat_internal (service);

  return seat;
}

static gchar *
get_seat_internal (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;
  gchar *seat = NULL;

  if (!dbus_g_proxy_call (priv->session_proxy,
                          "GetSeatId",
                          &error,
                          G_TYPE_INVALID,
                          DBUS_TYPE_G_OBJECT_PATH, &seat,
                          G_TYPE_INVALID))
    {
      if (error)
        {
          g_debug ("Failed to call GetSeatId: %s", error->message);

          return NULL;
        }
    }

  return seat;
}

static gboolean
get_unix_user (UsersServiceDbus *service,
               const gchar      *session_id,
               uid_t            *uidp)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GError     *error = NULL;
  guint       uid;

  if (dbus_g_proxy_call (priv->session_proxy,
                         "GetUnixUser",
                         &error,
                         G_TYPE_INVALID,
                         G_TYPE_UINT, &uid,
                         G_TYPE_INVALID))
    {
      if (error)
        {
          g_warning ("Failed to call GetUnixUser: %s", error->message);
          g_error_free (error);
        }

      return FALSE;
    }

  if (uidp != NULL)
    {
      *uidp = (uid_t)uid;
    }

  return TRUE;
}

static gboolean
do_add_session (UsersServiceDbus *service,
                UserData         *user,
                const gchar      *ssid)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GError *error = NULL;
  gchar *seat = NULL;
  gchar *xdisplay = NULL;
  GList *l;

  seat = get_seat_internal (service);

  if (!seat || !priv->seat || strcmp (seat, priv->seat) != 0)
    return FALSE;

   if (!dbus_g_proxy_call (priv->session_proxy,
                          "GetX11Display",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &xdisplay,
                          G_TYPE_INVALID))
    {
      if (error)
        {
          g_debug ("Failed to call GetX11Display: %s", error->message);
          g_error_free (error);
        }

      return FALSE;
    }

  if (!xdisplay || xdisplay[0] == '\0')
    return FALSE;

  g_hash_table_insert (priv->sessions,
                       g_strdup (ssid),
                       g_strdup (user->user_name));

  l = g_list_find_custom (user->sessions, ssid, (GCompareFunc)g_strcmp0);
  if (l == NULL)
    {
      g_debug ("Adding session %s", ssid);

      user->sessions = g_list_prepend (user->sessions, g_strdup (ssid));

      if (user->menuitem != NULL) {
        dbusmenu_menuitem_property_set_bool(user->menuitem, USER_ITEM_PROP_LOGGED_IN, TRUE);
      }
    }
  else
    {
      g_debug ("User %s already has session %s", user->user_name, ssid);
    }

  return TRUE;
}

static void
add_sessions_for_user (UsersServiceDbus *self,
                       UserData         *user)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError          *error;
  GPtrArray       *sessions;
  int              i;

  error = NULL;
  if (!org_freedesktop_ConsoleKit_Manager_get_sessions_for_unix_user(priv->ck_proxy, user->uid, &sessions, &error))
    {
      g_debug ("Failed to call GetSessionsForUnixUser: %s", error->message);
      g_error_free (error);

      return;
    }

  for (i = 0; i < sessions->len; i++)
    {
      char *ssid;

      ssid = g_ptr_array_index (sessions, i);
      do_add_session (self, user, ssid);
    }

  g_ptr_array_foreach (sessions, (GFunc)g_free, NULL);
  g_ptr_array_free (sessions, TRUE);
}


static void
seat_proxy_session_added (DBusGProxy       *seat_proxy,
                          const gchar      *session_id,
                          UsersServiceDbus *service)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  uid_t          uid;
  gboolean       res;
  struct passwd *pwent;
  UserData      *user;

  if (!get_unix_user (service, session_id, &uid))
    {
      g_warning ("Failed to lookup user for session");
      return;
    }

  errno = 0;
  pwent = getpwuid (uid);
  if (!pwent)
    {
      g_warning ("Failed to lookup user id %d: %s", (int)uid, g_strerror (errno));
      return;
    }

  /* We need to special case guest here because it doesn't
     show up in the GDM user tables. */
  if (g_strcmp0("guest", pwent->pw_name) == 0) {
    if (priv->guest_item != NULL) {
      dbusmenu_menuitem_property_set_bool(priv->guest_item, USER_ITEM_PROP_LOGGED_IN, TRUE);
    }
	priv->guest_session_id = g_strdup(session_id);
    return;
  }

  user = g_hash_table_lookup (priv->users, pwent->pw_name);
  if (!user)
    {
      return;
    }

  res = do_add_session (service, user, session_id);
}

static void
seat_proxy_session_removed (DBusGProxy       *seat_proxy,
                            const gchar      *session_id,
                            UsersServiceDbus *service)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user;
  gchar    *username;
  GList    *l;

  username = g_hash_table_lookup (priv->sessions, session_id);
  if (!username) {
    if (g_strcmp0(session_id, priv->guest_session_id) == 0) {
      dbusmenu_menuitem_property_set_bool(priv->guest_item, USER_ITEM_PROP_LOGGED_IN, FALSE);
      g_free(priv->guest_session_id);
      priv->guest_session_id = NULL;
    }
    return;
  }

  user = g_hash_table_lookup (priv->users, username);
  if (!user)
    return;

  l = g_list_find_custom (user->sessions,
                          session_id,
                          (GCompareFunc)g_strcmp0);
  if (l)
    {
      g_debug ("Removing session %s", session_id);

      g_free (l->data);
      user->sessions = g_list_delete_link (user->sessions, l);
      if (user->menuitem != NULL && user->sessions == NULL) {
        dbusmenu_menuitem_property_set_bool(user->menuitem, USER_ITEM_PROP_LOGGED_IN, FALSE);
      }
    }
  else
    {
      g_debug ("Session not found: %s", session_id);
    }
}

static void
sync_users (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  if (g_hash_table_size (priv->users) > 0)
    {
      return;
    }

  if (priv->count > MINIMUM_USERS && priv->count < MAXIMUM_USERS)
    {
      GArray *uids = NULL;
      GPtrArray *users_info = NULL;
      GError *error = NULL;
      gint i;

      uids = g_array_new (FALSE, FALSE, sizeof (gint64));

      if (!org_gnome_DisplayManager_UserManager_get_user_list (priv->gdm_proxy,
                                                               &uids,
                                                               &error))
        {
          g_warning ("failed to retrieve user list: %s", error->message);
          g_error_free (error);

          return;
        }

      users_info = g_ptr_array_new ();

      if (!org_gnome_DisplayManager_UserManager_get_users_info (priv->gdm_proxy,
                                                                uids,
                                                                &users_info,
                                                                &error))
        {
          g_warning ("failed to retrieve user info: %s", error->message);
          g_error_free (error);

          return;
        }

      for (i = 0; i < users_info->len; i++)
        {
          GValueArray *values;
          UserData *user;

          values = g_ptr_array_index (users_info, i);

          user = g_new0 (UserData, 1);

          user->uid         = g_value_get_int64  (g_value_array_get_nth (values, 0));
          user->user_name   = g_strdup (g_value_get_string (g_value_array_get_nth (values, 1)));
          user->real_name   = g_strdup (g_value_get_string (g_value_array_get_nth (values, 2)));
          user->shell       = g_strdup (g_value_get_string (g_value_array_get_nth (values, 3)));
          user->login_count = g_value_get_int    (g_value_array_get_nth (values, 4));
          user->icon_url    = g_strdup (g_value_get_string (g_value_array_get_nth (values, 5)));
          user->real_name_conflict = FALSE;
		  user->menuitem    = NULL;

          g_hash_table_insert (priv->users,
                               g_strdup (user->user_name),
                               user);

          add_sessions_for_user (self, user);
        }
    }
}

static void
users_loaded (DBusGProxy *proxy,
              gpointer    user_data)
{
  UsersServiceDbus        *service;
  UsersServiceDbusPrivate *priv;
  GError                  *error = NULL;
  gint                     count;

  g_return_if_fail (proxy != NULL);

  service = (UsersServiceDbus *)user_data;
  priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);

  if (!org_gnome_DisplayManager_UserManager_count_users (proxy,
                                                         &count,
                                                         &error))
    {
      g_warning ("failed to retrieve user count: %s", error->message);
      g_error_free (error);

      return;
    }

  priv->count = count;

  sync_users (service);
}

static gboolean
session_is_login_window (UsersServiceDbus *self,
                         const char       *ssid)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  DBusGProxy *proxy = NULL;
  GError     *error = NULL;
  char       *type = NULL;

  if (!(proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                           "org.freedesktop.ConsoleKit",
                                           ssid,
                                           "org.freedesktop.ConsoleKit.Session")))
    {
      g_warning ("Failed to get ConsoleKit proxy");

      return FALSE;
    }

  if (!dbus_g_proxy_call (proxy,
                          "GetSessionType",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_STRING, &type,
                          G_TYPE_INVALID))
    {
      g_warning ("Can't call GetSessionType: %s", error->message);
      g_error_free (error);

      if (proxy)
        g_object_unref (proxy);

      return FALSE;
    }

  if (proxy)
    g_object_unref (proxy);

  return (type && type[0] != '\0' && strcmp (type, "LoginWindow") == 0);
}

static char *
get_login_session (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  gboolean    can_activate;
  GError     *error = NULL;
  GPtrArray  *sessions = NULL;
  char       *ssid = NULL;
  int         i;

  if (!priv->seat || priv->seat[0] == '\0')
    {
      return NULL;
    }

  if (!dbus_g_proxy_call (priv->seat_proxy,
                          "CanActivateSessions",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &can_activate,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to call CanActivateSessions: %s", error->message);
      g_error_free (error);

      return NULL;
    }

  if (!can_activate)
    {
      return NULL;
    }

  error = NULL;
  if (!dbus_g_proxy_call (priv->seat_proxy,
                          "GetSessions",
                          &error,
                          G_TYPE_INVALID,
                          dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &sessions,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to call GetSessions: %s", error->message);
      g_error_free (error);

      return NULL;
  }

  for (i = 0; i < sessions->len; i++)
    {
      char *s = g_ptr_array_index (sessions, i);

      if (session_is_login_window (self, s))
        {
          ssid = g_strdup (s);
          break;
        }
    }

  g_ptr_array_foreach (sessions, (GFunc)g_free, NULL);
  g_ptr_array_free (sessions, TRUE);

  return ssid;
}

static gboolean
activate_user_session (UsersServiceDbus *self,
                       const char       *seat,
                       const char       *ssid)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  DBusMessage *message = NULL;
  DBusMessage *reply   = NULL;
  DBusError    error;

  if (!(message = dbus_message_new_method_call ("org.freedesktop.ConsoleKit",
                                                seat,
                                                "org.freedesktop.ConsoleKit.Seat",
                                                "ActivateSession")))
    {
      return FALSE;
    }

  if (!dbus_message_append_args (message,
                                 DBUS_TYPE_OBJECT_PATH, &ssid,
                                 DBUS_TYPE_INVALID))
    {
      return FALSE;
    }

  dbus_error_init (&error);
  if (!(reply = dbus_connection_send_with_reply_and_block (dbus_g_connection_get_connection (priv->system_bus),
                                                           message,
                                                           -1,
                                                           &error)))
    {
      if (dbus_error_is_set (&error))
        {
          g_warning ("Can't activate session: %s", error.message);
          dbus_error_free (&error);

          return FALSE;
        }
    }

  if (message)
    {
      dbus_message_unref (message);
    }

  if (reply)
    {
      dbus_message_unref (reply);
    }

  return TRUE;
}

gboolean
start_new_user_session (UsersServiceDbus *self,
                        UserData         *user)
{
  g_return_val_if_fail (IS_USERS_SERVICE_DBUS (self), FALSE);

  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError   *error = NULL;
  char     *ssid;

  ssid = get_login_session (self);
  if (ssid)
    {
      if (!activate_user_session (self, priv->seat, ssid))
        {
          return FALSE;
        }
    }

  if (!g_spawn_command_line_async ("gdmflexiserver -s", &error))
    {
      g_warning ("Failed to start new login session: %s", error->message);
      g_error_free (error);

      return FALSE;
    }

  return TRUE;
}

static void
user_added (DBusGProxy *proxy,
            gint64      uid,
            gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user = g_new0 (UserData, 1);
  GError *error = NULL;

  priv->count++;

  if (priv->count < MAXIMUM_USERS)
    {
      if ((priv->count - g_hash_table_size (priv->users)) > 1)
        {
          sync_users (service);
        }
      else
        {
          if (!org_gnome_DisplayManager_UserManager_get_user_info (proxy,
                                                                   uid,
                                                                   &user->user_name,
                                                                   &user->real_name,
                                                                   &user->shell,
                                                                   &user->login_count,
                                                                   &user->icon_url,
                                                                   &error))
            {
              g_warning ("unable to retrieve user info: %s", error->message);
              g_error_free (error);

              g_free (user);

              return;
            }

          user->uid = uid;

          g_hash_table_insert (priv->users,
                               g_strdup (user->user_name),
                               user);

          g_signal_emit (G_OBJECT (service), signals[USER_ADDED], 0, user, TRUE);
        }
    }
}

static gboolean
compare_users_by_uid (gpointer key,
                      gpointer value,
                      gpointer user_data)
{
  return (GPOINTER_TO_UINT (value) == GPOINTER_TO_UINT (user_data));
}

static void
user_removed (DBusGProxy *proxy,
              gint64      uid,
              gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user;
  gint size;

  size = g_hash_table_size (priv->users);

  priv->count--;

  if (size == 0 || (size - priv->count) > 1)
    {
      sync_users (service);
    }
  else
    {
      user = g_hash_table_find (priv->users,
                                compare_users_by_uid,
                                GUINT_TO_POINTER ((gint)uid));

      if (user != NULL)
        {
          g_hash_table_remove (priv->users,
                               user->user_name);
        }
    }
}

static void
user_updated (DBusGProxy *proxy,
              guint       uid,
              gpointer    user_data)
{
#if 0
  // XXX - TODO
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user;

  user = g_hash_table_find (priv->users,
                            compare_users_by_uid,
                            GUINT_TO_POINTER (uid));
#endif
}

gint
users_service_dbus_get_user_count (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  return priv->count;
}

GList *
users_service_dbus_get_user_list (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  return g_hash_table_get_values (priv->users);
}

/* Activates the guest account if it can. */
gboolean
users_service_dbus_activate_guest_session (UsersServiceDbus *self)
{
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	return org_gnome_DisplayManager_LocalDisplayFactory_switch_to_user(priv->gdm_local_proxy, "guest", NULL, NULL);
}

/* Activates a specific user */
gboolean
users_service_dbus_activate_user_session (UsersServiceDbus *self,
                                          UserData         *user)
{
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	return org_gnome_DisplayManager_LocalDisplayFactory_switch_to_user(priv->gdm_local_proxy, user->user_name, NULL, NULL);
}

gboolean
users_service_dbus_can_activate_session (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  gboolean can_activate = FALSE;
  GError *error = NULL;

  if (!priv->seat_proxy)
    {
      create_seat_proxy (self);
    }

  if (!priv->seat || priv->seat[0] == '\0')
    {
      return FALSE;
    }

  if (!dbus_g_proxy_call (priv->seat_proxy,
                          "CanActivateSessions",
                          &error,
                          G_TYPE_INVALID,
                          G_TYPE_BOOLEAN, &can_activate,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to determine if seat can activate sessions: %s", error->message);
      g_error_free (error);

      return FALSE;
    }

  return can_activate;
}

/* Sets the menu item that represents the guest account */
void
users_service_dbus_set_guest_item (UsersServiceDbus * self, DbusmenuMenuitem * mi)
{
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	priv->guest_item = mi;

	if (priv->guest_session_id != NULL) {
      dbusmenu_menuitem_property_set_bool(priv->guest_item, USER_ITEM_PROP_LOGGED_IN, TRUE);
    }

	return;
}
