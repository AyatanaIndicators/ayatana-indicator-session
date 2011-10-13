/*
A small wrapper utility for connecting to GSettings.

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

#include <gio/gio.h>
#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"
#include "settings-helper.h"

static GSettings* settings = NULL;

static guint confirmation_notify = 0;
static guint logout_notify = 0;
static guint restart_notify = 0;
static guint shutdown_notify = 0;

static gboolean
build_settings (void) {
	if (settings == NULL) {
		settings = g_settings_new (SESSION_SCHEMA);
	}
	if (settings == NULL) {
		return FALSE;
	}
	return TRUE;
}

gboolean
supress_confirmations (void) {
	gboolean settings_built = build_settings();
	g_return_val_if_fail(settings_built, FALSE);
	return g_settings_get_boolean (settings, SUPPRESS_KEY) ;
}

gboolean
should_show_user_menu (void) {
	gboolean settings_built = build_settings();
	g_return_val_if_fail(settings_built, TRUE);
	return g_settings_get_boolean (settings, SHOW_USER_MENU) ;
}

gboolean
show_logout (void) {
	gboolean settings_built = build_settings();
	g_return_val_if_fail(settings_built, TRUE);
	return !g_settings_get_boolean (settings, LOGOUT_KEY) ;
}

gboolean
show_restart (void) {
	gboolean settings_built = build_settings();
	g_return_val_if_fail(settings_built, TRUE);
	return !g_settings_get_boolean (settings, RESTART_KEY) ;
}

gboolean
show_shutdown (void) {
	gboolean settings_built = build_settings();
	g_return_val_if_fail(settings_built, TRUE);
	return !g_settings_get_boolean (settings, SHUTDOWN_KEY) ;
}

static void update_menu_entries_callback (GSettings * settings, const gchar * key, gpointer data) {
	RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi = (RestartShutdownLogoutMenuItems*) data;

	if(g_strcmp0 (key, SUPPRESS_KEY) == 0) {
		if (g_settings_get_boolean (settings, key)) {
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->restart_mi, RESTART_ITEM_LABEL, _("Restart"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down"));
		} else {
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out…"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->restart_mi, RESTART_ITEM_LABEL, _("Restart…"));
			dbusmenu_menuitem_property_set(restart_shutdown_logout_mi->shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Shut Down…"));
		}
	}
}

static void
update_logout_callback (GSettings * settings, const gchar * key, gpointer data) {
	DbusmenuMenuitem * mi = (DbusmenuMenuitem*) data;

	if(g_strcmp0 (key, LOGOUT_KEY) == 0) {
		dbusmenu_menuitem_property_set_bool(mi, DBUSMENU_MENUITEM_PROP_VISIBLE, !g_settings_get_boolean(settings, key));
	}
}

static void
update_restart_callback (GSettings * settings, const gchar * key, gpointer data) {
	DbusmenuMenuitem * mi = (DbusmenuMenuitem*) data;

	if(g_strcmp0 (key, RESTART_KEY) == 0) {
		dbusmenu_menuitem_property_set_bool(mi, DBUSMENU_MENUITEM_PROP_VISIBLE, !g_settings_get_boolean(settings, key));
	}
}

static void
update_shutdown_callback (GSettings * settings, const gchar * key, gpointer data) {
	DbusmenuMenuitem * mi = (DbusmenuMenuitem*) data;

	if(g_strcmp0 (key, SHUTDOWN_KEY) == 0) {
		dbusmenu_menuitem_property_set_bool(mi, DBUSMENU_MENUITEM_PROP_VISIBLE, !g_settings_get_boolean(settings, key));
	}
}

void
update_menu_entries(RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi) {
	/* If we don't have a client, build one. */
	gboolean settings_built = build_settings();
	g_return_if_fail(settings_built);

	if (confirmation_notify != 0) {
		g_signal_handler_disconnect (settings, confirmation_notify);
		confirmation_notify = 0;
	}

	if (logout_notify != 0) {
		g_signal_handler_disconnect (settings, logout_notify);
		logout_notify = 0;
	}

	if (restart_notify != 0) {
		g_signal_handler_disconnect (settings, restart_notify);
		restart_notify = 0;
	}

	if (shutdown_notify != 0) {
		g_signal_handler_disconnect (settings, shutdown_notify);
		shutdown_notify = 0;
	}

	confirmation_notify = g_signal_connect (settings, "changed::" SUPPRESS_KEY,
				G_CALLBACK(update_menu_entries_callback), restart_shutdown_logout_mi);
	logout_notify = g_signal_connect (settings, "changed::" LOGOUT_KEY,
				G_CALLBACK(update_logout_callback), restart_shutdown_logout_mi->logout_mi);
	restart_notify = g_signal_connect (settings, "changed::" RESTART_KEY,
				G_CALLBACK(update_restart_callback), restart_shutdown_logout_mi->restart_mi);
	shutdown_notify = g_signal_connect (settings, "changed::" SHUTDOWN_KEY,
				G_CALLBACK(update_shutdown_callback), restart_shutdown_logout_mi->shutdown_mi);

	return;
}

