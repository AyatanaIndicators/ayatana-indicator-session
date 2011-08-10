/*
The Dbus object on the bus for the indicator.

Copyright 2010 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#ifndef __SESSION_DBUS_H__
#define __SESSION_DBUS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define SESSION_DBUS_TYPE            (session_dbus_get_type ())
#define SESSION_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SESSION_DBUS_TYPE, SessionDbus))
#define SESSION_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SESSION_DBUS_TYPE, SessionDbusClass))
#define IS_SESSION_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SESSION_DBUS_TYPE))
#define IS_SESSION_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SESSION_DBUS_TYPE))
#define SESSION_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SESSION_DBUS_TYPE, SessionDbusClass))

typedef struct _SessionDbus      SessionDbus;
typedef struct _SessionDbusClass SessionDbusClass;

struct _SessionDbusClass {
	GObjectClass parent_class;
	void (*icon_updated) (SessionDbus * session, gchar * icon, gpointer user_data);
};

struct _SessionDbus {
	GObject parent;
};

GType session_dbus_get_type (void);
SessionDbus * session_dbus_new (void);
void session_dbus_set_name (SessionDbus * session, const gchar * name);
void session_dbus_set_users_real_name (SessionDbus * session, const gchar * name);
void session_dbus_set_user_menu_visibility (SessionDbus* session, gboolean visible);
void session_dbus_restart_required (SessionDbus* session);
G_END_DECLS

#endif
