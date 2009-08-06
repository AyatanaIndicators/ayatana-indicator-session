#ifndef __STATUS_SERVICE_DBUS_H__
#define __STATUS_SERVICE_DBUS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define STATUS_SERVICE_DBUS_TYPE            (status_service_dbus_get_type ())
#define STATUS_SERVICE_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbus))
#define STATUS_SERVICE_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbusClass))
#define IS_STATUS_SERVICE_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATUS_SERVICE_DBUS_TYPE))
#define IS_STATUS_SERVICE_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), STATUS_SERVICE_DBUS_TYPE))
#define STATUS_SERVICE_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), STATUS_SERVICE_DBUS_TYPE, StatusServiceDbusClass))

typedef struct _StatusServiceDbus      StatusServiceDbus;
typedef struct _StatusServiceDbusClass StatusServiceDbusClass;

struct _StatusServiceDbusClass {
	GObjectClass parent_class;

	/* Signals */
	gboolean (*user_changed) (StatusServiceDbus * self, gchar ** name, gpointer user_data);
	gboolean (*status_icons_changed) (StatusServiceDbus * self, GArray ** icons, gpointer user_data);

};

struct _StatusServiceDbus {
	GObject parent;
};

GType status_service_dbus_get_type (void);

G_END_DECLS

#endif
