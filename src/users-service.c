/*
 * A small wrapper utility to load indicators and put them as menu items
 * into the gnome-panel using it's applet interface.
 *
 * Copyright 2009 Canonical Ltd.
 *
 * Authors:
 *    Ted Gould <ted@canonical.com>
 *    Cody Russell <crussell@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 *with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <unistd.h>

#include <glib/gi18n.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>

#include "dbus-shared-names.h"
#include "users-service-dbus.h"

#define GUEST_SESSION_LAUNCHER  "/usr/share/gdm/guest-session/guest-session-launch"

typedef struct _ActivateData ActivateData;
struct _ActivateData
{
  UsersServiceDbus *service;
  UserData *user;
};

static DBusGConnection   *session_bus = NULL;
static DBusGConnection   *system_bus = NULL;
static DBusGProxy        *bus_proxy = NULL;
static DBusGProxy        *gdm_proxy = NULL;
static DbusmenuMenuitem  *root_menuitem = NULL;
static GMainLoop         *mainloop = NULL;
static UsersServiceDbus  *dbus_interface = NULL;

static gint   count;
static GList *users;

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

static void
activate_guest_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async(GUEST_SESSION_LAUNCHER, &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

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

static void
activate_new_session (DbusmenuMenuitem * mi, gpointer user_data)
{
	GError * error = NULL;
	if (!g_spawn_command_line_async("gdmflexiserver --startnew", &error)) {
		g_warning("Unable to start guest session: %s", error->message);
		g_error_free(error);
	}

	return;
}

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
activate_user_session (DbusmenuMenuitem *mi, gpointer user_data)
{
  UserData *user = (UserData *)user_data;
  UsersServiceDbus *service = user->service;

  users_service_dbus_activate_user_session (service, user);
}

static void
remove_menu_item (DbusmenuMenuitem *root, gpointer user_data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)user_data;

  dbusmenu_menuitem_child_delete (root, child);
}

static void
rebuild_items (DbusmenuMenuitem *root,
               UsersServiceDbus *service)
{
  DbusmenuMenuitem *mi = NULL;
  GList *u;
  UserData *user;

  dbusmenu_menuitem_foreach (root, remove_menu_item, NULL);

  mi = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Lock Screen"));
  g_signal_connect(G_OBJECT(mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(lock_screen), NULL);
  dbusmenu_menuitem_child_append(root, mi);

  if (check_guest_session ())
    {
      mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, _("Guest Session"));
      dbusmenu_menuitem_child_append (root, mi);
      g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_guest_session), NULL);
    }

  if (count > 1 && count < 7)
    {
      if (count > MINIMUM_USERS && count < MAXIMUM_USERS)
        {
          if (users != NULL)
            {
              GList *l = NULL;

              for (l = users; l != NULL; l = l->next)
                {
                  users = g_list_delete_link (users, l);
                }

              users = NULL;
            }

          users = users_service_dbus_get_user_list (service);
        }

      for (u = users; u != NULL; u = g_list_next (u))
        {
          user = u->data;

          user->service = service;

          mi = dbusmenu_menuitem_new ();
          dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, user->real_name);
          dbusmenu_menuitem_child_append (root, mi);
          g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_user_session), user);
        }
    }

  if (check_new_session ())
    {
      mi = dbusmenu_menuitem_new ();
      dbusmenu_menuitem_property_set (mi, DBUSMENU_MENUITEM_PROP_LABEL, _("New Session..."));
      dbusmenu_menuitem_child_append (root, mi);
      g_signal_connect (G_OBJECT (mi), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK (activate_new_session), NULL);
    }
}

static void
user_added (UsersServiceDbus *service,
            UserData         *user,
            gpointer          user_data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;

  count++;

  rebuild_items (root, service);
}

static void
user_removed (UsersServiceDbus *service,
              UserData         *user,
              gpointer          user_data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)user_data;

  count--;

  rebuild_items (root, service);
}

static void
create_items (DbusmenuMenuitem *root,
              UsersServiceDbus *service)
{
  g_return_if_fail (IS_USERS_SERVICE_DBUS (service));

  count = users_service_dbus_get_user_count (service);
  if (count > MINIMUM_USERS && count < MAXIMUM_USERS)
    {
      users = users_service_dbus_get_user_list (service);
    }

  rebuild_items (root, service);
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

    session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
    bus_proxy = dbus_g_proxy_new_for_name (session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    GError * error = NULL;
    guint nameret = 0;

    if (!org_freedesktop_DBus_request_name(bus_proxy, INDICATOR_USERS_DBUS_NAME, 0, &nameret, &error)) {
        g_error("Unable to call to request name");
        return 1;
    }

    if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_error("Unable to get name");
        return 1;
    }

    dbus_interface = g_object_new (USERS_SERVICE_DBUS_TYPE, NULL);

    root_menuitem = dbusmenu_menuitem_new ();
    g_debug ("Root ID: %d", dbusmenu_menuitem_get_id (root_menuitem));

    create_items (root_menuitem, dbus_interface);

    DbusmenuServer * server = dbusmenu_server_new(INDICATOR_USERS_DBUS_OBJECT);
    dbusmenu_server_set_root(server, root_menuitem);

    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-added",
                      G_CALLBACK (user_added),
                      root_menuitem);
    g_signal_connect (G_OBJECT (dbus_interface),
                      "user-removed",
                      G_CALLBACK (user_removed),
                      root_menuitem);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return 0;
}

