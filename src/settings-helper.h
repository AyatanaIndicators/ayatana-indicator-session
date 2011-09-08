/*
A small wrapper utility for connecting to GSettings.

Copyright 2009 Canonical Ltd.

Authors:
    Christoph Korn <c_korn@gmx.de>

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


#ifndef __GCONF_HELPER_H__
#define __GCONF_HELPER_H__ 

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#define SESSION_SCHEMA                "com.canonical.indicator.session"
#define SUPPRESS_KEY                  "suppress-logout-restart-shutdown"
#define LOGOUT_KEY                    "suppress-logout-menuitem"
#define RESTART_KEY                   "suppress-restart-menuitem"
#define SHUTDOWN_KEY                  "suppress-shutdown-menuitem"
#define SHOW_USER_MENU                "user-show-menu"
#define USER_USERNAME_IN_SWITCH_ITEM  "use-username-in-switch-item"

#define LOCKDOWN_SCHEMA           "org.gnome.desktop.lockdown"
#define LOCKDOWN_KEY_USER         "disable-user-switching"
#define LOCKDOWN_KEY_SCREENSAVER  "disable-lock-screen"
#define KEYBINDING_SCHEMA         "org.gnome.settings-daemon.plugins.media-keys"
#define KEY_LOCK_SCREEN           "screensaver"

typedef struct _RestartShutdownLogoutMenuItems
{
	DbusmenuMenuitem * logout_mi;
	DbusmenuMenuitem * restart_mi;
	DbusmenuMenuitem * shutdown_mi;
}
RestartShutdownLogoutMenuItems;

void update_menu_entries(RestartShutdownLogoutMenuItems*);
gboolean supress_confirmations (void);
gboolean show_logout (void);
gboolean show_restart (void);
gboolean show_shutdown (void);
gboolean should_show_user_menu (void);


#endif /* __GCONF_HELPER__ */
