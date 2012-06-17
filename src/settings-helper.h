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

#define SESSION_SCHEMA                "com.canonical.indicator.session"
#define SUPPRESS_KEY                  "suppress-logout-restart-shutdown"
#define LOGOUT_KEY                    "suppress-logout-menuitem"
#define RESTART_KEY                   "suppress-restart-menuitem"
#define SHUTDOWN_KEY                  "suppress-shutdown-menuitem"
#define SHOW_USER_MENU                "user-show-menu"

gboolean supress_confirmations (void);

#endif /* __GCONF_HELPER__ */
