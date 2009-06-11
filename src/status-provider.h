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

#define STATUS_PROVIDER_SIGNAL_STATUS_CHANGED  "status-changed"

typedef struct _StatusProvider      StatusProvider;
struct _StatusProvider {
	GObject parent;
};

typedef struct _StatusProviderClass StatusProviderClass;
struct _StatusProviderClass {
	/* Signals */
	void (*status_changed) (guint newstatus);

	/* Virtual Functions */
	void  (*set_status) (StatusProvider * sp, guint newstatus);
	guint (*get_status) (StatusProvider * sp);

	GObjectClass parent_class;
};

GType status_provider_get_type (void);

void status_provider_set_status (StatusProvider * sp, guint status);
guint status_provider_get_status (StatusProvider * sp);

void status_provider_emit_status_changed (StatusProvider * sp, guint newstatus);

G_END_DECLS

#endif
