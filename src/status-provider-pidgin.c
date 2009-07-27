#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "status-provider.h"
#include "status-provider-pidgin.h"

#include <dbus/dbus-glib.h>

typedef enum {
	PG_STATUS_UNKNOWN,
	PG_STATUS_OFFLINE,
	PG_STATUS_AVAILABLE,
	PG_STATUS_UNAVAILABLE,
	PG_STATUS_INVISIBLE,
	PG_STATUS_AWAY,
	PG_STATUS_EXTENDEND_AWAY,
	PG_STATUS_MOBILE,
	PG_STATUS_TUNE
} pg_status_t;

static const StatusProviderStatus pg_to_sp_map[] = {
	/* PG_STATUS_UNKNOWN,        */   STATUS_PROVIDER_STATUS_OFFLINE,
	/* PG_STATUS_OFFLINE,        */   STATUS_PROVIDER_STATUS_OFFLINE,
	/* PG_STATUS_AVAILABLE,      */   STATUS_PROVIDER_STATUS_ONLINE,
	/* PG_STATUS_UNAVAILABLE,    */   STATUS_PROVIDER_STATUS_DND,
	/* PG_STATUS_INVISIBLE,      */   STATUS_PROVIDER_STATUS_INVISIBLE,
	/* PG_STATUS_AWAY,           */   STATUS_PROVIDER_STATUS_AWAY,
	/* PG_STATUS_EXTENDEND_AWAY, */   STATUS_PROVIDER_STATUS_AWAY,
	/* PG_STATUS_MOBILE,         */   STATUS_PROVIDER_STATUS_OFFLINE,
	/* PG_STATUS_TUNE            */   STATUS_PROVIDER_STATUS_OFFLINE
};

static const pg_status_t sp_to_pg_map[STATUS_PROVIDER_STATUS_LAST] = {
	/* STATUS_PROVIDER_STATUS_ONLINE,  */  PG_STATUS_AVAILABLE,
	/* STATUS_PROVIDER_STATUS_AWAY,    */  PG_STATUS_AWAY,
	/* STATUS_PROVIDER_STATUS_DND      */  PG_STATUS_UNAVAILABLE,
	/* STATUS_PROVIDER_STATUS_INVISIBLE*/  PG_STATUS_INVISIBLE,
	/* STATUS_PROVIDER_STATUS_OFFLINE  */  PG_STATUS_OFFLINE
};

typedef struct _StatusProviderPidginPrivate StatusProviderPidginPrivate;
struct _StatusProviderPidginPrivate {
	DBusGProxy * proxy;
	pg_status_t  pg_status;
};

#define STATUS_PROVIDER_PIDGIN_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_PROVIDER_PIDGIN_TYPE, StatusProviderPidginPrivate))

/* Prototypes */
/* GObject stuff */
static void status_provider_pidgin_class_init (StatusProviderPidginClass *klass);
static void status_provider_pidgin_init       (StatusProviderPidgin *self);
static void status_provider_pidgin_dispose    (GObject *object);
static void status_provider_pidgin_finalize   (GObject *object);
/* Internal Funcs */
static void set_status (StatusProvider * sp, StatusProviderStatus status);
static StatusProviderStatus get_status (StatusProvider * sp);

G_DEFINE_TYPE (StatusProviderPidgin, status_provider_pidgin, STATUS_PROVIDER_TYPE);

static void
status_provider_pidgin_class_init (StatusProviderPidginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (StatusProviderPidginPrivate));

	object_class->dispose = status_provider_pidgin_dispose;
	object_class->finalize = status_provider_pidgin_finalize;

	StatusProviderClass * spclass = STATUS_PROVIDER_CLASS(klass);

	spclass->set_status = set_status;
	spclass->get_status = get_status;

	return;
}

static void
status_provider_pidgin_init (StatusProviderPidgin *self)
{
	StatusProviderPidginPrivate * priv = STATUS_PROVIDER_PIDGIN_GET_PRIVATE(self);

	priv->proxy = NULL;
	priv->pg_status = PG_STATUS_OFFLINE;

	DBusGConnection * bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	g_return_if_fail(bus != NULL); /* Can't do anymore DBus stuff without this,
	                                  all non-DBus stuff should be done */

	GError * error = NULL;
	priv->proxy = dbus_g_proxy_new_for_name_owner (bus,
	                                               "im.pidgin.purple.PurpleService",
	                                               "/im/pidgin/purple/PurpleObject",
	                                               "im.pidgin.purple.PurpleInterface",
	                                               &error);
	if (error != NULL) {
		g_debug("Unable to get Pidgin proxy: %s", error->message);
		g_error_free(error);
	}

	return;
}

static void
status_provider_pidgin_dispose (GObject *object)
{

	G_OBJECT_CLASS (status_provider_pidgin_parent_class)->dispose (object);
	return;
}

static void
status_provider_pidgin_finalize (GObject *object)
{

	G_OBJECT_CLASS (status_provider_pidgin_parent_class)->finalize (object);
	return;
}

/**
	status_provider_pidgin_new:

	Creates a new #StatusProviderPidgin object.  No parameters or anything
	like that.  Just a convience function.

	Return value: A new instance of #StatusProviderPidgin
*/
StatusProvider *
status_provider_pidgin_new (void)
{
	return STATUS_PROVIDER(g_object_new(STATUS_PROVIDER_PIDGIN_TYPE, NULL));
}

/* Takes the status provided generically for Status providers
   and turns it into a Pidgin status and sends it to Pidgin. */
static void
set_status (StatusProvider * sp, StatusProviderStatus status)
{
	g_debug("\tSetting Pidgin Status: %d", status);
	g_return_if_fail(IS_STATUS_PROVIDER_PIDGIN(sp));
	StatusProviderPidginPrivate * priv = STATUS_PROVIDER_PIDGIN_GET_PRIVATE(sp);
	pg_status_t pg_status = sp_to_pg_map[status];
	priv->pg_status = pg_status;
	return;
}

/* Takes the cached Pidgin status and makes it into the generic
   Status provider status */
static StatusProviderStatus
get_status (StatusProvider * sp)
{
	g_return_val_if_fail(IS_STATUS_PROVIDER_PIDGIN(sp), STATUS_PROVIDER_STATUS_OFFLINE);
	StatusProviderPidginPrivate * priv = STATUS_PROVIDER_PIDGIN_GET_PRIVATE(sp);
	return pg_to_sp_map[priv->pg_status];
}
