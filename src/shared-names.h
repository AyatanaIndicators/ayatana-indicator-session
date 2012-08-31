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
#define ICON_INFO                 "system-devices-panel-information"
#define ICON_ALERT                "system-devices-panel-alert"

#define GREETER_ICON_DEFAULT      "system-shutdown-panel"
#define GREETER_ICON_RESTART      "system-shutdown-panel-restart"

/* the session indicator's settings */
#define SESSION_SCHEMA                "com.canonical.indicator.session"
#define SUPPRESS_KEY                  "suppress-logout-restart-shutdown"
#define LOGOUT_KEY                    "suppress-logout-menuitem"
#define RESTART_KEY                   "suppress-restart-menuitem"
#define SHUTDOWN_KEY                  "suppress-shutdown-menuitem"
#define SHOW_USER_MENU                "user-show-menu"


#endif /* __DBUS_SHARED_NAMES_H__ */
