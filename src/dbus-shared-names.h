/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#ifndef __DBUS_SHARED_NAMES_H__
#define __DBUS_SHARED_NAMES_H__ 

typedef enum {
  UP_TO_DATE,
  CHECKING_FOR_UPDATES,
  UPDATES_AVAILABLE,
  UPGRADE_IN_PROGRESS, 
  FINISHED, 
  RESTART_NEEDED,
  DONT_KNOW
}AptState;

typedef enum {
  SIMULATION,
  REAL
}TransactionType;

#define INDICATOR_USERS_DBUS_NAME  INDICATOR_SESSION_DBUS_NAME
#define INDICATOR_USERS_DBUS_OBJECT "/com/canonical/indicator/users/menu"
#define INDICATOR_USERS_SERVICE_DBUS_OBJECT "/org/gnome/DisplayManager/UserManager"
#define INDICATOR_USERS_SERVICE_DBUS_INTERFACE "org.gnome.DisplayManager.UserManager"

#define INDICATOR_SESSION_DBUS_NAME  "com.canonical.indicator.session"
#define INDICATOR_SESSION_DBUS_OBJECT "/com/canonical/indicator/session/menu"
#define INDICATOR_SESSION_DBUS_VERSION  0

#define INDICATOR_SESSION_SERVICE_DBUS_OBJECT "/com/canonical/indicator/session/service"
#define INDICATOR_SESSION_SERVICE_DBUS_IFACE  "com.canonical.indicator.session.service"

#define USER_ITEM_TYPE            "x-canonical-user-item"
#define USER_ITEM_PROP_NAME       "user-item-name"
#define USER_ITEM_PROP_LOGGED_IN  "user-item-logged-in"
#define USER_ITEM_PROP_IS_CURRENT_USER "user-item-is-current-user"
#define USER_ITEM_PROP_ICON       "user-item-icon-path"
#define USER_ITEM_ICON_DEFAULT    "avatar-default"

#define RESTART_ITEM_TYPE         "x-canonical-restart-item"
#define RESTART_ITEM_LABEL        "restart-label"
#define RESTART_ITEM_ICON         "restart-icon"

#define ICON_DEFAULT              "system-devices-panel"
#define ICON_RESTART              "system-devices-panel-alert"
#define GREETER_ICON_DEFAULT      "system-shutdown-panel"
#define GREETER_ICON_RESTART      "system-shutdown-panel-restart"

#endif /* __DBUS_SHARED_NAMES_H__ */
