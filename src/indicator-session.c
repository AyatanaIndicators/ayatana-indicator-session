/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


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
static GtkImage * status_image = NULL;

static GtkWidget * status_separator = NULL;
static GtkWidget * users_separator = NULL;
#define SEPARATOR_SHOWN(sep) (sep != NULL && GTK_WIDGET_VISIBLE(sep))
static GtkWidget * loading_item = NULL;

static DBusGConnection * connection = NULL;
static DBusGProxy * proxy = NULL;
static DBusGProxy * status_proxy = NULL;

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
static void child_realized (DbusmenuMenuitem * child, gpointer userdata);
static gboolean start_service (gpointer userdata);
static void start_service_phase2 (DBusGProxy * proxy, guint status, GError * error, gpointer data);

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
	g_debug("Changing status icon: '%s'", "user-offline");
	status_image = GTK_IMAGE(gtk_image_new_from_icon_name("user-offline", GTK_ICON_SIZE_MENU));
	gtk_widget_show(GTK_WIDGET(status_image));
	return status_image;
}

typedef struct _realized_data_t realized_data_t;
struct _realized_data_t {
	guint position;
	section_t section;
};

static void
child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint position, gpointer section)
{
	realized_data_t * data = g_new0(realized_data_t, 1);
	if (data == NULL) {
		g_warning("Unable to allocate data for realization of item");
		return;
	}

	data->position = position;
	data->section = GPOINTER_TO_UINT(section);

	g_signal_connect(G_OBJECT(child), DBUSMENU_MENUITEM_SIGNAL_REALIZED, G_CALLBACK(child_realized), data);
	return;
}

