#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "status-provider.h"
#include "status-provider-pidgin.h"

typedef struct _StatusProviderPidginPrivate StatusProviderPidginPrivate;
struct _StatusProviderPidginPrivate {
};

#define STATUS_PROVIDER_PIDGIN_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_PROVIDER_PIDGIN_TYPE, StatusProviderPidginPrivate))

static void status_provider_pidgin_class_init (StatusProviderPidginClass *klass);
static void status_provider_pidgin_init       (StatusProviderPidgin *self);
static void status_provider_pidgin_dispose    (GObject *object);
static void status_provider_pidgin_finalize   (GObject *object);

G_DEFINE_TYPE (StatusProviderPidgin, status_provider_pidgin, STATUS_PROVIDER_TYPE);

static void
status_provider_pidgin_class_init (StatusProviderPidginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (StatusProviderPidginPrivate));

	object_class->dispose = status_provider_pidgin_dispose;
	object_class->finalize = status_provider_pidgin_finalize;

	return;
}

static void
status_provider_pidgin_init (StatusProviderPidgin *self)
{

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
