
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

#include "status-service-dbus.h"

#include "status-provider.h"
#include "status-provider-pidgin.h"

typedef StatusProvider * (*newfunc) (void);
#define STATUS_PROVIDER_CNT   1
static newfunc status_provider_newfuncs[STATUS_PROVIDER_CNT] = {
	status_provider_pidgin_new
};
static StatusProvider * status_providers[STATUS_PROVIDER_CNT] = { 0 };

static const gchar * status_strings [STATUS_PROVIDER_STATUS_LAST] = {
  /* STATUS_PROVIDER_STATUS_ONLINE,    */ N_("Available"),
  /* STATUS_PROVIDER_STATUS_AWAY,      */ N_("Away"),
  /* STATUS_PROVIDER_STATUS_DND        */ N_("Busy"),
  /* STATUS_PROVIDER_STATUS_INVISIBLE  */ N_("Invisible"),
  /* STATUS_PROVIDER_STATUS_OFFLINE,   */ N_("Offline")
};

static const gchar * status_icons[STATUS_PROVIDER_STATUS_LAST] = {
  /* STATUS_PROVIDER_STATUS_ONLINE, */     "user-online",
  /* STATUS_PROVIDER_STATUS_AWAY, */       "user-away",
  /* STATUS_PROVIDER_STATUS_DND, */        "user-busy",
  /* STATUS_PROVIDER_STATUS_INVISIBLE, */  "user-invisible",
  /* STATUS_PROVIDER_STATUS_OFFLINE */     "user-offline"
};


static DbusmenuMenuitem * root_menuitem = NULL;
static DbusmenuMenuitem * status_menuitem = NULL;
static GMainLoop * mainloop = NULL;

/* A fun little function to actually lock the screen.  If,
   that's what you want, let's do it! */
static void
lock_screen (DbusmenuMenuitem * mi, gpointer data)
{
	g_debug("Lock Screen");

	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	g_return_if_fail(session_bus != NULL);

	DBusGProxy * proxy = dbus_g_proxy_new_for_name_owner(session_bus,
	                                                     "org.gnome.ScreenSaver",
	                                                     "/",
	                                                     "org.gnome.ScreenSaver",
	                                                     NULL);
	g_return_if_fail(proxy != NULL);

	dbus_g_proxy_call_no_reply(proxy,
	                           "Lock",
	                           G_TYPE_INVALID,
	                           G_TYPE_INVALID);

	g_object_unref(proxy);

	return;
}

static void
status_menu_click (DbusmenuMenuitem * mi, gpointer data)
{
	StatusProviderStatus status = (StatusProviderStatus)GPOINTER_TO_INT(data);
	g_debug("Setting status: %d", status);
	int i;
	for (i = 0; i < STATUS_PROVIDER_CNT; i++) {
		status_provider_set_status(status_providers[i], status);
	}

	return;
}

static gboolean
build_providers (gpointer data)
{
	int i;
	for (i = 0; i < STATUS_PROVIDER_CNT; i++) {
		status_providers[i] = status_provider_newfuncs[i]();
	}

	return FALSE;
}

static gboolean
build_menu (gpointer data)
{
	DbusmenuMenuitem * root = DBUSMENU_MENUITEM(data);
	g_return_val_if_fail(root != NULL, FALSE);

	status_menuitem = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(status_menuitem, "label", "Status");
	dbusmenu_menuitem_child_append(root, status_menuitem);

	StatusProviderStatus i;
	for (i = STATUS_PROVIDER_STATUS_ONLINE; i < STATUS_PROVIDER_STATUS_LAST; i++) {
		DbusmenuMenuitem * mi = dbusmenu_menuitem_new();

		dbusmenu_menuitem_property_set(mi, "label", _(status_strings[i]));
		dbusmenu_menuitem_property_set(mi, "icon", status_icons[i]);
		g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(status_menu_click), GINT_TO_POINTER(i));

		dbusmenu_menuitem_child_append(status_menuitem, mi);

		g_debug("Built %s", status_strings[i]);
	}

	DbusmenuMenuitem * mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(mi, "label", _("Lock Screen"));
	g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(lock_screen), GINT_TO_POINTER(i));
	dbusmenu_menuitem_child_append(root, mi);

	return FALSE;
}

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

	g_idle_add(build_providers, NULL);

    root_menuitem = dbusmenu_menuitem_new();
    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_STATUS_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

	g_idle_add(build_menu, root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

