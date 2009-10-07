/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
    Christoph Korn <c_korn@gmx.de>

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

#include <config.h>

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

#include "gtk-dialog/gconf-helper.h"

#include "lock-helper.h"

#define DKP_ADDRESS    "org.freedesktop.DeviceKit.Power"
#define DKP_OBJECT     "/org/freedesktop/DeviceKit/Power"
#define DKP_INTERFACE  "org.freedesktop.DeviceKit.Power"

static DbusmenuMenuitem * root_menuitem = NULL;
static GMainLoop * mainloop = NULL;
static DBusGProxy * dkp_main_proxy = NULL;
static DBusGProxy * dkp_prop_proxy = NULL;

static DBusGProxyCall * suspend_call = NULL;
static DBusGProxyCall * hibernate_call = NULL;

static DbusmenuMenuitem * hibernate_mi = NULL;
static DbusmenuMenuitem * suspend_mi = NULL;
static DbusmenuMenuitem * logout_mi = NULL;
static DbusmenuMenuitem * restart_mi = NULL;
static DbusmenuMenuitem * shutdown_mi = NULL;


/* Let's put this machine to sleep, with some info on how
   it should sleep.  */
static void
sleep (DbusmenuMenuitem * mi, gpointer userdata)
{
	gchar * type = (gchar *)userdata;

	if (dkp_main_proxy == NULL) {
		g_warning("Can not %s as no DeviceKit Power Proxy", type);
	}

	lock_screen(NULL, NULL);

	dbus_g_proxy_call_no_reply(dkp_main_proxy,
	                           type,
	                           G_TYPE_INVALID,
	                           G_TYPE_INVALID);

	return;
}

/* A response to getting the suspend property */
static void
suspend_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
	suspend_call = NULL;

	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check suspend: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Suspend: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	if (suspend_mi != NULL) {
		dbusmenu_menuitem_property_set(suspend_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, g_value_get_boolean(&candoit) ? "true" : "false");
	}

	return;
}

/* Response to getting the hibernate property */
static void
hibernate_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
	hibernate_call = NULL;

	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check hibernate: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Hibernate: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	if (suspend_mi != NULL) {
		dbusmenu_menuitem_property_set(hibernate_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, g_value_get_boolean(&candoit) ? "true" : "false");
	}

	return;
}

/* A signal that we need to recheck to ensure we can still
   hibernate and/or suspend */
static void
dpk_changed_cb (DBusGProxy * proxy, gpointer user_data)
{
	/* Start Async call to see if we can hibernate */
	if (suspend_call == NULL) {
		suspend_call = dbus_g_proxy_begin_call(dkp_prop_proxy,
		                                       "Get",
		                                       suspend_prop_cb,
		                                       NULL,
		                                       NULL,
		                                       G_TYPE_STRING,
		                                       DKP_INTERFACE,
		                                       G_TYPE_STRING,
		                                       "can-suspend",
		                                       G_TYPE_INVALID,
		                                       G_TYPE_VALUE,
		                                       G_TYPE_INVALID);
	}

	/* Start Async call to see if we can suspend */
	if (hibernate_call == NULL) {
		hibernate_call = dbus_g_proxy_begin_call(dkp_prop_proxy,
		                                         "Get",
		                                         hibernate_prop_cb,
		                                         NULL,
		                                         NULL,
		                                         G_TYPE_STRING,
		                                         DKP_INTERFACE,
		                                         G_TYPE_STRING,
		                                         "can-hibernate",
		                                         G_TYPE_INVALID,
		                                         G_TYPE_VALUE,
		                                         G_TYPE_INVALID);
	}

	return;
}

/* This function goes through and sets up what we need for
   DKp checking.  We're even setting up the calls for the props
   we need */
