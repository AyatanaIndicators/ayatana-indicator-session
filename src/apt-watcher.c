/*
Copyright 2011 Canonical Ltd.

Authors:
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

// Conor - 6/2/2012
// Please pull in packagekit's client lib
// using apt-get install --no-install-recommends libpackagekit-glib2-dev
// make sure you don't install package-kit
#define I_KNOW_THE_PACKAGEKIT_GLIB2_API_IS_SUBJECT_TO_CHANGE

#include <glib/gi18n.h>
#include <packagekit-glib2/packagekit.h>
#include "apt-watcher.h"
#include "dbus-shared-names.h"

static guint watcher_id;

struct _AptWatcher
{
	GObject parent_instance;
  SessionDbus* session_dbus_interface;
  DbusmenuMenuitem* apt_item;
  AptState current_state;
  PkClient* pkclient;  
};
                                                                
static void apt_watcher_show_apt_dialog (DbusmenuMenuitem* mi,
                                         guint timestamp,
                                         gpointer userdata);

G_DEFINE_TYPE (AptWatcher, apt_watcher, G_TYPE_OBJECT);

static void
apt_watcher_init (AptWatcher *self)
{
  self->current_state = UP_TO_DATE;
  self->pkclient = pk_client_new ();
  /*g_timeout_add_seconds (60,
                         apt_watcher_start_apt_interaction,
                         self); */
}

static void
apt_watcher_finalize (GObject *object)
{
  g_bus_unwatch_name (watcher_id);  
  //AptWatcher* self = APT_WATCHER (object);
           
	G_OBJECT_CLASS (apt_watcher_parent_class)->finalize (object);
}

static void
apt_watcher_class_init (AptWatcherClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = apt_watcher_finalize;
}


static void
apt_watcher_show_apt_dialog (DbusmenuMenuitem * mi,
                             guint timestamp,
                             gpointer userdata)
{
  GError * error = NULL;
  g_return_if_fail (APT_IS_WATCHER (userdata));
  AptWatcher* self = APT_WATCHER (userdata);
  const gchar* disposition = NULL;
  disposition = dbusmenu_menuitem_property_get (self->apt_item,
                                                DBUSMENU_MENUITEM_PROP_DISPOSITION);
                                    
  if (g_strcmp0 (disposition, DBUSMENU_MENUITEM_DISPOSITION_ALERT) == 0){  	
    gchar * helper = g_build_filename (LIBEXECDIR, "gtk-logout-helper", NULL);
	  gchar * dialog_line = g_strdup_printf ("%s --%s", helper, "restart");
  	g_free(helper);
	  if (!g_spawn_command_line_async(dialog_line, &error)) {
		  g_warning("Unable to show dialog: %s", error->message);
		  g_error_free(error);
	  }
	  g_free(dialog_line);
  } 
  else{
    if (!g_spawn_command_line_async("update-manager", &error))
    {
      g_warning("Unable to show update-manager: %s", error->message);
      g_error_free(error);
    }
  }   
}

AptWatcher* apt_watcher_new (SessionDbus* session_dbus,
                             DbusmenuMenuitem* item)
{
  AptWatcher* watcher = g_object_new (APT_TYPE_WATCHER, NULL);
  watcher->session_dbus_interface = session_dbus;
  watcher->apt_item = item;
  g_signal_connect (G_OBJECT(watcher->apt_item),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK(apt_watcher_show_apt_dialog), watcher);
  return watcher;
}
                               
