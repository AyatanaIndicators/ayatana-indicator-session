
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

static DbusmenuMenuitem * root_menuitem = NULL;
static GMainLoop * mainloop = NULL;

static void
activate_guest_session (DbusmenuMenuitem * mi, gpointer user_data)
{

}

static void
activate_new_session (DbusmenuMenuitem * mi, gpointer user_data)
{

}

static void
create_items (DbusmenuMenuitem * root) {
	DbusmenuMenuitem * mi = NULL;

	mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("Guest Session"));
	dbusmenu_menuitem_child_append(root, mi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(activate_guest_session), NULL);

	mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("New Session..."));
	dbusmenu_menuitem_child_append(root, mi);
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(activate_new_session), NULL);

	return;
}

int
main (int argc, char ** argv)
{
    g_type_init();

    DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
    DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    GError * error = NULL;
    guint nameret = 0;

    if (!org_freedesktop_DBus_request_name(bus_proxy, INDICATOR_USERS_DBUS_NAME, 0, &nameret, &error)) {
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

    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_USERS_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

