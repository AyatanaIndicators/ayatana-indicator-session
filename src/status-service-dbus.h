/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

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

#ifndef __STATUS_SERVICE_DBUS_H__
#define __STATUS_SERVICE_DBUS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define STATUS_SERVICE_DBUS_TYPE            (status_service_dbus_get_type ())
#define STATUS_SERVICE_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbus))
#define STATUS_SERVICE_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbusClass))
#define IS_STATUS_SERVICE_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATUS_SERVICE_DBUS_TYPE))
#define IS_STATUS_SERVICE_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), STATUS_SERVICE_DBUS_TYPE))
#define STATUS_SERVICE_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbusClass))

typedef struct _StatusServiceDbus      StatusServiceDbus;
typedef struct _StatusServiceDbusClass StatusServiceDbusClass;

struct _StatusServiceDbusClass {
	GObjectClass parent_class;

	/* Signals */
	gboolean (*user_changed) (StatusServiceDbus * self, gchar ** name, gpointer user_data);
	gboolean (*status_icons_changed) (StatusServiceDbus * self, GArray ** icons, gpointer user_data);

};

struct _StatusServiceDbus {
	GObject parent;
};

GType status_service_dbus_get_type (void);
void status_service_dbus_set_status (StatusServiceDbus * self, const gchar * icon);

G_END_DECLS

#endif
