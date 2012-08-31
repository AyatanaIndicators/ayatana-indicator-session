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

#ifndef _ONLINE_ACCOUNTS_MGR_H_
#define _ONLINE_ACCOUNTS_MGR_H_

#include <glib-object.h>
#include <libdbusmenu-glib/menuitem.h>

G_BEGIN_DECLS

#define ONLINE_ACCOUNTS_TYPE_MGR             (online_accounts_mgr_get_type ())
#define ONLINE_ACCOUNTS_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ONLINE_ACCOUNTS_TYPE_MGR, OnlineAccountsMgr))
#define ONLINE_ACCOUNTS_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ONLINE_ACCOUNTS_TYPE_MGR, OnlineAccountsMgrClass))
#define ONLINE_ACCOUNTS_IS_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ONLINE_ACCOUNTS_TYPE_MGR))
#define ONLINE_ACCOUNTS_IS_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ONLINE_ACCOUNTS_TYPE_MGR))
#define ONLINE_ACCOUNTS_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ONLINE_ACCOUNTS_TYPE_MGR, OnlineAccountsMgrClass))

typedef struct _OnlineAccountsMgrClass OnlineAccountsMgrClass;
typedef struct _OnlineAccountsMgr OnlineAccountsMgr;

struct _OnlineAccountsMgrClass
{
  GObjectClass parent_class;
};

GType online_accounts_mgr_get_type (void) G_GNUC_CONST;
OnlineAccountsMgr *online_accounts_mgr_new (void);

DbusmenuMenuitem *online_accounts_mgr_get_menu_item (OnlineAccountsMgr *self);

G_END_DECLS

#endif /* _ONLINE_ACCOUNTS_MGR_H_ */
