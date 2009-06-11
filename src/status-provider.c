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

	klass->status_changed = NULL;

	klass->set_status = NULL;
	klass->get_status = NULL;

	return;
}

static void
status_provider_init (StatusProvider *self)
{

	return;
}

void
status_provider_set_status (StatusProvider * sp, StatusProviderStatus status)
{
	g_return_if_fail(IS_STATUS_PROVIDER(sp));

	StatusProviderClass * class = STATUS_PROVIDER_CLASS(sp);
	g_return_if_fail(class->set_status != NULL);

	return class->set_status(sp, status);
}

StatusProviderStatus
status_provider_get_status (StatusProvider * sp)
{
	g_return_val_if_fail(IS_STATUS_PROVIDER(sp), STATUS_PROVIDER_STATUS_OFFLINE);

	StatusProviderClass * class = STATUS_PROVIDER_CLASS(sp);
	g_return_val_if_fail(class->get_status != NULL, STATUS_PROVIDER_STATUS_OFFLINE);

	return class->get_status(sp);
}

void
status_provider_emit_status_changed (StatusProvider * sp, StatusProviderStatus newstatus)
{

	return;
}
