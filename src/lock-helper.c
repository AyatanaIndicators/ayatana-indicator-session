/*
A small helper for locking the screen.

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

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <dbus/dbus-glib.h>
#include "lock-helper.h"

#define SCREENSAVER_SCHEMA            "org.gnome.desktop.screensaver"
#define SCREENSAVER_LOCK_ENABLED_KEY  "lock-enabled"

static DBusGProxy * gss_proxy = NULL;
static GMainLoop * gss_mainloop = NULL;

static gboolean is_guest = FALSE;

static GSettings * settings = NULL;

void build_gss_proxy (void);

/* This is our logic on whether the screen should be locked
   or not.  It effects everything else. */
gboolean
will_lock_screen (void)
{
	if (is_guest) {
		return FALSE;
	}

	if (settings == NULL) {
		settings = g_settings_new (SCREENSAVER_SCHEMA);
	}

	return g_settings_get_boolean (settings, SCREENSAVER_LOCK_ENABLED_KEY);
}

/* When the screensave go active, if we've got a mainloop
   running we should quit it. */
static void
gss_active_changed (DBusGProxy * proxy, gboolean active, gpointer data)
{
	if (active && gss_mainloop != NULL) {
		g_main_loop_quit(gss_mainloop);
	}

	return;
}

static gboolean
get_greeter_mode (void)
{
	const gchar *var;
	var = g_getenv("INDICATOR_GREETER_MODE");
	return (g_strcmp0(var, "1") == 0);
}

/* Build the gss proxy and set up it's signals */
void
build_gss_proxy (void)
{
	if (gss_proxy == NULL) {
		if (get_greeter_mode ())
			return; /* Don't start/lock the screensaver from the login screen */

		DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
		g_return_if_fail(session_bus != NULL);

		gss_proxy = dbus_g_proxy_new_for_name(session_bus,
		                                      "org.gnome.ScreenSaver",
		                                      "/",
		                                      "org.gnome.ScreenSaver");
		g_return_if_fail(gss_proxy != NULL);

		dbus_g_proxy_add_signal(gss_proxy, "ActiveChanged", G_TYPE_BOOLEAN, G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(gss_proxy, "ActiveChanged", G_CALLBACK(gss_active_changed), NULL, NULL);
	}

	return;
}

/* This is a timeout, we only want to wait for the screen to
   lock for a little bit, but not forever. */
static gboolean
activate_timeout (gpointer data)
{
	/* Clear the ID for the timeout */
	guint * address = (guint *)data;
	*address = 0;

	/* Quit the mainloop */
	if (gss_mainloop != NULL) {
		g_main_loop_quit(gss_mainloop);
	}
	
	return FALSE;
}

/* A fun little function to actually lock the screen.  If,
   that's what you want, let's do it! */
void
lock_screen (DbusmenuMenuitem * mi, guint timestamp, gpointer data)
{
	g_debug("Lock Screen");

	build_gss_proxy();
	g_return_if_fail(gss_proxy != NULL);

	dbus_g_proxy_call_no_reply(gss_proxy,
	                           "Lock",
	                           G_TYPE_INVALID,
	                           G_TYPE_INVALID);

	if (gss_mainloop == NULL) {
		gss_mainloop = g_main_loop_new(NULL, FALSE);
	}

	guint timer = g_timeout_add_seconds(1, activate_timeout, &timer);

	g_main_loop_run(gss_mainloop);

	if (timer != 0) {
		g_source_remove(timer);
	}

	return;
}

/* Do what it takes to make the lock screen function work
   and be happy. */
gboolean
lock_screen_setup (gpointer data)
{
	if (!g_strcmp0(g_get_user_name(), "guest")) {
		is_guest = TRUE;
	}

	return FALSE;
}

