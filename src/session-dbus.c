#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "session-dbus.h"
#include "dbus-shared-names.h"

static gboolean _session_dbus_server_get_icon (SessionDbus * service, gchar ** icon, GError ** error);

#include "session-dbus-server.h"

typedef struct _SessionDbusPrivate SessionDbusPrivate;
struct _SessionDbusPrivate {
	gchar * name;
};

/* Signals */
enum {
	ICON_UPDATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define SESSION_DBUS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), SESSION_DBUS_TYPE, SessionDbusPrivate))

static void session_dbus_class_init (SessionDbusClass *klass);
static void session_dbus_init       (SessionDbus *self);
static void session_dbus_dispose    (GObject *object);
static void session_dbus_finalize   (GObject *object);

G_DEFINE_TYPE (SessionDbus, session_dbus, G_TYPE_OBJECT);

static void
session_dbus_class_init (SessionDbusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (SessionDbusPrivate));

	object_class->dispose = session_dbus_dispose;
	object_class->finalize = session_dbus_finalize;

	signals[ICON_UPDATED] = g_signal_new ("icon-updated",
	                                      G_TYPE_FROM_CLASS (klass),
	                                      G_SIGNAL_RUN_LAST,
	                                      G_STRUCT_OFFSET (SessionDbusClass, icon_updated),
	                                      NULL, NULL,
	                                      g_cclosure_marshal_VOID__STRING,
	                                      G_TYPE_NONE, 1, G_TYPE_STRING);

	dbus_g_object_type_install_info(SESSION_DBUS_TYPE, &dbus_glib__session_dbus_server_object_info);

	return;
}

static void
session_dbus_init (SessionDbus *self)
{
	DBusGConnection * session = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_g_connection_register_g_object(session, INDICATOR_SESSION_SERVICE_DBUS_OBJECT, G_OBJECT(self));

	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(self);

	priv->name = g_strdup(ICON_DEFAULT);

	return;
}

static void
session_dbus_dispose (GObject *object)
{

	G_OBJECT_CLASS (session_dbus_parent_class)->dispose (object);
	return;
}

static void
session_dbus_finalize (GObject *object)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(object);

	if (priv->name != NULL) {
		g_free(priv->name);
		priv->name = NULL;
	}

	G_OBJECT_CLASS (session_dbus_parent_class)->finalize (object);
	return;
}

static gboolean
_session_dbus_server_get_icon (SessionDbus * service, gchar ** icon, GError ** error)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(service);
	*icon = g_strdup(priv->name);
	return TRUE;
}

SessionDbus *
session_dbus_new (void)
{
	return SESSION_DBUS(g_object_new(SESSION_DBUS_TYPE, NULL));
}

void
session_dbus_set_name (SessionDbus * session, const gchar * name)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(session);
	if (priv->name != NULL) {
		g_free(priv->name);
		priv->name = NULL;
	}
	priv->name = g_strdup(name);
	g_signal_emit(G_OBJECT(session), signals[ICON_UPDATED], 0, priv->name, TRUE);
	return;
}