static void
setup_dkp (void) {
	DBusGConnection * bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	g_return_if_fail(bus != NULL);

	if (dkp_main_proxy == NULL) {
		dkp_main_proxy = dbus_g_proxy_new_for_name(bus,
		                                           DKP_ADDRESS,
		                                           DKP_OBJECT,
		                                           DKP_INTERFACE);
	}
	g_return_if_fail(dkp_main_proxy != NULL);

	if (dkp_prop_proxy == NULL) {
		dkp_prop_proxy = dbus_g_proxy_new_for_name(bus,
		                                           DKP_ADDRESS,
		                                           DKP_OBJECT,
		                                           DBUS_INTERFACE_PROPERTIES);
	}
	g_return_if_fail(dkp_prop_proxy != NULL);

	/* Connect to changed signal */
	dbus_g_proxy_add_signal(dkp_main_proxy,
	                        "Changed",
	                        G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(dkp_main_proxy,
	                            "Changed",
	                            G_CALLBACK(dpk_changed_cb),
	                            NULL,
	                            NULL);

	/* Force an original "changed" event */
	dpk_changed_cb(dkp_main_proxy, NULL);

	return;
}

/* This is the function to show a dialog on actions that
   can destroy data.  Currently it just calls the GTK version
   but it seems that in the future it should figure out
   what's going on and something better. */
static void
show_dialog (DbusmenuMenuitem * mi, gchar * type)
{
	gchar * helper = g_build_filename(LIBEXECDIR, "gtk-logout-helper", NULL);
	gchar * dialog_line = g_strdup_printf("%s --%s", helper, type);
	g_free(helper);

	g_debug("Showing dialog '%s'", dialog_line);

	GError * error = NULL;
	if (!g_spawn_command_line_async(dialog_line, &error)) {
		g_warning("Unable to show dialog: %s", error->message);
		g_error_free(error);
	}

	g_free(dialog_line);

	return;
}

/* This function creates all of the menuitems that the service
   provides in the UI.  It also connects them to the callbacks. */
static void
create_items (DbusmenuMenuitem * root) {
	logout_mi = dbusmenu_menuitem_new();
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out"));
	} else {
		dbusmenu_menuitem_property_set(logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out..."));
	}
	dbusmenu_menuitem_child_append(root, logout_mi);
	g_signal_connect(G_OBJECT(logout_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "logout");

	suspend_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(suspend_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, "false");
	dbusmenu_menuitem_property_set(suspend_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Suspend"));
	dbusmenu_menuitem_child_append(root, suspend_mi);
	g_signal_connect(G_OBJECT(suspend_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(sleep), "Suspend");

	hibernate_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(hibernate_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, "false");
	dbusmenu_menuitem_property_set(hibernate_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Hibernate"));
	dbusmenu_menuitem_child_append(root, hibernate_mi);
	g_signal_connect(G_OBJECT(hibernate_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(sleep), "Hibernate");

	restart_mi = dbusmenu_menuitem_new();
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(restart_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Restart"));
	} else {
		dbusmenu_menuitem_property_set(restart_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Restart..."));
	}
	dbusmenu_menuitem_child_append(root, restart_mi);
	g_signal_connect(G_OBJECT(restart_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "restart");

	shutdown_mi = dbusmenu_menuitem_new();
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down"));
	} else {
		dbusmenu_menuitem_property_set(shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down..."));
	}
	dbusmenu_menuitem_child_append(root, shutdown_mi);
	g_signal_connect(G_OBJECT(shutdown_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "shutdown");

	RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi = g_new0 (RestartShutdownLogoutMenuItems, 1);
	restart_shutdown_logout_mi->logout_mi = logout_mi;
	restart_shutdown_logout_mi->restart_mi = restart_mi;
	restart_shutdown_logout_mi->shutdown_mi = shutdown_mi;

	update_menu_entries(restart_shutdown_logout_mi);

	return;
}

/* Main, is well, main.  It brings everything up and throws
   us into the mainloop of no return. */
int
main (int argc, char ** argv)
{
    g_type_init();

	/* Setting up i18n and gettext.  Apparently, we need
	   all of these. */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

    DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
    DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    GError * error = NULL;
    guint nameret = 0;

    if (!org_freedesktop_DBus_request_name(bus_proxy, INDICATOR_SESSION_DBUS_NAME, 0, &nameret, &error)) {
        g_error("Unable to call to request name");
        return 1;
    }   

    if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_error("Unable to get name");
        return 1;
    }   

	g_idle_add(lock_screen_setup, NULL);

    root_menuitem = dbusmenu_menuitem_new();
	g_debug("Root ID: %d", dbusmenu_menuitem_get_id(root_menuitem));

	create_items(root_menuitem);
	setup_dkp();

    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_SESSION_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

