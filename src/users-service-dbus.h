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

#ifndef __USERS_SERVICE_DBUS_H__
#define __USERS_SERVICE_DBUS_H__

#include <glib.h>
#include <glib-object.h>

#include "dbus-user.h" /* for AccountsUser */

G_BEGIN_DECLS

#define USERS_SERVICE_DBUS_TYPE  (users_service_dbus_get_type ())
#define USERS_SERVICE_DBUS(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), USERS_SERVICE_DBUS_TYPE, UsersServiceDbus))
#define IS_USERS_SERVICE_DBUS(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), USERS_SERVICE_DBUS_TYPE))

typedef struct _UsersServiceDbus        UsersServiceDbus;
typedef struct _UsersServiceDbusClass   UsersServiceDbusClass;
typedef struct _UsersServiceDbusPrivate UsersServiceDbusPrivate;

/**
 * A facade class which interacts with multiple DBus services to
 * track info which is useful to the interactor's user menu:
 *
 *  1. A list of users to add to the user menu.
 *
 *     Each user is an AccountsUser object, which is a GDBusProxy
 *     to an org.freedesktop.Accounts.User object.
 *
 *     We initially build this list by calling org.freedesktop.Accounts'
 *     GetCachedUsers method. We also monitor o.f.Accounts' UserAdded
 *     and UserDeleted and update the list accordingly.
 *
 *  2. Track which users currently have X sessions.
 *     This is used for the menuitems' USER_ITEM_PROP_LOGGED_IN property.
 *
 *     We initially build this list by calling org.freedesktop.ConsoleKit.Seat's
 *     GetDevices method. We also monitor the seat for SessionAdded and
 *     SessionRemoved and update the list accordingly.
 *
 *  3. Provide an API for user switching and guest sessions.
 *     These are typically pass-through functions to GDBusProxies.
 *
 */
struct _UsersServiceDbus
{
  /*< private >*/
  GObject parent;
  UsersServiceDbusPrivate * priv;
};

struct _UsersServiceDbusClass
{
  GObjectClass parent_class;

  /* Signals */
  void (* user_list_changed)       (UsersServiceDbus*, gpointer);
  void (* user_logged_in_changed)  (UsersServiceDbus*, AccountsUser*, gpointer);
  void (* guest_logged_in_changed) (UsersServiceDbus*, gpointer);
};

GType     users_service_dbus_get_type               (void) G_GNUC_CONST;

GList   * users_service_dbus_get_user_list          (UsersServiceDbus * self);

gboolean  users_service_dbus_is_guest_logged_in     (UsersServiceDbus * self);
gboolean  users_service_dbus_is_user_logged_in      (UsersServiceDbus * self,
                                                     AccountsUser     * user);

void      users_service_dbus_show_greeter           (UsersServiceDbus * self);
gboolean  users_service_dbus_guest_session_enabled  (UsersServiceDbus * self);
gboolean  users_service_dbus_can_activate_session   (UsersServiceDbus * self);
void      users_service_dbus_activate_guest_session (UsersServiceDbus * self);
void      users_service_dbus_activate_user_session  (UsersServiceDbus * self,
                                                     AccountsUser     * user);

G_END_DECLS

#endif
