/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * udev-mgr.c
 * Copyright (C) Conor Curran 2011 <ronoc@rhrOneiric>
 * 
 * udev-mgr.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * udev-mgr.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef _UDEV_MGR_H_
#define _UDEV_MGR_H_

#include <glib-object.h>

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

struct _UdevMgr
{
	GObject parent_instance;
};

GType udev_mgr_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _UDEV_MGR_H_ */
