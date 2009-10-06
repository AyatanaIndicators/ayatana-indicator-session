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

#include <config.h>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"

#include "status-service-dbus.h"

#include "status-provider.h"
#include "status-provider-mc5.h"
#include "status-provider-pidgin.h"
#include "status-provider-telepathy.h"

typedef StatusProvider * (*newfunc) (void);
#define STATUS_PROVIDER_CNT   3
static newfunc status_provider_newfuncs[STATUS_PROVIDER_CNT] = {
	status_provider_mc5_new,
	status_provider_pidgin_new,
	status_provider_telepathy_new
};
static StatusProvider * status_providers[STATUS_PROVIDER_CNT] = { 0 };

static const gchar * status_strings [STATUS_PROVIDER_STATUS_LAST] = {
  /* STATUS_PROVIDER_STATUS_ONLINE,    */ N_("Available"),
  /* STATUS_PROVIDER_STATUS_AWAY,      */ N_("Away"),
  /* STATUS_PROVIDER_STATUS_DND        */ N_("Busy"),
  /* STATUS_PROVIDER_STATUS_INVISIBLE  */ N_("Invisible"),
  /* STATUS_PROVIDER_STATUS_OFFLINE,   */ N_("Offline"),
  /* STATUS_PROVIDER_STATUS_DISCONNECTED*/ N_("Offline")
};

static const gchar * status_icons[STATUS_PROVIDER_STATUS_LAST] = {
  /* STATUS_PROVIDER_STATUS_ONLINE, */     "user-available",
  /* STATUS_PROVIDER_STATUS_AWAY, */       "user-away",
  /* STATUS_PROVIDER_STATUS_DND, */        "user-busy",
  /* STATUS_PROVIDER_STATUS_INVISIBLE, */  "user-invisible",
  /* STATUS_PROVIDER_STATUS_OFFLINE */     "user-offline",
  /* STATUS_PROVIDER_STATUS_DISCONNECTED */"system-shutdown"
};


static DbusmenuMenuitem * root_menuitem = NULL;
static DbusmenuMenuitem * status_menuitem = NULL;
static DbusmenuMenuitem * status_menuitems[STATUS_PROVIDER_STATUS_LAST] = {0};
static GMainLoop * mainloop = NULL;
static StatusServiceDbus * dbus_interface = NULL;
static StatusProviderStatus global_status = STATUS_PROVIDER_STATUS_DISCONNECTED;

static void
status_update (void) {
	StatusProviderStatus oldglobal = global_status;
	global_status = STATUS_PROVIDER_STATUS_DISCONNECTED;

	/* Ask everyone what they think the status should be, if
	   they're more connected, up the global level */
	int i;
	for (i = 0; i < STATUS_PROVIDER_CNT; i++) {
		StatusProviderStatus localstatus = status_provider_get_status(status_providers[i]);
		if (localstatus < global_status) {
			global_status = localstatus;
		}
	}

	/* If changed */
	if (global_status != oldglobal) {
		g_debug("Global status changed to: %s", _(status_strings[global_status]));

		/* Configure the icon on the panel */
		status_service_dbus_set_status(dbus_interface, status_icons[global_status]);

		/* If we're now disconnected, make setting the statuses
		   insensitive. */
		if (global_status == STATUS_PROVIDER_STATUS_DISCONNECTED) {
			StatusProviderStatus i;
			for (i = STATUS_PROVIDER_STATUS_ONLINE; i < STATUS_PROVIDER_STATUS_LAST; i++) {
				if (status_menuitems[i] == NULL) continue;
				dbusmenu_menuitem_property_set(status_menuitems[i], DBUSMENU_MENUITEM_PROP_SENSITIVE, "false");
			}
		}

		/* If we're now back to a state where we have an IM client
		   connected then we need to resensitize the items. */
		if (oldglobal == STATUS_PROVIDER_STATUS_DISCONNECTED) {
			StatusProviderStatus i;
			for (i = STATUS_PROVIDER_STATUS_ONLINE; i < STATUS_PROVIDER_STATUS_LAST; i++) {
				if (status_menuitems[i] == NULL) continue;
				dbusmenu_menuitem_property_set(status_menuitems[i], DBUSMENU_MENUITEM_PROP_SENSITIVE, "true");
			}
		}
	}

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

		if (status_providers[i] != NULL) {
			g_signal_connect(G_OBJECT(status_providers[i]), STATUS_PROVIDER_SIGNAL_STATUS_CHANGED, G_CALLBACK(status_update), NULL);
		}
	}

	status_update();

	return FALSE;
}

static void
build_user_item (DbusmenuMenuitem * root)
{
	struct passwd * pwd = NULL;

	pwd = getpwuid(getuid());

	if (pwd != NULL && pwd->pw_gecos != NULL) {
		gchar * name = g_strdup(pwd->pw_gecos);
		gchar * walker = name;
		while (*walker != '\0' && *walker != ',') { walker++; }
		*walker = '\0';

		if (name[0] != '\0') {
			DbusmenuMenuitem * useritem = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(useritem, DBUSMENU_MENUITEM_PROP_LABEL, name);
			dbusmenu_menuitem_property_set(useritem, DBUSMENU_MENUITEM_PROP_SENSITIVE, "false");
			dbusmenu_menuitem_child_append(root, useritem);
		}

		g_free(name);
	} else {
		g_debug("PWD: %s", (pwd == NULL ? "(pwd null)" : (pwd->pw_gecos == NULL ? "(gecos null)" : pwd->pw_gecos)));
	}

	return;
}

static gboolean
build_menu (gpointer data)
{
	DbusmenuMenuitem * root = DBUSMENU_MENUITEM(data);
	g_return_val_if_fail(root != NULL, FALSE);

	build_user_item(root);

	status_menuitem = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(status_menuitem, DBUSMENU_MENUITEM_PROP_LABEL, _("Set Status"));
	dbusmenu_menuitem_child_append(root, status_menuitem);

	StatusProviderStatus i;
	for (i = STATUS_PROVIDER_STATUS_ONLINE; i < STATUS_PROVIDER_STATUS_LAST; i++) {
		if (i == STATUS_PROVIDER_STATUS_DISCONNECTED) {
			/* We don't want an item for the disconnected status.  Users
			   can't set that value through the menu :) */
			continue;
		}

		status_menuitems[i] = dbusmenu_menuitem_new();

		dbusmenu_menuitem_property_set(status_menuitems[i], "type", DBUSMENU_CLIENT_TYPES_IMAGE);
		dbusmenu_menuitem_property_set(status_menuitems[i], DBUSMENU_MENUITEM_PROP_LABEL, _(status_strings[i]));
		dbusmenu_menuitem_property_set(status_menuitems[i], DBUSMENU_MENUITEM_PROP_ICON, status_icons[i]);
		if (global_status == STATUS_PROVIDER_STATUS_DISCONNECTED) {
			dbusmenu_menuitem_property_set(status_menuitems[i], DBUSMENU_MENUITEM_PROP_SENSITIVE, "false");
		}
		g_signal_connect(G_OBJECT(status_menuitems[i]), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(status_menu_click), GINT_TO_POINTER(i));

		dbusmenu_menuitem_child_append(status_menuitem, status_menuitems[i]);

		g_debug("Built %s", status_strings[i]);
	}

	return FALSE;
}

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

	dbus_interface = g_object_new(STATUS_SERVICE_DBUS_TYPE, NULL);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

