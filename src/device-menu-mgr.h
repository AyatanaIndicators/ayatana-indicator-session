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


#ifndef _DEVICE_MENU_MGR_H_
#define _DEVICE_MENU_MGR_H_

#include <glib-object.h>

#include "session-dbus.h"

G_BEGIN_DECLS

#define DEVICE_TYPE_MENU_MGR             (device_menu_mgr_get_type ())
#define DEVICE_MENU_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DEVICE_TYPE_MENU_MGR, DeviceMenuMgr))
#define DEVICE_MENU_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DEVICE_TYPE_MENU_MGR, DeviceMenuMgrClass))
#define DEVICE_IS_MENU_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DEVICE_TYPE_MENU_MGR))
#define DEVICE_IS_MENU_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DEVICE_TYPE_MENU_MGR))
#define DEVICE_MENU_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DEVICE_TYPE_MENU_MGR, DeviceMenuMgrClass))

typedef struct _DeviceMenuMgrClass DeviceMenuMgrClass;
typedef struct _DeviceMenuMgr DeviceMenuMgr;

struct _DeviceMenuMgrClass
{
	GObjectClass parent_class;
};

GType device_menu_mgr_get_type (void) G_GNUC_CONST;

DeviceMenuMgr* device_menu_mgr_new (SessionDbus* session_dbus, gboolean greeter_mode);

DbusmenuMenuitem* device_mgr_get_root_item (DeviceMenuMgr* self);

G_END_DECLS

#endif /* _DEVICE_MENU_MGR_H_ */
