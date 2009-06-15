
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menu.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
INDICATOR_SET_VERSION
INDICATOR_SET_NAME("users-status-session")

#include "dbus-shared-names.h"
#include "status-service-client.h"

static GtkMenu * status_menu = NULL;
static GtkMenu * users_menu = NULL;
static GtkMenu * session_menu = NULL;
static GtkMenu * main_menu = NULL;

GtkLabel *
get_label (void)
{
	GtkLabel * returnval = GTK_LABEL(gtk_label_new("Ted Gould"));
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

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_STATUS_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return NULL;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return NULL;
	}

	status_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_STATUS_DBUS_NAME, INDICATOR_STATUS_DBUS_OBJECT));

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_USERS_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return NULL;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return NULL;
	}

	users_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_USERS_DBUS_NAME, INDICATOR_USERS_DBUS_OBJECT));

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_SESSION_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return NULL;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return NULL;
	}

	session_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT));

	main_menu = GTK_MENU(gtk_menu_new());

	return main_menu;
}


