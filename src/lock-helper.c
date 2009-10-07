
#include <dbus/dbus-glib.h>
#include "lock-helper.h"

static DBusGProxy * gdm_settings_proxy = NULL;
static gboolean gdm_auto_login = FALSE;
static const gchar * gdm_auto_login_string = "daemon/AutomaticLoginEnable";

static gboolean is_guest = FALSE;

static gdm_autologin_cb_t gdm_autologin_cb = NULL;

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

gboolean
lock_screen_setup (gpointer data)
{
	if (!g_strcmp0(g_get_user_name(), "guest")) {
		is_guest = TRUE;
	}

	build_gdm_proxy();

	return FALSE;
}

