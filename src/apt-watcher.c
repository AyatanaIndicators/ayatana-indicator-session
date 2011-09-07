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

#include <gio/gio.h>
#include <glib/gi18n.h>
#include "apt-watcher.h"
#include "apt-transaction.h"

static guint watcher_id;

struct _AptWatcher
{
	GObject parent_instance;
  guint reboot_query;
	GCancellable * proxy_cancel;
	GDBusProxy * proxy;  
  SessionDbus* session_dbus_interface;
  DbusmenuMenuitem* apt_item;
  AptState current_state;
  AptTransaction* current_transaction;
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
static void fetch_proxy_cb (GObject * object,
                            GAsyncResult * res,
                            gpointer user_data);

static void apt_watcher_upgrade_system_cb (GObject * obj,
                                           GAsyncResult * res,
                                           gpointer user_data);
                                                 
                
static void apt_watcher_show_apt_dialog (DbusmenuMenuitem* mi,
                                         guint timestamp,
                                         gpointer userdata);

static void apt_watcher_signal_cb (GDBusProxy* proxy,
                                   gchar* sender_name,
                                   gchar* signal_name,
                                   GVariant* parameters,
                                   gpointer user_data);
static void  apt_watcher_manage_transactions (AptWatcher* self,
                                              gchar* transaction_id);
static gboolean apt_watcher_query_reboot_status (gpointer self);
static void apt_watcher_transaction_state_simulation_update_cb (AptTransaction* trans,
                                                                gint update,
                                                                gpointer user_data);
static void apt_watcher_transaction_state_real_update_cb (AptTransaction* trans,
                                                          gint update,
                                                          gpointer user_data);

                                   


G_DEFINE_TYPE (AptWatcher, apt_watcher, G_TYPE_OBJECT);

static void
apt_watcher_init (AptWatcher *self)
{
  self->current_state = UP_TO_DATE;
  self->proxy_cancel = g_cancellable_new();
  self->proxy = NULL;
  self->reboot_query = 0;
  self->current_transaction = NULL;
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
  AptWatcher* self = APT_WATCHER (object);
           
  if (self->proxy != NULL)
    g_object_unref (self->proxy);

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
  
	g_signal_connect (self->proxy,
                    "g-signal",
                    G_CALLBACK(apt_watcher_signal_cb),
                    self);   
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
                     "UpgradeSystem",
                     g_variant_new("(b)", TRUE),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     apt_watcher_upgrade_system_cb,
                     user_data);
}

static void
apt_watcher_on_name_vanished (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data)
{
  g_debug ("Name %s does not exist or has just vanished",
           name);
  g_return_if_fail (APT_IS_WATCHER (user_data));
}

static void
apt_watcher_upgrade_system_cb (GObject * obj,
                               GAsyncResult * res,
                               gpointer user_data)
{
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* self = APT_WATCHER (user_data);

	GError * error = NULL;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->proxy, res, &error);

	if (error != NULL) {
    g_warning ("unable to complete the UpgradeSystem apt call");
    g_error_free (error);
		return;
	}
  
  gchar* transaction_id = NULL;
  g_variant_get (result, "(s)", &transaction_id);

  if (transaction_id == NULL){
    g_warning ("apt_watcher_upgrade_system_cb - transaction id is null");
    return;
  }  
  
  apt_watcher_manage_transactions (self, transaction_id);
  
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

static void
apt_watcher_transaction_state_real_update_cb (AptTransaction* trans,
                                              gint update,
                                              gpointer user_data)
{
  g_debug ("apt-watcher -transaction update %i", update);
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* self = APT_WATCHER (user_data);
  
  AptState state = (AptState)update;
  if (state == UP_TO_DATE){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Software Up to Date"));   
    if (self->reboot_query != 0){
      g_source_remove (self->reboot_query);
      self->reboot_query = 0;
    }
    self->reboot_query = g_timeout_add_seconds (2,
                                                apt_watcher_query_reboot_status,
                                                self); 
  }
  else if (state == UPDATES_AVAILABLE){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Updates Available…"));    
  }
  else if (state == UPGRADE_IN_PROGRESS){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Updates Installing…"));    
  }  
  else if (state == FINISHED){
    // Only query if the previous state was an upgrade.
    if (self->current_state == UPGRADE_IN_PROGRESS){
      if (self->reboot_query != 0){
        g_source_remove (self->reboot_query);
        self->reboot_query = 0;
      }
      // Wait a sec before querying for reboot status, 
      // race condition with Apt has been observed.
      self->reboot_query = g_timeout_add_seconds (2,
                                                  apt_watcher_query_reboot_status,
                                                  self); 
    }
  }  
  // Set the current state
  self->current_state = state;
  
  if (self->current_state != UPGRADE_IN_PROGRESS){
    g_object_unref (G_OBJECT(self->current_transaction));
    self->current_transaction = NULL;
  }  
}                                              


