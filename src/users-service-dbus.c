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
#include "users-service-dbus.h"
#include "users-service-client.h"
#include "users-service-marshal.h"

static void      users_service_dbus_class_init         (UsersServiceDbusClass *klass);
static void      users_service_dbus_init               (UsersServiceDbus  *self);
static void      users_service_dbus_dispose            (GObject           *object);
static void      users_service_dbus_finalize           (GObject           *object);
static gboolean _users_service_server_count_users      (UsersServiceDbus  *service,
                                                        gint              *count,
                                                        GError           **error);
static gboolean _users_service_server_get_users_loaded (UsersServiceDbus  *self,
                                                        gboolean          *is_loaded,
                                                        GError           **error);
static gboolean _users_service_server_get_user_list    (UsersServiceDbus  *service,
                                                        GArray           **list,
                                                        GError           **error);
static gboolean _users_service_server_get_user_info    (UsersServiceDbus  *service,
                                                        const gint64       uid,
                                                        gchar            **username,
                                                        gchar            **real_name,
                                                        gchar            **shell,
                                                        gint              *login_count,
                                                        gchar            **icon_url,
                                                        GError           **error);
static gboolean _users_service_server_get_users_info   (UsersServiceDbus  *service,
                                                        const GArray      *uids,
                                                        GPtrArray        **user_info,
                                                        GError           **error);
static void     create_session_proxy                   (UsersServiceDbus  *self);
static void     create_system_proxy                    (UsersServiceDbus  *self);
static void     create_gdm_proxy                       (UsersServiceDbus  *self);
static void     create_seat_proxy                      (UsersServiceDbus  *self);
static void     create_ck_proxy                        (UsersServiceDbus *self);
static void     create_cksession_proxy                 (UsersServiceDbus *self);
static gchar   *get_seat                               (UsersServiceDbus *service);
static void     users_loaded                           (DBusGProxy        *proxy,
                                                        gpointer           user_data);
static void     user_added                             (DBusGProxy        *proxy,
                                                        guint              uid,
                                                        gpointer           user_data);
static void     user_removed                           (DBusGProxy        *proxy,
                                                        guint              uid,
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

#include "users-service-server.h"

/* Private */
typedef struct _UsersServiceDbusPrivate UsersServiceDbusPrivate;

struct _UsersServiceDbusPrivate
{
  GHashTable *users;
  gint        count;
  gchar      *seat;
  gchar      *ssid;

  DBusGConnection *session_bus;
  DBusGConnection *system_bus;

  DBusGProxy *dbus_proxy_session;
  DBusGProxy *dbus_proxy_system;
  DBusGProxy *gdm_proxy;
  DBusGProxy *ck_proxy;
  DBusGProxy *seat_proxy;
  DBusGProxy *session_proxy;

  GHashTable *exclusions;
  GHashTable *sessions;
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

  dbus_g_object_type_install_info (USERS_SERVICE_DBUS_TYPE, &dbus_glib__users_service_server_object_info);
}

static void
users_service_dbus_init (UsersServiceDbus *self)
{
  GError *error = NULL;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  g_print ("INIT\n");

  priv->users = NULL;
  priv->count = 0;

  /* Get the buses */
  priv->session_bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (error != NULL)
    {
      g_error ("Unable to get session bus: %s", error->message);
      g_error_free (error);

      return;
    }

  priv->system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (error != NULL)
    {
      g_error("Unable to get system bus: %s", error->message);
      g_error_free(error);

      return;
    }

  priv->exclusions = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            NULL);

  priv->users = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       NULL);

  dbus_g_connection_register_g_object (priv->session_bus,
                                       INDICATOR_USERS_SERVICE_DBUS_OBJECT,
                                       G_OBJECT (self));

  dbus_g_object_register_marshaller (_users_service_marshal_VOID__INT64,
                                     G_TYPE_NONE,
                                     G_TYPE_INT64,
                                     G_TYPE_INVALID);

  create_session_proxy (self);
  create_system_proxy (self);
  create_gdm_proxy (self);
  create_ck_proxy (self);
  create_seat_proxy (self);

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
  G_OBJECT_CLASS (users_service_dbus_parent_class)->finalize (object);
}

static void
create_session_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;

  priv->dbus_proxy_session = dbus_g_proxy_new_for_name_owner (priv->session_bus,
                                                              DBUS_SERVICE_DBUS,
                                                              DBUS_PATH_DBUS,
                                                              DBUS_INTERFACE_DBUS,
                                                              &error);
  if (error != NULL)
    {
      g_error ("Unable to get dbus proxy on session bus: %s", error->message);
      g_error_free (error);

      return;
    }
}

static void
create_system_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;

  priv->dbus_proxy_system = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                             DBUS_SERVICE_DBUS,
                                                             DBUS_PATH_DBUS,
                                                             DBUS_INTERFACE_DBUS,
                                                             &error);

  if (!priv->dbus_proxy_system)
    {
      if (error != NULL)
        {
          g_error ("Unable to get dbus proxy on system bus: %s", error->message);
          g_error_free (error);

          return;
        }
    }
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
          g_error ("Unable to get DisplayManager proxy on system bus: %s", error->message);
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
      g_warning ("Failed to setup ConsoleKit proxy.");
      return;
    }
}

