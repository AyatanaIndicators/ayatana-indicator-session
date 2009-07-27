
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

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

/* Let's put this machine to sleep, with some info on how
   it should sleep.  */
static void
sleep (DbusmenuMenuitem * mi, gpointer userdata)
{
	gchar * type = (gchar *)userdata;

	if (dkp_main_proxy == NULL) {
		g_warning("Can not %s as no DeviceKit Power Proxy", type);
	}

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
		dbusmenu_menuitem_property_set(suspend_mi, "visible", g_value_get_boolean(&candoit) ? "true" : "false");
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
		dbusmenu_menuitem_property_set(hibernate_mi, "visible", g_value_get_boolean(&candoit) ? "true" : "false");
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
	DbusmenuMenuitem * mi = NULL;

	mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("Log Out"));
	dbusmenu_menuitem_child_append(root, mi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "logout");

	suspend_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(suspend_mi, "visible", "false");
	dbusmenu_menuitem_property_set(suspend_mi, "label", _("Suspend"));
	dbusmenu_menuitem_child_append(root, suspend_mi);
	g_signal_connect(G_OBJECT(suspend_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(sleep), "Suspend");

	hibernate_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(hibernate_mi, "visible", "false");
	dbusmenu_menuitem_property_set(hibernate_mi, "label", _("Hibernate"));
	dbusmenu_menuitem_child_append(root, hibernate_mi);
	g_signal_connect(G_OBJECT(hibernate_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(sleep), "Hibernate");

	mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("Restart"));
	dbusmenu_menuitem_child_append(root, mi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "restart");

	mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("Shutdown"));
	dbusmenu_menuitem_child_append(root, mi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "shutdown");

	return;
}

/* Main, is well, main.  It brings everything up and throws
   us into the mainloop of no return. */
int
main (int argc, char ** argv)
{
    g_type_init();

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