static void
apt_watcher_transaction_state_simulation_update_cb (AptTransaction* trans,
                                                    gint update,
                                                    gpointer user_data)
{
  g_debug ("apt-watcher -transaction update %i", update);
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* self = APT_WATCHER (user_data);
  
  AptState state = (AptState)update;
  
  if (state == UP_TO_DATE){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Software Up to Date"));   
    if (self->reboot_query != 0){
      g_source_remove (self->reboot_query);
      self->reboot_query = 0;
    }
    self->reboot_query = g_timeout_add_seconds (2,
                                                apt_watcher_query_reboot_status,
                                                self); 
  }
  else if (state == UPDATES_AVAILABLE){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Updates Available…"));    
  }
  else if (state == UPGRADE_IN_PROGRESS){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Updates Installing…"));    
  }  
  
  self->current_state = state;
  
  if (self->current_state != UPGRADE_IN_PROGRESS){
    g_object_unref (G_OBJECT(self->current_transaction));
    self->current_transaction = NULL;
  }
} 
 
static void
apt_watcher_manage_transactions (AptWatcher* self, gchar* transaction_id)
{
    if (self->current_transaction == NULL){
      self->current_transaction = apt_transaction_new (transaction_id, SIMULATION);
      g_signal_connect (G_OBJECT(self->current_transaction),
                        "state-update",
                        G_CALLBACK(apt_watcher_transaction_state_simulation_update_cb), self);
    }
}

static gboolean
apt_watcher_query_reboot_status (gpointer data)
{
  g_return_val_if_fail (APT_IS_WATCHER (data), FALSE);
  AptWatcher* self = APT_WATCHER (data);
  
  GVariant* reboot_result = g_dbus_proxy_get_cached_property (self->proxy,
                                                             "RebootRequired");
  gboolean reboot;
  g_variant_get (reboot_result, "b", &reboot);
  g_debug ("apt_watcher_query_reboot_status: reboot prop = %i", reboot);
  
  if (reboot == FALSE){
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Software Up to Date"));
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_DISPOSITION,
                                    DBUSMENU_MENUITEM_DISPOSITION_NORMAL);
                                    
  }
  else{
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Reboot Required"));
    dbusmenu_menuitem_property_set (self->apt_item,
                                    DBUSMENU_MENUITEM_PROP_DISPOSITION,
                                    DBUSMENU_MENUITEM_DISPOSITION_ALERT);
    session_dbus_restart_required (self->session_dbus_interface);
    self->current_state = RESTART_NEEDED;
  }
  self->reboot_query = 0;
  return FALSE;
}

static void apt_watcher_signal_cb ( GDBusProxy* proxy,
                                    gchar* sender_name,
                                    gchar* signal_name,
                                    GVariant* parameters,
                                    gpointer user_data)
{
  g_return_if_fail (APT_IS_WATCHER (user_data));
  AptWatcher* self = APT_WATCHER (user_data);

  g_variant_ref (parameters);
  GVariant *value = g_variant_get_child_value (parameters, 0);

  if (g_strcmp0(signal_name, "ActiveTransactionsChanged") == 0){
    gchar* current = NULL;
    g_debug ("ActiveTransactionsChanged");

    g_variant_get(value, "s", &current);

    if (g_str_has_prefix (current, "/org/debian/apt/transaction/") == TRUE){
      g_debug ("ActiveTransactionsChanged - current is %s", current);
     
      // Cancel all existing operations.
      if (self->reboot_query != 0){
        g_source_remove (self->reboot_query);
        self->reboot_query = 0;
      }

      if (self->current_transaction != NULL)
      {
        g_object_unref (G_OBJECT(self->current_transaction));
        self->current_transaction = NULL;
      }

      self->current_transaction = apt_transaction_new (current, REAL);
      g_signal_connect (G_OBJECT(self->current_transaction),
                        "state-update",
                        G_CALLBACK(apt_watcher_transaction_state_real_update_cb), self);              
    }
  }
  g_variant_unref (parameters);
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
                               
