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
static void _status_service_server_watch   (void);
static void _status_service_server_status_icons   (void);
static void _status_service_server_pretty_user_name   (void);

#include "status-service-server.h"

G_DEFINE_TYPE (StatusServiceDbus, status_service_dbus, G_TYPE_OBJECT);

static void
status_service_dbus_class_init (StatusServiceDbusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = status_service_dbus_dispose;
	object_class->finalize = status_service_dbus_finalize;

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

static void
_status_service_server_watch (void)
{

	return;
}

static void
_status_service_server_status_icons (void)
{

	return;
}

static void
_status_service_server_pretty_user_name (void)
{

	return;
}
