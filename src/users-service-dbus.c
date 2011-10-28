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
#include "display-manager-client.h"
#include "users-service-dbus.h"
#include "accounts-service-client.h"
#include "consolekit-manager-client.h"
#include "consolekit-session-client.h"
#include "consolekit-seat-client.h"

#define CK_ADDR             "org.freedesktop.ConsoleKit"
#define CK_SESSION_IFACE    "org.freedesktop.ConsoleKit.Session"


static void     users_service_dbus_class_init         (UsersServiceDbusClass *klass);
static void     users_service_dbus_init               (UsersServiceDbus  *self);
static void     users_service_dbus_dispose            (GObject           *object);
static void     users_service_dbus_finalize           (GObject           *object);
static void     create_display_manager_proxy          (UsersServiceDbus  *self);
static void     create_accounts_service_proxy          (UsersServiceDbus  *self);
static void     create_seat_proxy                      (UsersServiceDbus  *self);
static void     create_ck_proxy                        (UsersServiceDbus *self);
static void     create_cksession_proxy                 (UsersServiceDbus *self);
static gchar   *get_seat                               (UsersServiceDbus *service);
static void     user_added                             (DBusGProxy        *proxy,
                                                        const gchar       *user_id,
                                                        gpointer           user_data);
static void     user_deleted                           (DBusGProxy        *proxy,
                                                        const gchar       *user_id,
                                                        gpointer           user_data);
static void     user_changed                            (DBusGProxy       *proxy,
                                                         gpointer          user_data);                                                        
static void     seat_proxy_session_added               (DBusGProxy        *seat_proxy,
                                                        const gchar       *session_id,
                                                        UsersServiceDbus  *service);
static void     seat_proxy_session_removed             (DBusGProxy        *seat_proxy,
                                                        const gchar       *session_id,
                                                        UsersServiceDbus  *service);
static void     sync_users                             (UsersServiceDbus *self);
static gboolean do_add_session                         (UsersServiceDbus  *service,
                                                        UserData          *user,
                                                        const gchar       *ssid);
static gchar *  get_seat_internal                      (DBusGProxy        *proxy);

/* Private */
typedef struct _UsersServiceDbusPrivate UsersServiceDbusPrivate;

struct _UsersServiceDbusPrivate
{
  GHashTable *users;
  gint        count;
  gchar      *seat;
  gchar      *ssid;

  DBusGConnection *system_bus;

  DBusGProxy *accounts_service_proxy;
  DBusGProxy *display_manager_proxy;
  DBusGProxy *display_manager_props_proxy;
  DBusGProxy *ck_proxy;
  DBusGProxy *seat_proxy;
  DBusGProxy *session_proxy;

  GHashTable *exclusions;
  GHashTable *sessions;

  DbusmenuMenuitem * guest_item;
  gchar * guest_session_id;
  gboolean guest_session_enabled;
};

#define USERS_SERVICE_DBUS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), USERS_SERVICE_DBUS_TYPE, UsersServiceDbusPrivate))

/* Signals */
enum {
  USER_ADDED,
  USER_DELETED,
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

  signals[USER_ADDED] = g_signal_new ("user-added",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_LAST,
                                      G_STRUCT_OFFSET (UsersServiceDbusClass, user_added),
                                      NULL, NULL,
                                      g_cclosure_marshal_VOID__STRING,
                                      G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[USER_DELETED] = g_signal_new ("user-deleted",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (UsersServiceDbusClass, user_deleted),
                                        NULL, NULL,
                                        g_cclosure_marshal_VOID__STRING,
                                        G_TYPE_NONE, 1, G_TYPE_STRING);
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
  
  priv->guest_session_enabled = FALSE;

  /* Get the system bus */
  priv->system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (error != NULL)
    {
      g_error ("Unable to get system bus");
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

  create_ck_proxy (self);
  create_seat_proxy (self);
  create_display_manager_proxy (self);
  create_accounts_service_proxy (self);
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
create_display_manager_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GError *error = NULL;
  const gchar *seat = NULL;
  
  seat = g_getenv ("XDG_SEAT_PATH");
  g_debug ("CREATING DM PROXIES WITH %s", seat);
  priv->display_manager_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                           "org.freedesktop.DisplayManager",
                                                           seat,
                                                           "org.freedesktop.DisplayManager.Seat");

  priv->display_manager_props_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                                 "org.freedesktop.DisplayManager",
                                                                 seat,
                                                                 "org.freedesktop.DBus.Properties");


