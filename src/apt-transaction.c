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


static void apt_transaction_on_properties_changed (GDBusProxy          *proxy,
                                                   GVariant            *changed_properties,
                                                   const gchar* const  *invalidated_properties,
                                                   gpointer             user_data);

static void apt_transaction_investigate (AptTransaction* self);
static void apt_transaction_simulate_transaction_cb (GObject * obj,
                                                     GAsyncResult * res,
                                                     gpointer user_data);

struct _AptTransaction
{
	GObject       parent_instance;
	GDBusProxy *  proxy;    
  gchar*        id;
};

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
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	object_class->finalize = apt_transaction_finalize;
}

static void
apt_transaction_investigate(AptTransaction* self)
{
  GError * error = NULL;

  self->proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                               G_DBUS_PROXY_FLAGS_NONE,
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
  g_debug ("connecting to the properties changed signal on the transaction object");
  g_signal_connect (self->proxy,
                    "g-properties-changed",
                    G_CALLBACK (apt_transaction_on_properties_changed),
                    self);    

  g_debug ("calling simulate on the transaction object");
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
apt_transaction_on_properties_changed (GDBusProxy          *proxy,
                                       GVariant            *changed_properties,
                                       const gchar* const  *invalidated_properties,
                                       gpointer             user_data)
{
  g_print (" *** Apt Transaction Properties Changed:\n");

  if (g_variant_n_children (changed_properties) > 0)
  {
    GVariantIter *iter;
    const gchar *key;
    GVariant *value;

    g_print (" *** Apt Transaction Properties Changed:\n");
    g_variant_get (changed_properties,
                   "a{sv}",
                   &iter);
    while (g_variant_iter_loop (iter, "{&sv}", &key, &value))
    {
      gchar *value_str;
      value_str = g_variant_print (value, TRUE);
      g_print ("      %s -> %s\n", key, value_str);
      g_free (value_str);
    }
    g_variant_iter_free (iter);
  }    
}

static void
apt_transaction_simulate_transaction_cb (GObject * obj,
                                         GAsyncResult * res,
                                         gpointer user_data)
{
  g_debug ("Simulate return");
  g_return_if_fail (APT_IS_TRANSACTION (user_data));
  AptTransaction* self = APT_TRANSACTION (user_data);

	GError * error = NULL;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->proxy, res, &error);

	if (error != NULL) {
    g_warning ("unable to complete the simulate call");
    g_error_free (error);
		return;
	}
  
  g_debug ("simulate returned a %s",
           g_variant_get_type_string (result));
  
}

AptTransaction* apt_transaction_new (gchar* transaction_id)
{
  AptTransaction* tr = g_object_new (APT_TYPE_TRANSACTION, NULL);
  tr->id = transaction_id;
  g_debug ("Apt transaction new id = %s", tr->id);
  apt_transaction_investigate (tr);
  return tr;
}
