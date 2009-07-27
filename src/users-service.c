
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

#define GUEST_SESSION_LAUNCHER  "/usr/share/gdm/guest-session/guest-session-launch"

static DBusGConnection * session_bus = NULL;
static DBusGConnection * system_bus = NULL;
static DBusGProxy * bus_proxy = NULL;
static DBusGProxy * gdm_proxy = NULL;
static DbusmenuMenuitem * root_menuitem = NULL;
static GMainLoop * mainloop = NULL;

static gboolean
check_guest_session (void)
{
	if (geteuid() < 500) {
		/* System users shouldn't have guest account shown.  Mosly
		   this would be the case of the guest user itself. */
		return FALSE;
	}
	if (!g_file_test(GUEST_SESSION_LAUNCHER, G_FILE_TEST_IS_EXECUTABLE)) {
		/* It doesn't appear that the Guest session stuff is
		   installed.  So let's not use it then! */
		return FALSE;
	}

	return TRUE;
}

static void
activate_guest_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async(GUEST_SESSION_LAUNCHER, &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

static gboolean
check_new_session (void)
{
	if (system_bus == NULL) {
		system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	}

	if (system_bus == NULL) {
		return FALSE;
	}

	if (gdm_proxy == NULL) {
		gdm_proxy = dbus_g_proxy_new_for_name(system_bus,
		                                      "org.gnome.DisplayManager",
		                                      "/org/gnome/DisplayManager/LocalDisplayFactory",
		                                      "org.gnome.DisplayManager.LocalDisplayFactory");
	}

	if (gdm_proxy == NULL) {
		return FALSE;
	}

	return TRUE;
}

static void
activate_new_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async("gdmflexiserver --startnew", &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

static void
create_items (DbusmenuMenuitem * root) {
	DbusmenuMenuitem * mi = NULL;

	if (check_guest_session()) {
		mi = dbusmenu_menuitem_new();
		dbusmenu_menuitem_property_set(mi, "label", _("Guest Session"));
		dbusmenu_menuitem_child_append(root, mi);
		g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(activate_guest_session), NULL);
	}

	if (check_new_session()) {
		mi = dbusmenu_menuitem_new();
		dbusmenu_menuitem_property_set(mi, "label", _("New Session..."));
		dbusmenu_menuitem_child_append(root, mi);
		g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(activate_new_session), NULL);
	}

	return;
}

int
main (int argc, char ** argv)
{
    g_type_init();

    session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
    bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
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

