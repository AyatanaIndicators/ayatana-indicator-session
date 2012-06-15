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


#ifndef _SESSION_MENU_MGR_H_
#define _SESSION_MENU_MGR_H_

#include <glib-object.h>

#include "session-dbus.h"

G_BEGIN_DECLS

#define SESSION_TYPE_MENU_MGR             (session_menu_mgr_get_type ())
#define SESSION_MENU_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SESSION_TYPE_MENU_MGR, SessionMenuMgr))
#define SESSION_MENU_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SESSION_TYPE_MENU_MGR, SessionMenuMgrClass))
#define IS_SESSION_MENU_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SESSION_TYPE_MENU_MGR))
#define IS_SESSION_MENU_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SESSION_TYPE_MENU_MGR))
#define SESSION_MENU_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SESSION_TYPE_MENU_MGR, SessionMenuMgrClass))

typedef struct _SessionMenuMgrClass SessionMenuMgrClass;
typedef struct _SessionMenuMgr SessionMenuMgr;

struct _SessionMenuMgrClass
{
  GObjectClass parent_class;
};

GType session_menu_mgr_get_type (void) G_GNUC_CONST;

SessionMenuMgr* session_menu_mgr_new (DbusmenuMenuitem  * parent_mi,
                                    SessionDbus       * session_dbus,
                                    gboolean            greeter_mode);

G_END_DECLS

#endif /* _SESSION_MENU_MGR_H_ */
