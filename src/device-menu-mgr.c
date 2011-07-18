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


#include "device-menu-mgr.h"

struct _DeviceMenuMgr
{
	GObject parent_instance;
  DbusmenuMenuitem* root_item;
  SessionDbus* session_dbus_interface;  
};

G_DEFINE_TYPE (DeviceMenuMgr, device_menu_mgr, G_TYPE_OBJECT);

static void
device_menu_mgr_init (DeviceMenuMgr *object)
{
	/* TODO: Add initialization code here */
}

static void
device_menu_mgr_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (device_menu_mgr_parent_class)->finalize (object);
}

static void
device_menu_mgr_class_init (DeviceMenuMgrClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	object_class->finalize = device_menu_mgr_finalize;
}

DbusmenuMenuitem*
device_mgr_get_root_item (DeviceMenuMgr* self)
{
  return self->root_item;
}

/*
 * Clean Entry Point 
 */
DeviceMenuMgr* device_menu_mgr_new (SessionDbus* session_dbus)
{
  DeviceMenuMgr* device_mgr = g_object_new (DEVICE_TYPE_MENU_MGR, NULL);
  device_mgr->session_dbus_interface = session_dbus;
  //device_menu_mgr_rebuild_items (device_mgr);    
  return device_mgr;
}
