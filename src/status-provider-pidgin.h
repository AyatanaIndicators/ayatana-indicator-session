#ifndef __STATUS_PROVIDER_PIDGIN_H__
#define __STATUS_PROVIDER_PIDGIN_H__

#include <glib.h>
#include <glib-object.h>

#include "status-provider.h"

G_BEGIN_DECLS

#define STATUS_PROVIDER_PIDGIN_TYPE            (status_provider_pidgin_get_type ())
#define STATUS_PROVIDER_PIDGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATUS_PROVIDER_PIDGIN_TYPE, StatusProviderPidgin))
#define STATUS_PROVIDER_PIDGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STATUS_PROVIDER_PIDGIN_TYPE, StatusProviderPidginClass))
#define IS_STATUS_PROVIDER_PIDGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATUS_PROVIDER_PIDGIN_TYPE))
#define IS_STATUS_PROVIDER_PIDGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), STATUS_PROVIDER_PIDGIN_TYPE))
#define STATUS_PROVIDER_PIDGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STATUS_PROVIDER_PIDGIN_TYPE, StatusProviderPidginClass))


typedef struct _StatusProviderPidginClass StatusProviderPidginClass;
struct _StatusProviderPidginClass {
	StatusProviderClass parent_class;
};

typedef struct _StatusProviderPidgin      StatusProviderPidgin;
struct _StatusProviderPidgin {
	StatusProvider parent;
};

GType status_provider_pidgin_get_type (void);
StatusProvider * status_provider_pidgin_new (void);

G_END_DECLS

#endif
