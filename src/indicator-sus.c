
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

static GtkWidget * status_separator = NULL;
static GtkWidget * users_separator = NULL;
#define SEPARATOR_SHOWN(sep) (sep != NULL && GTK_WIDGET_VISIBLE(sep))
static GtkWidget * loading_item = NULL;

static DBusGConnection * connection = NULL;
static DBusGProxy * proxy = NULL;

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

static void
menu_add (GtkContainer * source, GtkWidget * addee, GtkMenu * addto, guint positionoffset)
{
	GList * location = g_list_find(GTK_MENU_SHELL(source)->children, addee);
	guint position = g_list_position(GTK_MENU_SHELL(source)->children, location);

	position += positionoffset;
	g_debug("Adding a widget: %d", position);

	gtk_menu_insert(addto, addee, position);
	gtk_widget_show(addee);

	gtk_widget_hide(loading_item);

	return;
}

static void
status_menu_add (GtkContainer * container, GtkWidget * widget, gpointer userdata)
{
	menu_add(container, widget, GTK_MENU(userdata), 0);
	gtk_widget_show(status_separator);
	return;
}

static void
users_menu_add (GtkContainer * container, GtkWidget * widget, gpointer userdata)
{
	guint position = 0;
	if (SEPARATOR_SHOWN(status_separator)) {
		GList * location = g_list_find(GTK_MENU_SHELL(main_menu)->children, status_separator);
		position = g_list_position(GTK_MENU_SHELL(main_menu)->children, location) + 1;
	}

	menu_add(container, widget, GTK_MENU(userdata), position);
	gtk_widget_show(users_separator);
	return;
}

static void
session_menu_add (GtkContainer * container, GtkWidget * widget, gpointer userdata)
{
	guint position = 0;
	if (SEPARATOR_SHOWN(users_separator)) {
		GList * location = g_list_find(GTK_MENU_SHELL(main_menu)->children, users_separator);
		position = g_list_position(GTK_MENU_SHELL(main_menu)->children, location) + 1;
	}

	menu_add(container, widget, GTK_MENU(userdata), position);
	return;
}


static gboolean
build_status_menu (gpointer userdata)
{
	guint returnval = 0;
	GError * error = NULL;

	if (proxy == NULL) {
		/* If we don't have DBus, let's stay in the idle loop */
		return TRUE;
	}

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_STATUS_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return FALSE;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return FALSE;
	}

	status_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_STATUS_DBUS_NAME, INDICATOR_STATUS_DBUS_OBJECT));
	g_signal_connect(G_OBJECT(status_menu), "add", G_CALLBACK(status_menu_add), main_menu);

	status_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), status_separator);
	gtk_widget_hide(status_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	return FALSE;
}

static gboolean
build_users_menu (gpointer userdata)
{
	guint returnval = 0;
	GError * error = NULL;

	if (proxy == NULL) {
		/* If we don't have DBus, let's stay in the idle loop */
		return TRUE;
	}

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_USERS_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return FALSE;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return FALSE;
	}

	users_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_USERS_DBUS_NAME, INDICATOR_USERS_DBUS_OBJECT));
	g_signal_connect(G_OBJECT(users_menu), "add", G_CALLBACK(users_menu_add), main_menu);

	users_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), users_separator);
	gtk_widget_hide(users_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	return FALSE;
}

static gboolean
build_session_menu (gpointer userdata)
{
	guint returnval = 0;
	GError * error = NULL;

	if (proxy == NULL) {
		/* If we don't have DBus, let's stay in the idle loop */
		return TRUE;
	}

	if (!org_freedesktop_DBus_start_service_by_name (proxy, INDICATOR_SESSION_DBUS_NAME, 0, &returnval, &error)) {
		g_error("Unable to send message to DBus to start service: %s", error != NULL ? error->message : "(NULL error)" );
		g_error_free(error);
		return FALSE;
	}

	if (returnval != DBUS_START_REPLY_SUCCESS && returnval != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_error("Return value isn't indicative of success: %d", returnval);
		return FALSE;
	}

	session_menu = GTK_MENU(dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT));
	g_signal_connect(G_OBJECT(session_menu), "add", G_CALLBACK(session_menu_add), main_menu);

	return FALSE;
}

GtkMenu *
get_menu (void)
{
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	g_idle_add(build_status_menu, NULL);
	g_idle_add(build_users_menu, NULL);
	g_idle_add(build_session_menu, NULL);

	main_menu = GTK_MENU(gtk_menu_new());
	loading_item = gtk_menu_item_new_with_label("Loading...");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), loading_item);

	return main_menu;
}


