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

#include "apt-transaction.h"
#include "dbus-shared-names.h"

static void apt_transaction_investigate (AptTransaction* self);
static void apt_transaction_simulate_transaction_cb (GObject * obj,
                                                     GAsyncResult * res,
                                                     gpointer user_data);
static void apt_transaction_receive_signal (GDBusProxy * proxy,
                                            gchar * sender_name,
                                            gchar * signal_name,
                                            GVariant * parameters,
                                            gpointer user_data);

struct _AptTransaction
{
	GObject       parent_instance;
	GDBusProxy *  proxy;    
  gchar*        id;
  AptState      current_state;
};

enum {
  UPDATE,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AptTransaction, apt_transaction, G_TYPE_OBJECT);

static void
apt_transaction_init (AptTransaction *self)
{
  self->proxy = NULL;
  self->id = NULL;
  
}

static void
apt_transaction_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
  AptTransaction* self = APT_TRANSACTION(object);
  if (self->proxy != NULL){
    g_object_unref (self->proxy);
    self->proxy = NULL;
  }    
  g_free (self->id);
	G_OBJECT_CLASS (apt_transaction_parent_class)->finalize (object);
}

static void
apt_transaction_class_init (AptTransactionClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = apt_transaction_finalize;

  signals[UPDATE] =  g_signal_new("state-update",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_SIGNAL_RUN_LAST,
                                   0,
                                   NULL, NULL,
                                   g_cclosure_marshal_VOID__INT,
                                   G_TYPE_NONE, 1, G_TYPE_INT);  
}

static void
apt_transaction_investigate(AptTransaction* self)
{
  GError * error = NULL;

  self->proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                               G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                               NULL, /* GDBusInterfaceInfo */
                                               "org.debian.apt",
                                               self->id,
                                               "org.debian.apt.transaction",
                                               NULL, /* GCancellable */
                                               &error);        
  if (error != NULL) {
    g_warning ("unable to fetch proxy for transaction object path %s", self->id);
    g_error_free (error);
    return;
  }

  g_signal_connect (G_OBJECT(self->proxy),
                    "g-signal",
                    G_CALLBACK (apt_transaction_receive_signal),
                    self);    

  g_dbus_proxy_call (self->proxy,
                     "Simulate",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     apt_transaction_simulate_transaction_cb,
                     self);                                                                                        
}

static void
apt_transaction_receive_signal (GDBusProxy * proxy,
                                gchar * sender_name,
                                gchar * signal_name,
                                GVariant * parameters,
                                gpointer user_data)
{
  g_return_if_fail (APT_IS_TRANSACTION (user_data));
  AptTransaction* self = APT_TRANSACTION(user_data);      

  if (g_strcmp0(signal_name, "PropertyChanged") == 0) 
  {
    gchar* prop_name= NULL;
    GVariant* value = NULL;
    g_variant_get (parameters, "(sv)", &prop_name, &value);
    
    if (g_strcmp0 (prop_name, "Dependencies") == 0){
      
      
      gchar** install = NULL;
      gchar** reinstall = NULL;
      gchar** remove = NULL;
      gchar** purge = NULL;
      gchar** upgrade = NULL;
      gchar** downgrade = NULL;
      gchar** keep = NULL;
      g_variant_get (value, "(asasasasasasas)", &install, 
                     &reinstall, &remove, &purge, &upgrade, &downgrade,
                     &keep);
      
      //g_debug ("Seemed to uppack dependencies without any warnings");
      //g_debug ("Upgrade quantity : %u", g_strv_length(upgrade));
      gboolean upgrade_needed = (g_strv_length(upgrade) > 0) ||
                                (g_strv_length(install) > 0) ||
                                (g_strv_length(reinstall) > 0) ||
                                (g_strv_length(remove) > 0) ||
                                (g_strv_length(purge) > 0);
      if (upgrade_needed == TRUE){
        g_signal_emit (self,
                       signals[UPDATE],
                       0,
                       UPDATES_AVAILABLE);
        
      }
      else{
        g_signal_emit (self,
                       signals[UPDATE],
                       0,
                       UP_TO_DATE);
      }
    }
  }
}

static void
apt_transaction_simulate_transaction_cb (GObject * obj,
                                         GAsyncResult * res,
                                         gpointer user_data)
{
	GError * error = NULL;

	if (error != NULL) {
    g_warning ("unable to complete the simulate call");
    g_error_free (error);
		return;
	}
}

AptTransaction* apt_transaction_new (gchar* transaction_id)
{
  AptTransaction* tr = g_object_new (APT_TYPE_TRANSACTION, NULL);
  tr->id = transaction_id;
  g_debug ("Apt transaction new id = %s", tr->id);
  apt_transaction_investigate (tr);
  return tr;
}
