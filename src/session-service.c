/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
    Christoph Korn <c_korn@gmx.de>
    Cody Russell <crussell@canonical.com>

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

#include <unistd.h>

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/client.h>

#include <libindicator/indicator-service.h>

#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"

#include "gconf-helper.h"

#include "session-dbus.h"
#include "users-service-dbus.h"
#include "lock-helper.h"

#define UP_ADDRESS    "org.freedesktop.UPower"
#define UP_OBJECT     "/org/freedesktop/UPower"
#define UP_INTERFACE  "org.freedesktop.UPower"

#define DESKTOP_FILE  "/usr/share/applications/indicator-session-extra.desktop"

#define GUEST_SESSION_LAUNCHER  "/usr/share/gdm/guest-session/guest-session-launch"

#define LOCKDOWN_DIR              "/desktop/gnome/lockdown"
#define LOCKDOWN_KEY_USER         LOCKDOWN_DIR "/disable_user_switching"
#define LOCKDOWN_KEY_SCREENSAVER  LOCKDOWN_DIR "/disable_lock_screen"

typedef struct _ActivateData ActivateData;
struct _ActivateData
{
  UsersServiceDbus *service;
  UserData *user;
};

static DBusGConnection   *system_bus = NULL;
static DBusGProxy        *gdm_proxy = NULL;
static UsersServiceDbus  *dbus_interface = NULL;
static SessionDbus       *session_dbus = NULL;

static DbusmenuMenuitem  *lock_menuitem = NULL;
static DbusmenuMenuitem  *switch_menuitem = NULL;

static DbusmenuMenuitem * root_menuitem = NULL;
static GMainLoop * mainloop = NULL;
static DBusGProxy * up_main_proxy = NULL;
static DBusGProxy * up_prop_proxy = NULL;

static DBusGProxyCall * suspend_call = NULL;
static DBusGProxyCall * hibernate_call = NULL;

static DbusmenuMenuitem * hibernate_mi = NULL;
static DbusmenuMenuitem * suspend_mi = NULL;
static DbusmenuMenuitem * logout_mi = NULL;
static DbusmenuMenuitem * restart_mi = NULL;
static DbusmenuMenuitem * shutdown_mi = NULL;

static GConfClient * gconf_client = NULL;

static void rebuild_items (DbusmenuMenuitem *root, UsersServiceDbus *service);

static void
lockdown_changed (GConfClient *client,
                  guint        cnxd_id,
                  GConfEntry  *entry,
                  gpointer     user_data)
{
	GConfValue  *value = gconf_entry_get_value (entry);
	const gchar *key   = gconf_entry_get_key (entry);

	if (value == NULL || key == NULL) {
		return;
	}

	if (g_strcmp0 (key, LOCKDOWN_KEY_USER) == 0 || g_strcmp0 (key, LOCKDOWN_KEY_SCREENSAVER) == 0) {
		rebuild_items(root_menuitem, dbus_interface);
	}

	return;
}

/* Ensures that we have a GConf client and if we build one
   set up the signal handler. */
static void
ensure_gconf_client (void)
{
	if (!gconf_client) {
		gconf_client = gconf_client_get_default ();
		gconf_client_add_dir(gconf_client, LOCKDOWN_DIR, GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
		gconf_client_notify_add(gconf_client, LOCKDOWN_DIR, lockdown_changed, NULL, NULL, NULL);
	}
	return;
}

/* A return from the command to sleep the system.  Make sure
   that we unthrottle the screensaver. */
static void
sleep_response (DBusGProxy * proxy, DBusGProxyCall * call, gpointer data)
{
	screensaver_unthrottle();
	return;
}

/* Let's put this machine to sleep, with some info on how
   it should sleep.  */
static void
machine_sleep (DbusmenuMenuitem * mi, guint timestamp, gpointer userdata)
{
	gchar * type = (gchar *)userdata;

	if (up_main_proxy == NULL) {
		g_warning("Can not %s as no upower proxy", type);
	}

	screensaver_throttle(type);
	lock_screen(NULL, 0, NULL);

	dbus_g_proxy_begin_call(up_main_proxy,
	                        type,
	                        sleep_response,
	                        NULL,
	                        NULL,
	                        G_TYPE_INVALID);

	return;
}

/* A response to getting the suspend property */
static void
suspend_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
	suspend_call = NULL;

	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check suspend: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Suspend: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	if (suspend_mi != NULL) {
		dbusmenu_menuitem_property_set_value(suspend_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, &candoit);
	}

	return;
}

