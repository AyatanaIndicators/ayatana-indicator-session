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
	                                             g_cclosure_marshal_VOID__POINTER,
	                                             G_TYPE_NONE, 1, G_TYPE_POINTER);

	dbus_g_object_type_install_info(STATUS_SERVICE_DBUS_TYPE, &dbus_glib__status_service_server_object_info);
	
	return;
}

static void
status_service_dbus_init (StatusServiceDbus *self)
{

	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_g_connection_register_g_object(connection,
										INDICATOR_STATUS_SERVICE_DBUS_OBJECT,
										G_OBJECT(self));

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

	return TRUE;
}
