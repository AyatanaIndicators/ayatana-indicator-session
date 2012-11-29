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

#include "config.h"

#include <locale.h>
#include <glib.h>
#include <glib/gi18n.h> /* textdomain(), bindtextdomain() */
#include <gtk/gtk.h>
#include "dialog.h"
#include "shared-names.h"

static GVariant *
call_console_kit (const gchar *method, GVariant *parameters, GError **error)
{
	GDBusConnection * bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error);
	if (!bus)
	{
		g_variant_unref (parameters);
		return NULL;
	}

	GVariant *result = g_dbus_connection_call_sync(bus,
	                                               "org.freedesktop.ConsoleKit",
	                                               "/org/freedesktop/ConsoleKit/Manager",
	                                               "org.freedesktop.ConsoleKit.Manager",
	                                               method,
	                                               parameters,
	                                               NULL,
	                                               G_DBUS_CALL_FLAGS_NONE,
	                                               -1,
	                                               NULL,
	                                               error);
	g_object_unref (bus);

	return result;
}

static void
consolekit_fallback (LogoutDialogType action)
{
	GError * error = NULL;
	GVariant *result = NULL;

	g_debug("Falling back to using ConsoleKit for action");

	switch (action) {
		case LOGOUT_DIALOG_TYPE_LOG_OUT:
			g_warning("Unable to fallback to ConsoleKit for logout as it's a session issue.  We need some sort of session handler.");
			break;
		case LOGOUT_DIALOG_TYPE_SHUTDOWN:
			g_debug("Telling ConsoleKit to 'Stop'");
			result = call_console_kit ("Stop", g_variant_new ("()"), &error);
			break;
		case LOGOUT_DIALOG_TYPE_RESTART:
			g_debug("Telling ConsoleKit to 'Restart'");
			result = call_console_kit ("Restart", g_variant_new ("()"), &error);
			break;
		default:
			g_warning("Unknown action");
			break;
	}

	if (!result) {
		if (error != NULL) {
			g_warning ("ConsoleKit action failed: %s", error->message);
		} else {
			g_warning ("ConsoleKit action failed: unknown error");
		}

		consolekit_fallback(action);
	}
	else
		g_variant_unref (result);
	g_clear_error (&error);

	return;
}

static GVariant *
call_gnome_session (const gchar *method, GVariant *parameters, GError **error)
{
	GDBusConnection * bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, error);
	if (!bus)
	{
		g_variant_unref (parameters);
		return NULL;
	}
  
	GVariant *result = g_dbus_connection_call_sync(bus,
	                                               "org.gnome.SessionManager",
	                                               "/org/gnome/SessionManager",
	                                               "org.gnome.SessionManager",
	                                               method,
	                                               parameters,
	                                               NULL,
	                                               G_DBUS_CALL_FLAGS_NONE,
	                                               G_MAXINT,
	                                               NULL,
	                                               error);
	g_object_unref (bus);

	return result;
}

static void
session_action (LogoutDialogType action)
{
	GError * error = NULL;
	GVariant *result = NULL;

	if (action == LOGOUT_DIALOG_TYPE_LOG_OUT) {
		g_debug("Asking Session manager to 'Logout'");
		result = call_gnome_session ("Logout", g_variant_new ("(u)", 1), &error);
	} else if (action == LOGOUT_DIALOG_TYPE_SHUTDOWN) {
		g_debug("Asking Session manager to 'RequestShutdown'");
		result = call_gnome_session ("RequestShutdown", g_variant_new ("()"), &error);
	} else if (action == LOGOUT_DIALOG_TYPE_RESTART) {
		g_debug("Asking Session manager to 'RequestReboot'");
		result = call_gnome_session ("RequestReboot", g_variant_new ("()"), &error);
	} else {
		g_warning ("Unknown session action");
	}
	
	if (!result) {
		if (error != NULL) {
			g_warning ("SessionManager action failed: %s", error->message);
		} else {
			g_warning ("SessionManager action failed: unknown error");
		}

		consolekit_fallback(action);
	}
	else
		g_variant_unref (result);
	g_clear_error (&error);
	
	return;
}	

static LogoutDialogType type = LOGOUT_DIALOG_TYPE_LOG_OUT;

static gboolean
option_logout (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_TYPE_LOG_OUT;
	g_debug("Dialog type: logout");
	return TRUE;
}

static gboolean
option_shutdown (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_TYPE_SHUTDOWN;
	g_debug("Dialog type: shutdown");
	return TRUE;
}

static gboolean
option_restart (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = LOGOUT_DIALOG_TYPE_RESTART;
	g_debug("Dialog type: restart");
	return TRUE;
}

static GOptionEntry options[] = {
	{"logout",     'l',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_logout,   "Log out of the current session",   NULL},
	{"shutdown",   's',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_shutdown, "Switch off the entire system",     NULL},
	{"restart",    'r',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_restart,  "Restart the system",               NULL},

	{NULL}
};

static gboolean
suppress_confirmations (void)
{
  GSettings * s = g_settings_new (SESSION_SCHEMA);
  const gboolean suppress = g_settings_get_boolean (s, SUPPRESS_KEY);
  g_clear_object (&s);
  return suppress;
}



int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	/* Setting up i18n and gettext.  Apparently, we need
	   all of these. */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

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

	/* Init some theme/icon stuff */
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
	                                  INDICATOR_ICONS_DIR);

	GtkWidget * dialog = NULL;
	if (!suppress_confirmations()) {
		g_debug("Showing dialog to ask for user confirmation");
		dialog = GTK_WIDGET(logout_dialog_new(type));
	}

	if (dialog != NULL) {
		GtkResponseType response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_hide(dialog);

		if (response == GTK_RESPONSE_OK) {
			g_debug("Dialog return response: 'okay'");
		} else if (response == GTK_RESPONSE_HELP) {
			g_debug("Dialog return response: 'help'");
		} else {
			g_debug("Dialog return response: %d", response);
		}

		if (response == GTK_RESPONSE_HELP) {
			type = LOGOUT_DIALOG_TYPE_RESTART;
			response = GTK_RESPONSE_OK;
		}

		if (response != GTK_RESPONSE_OK) {
			g_debug("Final response was not okay, quiting");
			return 0;
		}
	}

	session_action(type);
	g_debug("Finished action, quiting");

	return 0;
}
