/*
Copyright 2012 Canonical Ltd.

Authors:
    Alberto Mardegan <alberto.mardegan@canonical.com>

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

#ifndef _WEBCREDENTIALS_MGR_H_
#define _WEBCREDENTIALS_MGR_H_

#include <glib-object.h>
#include <libdbusmenu-glib/menuitem.h>

G_BEGIN_DECLS

#define WEBCREDENTIALS_TYPE_MGR             (webcredentials_mgr_get_type ())
#define WEBCREDENTIALS_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEBCREDENTIALS_TYPE_MGR, WebcredentialsMgr))
#define WEBCREDENTIALS_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), WEBCREDENTIALS_TYPE_MGR, WebcredentialsMgrClass))
#define WEBCREDENTIALS_IS_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEBCREDENTIALS_TYPE_MGR))
#define WEBCREDENTIALS_IS_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), WEBCREDENTIALS_TYPE_MGR))
#define WEBCREDENTIALS_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), WEBCREDENTIALS_TYPE_MGR, WebcredentialsMgrClass))

typedef struct _WebcredentialsMgrClass WebcredentialsMgrClass;
typedef struct _WebcredentialsMgr WebcredentialsMgr;

struct _WebcredentialsMgrClass
{
  GObjectClass parent_class;
};

GType webcredentials_mgr_get_type (void) G_GNUC_CONST;
WebcredentialsMgr *webcredentials_mgr_new (void);

DbusmenuMenuitem *webcredentials_mgr_get_menu_item (WebcredentialsMgr *self);

G_END_DECLS

#endif /* _WEBCREDENTIALS_MGR_H_ */
