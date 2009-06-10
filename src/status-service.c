
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

typedef enum
{
  STATUS_SERVICE_STATUS_ONLINE,
  STATUS_SERVICE_STATUS_AWAY,
  STATUS_SERVICE_STATUS_DND,
  STATUS_SERVICE_STATUS_OFFLINE,
  /* Leave as last */
  STATUS_SERVICE_STATUS_LAST
}
StatusServiceStatus;

static const gchar * status_strings [STATUS_SERVICE_STATUS_LAST] = {
  /* STATUS_SERVICE_STATUS_ONLINE,   */ N_("Available"),
  /* STATUS_SERVICE_STATUS_AWAY,     */ N_("Away"),
  /* STATUS_SERVICE_STATUS_DND       */ N_("Busy"),
  /* STATUS_SERVICE_STATUS_OFFLINE,  */ N_("Offline")
};

static const gchar * status_icons[STATUS_SERVICE_STATUS_LAST] = {
  /* STATUS_SERVICE_STATUS_ONLINE, */ "user-online",
  /* STATUS_SERVICE_STATUS_AWAY, */   "user-away",
  /* STATUS_SERVICE_STATUS_DND, */    "user-busy",
  /* STATUS_SERVICE_STATUS_OFFLINE */ "user-offline"
};


static DbusmenuMenuitem * root_menuitem = NULL;
static GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
    g_type_init();

    DBusGConnection * connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
    DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    GError * error = NULL;
    guint nameret = 0;

    if (!org_freedesktop_DBus_request_name(bus_proxy, INDICATOR_STATUS_DBUS_NAME, 0, &nameret, &error)) {
        g_error("Unable to call to request name");
        return 1;
    }   

    if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_error("Unable to get name");
        return 1;
    }   

    root_menuitem = dbusmenu_menuitem_new();
    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_STATUS_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

