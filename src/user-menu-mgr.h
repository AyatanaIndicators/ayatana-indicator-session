/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * user-menu-mgr.c
 * Copyright (C) Conor Curran 2011 <conor.curran@canonical.com>
 * 
 * user-menu-mgr.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * user-menu-mgr.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef _USER_MENU_MGR_H_
#define _USER_MENU_MGR_H_

#include <glib-object.h>
#include "users-service-dbus.h"

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

struct _UserMenuMgr
{
	GObject parent_instance;
  UsersServiceDbus* users_dbus_interface;
  DbusmenuMenuitem* root_item;
  gint user_count;
};

GType user_menu_mgr_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _USER_MENU_MGR_H_ */
