/*
A small wrapper utility for connecting to gconf.

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
#define __GCONF_HELPER_H__ 1

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#define SESSION_SCHEMA      "org.canonical.indicator.session"
#define SUPPRESS_KEY        "suppress_logout_restart_shutdown"
#define LOGOUT_KEY          "suppress_logout_menuitem"
#define RESTART_KEY         "suppress_restart_menuitem"
#define SHUTDOWN_KEY        "suppress_shutdown_menuitem"

#define LOCKDOWN_DIR              "/desktop/gnome/lockdown"
#define LOCKDOWN_KEY_USER         LOCKDOWN_DIR "/disable_user_switching"
#define LOCKDOWN_KEY_SCREENSAVER  LOCKDOWN_DIR "/disable_lock_screen"
#define KEYBINDING_DIR            "/apps/gnome_settings_daemon/keybindings"
#define KEY_LOCK_SCREEN           KEYBINDING_DIR "/screensaver"

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

#endif /* __GCONF_HELPER__ */
