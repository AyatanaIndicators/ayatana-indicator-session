/*
Copyright 2011 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

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


#ifndef _USER_MENU_MGR_H_
#define _USER_MENU_MGR_H_


#include <glib-object.h>
#include <libdbusmenu-gtk/menuitem.h>

#include "session-dbus.h"

G_BEGIN_DECLS

#define USER_TYPE_MENU_MGR             (user_menu_mgr_get_type ())
#define USER_MENU_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), USER_TYPE_MENU_MGR, UserMenuMgr))
#define USER_MENU_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), USER_TYPE_MENU_MGR, UserMenuMgrClass))
#define USER_IS_MENU_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), USER_TYPE_MENU_MGR))
#define USER_IS_MENU_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), USER_TYPE_MENU_MGR))
#define USER_MENU_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), USER_TYPE_MENU_MGR, UserMenuMgrClass))

typedef struct _UserMenuMgrClass UserMenuMgrClass;
typedef struct _UserMenuMgr UserMenuMgr;

struct _UserMenuMgrClass
{
	GObjectClass parent_class;
};

GType user_menu_mgr_get_type (void) G_GNUC_CONST;
UserMenuMgr* user_menu_mgr_new (SessionDbus* session_dbus,
                                gboolean greeter_mode);

DbusmenuMenuitem* user_mgr_get_root_item (UserMenuMgr* self);
G_END_DECLS

#endif /* _USER_MENU_MGR_H_ */
