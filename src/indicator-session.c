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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/client.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>

#include "dbus-shared-names.h"

#define INDICATOR_SESSION_TYPE            (indicator_session_get_type ())
#define INDICATOR_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INDICATOR_SESSION_TYPE, IndicatorSession))
#define INDICATOR_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INDICATOR_SESSION_TYPE, IndicatorSessionClass))
#define IS_INDICATOR_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INDICATOR_SESSION_TYPE))
#define IS_INDICATOR_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INDICATOR_SESSION_TYPE))
#define INDICATOR_SESSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INDICATOR_SESSION_TYPE, IndicatorSessionClass))

typedef struct _IndicatorSession      IndicatorSession;
typedef struct _IndicatorSessionClass IndicatorSessionClass;

struct _IndicatorSessionClass {
	IndicatorObjectClass parent_class;
};

struct _IndicatorSession {
	IndicatorObject parent;
};

GType indicator_session_get_type (void);

/* Indicator stuff */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_SESSION_TYPE)

/* Globals */
static DbusmenuGtkClient * users_client = NULL;
static DbusmenuGtkClient * session_client = NULL;

static GtkMenu * main_menu = NULL;
static GtkImage * status_image = NULL;

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

/* Prototypes */
static void child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint position, gpointer section);
static guint users_menu_pos_offset (void);
static guint session_menu_pos_offset (void);
static void child_realized (DbusmenuMenuitem * child, gpointer userdata);
static gboolean start_service (gpointer userdata);
static void start_service_phase2 (DBusGProxy * proxy, guint status, GError * error, gpointer data);
static GtkLabel * get_label (IndicatorObject * io);
static GtkImage * get_icon (IndicatorObject * io);
static GtkMenu * get_menu (IndicatorObject * io);

static void indicator_session_class_init (IndicatorSessionClass *klass);
static void indicator_session_init       (IndicatorSession *self);
static void indicator_session_dispose    (GObject *object);
static void indicator_session_finalize   (GObject *object);

G_DEFINE_TYPE (IndicatorSession, indicator_session, INDICATOR_OBJECT_TYPE);

static void
indicator_session_class_init (IndicatorSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = indicator_session_dispose;
	object_class->finalize = indicator_session_finalize;

	IndicatorObjectClass * io_class = INDICATOR_OBJECT_CLASS(klass);
	io_class->get_label = get_label;
	io_class->get_image = get_icon;
	io_class->get_menu = get_menu;

	return;
}

static void
indicator_session_init (IndicatorSession *self)
{

	return;
}

static void
indicator_session_dispose (GObject *object)
{

	G_OBJECT_CLASS (indicator_session_parent_class)->dispose (object);
	return;
}

static void
indicator_session_finalize (GObject *object)
{

	G_OBJECT_CLASS (indicator_session_parent_class)->finalize (object);
	return;
}

static GtkLabel *
get_label (IndicatorObject * io)
{
	GtkLabel * returnval = GTK_LABEL(gtk_label_new(g_get_user_name()));
	gtk_widget_show(GTK_WIDGET(returnval));
	return returnval;
}

static GtkImage *
get_icon (IndicatorObject * io)
{
	g_debug("Changing status icon: '%s'", "system-shutdown-panel");
	status_image = GTK_IMAGE(gtk_image_new_from_icon_name("system-shutdown-panel", GTK_ICON_SIZE_MENU));
	gtk_widget_show(GTK_WIDGET(status_image));
	return status_image;
}

typedef struct _realized_data_t realized_data_t;
struct _realized_data_t {
	section_t section;
};

static void 
resort_menu (void)
{
	guint location = 0;
	guint clientnum;

	for (clientnum = 0; clientnum < 3; clientnum++) {
		DbusmenuGtkClient * client = NULL;
		if (clientnum == 1) client = users_client;
		if (clientnum == 2) client = session_client;

		if (client == NULL) continue;

		DbusmenuMenuitem * root = dbusmenu_client_get_root(DBUSMENU_CLIENT(client));

		GList * children = dbusmenu_menuitem_get_children(root);
		if (children == NULL) {
			continue;
		}

		GList * child;
		for (child = children; child != NULL; child = g_list_next(child)) {
			GtkMenuItem * widget = dbusmenu_gtkclient_menuitem_get(client, DBUSMENU_MENUITEM(child->data));
			if (widget != NULL) {
				gtk_menu_reorder_child(main_menu, GTK_WIDGET(widget), location);
				location++;
			}
		}

		if (clientnum == 1) {
			gtk_menu_reorder_child(main_menu, users_separator, location);
			location++;
		}
	}

	return;
}

static void
child_added (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint position, gpointer section)
{
	realized_data_t * data = g_new0(realized_data_t, 1);
	if (data == NULL) {
		g_warning("Unable to allocate data for realization of item");
		return;
	}

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
	section_t section = data->section;
	g_free(data);

	DbusmenuGtkClient * client = NULL;
	gchar * errorstr = NULL;
	guint (*posfunc) (void) = NULL;

	switch (section) {
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

	GtkMenuItem * widget = dbusmenu_gtkclient_menuitem_get(client, child);

	if (widget == NULL) {
		g_warning("Had a menu item added to the %s menu, but yet it didn't have a GtkWidget with it.  Can't add that to a menu now can we?  You need to figure this @#$# out!", errorstr);
		return;
	}

	gtk_menu_append(main_menu, GTK_WIDGET(widget));
	gtk_widget_show(GTK_WIDGET(widget));

	resort_menu();

	gtk_widget_hide(loading_item);

	return;
}

static void
child_moved (DbusmenuMenuitem * parent, DbusmenuMenuitem * child, guint newpos, guint oldpos, guint (*posfunc) (void))
{


}

/* Users menu */

static guint
users_menu_pos_offset (void)
{
	guint position = 0;
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
static GtkMenu *
get_menu (IndicatorObject * io)
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


