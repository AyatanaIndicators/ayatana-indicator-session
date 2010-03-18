/*
A small wrapper utility for connecting to gconf.

Copyright 2009 Canonical Ltd.

Authors:
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


#include <gconf/gconf-client.h>

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"
#include "gconf-helper.h"

static GConfClient * gconf_client = NULL;
static guint confirmation_notify = 0;
static guint logout_notify = 0;

gboolean
supress_confirmations (void) {
	if(!gconf_client) {
		gconf_client = gconf_client_get_default ();
	}
	return gconf_client_get_bool (gconf_client, SUPPRESS_KEY, NULL) ;
}

gboolean
show_logout (void) {
	if(!gconf_client) {
		gconf_client = gconf_client_get_default ();
	}
	return !gconf_client_get_bool (gconf_client, LOGOUT_KEY, NULL) ;
}

static void update_menu_entries_callback (GConfClient *client, guint cnxn_id, GConfEntry  *entry, gpointer data) {
	RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi = (RestartShutdownLogoutMenuItems*) data;
	GConfValue * value = gconf_entry_get_value (entry);
	const gchar * key = gconf_entry_get_key (entry);

	if(g_strcmp0 (key, SUPPRESS_KEY) == 0) {
		if (gconf_value_get_bool (value)) {
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->restart_mi, RESTART_ITEM_LABEL, _("Restart"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down"));
		} else {
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out..."));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->restart_mi, RESTART_ITEM_LABEL, _("Restart..."));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down..."));
		}
	}
}

static void
update_logout_callback (GConfClient *client, guint cnxn_id, GConfEntry  *entry, gpointer data) {
	DbusmenuMenuitem * mi = (DbusmenuMenuitem*) data;
	GConfValue * value = gconf_entry_get_value (entry);
	const gchar * key = gconf_entry_get_key (entry);

	if(g_strcmp0 (key, LOGOUT_KEY) == 0) {
		dbusmenu_menuitem_property_set_bool(mi, DBUSMENU_MENUITEM_PROP_VISIBLE, !gconf_value_get_bool(value));
	}
}

void
update_menu_entries(RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi, DbusmenuMenuitem * logoutitem) {
	/* If we don't have a client, build one. */
	if(!gconf_client) {
		gconf_client = gconf_client_get_default ();
	}

	/* If we've not gotten any notifications, then we need
	   to add the directory for notifications to come from. */
	if (confirmation_notify == 0 || logout_notify == 0) {
		gconf_client_add_dir (gconf_client, GLOBAL_DIR,
		                      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	}

	if (confirmation_notify != 0) {
		gconf_client_notify_remove (gconf_client, confirmation_notify);
		confirmation_notify = 0;
	}

	if (logout_notify != 0) {
		gconf_client_notify_remove (gconf_client, logout_notify);
		logout_notify = 0;
	}

	confirmation_notify = gconf_client_notify_add (gconf_client, SUPPRESS_KEY,
				update_menu_entries_callback, restart_shutdown_logout_mi, NULL, NULL);
	logout_notify = gconf_client_notify_add (gconf_client, LOGOUT_KEY,
				update_logout_callback, logoutitem, NULL, NULL);
}