  if (!priv->display_manager_proxy)
    {
      g_warning ("Failed to get DisplayManager seat proxy.");
      return;
    }
  if (!priv->display_manager_props_proxy)
    {
      g_warning ("Failed to get DisplayManager Properties seat proxy.");
      return;
    }
  
  GValue has_guest_session = {0};
  g_value_init (&has_guest_session, G_TYPE_BOOLEAN);
  if (!dbus_g_proxy_call (priv->display_manager_props_proxy,
                          "Get",
                          &error,
                          G_TYPE_STRING,
                          "org.freedesktop.DisplayManager.Seat",
                          G_TYPE_STRING,
                          "HasGuestAccount",
                          G_TYPE_INVALID,
                          G_TYPE_VALUE,
                          &has_guest_session,
                          G_TYPE_INVALID))
    {
      g_warning ("Failed to get the HasGuestSession property from the DisplayManager Properties seat proxy. error: %s", error->message);
      g_error_free (error);      
      return;      
    }
    g_debug ("Does seat have a guest account = %i", g_value_get_boolean (&has_guest_session));
    priv->guest_session_enabled = g_value_get_boolean (&has_guest_session);                                                        
}

static void
create_accounts_service_proxy (UsersServiceDbus *self)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
  GPtrArray *users = g_ptr_array_new ();
  GError *error = NULL;

  priv->accounts_service_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                            "org.freedesktop.Accounts",
                                                            "/org/freedesktop/Accounts",
                                                            "org.freedesktop.Accounts");

  dbus_g_proxy_add_signal (priv->accounts_service_proxy,
                           "UserAdded",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->accounts_service_proxy,
                           "UserChanged",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);

  dbus_g_proxy_add_signal (priv->accounts_service_proxy,
                           "UserDeleted",
                           DBUS_TYPE_G_OBJECT_PATH,
                           G_TYPE_INVALID);

  dbus_g_proxy_connect_signal (priv->accounts_service_proxy,
                               "UserAdded",
                               G_CALLBACK (user_added),
                               self,
                               NULL);

  dbus_g_proxy_connect_signal (priv->accounts_service_proxy,
                               "UserDeleted",
                               G_CALLBACK (user_deleted),
                               self,
                               NULL);

  if (!org_freedesktop_Accounts_list_cached_users (priv->accounts_service_proxy,
                                                   &users,
                                                   &error))
    {
      g_warning ("failed to retrieve user count: %s", error->message);
      g_error_free (error);

      return;
    }

  priv->count = users->len;
  g_ptr_array_free (users, TRUE);
  sync_users (self);
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

/* Get the initial sessions when starting up */
static void 
get_cksessions_cb (DBusGProxy *proxy, GPtrArray * sessions, GError * error, gpointer userdata)
{
	if (error != NULL) {
		g_warning("Unable to get initial sessions: %s", error->message);
		return;
	}

	/* If there's no error we should at least get an
	   array of zero entries */
	g_return_if_fail(sessions != NULL);
	g_debug("Got %d initial sessions", sessions->len);

	int i;
	for (i = 0; i < sessions->len; i++) {
		seat_proxy_session_added(proxy, g_ptr_array_index(sessions, i), USERS_SERVICE_DBUS(userdata));
	}

	return;
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

  org_freedesktop_ConsoleKit_Seat_get_sessions_async (priv->seat_proxy, get_cksessions_cb, self);

  return;
}

static void
create_cksession_proxy (UsersServiceDbus *service)
{
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);

  priv->session_proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                                   CK_ADDR,
                                                   priv->ssid,
                                                   CK_SESSION_IFACE);

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

  seat = get_seat_internal (priv->session_proxy);

  return seat;
}

