
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/client.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
INDICATOR_SET_VERSION
INDICATOR_SET_NAME("users-status-session")

#include "dbus-shared-names.h"
#include "status-service-client.h"

static DbusmenuGtkClient * status_client = NULL;
static DbusmenuGtkClient * users_client = NULL;
static DbusmenuGtkClient * session_client = NULL;

static GtkMenu * main_menu = NULL;

static GtkWidget * status_separator = NULL;
static GtkWidget * users_separator = NULL;
#define SEPARATOR_SHOWN(sep) (sep != NULL && GTK_WIDGET_VISIBLE(sep))
static GtkWidget * loading_item = NULL;

static DBusGConnection * connection = NULL;
static DBusGProxy * proxy = NULL;

typedef enum {
	STATUS_SECTION,
	USERS_SECTION,
	SESSION_SECTION,
	END_OF_SECTIONS
} section_t;

static void child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint position, gpointer section);
static guint status_menu_pos_offset (void);
static guint users_menu_pos_offset (void);
static guint session_menu_pos_offset (void);

GtkLabel *
get_label (void)
{
	GtkLabel * returnval = GTK_LABEL(gtk_label_new(g_get_user_name()));
	gtk_widget_show(GTK_WIDGET(returnval));
	return returnval;
}

GtkImage *
get_icon (void)
{
	GtkImage * image = GTK_IMAGE(gtk_image_new_from_icon_name("user-offline", GTK_ICON_SIZE_MENU));
	return image;
}

static void
child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint position, gpointer section)
{
	DbusmenuGtkClient * client = NULL;
	gchar * errorstr = NULL;
	guint (*posfunc) (void) = NULL;

	switch (GPOINTER_TO_UINT(section)) {
		case STATUS_SECTION:
			client = status_client;
			errorstr = "Status";
			posfunc = status_menu_pos_offset;
			break;
		case USERS_SECTION:
			client = users_client;
			errorstr = "Users";
			posfunc = users_menu_pos_offset;
			break;
		case SESSION_SECTION:
			client = session_client;
			errorstr = "Session";
			posfunc = session_menu_pos_offset;
			break;
		default:
			g_warning("Child Added called with an unknown position function!");
			return;
	}

	position += posfunc();
	g_debug("SUS: Adding child: %d", position);
	GtkMenuItem * widget = dbusmenu_gtkclient_menuitem_get(client, child);

	if (widget == NULL) {
		g_warning("Had a menu item added to the %s menu, but yet it didn't have a GtkWidget with it.  Can't add that to a menu now can we?  You need to figure this @#$# out!", errorstr);
		return;
	}

	gtk_menu_insert(main_menu, GTK_WIDGET(widget), position);
	gtk_widget_show(GTK_WIDGET(widget));

	gtk_widget_hide(loading_item);

	return;
}

static void
child_moved (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint newpos, guint oldpos, guint (*posfunc) (void))
{


}


/* Status Menu */
static guint
status_menu_pos_offset (void)
{
	return 0;
}

static void
status_menu_added (DbusmenuMenuitem * root, DbusmenuMenuitem * child, guint position, gpointer user_data)
{
	gtk_widget_show(GTK_WIDGET(status_separator));
	return;
}

static void
status_menu_removed (DbusmenuMenuitem * root, DbusmenuMenuitem * child, gpointer user_data)
{
	if (g_list_length(dbusmenu_menuitem_get_children(root)) == 0) {
		gtk_widget_hide(GTK_WIDGET(status_separator));
	}

	return;
}

static void
status_menu_root_changed(DbusmenuGtkClient * client, DbusmenuMenuitem * newroot, GtkMenu * main)
{
	if (newroot == NULL) {
		gtk_widget_hide(GTK_WIDGET(status_separator));
		return;
	}

	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED,   G_CALLBACK(child_added),           GUINT_TO_POINTER(STATUS_SECTION));
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(status_menu_added),     NULL);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(status_menu_removed),   NULL);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED,   G_CALLBACK(child_moved),           GUINT_TO_POINTER(STATUS_SECTION));

	GList * child = NULL;
	guint count = 0;
	for (child = dbusmenu_menuitem_get_children(newroot); child != NULL; child = g_list_next(child), count++) {
		child_added(newroot, DBUSMENU_MENUITEM(child->data), count, GUINT_TO_POINTER(STATUS_SECTION));
	}

	if (count > 0) {
		gtk_widget_show(GTK_WIDGET(status_separator));
	}

	return;
}

static gboolean
build_status_menu (gpointer userdata)
{
	g_debug("Building Status Menu");
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

	status_client = dbusmenu_gtkclient_new(INDICATOR_STATUS_DBUS_NAME, INDICATOR_STATUS_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(status_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(status_menu_root_changed), main_menu);

	status_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), status_separator);
	gtk_widget_hide(status_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	return FALSE;
}

