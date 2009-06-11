#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "status-provider.h"

static void status_provider_class_init (StatusProviderClass *klass);
static void status_provider_init       (StatusProvider *self);

G_DEFINE_TYPE (StatusProvider, status_provider, G_TYPE_OBJECT);

static void
status_provider_class_init (StatusProviderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	return;
}

static void
status_provider_init (StatusProvider *self)
{

	return;
}