static void
create_seat_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;

  g_print ("create_seat_proxy ()\n");

  priv->seat = get_seat (self);
  if (priv->seat == NULL)
    {
      g_print (" ** no priv->seat, returning\n");
      return;
    }

  g_print ("setup priv->seat_proxy...\n");
  priv->seat_proxy = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                      "org.freedesktop.ConsoleKit",
                                                      priv->seat,
                                                      "org.freedesktop.ConsoleKit.Seat",
                                                      &error);

  if (!priv->seat_proxy)
    {
      g_print ("some error...\n");

      if (error != NULL)
        {
          g_warning ("Failed to connect to the ConsoleKit seat: %s", error->message);
          g_error_free (error);
        }

      return;
    }

  g_print ("... SessionAdded\n");
  dbus_g_proxy_add_signal (priv->seat_proxy,
                           "SessionAdded",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);
  g_print ("... SessionRemoved\n");
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
      g_warning ("Failed to connect to ConsoleKit session");
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
          g_debug ("Failed to get session: %s", error->message);
          g_error_free (error);
        }

      if (ssid)
        g_free (ssid);

      return NULL;
    }

  g_print ("get_seat(): ssid is %s\n", ssid);

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
  gchar *seat;

  if (!dbus_g_proxy_call (priv->session_proxy,
                          "GetSeatId",
                          &error,
                          G_TYPE_INVALID,
                          DBUS_TYPE_G_OBJECT_PATH, &seat,
                          G_TYPE_INVALID))
    {
      if (error)
        {
          g_debug ("Failed to identify the current seat: %s", error->message);
        }
    }

  g_print ("get_seat_internal: %s\n", seat);

  return seat;
}

static gboolean
get_unix_user (UsersServiceDbus *service,
               const gchar      *session_id,
               uid_t            *uidp)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GError     *error;
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
          g_warning ("Failed to get the session: %s", error->message);
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

static gint
session_compare (const gchar *a,
                 const gchar *b)
{
  if (a == NULL)
    return 1;
  else if (b == NULL)
    return -1;

  return strcmp (a, b);
}

