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

#ifndef _UDEV_MGR_H_
#define _UDEV_MGR_H_

#include <glib-object.h>
#include <libdbusmenu-glib/client.h>

#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

G_BEGIN_DECLS

#define UDEV_TYPE_MGR             (udev_mgr_get_type ())
#define UDEV_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), UDEV_TYPE_MGR, UdevMgr))
#define UDEV_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), UDEV_TYPE_MGR, UdevMgrClass))
#define UDEV_IS_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UDEV_TYPE_MGR))
#define UDEV_IS_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), UDEV_TYPE_MGR))
#define UDEV_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), UDEV_TYPE_MGR, UdevMgrClass))

typedef struct _UdevMgrClass UdevMgrClass;
typedef struct _UdevMgr UdevMgr;

struct _UdevMgrClass
{
	GObjectClass parent_class;
};


GType udev_mgr_get_type (void) G_GNUC_CONST;
UdevMgr* udev_mgr_new (DbusmenuMenuitem* scanner_item, 
                       DbusmenuMenuitem* webcam_item);

typedef enum {
  ADD,
  REMOVE
}UdevMgrDeviceAction;

G_END_DECLS

#endif /* _UDEV_MGR_H_ */
