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


#ifndef __CK_PK_HELPER_H__
#define __CK_PK_HELPER_H__ 1

#include <polkit-gnome/polkit-gnome.h>

gboolean pk_require_auth (LogoutDialogAction action);
gboolean pk_can_do_action (const gchar *action_id, PolKitResult * pol_result);

#endif /* __CK_PK_HELPER__ */
