#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "status-provider.h"

typedef struct _StatusProviderPrivate StatusProviderPrivate;
struct _StatusProviderPrivate {
};

#define STATUS_PROVIDER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_PROVIDER_TYPE, StatusProviderPrivate))

static void status_provider_class_init (StatusProviderClass *klass);
static void status_provider_init       (StatusProvider *self);
static void status_provider_dispose    (GObject *object);
static void status_provider_finalize   (GObject *object);

G_DEFINE_TYPE (StatusProvider, status_provider, G_TYPE_OBJECT);

static void
status_provider_class_init (StatusProviderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (StatusProviderPrivate));

	object_class->dispose = status_provider_dispose;
	object_class->finalize = status_provider_finalize;

	return;
}

static void
status_provider_init (StatusProvider *self)
{

	return;
}

static void
status_provider_dispose (GObject *object)
{

	G_OBJECT_CLASS (status_provider_parent_class)->dispose (object);
	return;
}

static void
status_provider_finalize (GObject *object)
{

	G_OBJECT_CLASS (status_provider_parent_class)->finalize (object);
	return;
}
