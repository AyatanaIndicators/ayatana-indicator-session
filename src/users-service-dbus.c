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

#include <dbus/dbus-glib.h>

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

#include "users-service-server.h"

/* Private */
typedef struct _UsersServiceDbusPrivate UsersServiceDbusPrivate;

struct _UsersServiceDbusPrivate
{
  GList *users;
  gint   count;

  DBusGConnection *session_bus;
  DBusGConnection *system_bus;
  DBusGProxy *dbus_proxy_session;
  DBusGProxy *dbus_proxy_system;
  DBusGProxy *gdm_proxy;
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

  priv->users = NULL;
  priv->count = 0;

  /* Get the buses */
  priv->session_bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (error != NULL) {
    g_error ("Unable to get session bus: %s", error->message);
    g_error_free (error);

    return;
  }

  priv->system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (error != NULL) {
    g_error("Unable to get system bus: %s", error->message);
    g_error_free(error);

    return;
  }

  dbus_g_connection_register_g_object (priv->session_bus,
                                       INDICATOR_USERS_SERVICE_DBUS_OBJECT,
                                       G_OBJECT (self));

  /* Set up the DBUS service proxies */
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

  priv->dbus_proxy_system = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                             DBUS_SERVICE_DBUS,
                                                             DBUS_PATH_DBUS,
                                                             DBUS_INTERFACE_DBUS,
                                                             &error);
  if (error != NULL)
    {
      g_error ("Unable to get dbus proxy on system bus: %s", error->message);
      g_error_free (error);

      return;
    }

  priv->gdm_proxy = dbus_g_proxy_new_for_name_owner (priv->system_bus,
                                                     "org.gnome.DisplayManager",
                                                     "/org/gnome/DisplayManager/UserManager",
                                                     "org.gnome.DisplayManager.UserManager",
                                                     &error);

  if (error != NULL)
    {
      g_error ("Unable to get DisplayManager proxy on system bus: %s", error->message);
      g_error_free (error);
    }

  dbus_g_object_register_marshaller (_users_service_marshal_VOID__INT64,
                                     G_TYPE_NONE,
                                     G_TYPE_INT64,
                                     G_TYPE_INVALID);

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

  for (i = 0; i < users_info->len; i++)
    {
      GValueArray *values;
      UserData *data;

      values = g_ptr_array_index (users_info, i);

      data = g_new0 (UserData, 1);

      data->uid         = g_value_get_int64  (g_value_array_get_nth (values, 0));
      data->user_name   = g_strdup (g_value_get_string (g_value_array_get_nth (values, 1)));
      data->real_name   = g_strdup (g_value_get_string (g_value_array_get_nth (values, 2)));
      data->shell       = g_strdup (g_value_get_string (g_value_array_get_nth (values, 3)));
      data->login_count = g_value_get_int    (g_value_array_get_nth (values, 4));
      data->icon_url    = g_strdup (g_value_get_string (g_value_array_get_nth (values, 5)));

      priv->users = g_list_prepend (priv->users, data);
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

  g_print ("users_service_dbus_get_user_list()\n");

  if (!priv->users)
    g_print ("users_service_dbus_get_user_list(): priv->users is NULL\n");

  return priv->users;
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

  priv->users = g_list_prepend (priv->users, user);

  g_signal_emit (G_OBJECT (service), signals[USER_ADDED], 0, user, TRUE);
}

static void
user_removed (DBusGProxy *proxy,
              guint       uid,
              gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GList *l;

  priv->count--;

  for (l = priv->users; l != NULL; l = g_list_next (l))
    {
      UserData *user = (UserData *)l->data;

      if (user->uid == uid)
        {
          priv->users = g_list_remove (priv->users, l->data);
          g_signal_emit (G_OBJECT (service), signals[USER_REMOVED], 0, user, TRUE);
        }
    }
}

static void
user_updated (DBusGProxy *proxy,
              guint       uid,
              gpointer    user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  GList *l;

  for (l = priv->users; l != NULL; l = g_list_next (l))
    {
      UserData *user = (UserData *)l->data;

      if (user->uid == uid)
        {
          // XXX - update user data
        }
    }
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