static gchar *
get_seat_internal (DBusGProxy *proxy)
{
  GError *error = NULL;
  gchar *seat = NULL;

  if (!org_freedesktop_ConsoleKit_Session_get_seat_id (proxy, &seat, &error))
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
  DBusGProxy *session_proxy;

  g_debug("Building session proxy for: %s", session_id);
  session_proxy = dbus_g_proxy_new_for_name_owner(priv->system_bus,
                                                  CK_ADDR,
                                                  session_id,
                                                  CK_SESSION_IFACE,
                                                  &error);

  if (error != NULL) {
    g_warning("Unable to get CK Session proxy: %s", error->message);
    g_error_free(error);
    return FALSE;
  }

  if (!org_freedesktop_ConsoleKit_Session_get_unix_user(session_proxy, &uid, &error))
    {
      if (error)
        {
          g_warning ("Failed to call GetUnixUser: %s", error->message);
          g_error_free (error);
        }

      g_object_unref(session_proxy);
      return FALSE;
    }

  if (uidp != NULL)
    {
      *uidp = (uid_t)uid;
    }

  g_object_unref(session_proxy);
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
  DBusGProxy * session_proxy;
  GList *l;

  session_proxy = dbus_g_proxy_new_for_name_owner(priv->system_bus,
                                                  CK_ADDR,
                                                  ssid,
                                                  CK_SESSION_IFACE,
                                                  &error);

  if (error != NULL) {
    g_warning("Unable to get CK Session proxy: %s", error->message);
    g_error_free(error);
    return FALSE;
  }

  seat = get_seat_internal (session_proxy);

  if (!seat || !priv->seat || strcmp (seat, priv->seat) != 0) {
    g_object_unref(session_proxy);
    return FALSE;
  }

   if (!org_freedesktop_ConsoleKit_Session_get_x11_display (session_proxy, &xdisplay, &error))
    {
      if (error)
        {
          g_debug ("Failed to call GetX11Display: %s", error->message);
          g_error_free (error);
        }

      g_object_unref(session_proxy);
      return FALSE;
    }

  g_object_unref(session_proxy);

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
  g_return_if_fail (IS_USERS_SERVICE_DBUS(self));

  g_debug ("!!!!!!!!!! - add_sessions_for_user %i %s",
          (int)user->uid, user->user_name);

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
  g_return_if_fail(IS_USERS_SERVICE_DBUS(service));
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  uid_t          uid;
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
	g_debug("Found guest session: %s", priv->guest_session_id);
    return;
  }

  user = users_service_dbus_get_user_by_username (service, pwent->pw_name);
  if (!user)
    return;

  do_add_session (service, user, session_id);
}

static void
seat_proxy_session_removed (DBusGProxy       *seat_proxy,
                            const gchar      *session_id,
                            UsersServiceDbus *service)
{
  g_return_if_fail(IS_USERS_SERVICE_DBUS(service));
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  UserData *user;
  gchar    *username;
  GList    *l;

  username = g_hash_table_lookup (priv->sessions, session_id);
  if (!username) {
    if (g_strcmp0(session_id, priv->guest_session_id) == 0) {
      g_debug("Removing guest session: %s", priv->guest_session_id); 
      if (priv->guest_item != NULL) {
        dbusmenu_menuitem_property_set_bool(priv->guest_item, USER_ITEM_PROP_LOGGED_IN, FALSE);
      }
      g_free(priv->guest_session_id);
      priv->guest_session_id = NULL;
    }
    return;
  }

  user = users_service_dbus_get_user_by_username (service, username);
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
  g_return_if_fail(IS_USERS_SERVICE_DBUS(self));
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  if (priv->count > MINIMUM_USERS)
    {
      GPtrArray *users = NULL;
      GError *error = NULL;
      gint i;

      users = g_ptr_array_new ();

      if (!org_freedesktop_Accounts_list_cached_users (priv->accounts_service_proxy,
                                                       &users,
                                                       &error))
        {
          g_warning ("failed to retrieve user list: %s", error->message);
          g_error_free (error);

          return;
        }

      for (i = 0; i < users->len; i++)
        {
          gchar *id;
          DBusGProxy *proxy;
          UserData *user;
          GError *error = NULL;

          id = g_ptr_array_index (users, i);

          proxy = dbus_g_proxy_new_for_name (priv->system_bus,
                                             "org.freedesktop.Accounts",
                                             id,
                                             "org.freedesktop.DBus.Properties");

          GHashTable *properties;
          if (!dbus_g_proxy_call (proxy, "GetAll", &error,
                                  G_TYPE_STRING, "org.freedesktop.Accounts.User", G_TYPE_INVALID,
                                  dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &properties, G_TYPE_INVALID))
            {
              g_warning ("unable to retrieve user info: %s", error->message);
              g_error_free (error);

              continue;
            }
          
          user = g_hash_table_lookup (priv->users, id);
            // Double check we havent processed this user already
          if (user != NULL)
            {
              g_free(user->user_name);
              g_free(user->real_name);
              g_free(user->icon_file);
              user->real_name_conflict = FALSE;              
              //continue;                                     
            }
          else
            {            
            user = g_new0 (UserData, 1);
            }
          // Can't subscribe to the Changed signal on each individual user path
          // for some reason.
          dbus_g_proxy_add_signal (proxy,
                                   "Changed",
                                   G_TYPE_INVALID);

          dbus_g_proxy_connect_signal (proxy, "Changed",
                                       G_CALLBACK(user_changed),
                                       self,
                                       NULL);
          user->uid         = g_value_get_uint64 (g_hash_table_lookup (properties, "Uid"));
          user->user_name   = g_strdup (g_value_get_string (g_hash_table_lookup (properties, "UserName")));
          user->real_name   = g_strdup (g_value_get_string (g_hash_table_lookup (properties, "RealName")));
          user->icon_file    = g_strdup (g_value_get_string (g_hash_table_lookup (properties, "IconFile")));
          user->real_name_conflict = FALSE;
		      user->menuitem    = NULL;

          g_hash_table_unref (properties);

          g_hash_table_insert (priv->users,
                               g_strdup (id),
                               user);

          add_sessions_for_user (self, user);
        }

      g_ptr_array_free (users, TRUE);
    }
}

