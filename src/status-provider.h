#ifndef __STATUS_PROVIDER_H__
#define __STATUS_PROVIDER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define STATUS_PROVIDER_TYPE            (status_provider_get_type ())
#define STATUS_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATUS_PROVIDER_TYPE, StatusProvider))
#define STATUS_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STATUS_PROVIDER_TYPE, StatusProviderClass))
#define IS_STATUS_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATUS_PROVIDER_TYPE))
#define IS_STATUS_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), STATUS_PROVIDER_TYPE))
#define STATUS_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STATUS_PROVIDER_TYPE, StatusProviderClass))


typedef struct _StatusProviderClass StatusProviderClass;
struct _StatusProviderClass {
	GObjectClass parent_class;
};

typedef struct _StatusProvider      StatusProvider;
struct _StatusProvider {
	GObject parent;
};

GType status_provider_get_type (void);

G_END_DECLS

#endif
