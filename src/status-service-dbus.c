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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>

#include "dbus-shared-names.h"
#include "status-service-dbus.h"

static void status_service_dbus_class_init (StatusServiceDbusClass *klass);
static void status_service_dbus_init       (StatusServiceDbus *self);
static void status_service_dbus_dispose    (GObject *object);
static void status_service_dbus_finalize   (GObject *object);
static gboolean _status_service_server_watch (StatusServiceDbus * service, GError ** error);
static gboolean _status_service_server_status_icons (StatusServiceDbus * service, GArray * array, GError ** error);
static gboolean _status_service_server_pretty_user_name (StatusServiceDbus * service, gchar ** username, GError ** error);

#include "status-service-server.h"

/* Private */
typedef struct _StatusServiceDbusPrivate StatusServiceDbusPrivate;
struct _StatusServiceDbusPrivate
{
	gchar * name;
};

#define STATUS_SERVICE_DBUS_GET_PRIVATE(o) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbusPrivate))

/* Signals */
enum {
	USER_CHANGED,
	STATUS_ICONS_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GObject Boilerplate */
G_DEFINE_TYPE (StatusServiceDbus, status_service_dbus, G_TYPE_OBJECT);

static void
status_service_dbus_class_init (StatusServiceDbusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof(StatusServiceDbusPrivate));

	object_class->dispose = status_service_dbus_dispose;
	object_class->finalize = status_service_dbus_finalize;

	/**
		StatusServiceDbus::user-changed:
		@arg0: The #StatusServiceDbus object.
		@arg1: The place to put the new user name

		Signals that the user name has changed and gives the
		new user name.
	*/
	signals[USER_CHANGED]      = g_signal_new("user-changed",
	                                          G_TYPE_FROM_CLASS(klass),
	                                          G_SIGNAL_RUN_LAST,
	                                          G_STRUCT_OFFSET(StatusServiceDbusClass, user_changed),
	                                          NULL, NULL,
	                                          g_cclosure_marshal_VOID__POINTER,
	                                          G_TYPE_NONE, 1, G_TYPE_POINTER);

	/**
		StatusServiceDbus::status-icons-changed:
		@arg0: The #StatusServiceDbus object.
		@arg1: The list of icon names representing the statuses in
		       the order they should be displayed.  Left to right.

		Signals that the user status set has changed and that
		new icons may need to be loaded.  The list of icons will
		always be complete.
	*/
	signals[STATUS_ICONS_CHANGED] = g_signal_new("status-icons-changed",
	                                             G_TYPE_FROM_CLASS(klass),
	                                             G_SIGNAL_RUN_LAST,
	                                             G_STRUCT_OFFSET(StatusServiceDbusClass, status_icons_changed),
	                                             NULL, NULL,
	                                             g_cclosure_marshal_VOID__STRING,
	                                             G_TYPE_NONE, 1, G_TYPE_STRING);

	dbus_g_object_type_install_info(STATUS_SERVICE_DBUS_TYPE, &dbus_glib__status_service_server_object_info);
	
	return;
}

static void
status_service_dbus_init (StatusServiceDbus *self)
{

	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_g_connection_register_g_object(connection,
										INDICATOR_STATUS_DBUS_OBJECT,
										G_OBJECT(self));

	StatusServiceDbusPrivate * priv = STATUS_SERVICE_DBUS_GET_PRIVATE(self);
	priv->name = "test";

	return;
}

static void
status_service_dbus_dispose (GObject *object)
{

	G_OBJECT_CLASS (status_service_dbus_parent_class)->dispose (object);
	return;
}

static void
status_service_dbus_finalize (GObject *object)
{

	G_OBJECT_CLASS (status_service_dbus_parent_class)->finalize (object);
	return;
}

static gboolean
_status_service_server_watch (StatusServiceDbus * service, GError ** error)
{

	return TRUE;
}

static gboolean
_status_service_server_status_icons (StatusServiceDbus * service, GArray * array, GError ** error)
{

	return TRUE;
}

static gboolean
_status_service_server_pretty_user_name (StatusServiceDbus * service, gchar ** username, GError ** error)
{
	if (!IS_STATUS_SERVICE_DBUS(service)) {
		g_warning("NO BAD EVIL!");
		return FALSE;
	}

	StatusServiceDbusPrivate * priv = STATUS_SERVICE_DBUS_GET_PRIVATE(service);
	if (priv->name == NULL) {
		*username = g_strdup("");
	} else {
		*username = g_strdup(priv->name);
	}

	return TRUE;
}

void
status_service_dbus_set_status (StatusServiceDbus * self, const gchar * icon)
{
	g_signal_emit(G_OBJECT(self), signals[STATUS_ICONS_CHANGED], 0, icon, TRUE);
	return;
}
