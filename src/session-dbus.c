#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "session-dbus.h"

static gboolean _session_dbus_server_get_icon (SessionDbus * service, gchar ** icon, GError ** error);

#include "session-dbus-server.h"

typedef struct _SessionDbusPrivate SessionDbusPrivate;
struct _SessionDbusPrivate {
	gint dummy;
};

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

	dbus_g_object_type_install_info(SESSION_DBUS_TYPE, &dbus_glib__session_dbus_server_object_info);

	return;
}

static void
session_dbus_init (SessionDbus *self)
{
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

	G_OBJECT_CLASS (session_dbus_parent_class)->finalize (object);
	return;
}

static gboolean
_session_dbus_server_get_icon (SessionDbus * service, gchar ** icon, GError ** error)
{

	return TRUE;
}
