/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
    Christoph Korn <c_korn@gmx.de>
    Cody Russell <crussell@canonical.com>
    Conor Curran <conor.curran@canonical.com>

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
#include <locale.h>

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/client.h>

#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menuitem.h>

#include <libindicator/indicator-service.h>

#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"
#include "users-service-dbus.h"
#include "user-menu-mgr.h"
#include "device-menu-mgr.h"
#include "session-dbus.h"

typedef struct _ActivateData ActivateData;
struct _ActivateData
{
  UsersServiceDbus *service;
  UserData *user;
};

//static UsersServiceDbus  *dbus_interface = NULL;
static SessionDbus       *session_dbus = NULL;
static GMainLoop * mainloop = NULL;


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

static gboolean
get_greeter_mode (void)
{
  const gchar *var;
  var = g_getenv("INDICATOR_GREETER_MODE");
  return (g_strcmp0(var, "1") == 0);
}

/* Main, is well, main.  It brings everything up and throws
   us into the mainloop of no return. */
int
main (int argc, char ** argv)
{
  gboolean greeter_mode;

  g_type_init();

	/* Setting up i18n and gettext.  Apparently, we need
	   all of these. */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	IndicatorService * service = indicator_service_new_version (INDICATOR_SESSION_DBUS_NAME,
      		                                                    INDICATOR_SESSION_DBUS_VERSION);
	g_signal_connect (G_OBJECT(service),
                    INDICATOR_SERVICE_SIGNAL_SHUTDOWN,
                    G_CALLBACK(service_shutdown), NULL);

	session_dbus = session_dbus_new();

  greeter_mode = get_greeter_mode();

  // Devices
  DeviceMenuMgr* device_mgr = device_menu_mgr_new (session_dbus, greeter_mode);
  DbusmenuServer * server = dbusmenu_server_new(INDICATOR_SESSION_DBUS_OBJECT);
  dbusmenu_server_set_root(server, device_mgr_get_root_item (device_mgr));
    
  if (!greeter_mode) {
    // Users
    UserMenuMgr* user_mgr = user_menu_mgr_new (session_dbus, greeter_mode);    
    DbusmenuServer* users_server = dbusmenu_server_new (INDICATOR_USERS_DBUS_OBJECT);
    dbusmenu_server_set_root (users_server, user_mgr_get_root_item (user_mgr));
  }

  mainloop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(mainloop);
  
  return 0;
}

