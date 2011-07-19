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

#include "apt-watcher.h"
#include <gio/gio.h>

static guint watcher_id;

struct _AptWatcher
{
	GObject parent_instance;
	GCancellable * proxy_cancel;
	GDBusProxy * proxy;  
};

static void
apt_watcher_on_name_appeared (GDBusConnection *connection,
                              const gchar     *name,
                              const gchar     *name_owner,
                              gpointer         user_data);
static void
apt_watcher_on_name_vanished (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data);
static void
apt_watcher_get_active_transactions_cb (GObject * obj,
                                        GAsyncResult * res,
                                        gpointer user_data);
static void
fetch_proxy_cb (GObject * object,
                GAsyncResult * res,
                gpointer user_data);

G_DEFINE_TYPE (AptWatcher, apt_watcher, G_TYPE_OBJECT);

static void
apt_watcher_init (AptWatcher *self)
{
  self->proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.debian.apt",
                            "/org/debian/apt",
                            "org.debian.apt",
                            self->proxy_cancel,
                            fetch_proxy_cb,
                            self);
}

static void
apt_watcher_finalize (GObject *object)
{
  g_bus_unwatch_name (watcher_id);  

	G_OBJECT_CLASS (apt_watcher_parent_class)->finalize (object);
}

static void
apt_watcher_class_init (AptWatcherClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = apt_watcher_finalize;
}

static void
fetch_proxy_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	AptWatcher* self = APT_WATCHER(user_data);
	g_return_if_fail(self != NULL);

	GDBusProxy * proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

	if (self->proxy_cancel != NULL) {
		g_object_unref(self->proxy_cancel);
		self->proxy_cancel = NULL;
	}

	if (error != NULL) {
		g_warning("Could not grab DBus proxy for %s: %s",
               "org.debian.apt", error->message);
		g_error_free(error);
		return;
	}

	self->proxy = proxy;
  // Set up the watch.
  watcher_id = g_bus_watch_name (G_BUS_TYPE_SYSTEM,
                                 "org.debian.apt",
                                 G_BUS_NAME_WATCHER_FLAGS_NONE,
                                 apt_watcher_on_name_appeared,
                                 apt_watcher_on_name_vanished,
                                 self,
                                 NULL);
  
  //We'll need to connect to the state changed signal                                 
	//g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);  
}


static void
apt_watcher_on_name_appeared (GDBusConnection *connection,
                              const gchar     *name,
                              const gchar     *name_owner,
                              gpointer         user_data)
{
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* watcher = APT_WATCHER (user_data);
  
  g_print ("Name %s on %s is owned by %s\n",
           name,
           "the system bus",
           name_owner);

           
  g_dbus_proxy_call (watcher->proxy,
                     "GetActiveTransactions",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     apt_watcher_get_active_transactions_cb,
                     user_data);
}

static void
apt_watcher_get_active_transactions_cb (GObject * obj,
                                        GAsyncResult * res,
                                        gpointer user_data)
{
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* self = APT_WATCHER (user_data);

	GError * error = NULL;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->proxy, res, &error);

	if (error != NULL) {
    g_warning ("unable to complete the fetching of active transactions");
    g_error_free (error);
		return;
	}
  
  g_debug ("WE GOT SOME ACTIVE TRANSACTIONS TO EXAMINE, type is %s",
            g_variant_get_type_string (result));
  //gchar ** transactions = g_variant_get_strv (result);             
  
}

static void
apt_watcher_on_name_vanished (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data)
{
  g_print ("Name %s does not exist on %s\n",
           name,
           "the system bus");
}