static void
child_realized (DbusmenuMenuitem * child, gpointer userdata)
{
	g_return_if_fail(userdata != NULL);
	g_return_if_fail(DBUSMENU_IS_MENUITEM(child));

	realized_data_t * data = (realized_data_t *)userdata;	
	guint position = data->position;
	section_t section = data->section;
	g_free(data);

	DbusmenuGtkClient * client = NULL;
	gchar * errorstr = NULL;
	guint (*posfunc) (void) = NULL;

	switch (section) {
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

	if (client == NULL) {
		g_warning("Child realized for a menu we don't have?  Section: %s", errorstr);
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

void
status_icon_cb (DBusGProxy * proxy, char * icons, GError *error, gpointer userdata)
{
	g_return_if_fail(status_image != NULL);
	g_return_if_fail(icons != NULL);
	g_return_if_fail(icons[0] != '\0');

	g_debug("Changing status icon: '%s'", icons);
	gtk_image_set_from_icon_name(status_image, icons, GTK_ICON_SIZE_MENU);

	return;
}

void
status_icon_changed (DBusGProxy * proxy, gchar * icon, gpointer userdata)
{
	return status_icon_cb(proxy, icon, NULL, NULL);
}


static gboolean
connect_to_status (gpointer userdata)
{
	if (status_proxy == NULL) {
		GError * error = NULL;

		DBusGConnection * sbus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

		status_proxy = dbus_g_proxy_new_for_name_owner(sbus,
		                                               INDICATOR_STATUS_DBUS_NAME,
		                                               INDICATOR_STATUS_SERVICE_DBUS_OBJECT,
		                                               INDICATOR_STATUS_SERVICE_DBUS_INTERFACE,
		                                               &error);

		if (error != NULL) {
			g_warning("Unable to get status proxy: %s", error->message);
			g_error_free(error);
			return FALSE;
		}

		dbus_g_proxy_add_signal(status_proxy, "StatusIconsChanged", G_TYPE_STRING, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(status_proxy, "StatusIconsChanged", G_CALLBACK(status_icon_changed), NULL, NULL);
	}

	org_ayatana_indicator_status_service_status_icons_async(status_proxy, status_icon_cb, NULL);

	return FALSE;
}

/* Follow up the service being started by connecting
   up the DBus Menu Client and creating our separator.
   Also creates an idle func to connect to the service for
   getting the icon that we should be using on the panel. */
static void
status_followup (void)
{
	status_client = dbusmenu_gtkclient_new(INDICATOR_STATUS_DBUS_NAME, INDICATOR_STATUS_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(status_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(status_menu_root_changed), main_menu);

	status_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), status_separator);
	gtk_widget_hide(status_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	g_idle_add(connect_to_status, NULL);

	return;
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

/* Follow up the service being started by connecting
   up the DBus Menu Client and creating our separator. */
static void
users_followup (void)
{
	users_client = dbusmenu_gtkclient_new(INDICATOR_USERS_DBUS_NAME, INDICATOR_USERS_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(users_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(users_menu_root_changed), main_menu);

	users_separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), users_separator);
	gtk_widget_hide(users_separator); /* Should be default, I'm just being explicit.  $(%*#$ hide already!  */

	return;
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

/* Follow up the service being started by connecting
   up the DBus Menu Client. */
static void
session_followup (void)
{
	session_client = dbusmenu_gtkclient_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT);
	g_signal_connect(G_OBJECT(session_client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(session_menu_root_changed), main_menu);

	return;
}

/* Base menu stuff */

/* This takes the response to the service starting up.
   It checks to see if it's started and if so, continues
   with the follow function for the particular area that
   it's working in. */
static void
start_service_phase2 (DBusGProxy * proxy, guint status, GError * error, gpointer data)
{
	/* If we've got an error respond to it */
	if (error != NULL) {
		g_critical("Starting service has resulted in error.");
		g_error_free(error);
		/* Try it all again, we need to get this started! */
		g_idle_add(start_service, data);
		return;
	}

	/* If it's not running or we started it, try again */
	if (status != DBUS_START_REPLY_SUCCESS && status != DBUS_START_REPLY_ALREADY_RUNNING) {
		g_critical("Return value isn't indicative of success: %d", status);
		/* Try it all again, we need to get this started! */
		g_idle_add(start_service, data);
		return;
	}

	/* Check which part of the menu we're in and do the
	   appropriate follow up from the service being started. */
	switch (GPOINTER_TO_INT(data)) {
	case STATUS_SECTION:
		status_followup();
		break;
	case USERS_SECTION:
		users_followup();
		break;
	case SESSION_SECTION:
		session_followup();
		break;
	default:
		g_critical("Oh, how can we get a value that we don't know!");
		break;
	}

	return;
}

/* Our idle service starter.  It looks at the section that
   we're doing and then asks async for that service to be
   started by dbus.  Probably not really useful to be in
   the idle loop as it's so quick, but why not. */
static gboolean
start_service (gpointer userdata)
{
	g_debug("Starting a service");

	if (proxy == NULL) {
		/* If we don't have DBus, let's stay in the idle loop */
		return TRUE;
	}

	const gchar * service = NULL;
	switch (GPOINTER_TO_INT(userdata)) {
	case STATUS_SECTION:
		service = INDICATOR_STATUS_DBUS_NAME;
		break;
	case USERS_SECTION:
		service = INDICATOR_USERS_DBUS_NAME;
		break;
	case SESSION_SECTION:
		service = INDICATOR_SESSION_DBUS_NAME;
		break;
	default:
		g_critical("Oh, how can we get a value that we don't know!");
		return FALSE;
	}

	org_freedesktop_DBus_start_service_by_name_async (proxy, service, 0 /* Flags */, start_service_phase2, userdata);

	return FALSE;
}

/* Indicator based function to get the menu for the whole
   applet.  This starts up asking for the parts of the menu
   from the various services. */
GtkMenu *
get_menu (void)
{
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	if (proxy == NULL) {
		g_warning("Unable to get proxy for DBus itself.  Seriously.");
	}

	/* Startup in the idle loop */
	g_idle_add(start_service, GINT_TO_POINTER(STATUS_SECTION));
	g_idle_add(start_service, GINT_TO_POINTER(USERS_SECTION));
	g_idle_add(start_service, GINT_TO_POINTER(SESSION_SECTION));

	/* Build a temp menu incase someone can ask for it
	   before the services start.  Fast user! */
	main_menu = GTK_MENU(gtk_menu_new());
	loading_item = gtk_menu_item_new_with_label("Loading...");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), loading_item);
	gtk_widget_show(GTK_WIDGET(loading_item));

	return main_menu;
}


