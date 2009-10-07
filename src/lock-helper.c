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

#include <dbus/dbus-glib.h>
#include "lock-helper.h"

static DBusGProxy * gss_proxy = NULL;
static GMainLoop * gss_mainloop = NULL;
static guint cookie = 0;
static DBusGProxyCall * cookie_call = NULL;

static DBusGProxy * gdm_settings_proxy = NULL;
static gboolean gdm_auto_login = FALSE;
static const gchar * gdm_auto_login_string = "daemon/AutomaticLoginEnable";

static gboolean is_guest = FALSE;

static gdm_autologin_cb_t gdm_autologin_cb = NULL;

/* Checks to see if there is an error and reports
   it.  Not much else we can do. */
static void
unthrottle_return (DBusGProxy * proxy, DBusGProxyCall * call, gpointer data)
{
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error,
	                      G_TYPE_INVALID);

	if (error != NULL) {
		g_warning("Unable to unthrottle: %s", error->message);
	}
	return;
}

/* Sends an unthrottle if we're throttled. */
void
screensaver_unthrottle (void)
{
	g_return_if_fail(cookie != 0);

	dbus_g_proxy_begin_call(gss_proxy, "UnThrottle",
	                        unthrottle_return, NULL,
	                        NULL,
	                        G_TYPE_UINT, cookie,
	                        G_TYPE_INVALID);

	cookie = 0;
	return;
}

/* Gets there return cookie from the throttle command
   and sets things valid */
static void
throttle_return (DBusGProxy * proxy, DBusGProxyCall * call, gpointer data)
{
	GError * error = NULL;
	cookie_call = NULL;

	dbus_g_proxy_end_call(proxy, call, &error,
	                      G_TYPE_UINT, &cookie,
	                      G_TYPE_INVALID);

	if (error != NULL) {
		g_warning("Unable to throttle the screensaver: %s", error->message);
		return;
	}


	if (cookie == 0) {
		g_warning("We didn't get a throttle cookie!");
	}

	return;
}

/* Throttling the screensaver by using the screen saver
   command. */
void
screensaver_throttle (gchar * reason)
{
	g_return_if_fail(cookie_call == NULL);
	g_return_if_fail(will_lock_screen());

	if (cookie != 0) {
		screensaver_unthrottle();
	}

	cookie_call = dbus_g_proxy_begin_call(gss_proxy, "Throttle",
	                                      throttle_return, NULL,
	                                      NULL,
	                                      G_TYPE_STRING, reason,
	                                      G_TYPE_INVALID);

	return;
}

/* Setting up a call back */
void
lock_screen_gdm_cb_set (gdm_autologin_cb_t cb)
{
	if (gdm_autologin_cb) {
		g_warning("Already had a callback, setting up a new one...");
	}

	gdm_autologin_cb = cb;
	return;
}

/* This is our logic on whether the screen should be locked
   or not.  It effects everything else. */
gboolean
will_lock_screen (void)
{
	if (gdm_auto_login) {
		return FALSE;
	}
	if (is_guest) {
		return FALSE;
	}

	return TRUE;
}

/* Respond to the signal of autologin changing to see if the
   setting for timed login changes. */
static void
gdm_settings_change (DBusGProxy * proxy, const gchar * value, const gchar * old, const gchar * new, gpointer data)
{
	if (g_strcmp0(value, gdm_auto_login_string)) {
		/* This is not a setting that we care about,
		   there is only one. */
		return;
	}
	g_debug("GDM Settings change: %s", new);

	if (g_strcmp0(new, "true") == 0) {
		gdm_auto_login = TRUE;
	} else {
		gdm_auto_login = FALSE;
	}

	if (gdm_autologin_cb != NULL) {
		gdm_autologin_cb();
	}

	return;
}

/* Get back the data from querying to see if there is auto
   login enabled in GDM */
static void
gdm_get_autologin (DBusGProxy * proxy, DBusGProxyCall * call, gpointer data)
{
	GError * error = NULL;
	gchar * value = NULL;

	if (!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_STRING, &value, G_TYPE_INVALID)) {
		g_warning("Unable to get autologin setting: %s", error != NULL ? error->message : "null");
		g_error_free(error);
		return;
	}

	g_return_if_fail(value != NULL);
	gdm_settings_change(proxy, gdm_auto_login_string, NULL, value, NULL);

	return;
}

/* Sets up the proxy and queries for the setting to know
   whether we're doing an autologin. */
static void
build_gdm_proxy (void)
{
	g_return_if_fail(gdm_settings_proxy == NULL);

	/* Grab the system bus */
	DBusGConnection * bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	g_return_if_fail(bus != NULL);

	/* Get the settings proxy */
	gdm_settings_proxy = dbus_g_proxy_new_for_name_owner(bus,
	                                                     "org.gnome.DisplayManager",
	                                                     "/org/gnome/DisplayManager/Settings",
	                                                     "org.gnome.DisplayManager.Settings", NULL);
	g_return_if_fail(gdm_settings_proxy != NULL);

	/* Signal for value changed */
	dbus_g_proxy_add_signal(gdm_settings_proxy,
	                        "ValueChanged",
	                        G_TYPE_STRING,
	                        G_TYPE_STRING,
	                        G_TYPE_STRING,
	                        G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(gdm_settings_proxy,
	                            "ValueChanged",
	                            G_CALLBACK(gdm_settings_change),
	                            NULL,
	                            NULL);

	/* Start to get the initial value */
	dbus_g_proxy_begin_call(gdm_settings_proxy,
	                        "GetValue",
	                        gdm_get_autologin,
	                        NULL,
	                        NULL,
	                        G_TYPE_STRING,
	                        gdm_auto_login_string,
	                        G_TYPE_INVALID);

	return;
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

/* Build the gss proxy and set up it's signals */
void
build_gss_proxy (void)
{
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	g_return_if_fail(session_bus != NULL);

	gss_proxy = dbus_g_proxy_new_for_name_owner(session_bus,
	                                            "org.gnome.ScreenSaver",
	                                            "/",
	                                            "org.gnome.ScreenSaver",
	                                            NULL);
	g_return_if_fail(gss_proxy != NULL);

	dbus_g_proxy_add_signal(gss_proxy, "ActiveChanged", G_TYPE_BOOLEAN, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(gss_proxy, "ActiveChanged", G_CALLBACK(gss_active_changed), NULL, NULL);

	return;
}

/* This is a timeout, we only want to wait for the screen to
   lock for a little bit, but not forever. */
static gboolean
activate_timeout (gpointer data)
{
	guint * address = (guint *)data;
	*address = 0;

	if (gss_mainloop != NULL) {
		g_main_loop_quit(gss_mainloop);
	}
	
	return FALSE;
}

/* A fun little function to actually lock the screen.  If,
   that's what you want, let's do it! */
void
lock_screen (DbusmenuMenuitem * mi, gpointer data)
{
	g_debug("Lock Screen");
	if (!will_lock_screen()) {
		g_debug("\tGDM set to autologin, blocking lock");
		return;
	}

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

	build_gdm_proxy();
	build_gss_proxy();

	return FALSE;
}