/* Response to getting the hibernate property */
static void
hibernate_prop_cb (DBusGProxy * proxy, DBusGProxyCall * call, gpointer userdata)
{
	hibernate_call = NULL;

	GValue candoit = {0};
	GError * error = NULL;
	dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &candoit, G_TYPE_INVALID);
	if (error != NULL) {
		g_warning("Unable to check hibernate: %s", error->message);
		g_error_free(error);
		return;
	}
	g_debug("Got Hibernate: %s", g_value_get_boolean(&candoit) ? "true" : "false");

	if (suspend_mi != NULL) {
		dbusmenu_menuitem_property_set_value(hibernate_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, &candoit);
	}

	return;
}

/* A signal that we need to recheck to ensure we can still
   hibernate and/or suspend */
static void
up_changed_cb (DBusGProxy * proxy, gpointer user_data)
{
	/* Start Async call to see if we can hibernate */
	if (suspend_call == NULL) {
		suspend_call = dbus_g_proxy_begin_call(up_prop_proxy,
		                                       "Get",
		                                       suspend_prop_cb,
		                                       NULL,
		                                       NULL,
		                                       G_TYPE_STRING,
		                                       UP_INTERFACE,
		                                       G_TYPE_STRING,
		                                       "CanSuspend",
		                                       G_TYPE_INVALID,
		                                       G_TYPE_VALUE,
		                                       G_TYPE_INVALID);
	}

	/* Start Async call to see if we can suspend */
	if (hibernate_call == NULL) {
		hibernate_call = dbus_g_proxy_begin_call(up_prop_proxy,
		                                         "Get",
		                                         hibernate_prop_cb,
		                                         NULL,
		                                         NULL,
		                                         G_TYPE_STRING,
		                                         UP_INTERFACE,
		                                         G_TYPE_STRING,
		                                         "CanHibernate",
		                                         G_TYPE_INVALID,
		                                         G_TYPE_VALUE,
		                                         G_TYPE_INVALID);
	}

	return;
}

/* This function goes through and sets up what we need for
   DKp checking.  We're even setting up the calls for the props
   we need */
