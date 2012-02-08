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

#ifndef _APT_WATCHER_H_
#define _APT_WATCHER_H_

#include <glib-object.h>

#include <libdbusmenu-glib/client.h>

#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

#include "session-dbus.h"

G_BEGIN_DECLS

#define APT_TYPE_WATCHER             (apt_watcher_get_type ())
#define APT_WATCHER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), APT_TYPE_WATCHER, AptWatcher))
#define APT_WATCHER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), APT_TYPE_WATCHER, AptWatcherClass))
#define APT_IS_WATCHER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APT_TYPE_WATCHER))
#define APT_IS_WATCHER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), APT_TYPE_WATCHER))
#define APT_WATCHER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), APT_TYPE_WATCHER, AptWatcherClass))

typedef struct _AptWatcherClass AptWatcherClass;
typedef struct _AptWatcher AptWatcher;

struct _AptWatcherClass
{
	GObjectClass parent_class;
};

GType apt_watcher_get_type (void) G_GNUC_CONST;

AptWatcher* apt_watcher_new (SessionDbus* session_dbus,
                             DbusmenuMenuitem* apt_item);
G_END_DECLS

#endif /* _APT_WATCHER_H_ */
