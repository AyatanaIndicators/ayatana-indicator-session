/*
A small helper for locking the screen.

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

#ifndef LOCK_HELPER_H__
#define LOCK_HELPER_H__

#include <libdbusmenu-glib/menuitem.h>

gboolean will_lock_screen (void);
void lock_screen (DbusmenuMenuitem * mi, guint timestamp, gpointer data);
gboolean lock_screen_setup (gpointer data);

#endif /* LOCK_HELPER_H__ */