/* Users menu */

static guint
users_menu_pos_offset (void)
{
	guint position = 0;
	if (SEPARATOR_SHOWN(status_separator)) {
		GList * location = g_list_find(GTK_MENU_SHELL(main_menu)->children, status_separator);
		position = g_list_position(GTK_MENU_SHELL(main_menu)->children, location) + 1;
	}

	return position;
}

static void
users_menu_added (DbusmenuMenuitem * root, DbusmenuMenuitem * child, guint position, gpointer user_data)
{
	gtk_widget_show(GTK_WIDGET(users_separator));
	return;
}

static void
users_menu_removed (DbusmenuMenuitem * root, DbusmenuMenuitem * child, gpointer user_data)
{
	if (g_list_length(dbusmenu_menuitem_get_children(root)) == 0) {
		gtk_widget_hide(GTK_WIDGET(users_separator));
	}

	return;
}

static void
users_menu_root_changed(DbusmenuGtkClient * client, DbusmenuMenuitem * newroot, GtkMenu * main)
{
	if (newroot == NULL) {
		gtk_widget_hide(GTK_WIDGET(users_separator));
		return;
	}

	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED,   G_CALLBACK(child_added),           GUINT_TO_POINTER(USERS_SECTION));
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(users_menu_added),      NULL);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(users_menu_removed),    NULL);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED,   G_CALLBACK(child_moved),           GUINT_TO_POINTER(USERS_SECTION));

	GList * child = NULL;
	guint count = 0;
	for (child = dbusmenu_menuitem_get_children(newroot); child != NULL; child = g_list_next(child), count++) {
		child_added(newroot, DBUSMENU_MENUITEM(child->data), count, GUINT_TO_POINTER(USERS_SECTION));
	}

	if (count > 0) {
		gtk_widget_show(GTK_WIDGET(users_separator));
	}

	return;
}

static gboolean
build_users_menu (gpointer userdata)
{
	g_debug("Building Users Menu");
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

	users_client = dbusmenu_gtkclient_new(INDICATOR_USERS_DBUS_NAME, INDICATOR_USERS_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(users_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(users_menu_root_changed), main_menu);

	users_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), users_separator);
	gtk_widget_hide(users_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	return FALSE;
}

/* Session Menu Stuff */

static guint
session_menu_pos_offset (void)
{
	guint position = 0;
	if (SEPARATOR_SHOWN(users_separator)) {
		GList * location = g_list_find(GTK_MENU_SHELL(main_menu)->children, users_separator);
		position = g_list_position(GTK_MENU_SHELL(main_menu)->children, location) + 1;
	} else if (SEPARATOR_SHOWN(status_separator)) {
		GList * location = g_list_find(GTK_MENU_SHELL(main_menu)->children, status_separator);
		position = g_list_position(GTK_MENU_SHELL(main_menu)->children, location) + 1;
	}

	return position;
}

static void
session_menu_removed (DbusmenuMenuitem * root, DbusmenuMenuitem * child, gpointer user_data)
{
	return;
}

static void
session_menu_root_changed(DbusmenuGtkClient * client, DbusmenuMenuitem * newroot, GtkMenu * main)
{
	if (newroot == NULL) {
		/* We're assuming this'll crash the least so it doesn't
		   hide a separator.  May be a bad choice. */
		return;
	}

	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED,   G_CALLBACK(child_added),           GUINT_TO_POINTER(SESSION_SECTION));
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(session_menu_removed),  NULL);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED,   G_CALLBACK(child_moved),           GUINT_TO_POINTER(SESSION_SECTION));

	GList * child = NULL;
	guint count = 0;
	for (child = dbusmenu_menuitem_get_children(newroot); child != NULL; child = g_list_next(child), count++) {
		child_added(newroot, DBUSMENU_MENUITEM(child->data), count, GUINT_TO_POINTER(SESSION_SECTION));
	}

	return;
}

static gboolean
build_session_menu (gpointer userdata)
{
	g_debug("Building Session Menu");
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

	session_client = dbusmenu_gtkclient_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(session_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(session_menu_root_changed), main_menu);

	return FALSE;
}

/* Base menu stuff */

GtkMenu *
get_menu (void)
{
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	if (proxy == NULL) {
		g_warning("Unable to get proxy for DBus itself.  Seriously.");
	}

	g_idle_add(build_status_menu, NULL);
	g_idle_add(build_users_menu, NULL);
	g_idle_add(build_session_menu, NULL);

	main_menu = GTK_MENU(gtk_menu_new());
	loading_item = gtk_menu_item_new_with_label("Loading...");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), loading_item);
	gtk_widget_show(GTK_WIDGET(loading_item));

	return main_menu;
}


