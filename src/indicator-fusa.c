
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menu.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
INDICATOR_SET_VERSION
INDICATOR_SET_NAME("users-status-shutdown")

#include "dbus-shared-names.h"

GtkLabel *
get_label (void)
{
	GtkLabel * returnval = gtk_label_new("Ted Gould");
	return returnval;
}

GtkImage *
get_icon (void)
{
	return NULL;
}

GtkMenu *
get_menu (void)
{
	guint returnval = 0;
	GError * error = NULL;

	DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	DBusGProxy * proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_FUSA_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return NULL;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return NULL;
	}

	return GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_FUSA_DBUS_NAME, INDICATOR_FUSA_DBUS_OBJECT));
}