static void
user_changed (DBusGProxy  *proxy,
              gpointer     user_data)
{
  g_debug ("JUST RESYNCED THE USERS FROM A USER CHANGE");
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  sync_users (service);
}

static void
user_added (DBusGProxy  *proxy,
            const gchar *user_id,
            gpointer     user_data)
{
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);
  priv->count++;
  sync_users (service);
  g_signal_emit (service,
                 signals[USER_ADDED],
                 0,
                 user_id);   
}

static void
user_deleted (DBusGProxy  *proxy,
              const gchar *user_id,
              gpointer     user_data)
{  
  UsersServiceDbus *service = (UsersServiceDbus *)user_data;
  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (service);

  priv->count--;
  g_hash_table_remove (priv->users, user_id);

  g_signal_emit (service,
                 signals[USER_DELETED],
                 0,
                 user_id);   
  
}

UserData *
users_service_dbus_get_user_by_username (UsersServiceDbus *self,
                                         const gchar *username)
{
  GHashTableIter iter;
  gpointer       value;

  g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), NULL);

  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  g_hash_table_iter_init (&iter, priv->users);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
       UserData *user = value;
       if (strcmp (user->user_name, username) == 0)
         return user;
    }

  return NULL;
}

GList *
users_service_dbus_get_user_list (UsersServiceDbus *self)
{
  g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), NULL);

  UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  return g_hash_table_get_values (priv->users);
}

gboolean
users_service_dbus_show_greeter (UsersServiceDbus *self)
{
	g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	return org_freedesktop_DisplayManager_Seat_switch_to_greeter(priv->display_manager_proxy, NULL);
}

/* Activates the guest account if it can. */
gboolean
users_service_dbus_activate_guest_session (UsersServiceDbus *self)
{
	g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	return org_freedesktop_DisplayManager_Seat_switch_to_guest(priv->display_manager_proxy, "", NULL);
}

/* Activates a specific user */
gboolean
users_service_dbus_activate_user_session (UsersServiceDbus *self,
                                          UserData         *user)
{
	g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	return org_freedesktop_DisplayManager_Seat_switch_to_user(priv->display_manager_proxy, user->user_name, "", NULL);
}

gboolean
users_service_dbus_can_activate_session (UsersServiceDbus *self)
{
  g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);
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
      if (error != NULL){
        g_warning ("Failed to determine if seat can activate sessions: %s",
                    error->message);
        g_error_free (error);
      }
      return FALSE;
    }

  return can_activate;
}

/* Sets the menu item that represents the guest account */
void
users_service_dbus_set_guest_item (UsersServiceDbus * self, DbusmenuMenuitem * mi)
{
	g_return_if_fail(IS_USERS_SERVICE_DBUS(self));
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);
	priv->guest_item = mi;

	if (priv->guest_session_id != NULL) {
      dbusmenu_menuitem_property_set_bool(priv->guest_item, USER_ITEM_PROP_LOGGED_IN, TRUE);
    }

	return;
}

gboolean users_service_dbus_guest_session_enabled (UsersServiceDbus * self)
{
	g_return_val_if_fail(IS_USERS_SERVICE_DBUS(self), FALSE);
	UsersServiceDbusPrivate *priv = USERS_SERVICE_DBUS_GET_PRIVATE (self);

  return priv->guest_session_enabled;
}                                                    

