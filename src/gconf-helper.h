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

#include <gconf/gconf-client.h>

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#define GLOBAL_DIR      "/apps/indicator-session"
#define SUPPRESS_KEY    GLOBAL_DIR "/suppress_logout_restart_shutdown"
#define LOGOUT_KEY      GLOBAL_DIR "/suppress_logout_menuitem"

typedef struct _RestartShutdownLogoutMenuItems
{
	DbusmenuMenuitem * logout_mi;
	DbusmenuMenuitem * restart_mi;
	DbusmenuMenuitem * shutdown_mi;
}
RestartShutdownLogoutMenuItems;

void update_menu_entries(RestartShutdownLogoutMenuItems*, DbusmenuMenuitem * logoutitem);
gboolean supress_confirmations (void);
gboolean show_logout (void);

#endif /* __GCONF_HELPER__ */