static void
setup_up (void) {
	DBusGConnection * bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	g_return_if_fail(bus != NULL);

	if (up_main_proxy == NULL) {
		up_main_proxy = dbus_g_proxy_new_for_name(bus,
		                                           UP_ADDRESS,
		                                           UP_OBJECT,
		                                           UP_INTERFACE);
	}
	g_return_if_fail(up_main_proxy != NULL);

	if (up_prop_proxy == NULL) {
		up_prop_proxy = dbus_g_proxy_new_for_name(bus,
		                                           UP_ADDRESS,
		                                           UP_OBJECT,
		                                           DBUS_INTERFACE_PROPERTIES);
	}
	g_return_if_fail(up_prop_proxy != NULL);

	/* Connect to changed signal */
	dbus_g_proxy_add_signal(up_main_proxy,
	                        "Changed",
	                        G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(up_main_proxy,
	                            "Changed",
	                            G_CALLBACK(up_changed_cb),
	                            NULL,
	                            NULL);

	/* Force an original "changed" event */
	up_changed_cb(up_main_proxy, NULL);

	return;
}

/* This is the function to show a dialog on actions that
   can destroy data.  Currently it just calls the GTK version
   but it seems that in the future it should figure out
   what's going on and something better. */
static void
show_dialog (DbusmenuMenuitem * mi, guint timestamp, gchar * type)
{
	gchar * helper = g_build_filename(LIBEXECDIR, "gtk-logout-helper", NULL);
	gchar * dialog_line = g_strdup_printf("%s --%s", helper, type);
	g_free(helper);

	g_debug("Showing dialog '%s'", dialog_line);

	GError * error = NULL;
	if (!g_spawn_command_line_async(dialog_line, &error)) {
		g_warning("Unable to show dialog: %s", error->message);
		g_error_free(error);
	}

	g_free(dialog_line);

	return;
}

/* Checks to see if we should show the guest suession item */
static gboolean
check_guest_session (void)
{
	if (geteuid() < 500) {
		/* System users shouldn't have guest account shown.  Mosly
		   this would be the case of the guest user itself. */
		return FALSE;
	}
	if (!g_file_test(GUEST_SESSION_LAUNCHER, G_FILE_TEST_IS_EXECUTABLE)) {
		/* It doesn't appear that the Guest session stuff is
		   installed.  So let's not use it then! */
		return FALSE;
	}

	return TRUE;
}

/* Called when someone clicks on the guest session item. */
static void
activate_guest_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async(GUEST_SESSION_LAUNCHER, &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

/* Checks to see if we can create sessions and get a proxy
   to the display manager (GDM) */
static gboolean
check_new_session (void)
{
	if (system_bus == NULL) {
		system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	}

	if (system_bus == NULL) {
		return FALSE;
	}

	if (gdm_proxy == NULL) {
		gdm_proxy = dbus_g_proxy_new_for_name(system_bus,
		                                      "org.gnome.DisplayManager",
		                                      "/org/gnome/DisplayManager/LocalDisplayFactory",
		                                      "org.gnome.DisplayManager.LocalDisplayFactory");
	}

	if (gdm_proxy == NULL) {
		return FALSE;
	}

	return TRUE;
}

/* Starts a new generic session */
static void
activate_new_session (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async("gdmflexiserver --startnew", &error)) {
		g_warning("Unable to start new session: %s", error->message);
		g_error_free(error);
	}

	return;
}

/* Activates a session for a particular user. */
static void
activate_user_session (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  UserData *user = (UserData *)user_data;
  UsersServiceDbus *service = user->service;

  users_service_dbus_activate_user_session (service, user);
}

/* Comparison function to look into the UserData struct
   to compare by using the username value */
static gint
compare_users_by_username (const gchar *a,
                           const gchar *b)
{
  UserData *user1 = (UserData *)a;
  UserData *user2 = (UserData *)b;

  gint retval = g_strcmp0 (user1->real_name, user2->real_name);

  /* If they're the same, they're both in conflict. */
  if (retval == 0) {
    user1->real_name_conflict = TRUE;
    user2->real_name_conflict = TRUE;
  }

  return retval;
}

/* Take a desktop file and execute it */
static void
desktop_activate_cb (DbusmenuMenuitem * mi, guint timestamp, gpointer data)
{
	GAppInfo * appinfo = G_APP_INFO(data);
	g_return_if_fail(appinfo != NULL);
	g_app_info_launch(appinfo, NULL, NULL, NULL);
	return;
}

/* Builds up the menu for us */
static void
rebuild_items (DbusmenuMenuitem *root,
               UsersServiceDbus *service)
{
  DbusmenuMenuitem *mi = NULL;
  DbusmenuMenuitem * guest_mi = NULL;
  GList *u;
  UserData *user;
  gboolean can_activate;
  gboolean can_lockscreen;
  GList *children;

  ensure_gconf_client ();

  can_activate = users_service_dbus_can_activate_session (service) &&
      !gconf_client_get_bool (gconf_client, LOCKDOWN_KEY_USER, NULL);
  can_lockscreen = !gconf_client_get_bool (gconf_client, LOCKDOWN_KEY_SCREENSAVER, NULL);

  children = dbusmenu_menuitem_take_children (root);
  g_list_foreach (children, (GFunc)g_object_unref, NULL);
  g_list_free (children);

  if (can_lockscreen) {
	lock_menuitem = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(lock_menuitem, DBUSMENU_MENUITEM_PROP_LABEL, _("Lock Screen"));
	g_signal_connect(G_OBJECT(lock_menuitem), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(lock_screen), NULL);
	dbusmenu_menuitem_child_append(root, lock_menuitem);
	if (!will_lock_screen()) {
		dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_ENABLED, FALSE);
	} else {
		dbusmenu_menuitem_property_set_bool(lock_menuitem, DBUSMENU_MENUITEM_PROP_ENABLED, TRUE);
	}
  }

  if (can_activate == TRUE)
    {
		if (can_lockscreen) {
			DbusmenuMenuitem * separator1 = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(separator1, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
			dbusmenu_menuitem_child_append(root, separator1);
		}

      if (check_guest_session ())
        {
          guest_mi = dbusmenu_menuitem_new ();
		  dbusmenu_menuitem_property_set (guest_mi, DBUSMENU_MENUITEM_PROP_TYPE, USER_ITEM_TYPE);
          dbusmenu_menuitem_property_set (guest_mi, USER_ITEM_PROP_NAME, _("Guest Session"));
          dbusmenu_menuitem_property_set_bool (guest_mi, USER_ITEM_PROP_LOGGED_IN, FALSE);
          dbusmenu_menuitem_child_append (root, guest_mi);
          g_signal_connect (G_OBJECT (guest_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_guest_session), NULL);
        }

      if (check_new_session ())
        {

          switch_menuitem = dbusmenu_menuitem_new ();
		  dbusmenu_menuitem_property_set (switch_menuitem, DBUSMENU_MENUITEM_PROP_TYPE, MENU_SWITCH_TYPE);
		  dbusmenu_menuitem_property_set (switch_menuitem, MENU_SWITCH_USER, g_get_user_name());
          dbusmenu_menuitem_child_append (root, switch_menuitem);
          g_signal_connect (G_OBJECT (switch_menuitem), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_new_session), NULL);
        }

		GList * users = NULL;
		users = users_service_dbus_get_user_list (service);
		guint user_count = g_list_length(users);

		if (user_count > MINIMUM_USERS && user_count < MAXIMUM_USERS) {
			users = g_list_sort (users, (GCompareFunc)compare_users_by_username);
		}

		for (u = users; u != NULL; u = g_list_next (u)) {
			user = u->data;
			user->service = service;

			if (user->uid == getuid()) {
				/* Hide me from the list */
				continue;
			}

			if (g_strcmp0(user->user_name, "guest") == 0) {
				/* Check to see if the guest has sessions and so therefore should
				   get a check mark. */
				if (user->sessions != NULL) {
					dbusmenu_menuitem_property_set_bool (guest_mi, USER_ITEM_PROP_LOGGED_IN, TRUE);
				}
				/* If we're showing user accounts, keep going through the list */
				if (user_count > MINIMUM_USERS && user_count < MAXIMUM_USERS) {
					continue;
				}
				/* If not, we can stop here */
				break;
			}

			if (user_count > MINIMUM_USERS && user_count < MAXIMUM_USERS) {
				mi = dbusmenu_menuitem_new ();
				dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_TYPE, USER_ITEM_TYPE);
				if (user->real_name_conflict) {
					gchar * conflictedname = g_strdup_printf("%s (%s)", user->real_name, user->user_name);
					dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, conflictedname);
					g_free(conflictedname);
				} else {
					dbusmenu_menuitem_property_set (mi, USER_ITEM_PROP_NAME, user->real_name);
				}
				dbusmenu_menuitem_property_set_bool (mi, USER_ITEM_PROP_LOGGED_IN, user->sessions != NULL);
				dbusmenu_menuitem_child_append (root, mi);
				g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_user_session), user);
			}
		}

		g_list_free(users);
	}

	if (can_lockscreen || can_activate) {
		DbusmenuMenuitem * separator = dbusmenu_menuitem_new();
		dbusmenu_menuitem_property_set(separator, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
		dbusmenu_menuitem_child_append(root, separator);
	}

	logout_mi = dbusmenu_menuitem_new();
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out"));
	} else {
		dbusmenu_menuitem_property_set(logout_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Log Out..."));
	}
	dbusmenu_menuitem_child_append(root, logout_mi);
	g_signal_connect(G_OBJECT(logout_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "logout");

	suspend_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set_bool(suspend_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, FALSE);
	dbusmenu_menuitem_property_set(suspend_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Sleep"));
	dbusmenu_menuitem_child_append(root, suspend_mi);
	g_signal_connect(G_OBJECT(suspend_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(machine_sleep), "Suspend");

	hibernate_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set_bool(hibernate_mi, DBUSMENU_MENUITEM_PROP_VISIBLE, FALSE);
	dbusmenu_menuitem_property_set(hibernate_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Hibernate"));
	dbusmenu_menuitem_child_append(root, hibernate_mi);
	g_signal_connect(G_OBJECT(hibernate_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(machine_sleep), "Hibernate");

	restart_mi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(restart_mi, DBUSMENU_MENUITEM_PROP_TYPE, RESTART_ITEM_TYPE);
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart"));
	} else {
		dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart..."));
	}
	dbusmenu_menuitem_child_append(root, restart_mi);
	g_signal_connect(G_OBJECT(restart_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "restart");

	shutdown_mi = dbusmenu_menuitem_new();
	if (supress_confirmations()) {
		dbusmenu_menuitem_property_set(shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Switch Off"));
	} else {
		dbusmenu_menuitem_property_set(shutdown_mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Switch Off..."));
	}
	dbusmenu_menuitem_child_append(root, shutdown_mi);
	g_signal_connect(G_OBJECT(shutdown_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(show_dialog), "shutdown");

	RestartShutdownLogoutMenuItems * restart_shutdown_logout_mi = g_new0 (RestartShutdownLogoutMenuItems, 1);
	restart_shutdown_logout_mi->logout_mi = logout_mi;
	restart_shutdown_logout_mi->restart_mi = restart_mi;
	restart_shutdown_logout_mi->shutdown_mi = shutdown_mi;

	update_menu_entries(restart_shutdown_logout_mi);

	if (g_file_test(DESKTOP_FILE, G_FILE_TEST_EXISTS)) {
		GAppInfo * appinfo = G_APP_INFO(g_desktop_app_info_new_from_filename(DESKTOP_FILE));

		if (appinfo != NULL) {
			DbusmenuMenuitem * separator = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(separator, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
			dbusmenu_menuitem_child_append(root, separator);

			DbusmenuMenuitem * desktop_mi = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(desktop_mi, DBUSMENU_MENUITEM_PROP_LABEL, g_app_info_get_name(appinfo));
			g_signal_connect(G_OBJECT(desktop_mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(desktop_activate_cb), appinfo);
			dbusmenu_menuitem_child_append(root, desktop_mi);
		}
	}

	return;
}

/* Signal called when a user is added.  It updates the count and
   rebuilds the menu */
static void
user_change (UsersServiceDbus *service,
             gint64            user,
             gpointer          user_data)
{
	DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;
	rebuild_items (root, service);
	return;
}

/* When the service interface starts to shutdown, we
   should follow it. */
void
service_shutdown (IndicatorService * service, gpointer user_data)
{
	if (mainloop != NULL) {
		g_debug("Service shutdown");
		g_main_loop_quit(mainloop);
	}
	return;
}

/* When the directory changes we need to figure out how our menu
   item should look. */
static void
restart_dir_changed (void)
{
	gboolean restart_required = g_file_test("/var/run/reboot-required", G_FILE_TEST_EXISTS);

	if (restart_required) {
		if (supress_confirmations()) {
			dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart Required"));
		} else {
			dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart Required..."));
		}
		dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_ICON, "emblem-important");
		if (session_dbus != NULL) {
			session_dbus_set_name(session_dbus, ICON_RESTART);
		}
	} else {	
		if (supress_confirmations()) {
			dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart"));
		} else {
			dbusmenu_menuitem_property_set(restart_mi, RESTART_ITEM_LABEL, _("Restart..."));
		}
		dbusmenu_menuitem_property_remove(restart_mi, RESTART_ITEM_ICON);
		if (session_dbus != NULL) {
			session_dbus_set_name(session_dbus, ICON_DEFAULT);
		}
	}

	return;
}

/* Buids a file watcher for the directory so that when it
   changes we can check to see if our reboot-required is
   there. */
static void
setup_restart_watch (void)
{
	GFile * filedir = g_file_new_for_path("/var/run");
	GFileMonitor * filemon = g_file_monitor_directory(filedir, G_FILE_MONITOR_NONE, NULL, NULL);
	if (filemon != NULL) {
		g_signal_connect(G_OBJECT(filemon), "changed", G_CALLBACK(restart_dir_changed), NULL);
	}
	restart_dir_changed();
	return;
}

/* Main, is well, main.  It brings everything up and throws
   us into the mainloop of no return. */
int
main (int argc, char ** argv)
{
    g_type_init();

	/* Setting up i18n and gettext.  Apparently, we need
	   all of these. */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	IndicatorService * service = indicator_service_new_version(INDICATOR_SESSION_DBUS_NAME,
	                                                           INDICATOR_SESSION_DBUS_VERSION);
	g_signal_connect(G_OBJECT(service),
	                 INDICATOR_SERVICE_SIGNAL_SHUTDOWN,
	                 G_CALLBACK(service_shutdown), NULL);

	session_dbus = session_dbus_new();

	g_idle_add(lock_screen_setup, NULL);

    root_menuitem = dbusmenu_menuitem_new();
	g_debug("Root ID: %d", dbusmenu_menuitem_get_id(root_menuitem));

    dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);

    rebuild_items (root_menuitem, dbus_interface);

    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-added",
                      G_CALLBACK (user_change),
                      root_menuitem);
    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-removed",
                      G_CALLBACK (user_change),
                      root_menuitem);

	setup_restart_watch();

	setup_up();

    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_SESSION_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

