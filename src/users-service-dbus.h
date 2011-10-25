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

#ifndef __USERS_SERVICE_DBUS_H__
#define __USERS_SERVICE_DBUS_H__

#include <glib.h>
#include <glib-object.h>
#include <libdbusmenu-glib/menuitem.h>

G_BEGIN_DECLS

#define USERS_SERVICE_DBUS_TYPE         (users_service_dbus_get_type ())
#define USERS_SERVICE_DBUS(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), USERS_SERVICE_DBUS_TYPE, UsersServiceDbus))
#define USERS_SERVICE_DBUS_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), USERS_SERVICE_DBUS_TYPE, UsersServiceDbusClass))
#define IS_USERS_SERVICE_DBUS(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), USERS_SERVICE_DBUS_TYPE))
#define IS_USERS_SERVICE_DBUS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), USERS_SERVICE_DBUS_TYPE))
#define USERS_SERVICE_DBUS_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), USERS_SERVICE_DBUS_TYPE, UsersServiceDbusClass))

typedef struct _UsersServiceDbus      UsersServiceDbus;
typedef struct _UsersServiceDbusClass UsersServiceDbusClass;
typedef struct _UserData              UserData;

struct _UserData
{
  gint64   uid;
  gchar   *user_name;
  gchar   *real_name;
  gchar   *icon_file;

  GList   *sessions;

  /* Whether the real name here conflicts with another in the system */
  gboolean real_name_conflict;
  /* The menuitem representing this user if there is one. */
  DbusmenuMenuitem * menuitem;

  UsersServiceDbus *service;
};

/* XXX - MAXIMUM_USERS should be set to 7 once we've
 *       got some gdm issues worked out.
 */
#define MINIMUM_USERS           0

struct _UsersServiceDbus {
  GObject parent;
};

struct _UsersServiceDbusClass {
  GObjectClass parent_class;

  /* Signals */
  void     (* user_added)         (UsersServiceDbus *self, const gchar *user_id, gpointer user_data);
  void     (* user_deleted)       (UsersServiceDbus *self, const gchar *user_id, gpointer user_data);
};

GType users_service_dbus_get_type  (void) G_GNUC_CONST;

UserData *users_service_dbus_get_user_by_username  (UsersServiceDbus *self,
                                                    const gchar *username);
GList    *users_service_dbus_get_user_list         (UsersServiceDbus *self);
gboolean  users_service_dbus_show_greeter          (UsersServiceDbus *self);
gboolean  users_service_dbus_can_activate_session  (UsersServiceDbus *self);
gboolean  users_service_dbus_activate_user_session (UsersServiceDbus *self,
                                                    UserData         *user);
gboolean  users_service_dbus_activate_guest_session (UsersServiceDbus *self);
void      users_service_dbus_set_guest_item        (UsersServiceDbus * self,
                                                    DbusmenuMenuitem * mi);

gboolean users_service_dbus_guest_session_enabled (UsersServiceDbus * self);

G_END_DECLS

#endif
