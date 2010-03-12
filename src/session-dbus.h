#ifndef __SESSION_DBUS_H__
#define __SESSION_DBUS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define SESSION_DBUS_TYPE            (session_dbus_get_type ())
#define SESSION_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SESSION_DBUS_TYPE, SessionDbus))
#define SESSION_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SESSION_DBUS_TYPE, SessionDbusClass))
#define IS_SESSION_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SESSION_DBUS_TYPE))
#define IS_SESSION_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SESSION_DBUS_TYPE))
#define SESSION_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SESSION_DBUS_TYPE, SessionDbusClass))

typedef struct _SessionDbus      SessionDbus;
typedef struct _SessionDbusClass SessionDbusClass;

struct _SessionDbusClass {
	GObjectClass parent_class;
	void (*icon_updated) (SessionDbus * session, gchar * icon, gpointer user_data);
};

struct _SessionDbus {
	GObject parent;
};

GType session_dbus_get_type (void);
SessionDbus * session_dbus_new (void);
void  session_dbus_set_name (SessionDbus * session, const gchar * name);

G_END_DECLS

#endif