static gchar *
get_session_for_user (UsersServiceDbus *service,
                      UserData         *user)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  gboolean    can_activate;
  GError     *error = NULL;
  GList      *l;

  if (!priv->seat_proxy)
    create_seat_proxy (service);

  if (priv->seat == NULL || priv->seat[0] == '\0')
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
      g_warning ("Failed to determine if seat can activate sessions: %s", error->message);
      g_error_free (error);

      return NULL;
    }

  if (!can_activate) {
    g_warning ("Can't activate sessions");
    return NULL;
  }

  if (!user->sessions || g_list_length (user->sessions) == 0)
    {
      g_print (" *** no user sessions\n");
      return NULL;
    }

  for (l = user->sessions; l != NULL; l = l->next)
    {
      const char *ssid;

      ssid = l->data;

      /* FIXME: better way to choose? */
      if (ssid != NULL)
        {
          g_print ("  ==== ssid is %s\n", ssid);
          return g_strdup (ssid);
        }
    }

  return NULL;
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

  g_print ("DO_ADD_SESSION (ssid is %s)\n", ssid);

  seat = get_seat_internal (service);
  g_print (" *** seat is %s\n", seat);
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
          g_debug ("Failed to get X11 display: %s", error->message);
          g_error_free (error);
        }

      return FALSE;
    }

  if (!xdisplay || xdisplay[0] == '\0')
    return FALSE;

  g_print ("xdisplay is %s\n", xdisplay);

  if (g_hash_table_lookup (priv->exclusions, user->user_name))
    return FALSE;

  g_hash_table_insert (priv->sessions,
                       g_strdup (ssid),
                       g_strdup (user->user_name));

  l = g_list_find_custom (user->sessions, ssid, (GCompareFunc)session_compare);
  if (l == NULL)
    {
      g_debug ("Adding session %s", ssid);

      user->sessions = g_list_prepend (user->sessions, g_strdup (ssid));
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
  if (!dbus_g_proxy_call (priv->ck_proxy,
                          "GetSessionsForUnixUser",
                          &error,
                          G_TYPE_UINT, user->uid,
                          G_TYPE_INVALID,
                          dbus_g_type_get_collection ("GPtrArray", DBUS_TYPE_G_OBJECT_PATH),
                          &sessions,
                          G_TYPE_INVALID))
    {
      g_debug ("Failed to find sessions for user: %s", error->message);
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

  if (g_hash_table_lookup (priv->exclusions, pwent->pw_name))
    {
      g_debug ("Excluding user %s", pwent->pw_name);
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
  if (!username)
    return;

  user = g_hash_table_lookup (priv->users, username);
  if (!user)
    return;

  //_user_remove_session (user, session_id);

  l = g_list_find_custom (user->sessions,
                          session_id,
                          (GCompareFunc)session_compare);
  if (l)
    {
      g_debug ("Removing session %s", session_id);

      g_free (l->data);
      user->sessions = g_list_delete_link (user->sessions, l);
    }
  else
    {
      g_debug ("Session not found: %s", session_id);
    }
}

static void
users_loaded (DBusGProxy *proxy,
              gpointer    user_data)
{
  UsersServiceDbus        *service;
  UsersServiceDbusPrivate *priv;
  GError                  *error = NULL;
  GArray                  *uids = NULL;
  GPtrArray               *users_info = NULL;
  gint                     count;
  int                      i;

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

  uids = g_array_new (FALSE, FALSE, sizeof (gint64));

  if (!org_gnome_DisplayManager_UserManager_get_user_list (proxy,
                                                           &uids,
                                                           &error))
    {
      g_warning ("failed to retrieve user list: %s", error->message);
      g_error_free (error);

      return;
    }

  users_info = g_ptr_array_new ();

  if (!org_gnome_DisplayManager_UserManager_get_users_info (proxy,
                                                            uids,
                                                            &users_info,
                                                            &error))
    {
      g_warning ("failed to retrieve user info: %s", error->message);
      g_error_free (error);

      return;
    }

  g_print ("***************\n");
  g_print ("*** users_info->len is %d\n", users_info->len);

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

      g_print ("*** username is %s\n", user->user_name);

      g_hash_table_insert (priv->users,
                           g_strdup (user->user_name),
                           user);

      g_print ("  ... adding sessions for %s\n", user->user_name);
      add_sessions_for_user (service, user);
      g_print ("***************\n");
    }
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

gboolean
users_service_dbus_activate_user_session (UsersServiceDbus *self,
                                          UserData         *user)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  DBusMessage *message = NULL;
  DBusMessage *reply = NULL;
  DBusError error;
  gchar *ssid;

  dbus_error_init (&error);

  if (!priv->seat)
    priv->seat = get_seat (self);

  ssid = get_session_for_user (self, user);

  g_print ("users_service_dbus_activate_user_session...\n");
  g_print ("seat is %s\n", priv->seat);
  g_print ("ssid is %s\n", ssid);

  if (!(message = dbus_message_new_method_call ("org.freedesktop.ConsoleKit",
                                                priv->seat,
                                                "org.freedesktop.ConsoleKit.Seat",
                                                "ActivateSession")))
    {
      g_warning ("failed to create new message");
      return FALSE;
    }

  if (!dbus_message_append_args (message,
                                 DBUS_TYPE_OBJECT_PATH,
                                 &ssid,
                                 DBUS_TYPE_INVALID))
    {
      g_warning ("failed to append args");
      return FALSE;
    }

  if (!(reply = dbus_connection_send_with_reply_and_block (dbus_g_connection_get_connection (priv->system_bus),
                                                           message,
                                                           -1,
                                                           &error)))
    {
      g_warning ("send_with_reply_and_block failed");

      if (dbus_error_is_set (&error))
        {
          g_warning ("Unable to activate session: %s", error.message);
          dbus_error_free (&error);

          return FALSE;
        }
    }

  g_print ("freeing shit up..\n");

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

static void
user_added (DBusGProxy *proxy,
            guint       uid,
            gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user = g_new0 (UserData, 1);
  GError *error = NULL;

  priv->count++;

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

static gboolean
compare_users_by_uid (gpointer key,
                      gpointer value,
                      gpointer user_data)
{
  return (GPOINTER_TO_UINT (value) == GPOINTER_TO_UINT (user_data));
}

static void
user_removed (DBusGProxy *proxy,
              guint       uid,
              gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user;

  user = g_hash_table_find (priv->users,
                            compare_users_by_uid,
                            GUINT_TO_POINTER (uid));

  g_hash_table_remove (priv->users,
                       user->user_name);

  priv->count--;
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

static gboolean
_users_service_server_count_users (UsersServiceDbus  *service,
                                   gint              *uids,
                                   GError           **error)
{
  return TRUE;
}

static gboolean
_users_service_server_get_user_list  (UsersServiceDbus  *service,
                                      GArray           **list,
                                      GError           **error)
{
  return TRUE;
}

static gboolean
_users_service_server_get_users_loaded (UsersServiceDbus  *self,
                                        gboolean          *is_loaded,
                                        GError           **error)
{
  return TRUE;
}

static gboolean
_users_service_server_get_user_info  (UsersServiceDbus  *service,
                                      const gint64       uid,
                                      gchar            **username,
                                      gchar            **real_name,
                                      gchar            **shell,
                                      gint              *login_count,
                                      char             **icon_url,
                                      GError           **error)
{
  return TRUE;
}

static gboolean
_users_service_server_get_users_info (UsersServiceDbus  *service,
                                      const GArray      *uids,
                                      GPtrArray        **user_info,
                                      GError           **error)
{
  return TRUE;
}
