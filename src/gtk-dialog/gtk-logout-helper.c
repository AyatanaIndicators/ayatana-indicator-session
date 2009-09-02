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


#include <glib.h>
#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>
#include "logout-dialog.h"
#include "ck-pk-helper.h"
#include "gconf-helper.h"

static void
session_action (LogoutDialogAction action)
{
	DBusGConnection * sbus;
	DBusGProxy * sm_proxy;
	GError * error = NULL;
	gboolean res = FALSE;
	
	sbus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL); 
	if (sbus == NULL) {
		g_warning("Unable to get DBus session bus.");
		return;
	}
	sm_proxy = dbus_g_proxy_new_for_name_owner (sbus,
                                            	"org.gnome.SessionManager",
                                            	"/org/gnome/SessionManager",
                                            	"org.gnome.SessionManager",
                                           	 	&error);
	if (sm_proxy == NULL) {
		g_warning("Unable to get DBus proxy to SessionManager interface: %s", error->message);
		g_error_free(error);
		return;
	}		
	
	g_clear_error (&error);
	
	if (action == LOGOUT_DIALOG_LOGOUT) {
		res = dbus_g_proxy_call_with_timeout (sm_proxy, "Logout", INT_MAX, &error, 
											  G_TYPE_UINT, 1, G_TYPE_INVALID, G_TYPE_INVALID);
	} else if (action == LOGOUT_DIALOG_SHUTDOWN) {
		res = dbus_g_proxy_call_with_timeout (sm_proxy, "RequestShutdown", INT_MAX, &error, 
											  G_TYPE_INVALID, G_TYPE_INVALID);
	} else if (action == LOGOUT_DIALOG_RESTART) {
		res = dbus_g_proxy_call_with_timeout (sm_proxy, "RequestReboot", INT_MAX, &error, 
											  G_TYPE_INVALID, G_TYPE_INVALID);
	} else {
		g_warning ("Unknown session action");
	}
	
	if (!res) {
		if (error != NULL) {
			g_warning ("SessionManager action failed: %s", error->message);
		} else {
			g_warning ("SessionManager action failed: unknown error");
		}
	}
	
	g_object_unref(sm_proxy);
	
	if (error != NULL) {
		g_error_free(error);
	}
	
	return;
}	

static LogoutDialogAction type = LOGOUT_DIALOG_LOGOUT;

static gboolean
option_logout (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_LOGOUT;
	return TRUE;
}

static gboolean
option_shutdown (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_SHUTDOWN;
	return TRUE;
}

static gboolean
option_restart (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_RESTART;
	return TRUE;
}

static GOptionEntry options[] = {
	{"logout",     'l',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_logout,   "Log out of the current session",   NULL},
	{"shutdown",   's',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_shutdown, "Shutdown the entire system",       NULL},
	{"restart",    'r',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_restart,  "Restart the system",               NULL},

	{NULL}
};

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	GError * error = NULL;
	GOptionContext * context = g_option_context_new(" - logout of the current session");
	g_option_context_add_main_entries(context, options, "gtk-logout-helper");
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_set_help_enabled(context, TRUE);

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_debug("Option parsing failed: %s", error->message);
		g_error_free(error);
		return 1;
	}

	GtkWidget * dialog = NULL;
	if (!pk_require_auth(type) && !supress_confirmations()) {
		dialog = logout_dialog_new(type);
	}

	if (dialog != NULL) {
		GtkResponseType response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_hide(dialog);

		if (response == GTK_RESPONSE_HELP) {
			type = LOGOUT_DIALOG_RESTART;
			response = GTK_RESPONSE_OK;
		}

		if (response != GTK_RESPONSE_OK) {
			return 0;
		}
	}

	session_action(type);

	return 0;
}
