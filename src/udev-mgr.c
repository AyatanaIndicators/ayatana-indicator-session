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

#include <gudev/gudev.h>

// TEMP
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "udev-mgr.h"
#include "sane-rules.h"

static void udevice_mgr_device_list_iterator (gpointer data,
                                              gpointer userdata);
static void udev_mgr_uevent_cb  (GUdevClient *client,
                                 gchar       *action,
                                 GUdevDevice *device,
                                 gpointer     user_data);   
/*static gboolean udev_mgr_compare_models (gconstpointer data1,
                                         gconstpointer data2);
*/                                                                 
struct _UdevMgr
{
	GObject parent_instance;
  DbusmenuMenuitem* scanner_item;
  DbusmenuMenuitem* webcam_item;  
  GUdevClient* client;  
  GHashTable* supported_scanners;
};

const char *subsystems[1] = {"usb"};
const gchar* usb_subsystem = "usb";

G_DEFINE_TYPE (UdevMgr, udev_mgr, G_TYPE_OBJECT);

/*static void
test_usb_scanners(gpointer data, gpointer user_data)
{
  gchar* model = (gchar*)data;
  g_debug ("in hash table for epsom model %s was found", model);
}*/

static void
udev_mgr_init (UdevMgr* self)
{
  self->client = NULL;
  self->supported_scanners = NULL;

  self->client = g_udev_client_new (subsystems);  
  self->supported_scanners = g_hash_table_new (g_str_hash, g_str_equal);
  populate_usb_scanners(self->supported_scanners);
  GList* devices_available  = g_udev_client_query_by_subsystem (self->client,
                                                                usb_subsystem);
  
  g_list_foreach (devices_available, udevice_mgr_device_list_iterator, self);
  g_list_free (devices_available);
  g_signal_connect (G_OBJECT (self->client),
                   "uevent",
                    G_CALLBACK (udev_mgr_uevent_cb),
                    self);
}
 

static void
udev_mgr_finalize (GObject *object)
{
	G_OBJECT_CLASS (udev_mgr_parent_class)->finalize (object);
}

static void
udev_mgr_class_init (UdevMgrClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = udev_mgr_finalize;
}

static void
udevice_mgr_device_list_iterator (gpointer data, gpointer userdata)
{
  g_return_if_fail (G_UDEV_IS_DEVICE (data));
  g_return_if_fail (UDEV_IS_MGR (userdata));
  
  UdevMgr* self = UDEV_MGR (userdata);
  
  GUdevDevice* device = G_UDEV_DEVICE (data);
  
  const gchar* name = g_udev_device_get_name (device);
  
  const gchar* vendor = NULL;
  const gchar* product = NULL;
  GList* vendor_list = NULL;
  
	vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
  
  if (vendor == NULL){
    g_object_unref (device);
    return;
  }

	product = g_udev_device_get_property (device, "ID_MODEL_ID");
  vendor_list = g_hash_table_lookup(self->supported_scanners, (gpointer)vendor);
  
  GList* model = NULL;
  
  if (vendor_list != NULL){
    model = g_list_find_custom(vendor_list, product, (GCompareFunc)g_strcmp0);
    if (model == NULL){
      g_debug ("CANT FIND THE MODEL %s FOR  VENDOR %s", product, vendor);
    }
    else{
      dbusmenu_menuitem
      g_debug ("WE HAVE A SUCCESSFUL MATCH!");
    }
  }
  g_object_unref (device);
}

static void udev_mgr_uevent_cb (GUdevClient *client,
                                gchar       *action,
                                GUdevDevice *device,
                                gpointer     user_data)   
{
  g_return_if_fail (UDEV_IS_MGR (user_data));

  g_debug ("just received a UEVENT with an action :  %s", action);

  const gchar* vendor;
  const gchar* product;
  const gchar* number;
  
	vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
	product = g_udev_device_get_property (device, "ID_MODEL_ID");
  number = g_udev_device_get_number (device);
 
  g_debug ("device vendor id %s and product id of %s and number of %s",
           g_strdup(vendor),
           g_strdup(product),
           g_strdup(number));
           
  const gchar *const *list;
  const gchar *const *iter;  
  char propstr[500];
  guint32 namelen = 0, i;  
  
  list = g_udev_device_get_property_keys(device);
  
  for (iter = list; iter && *iter; iter++) {
    if (strlen(*iter) > namelen)
      namelen = strlen(*iter);
  }
  namelen++;

  for (iter = list; iter && *iter; iter++) {
    strcpy(propstr, *iter);
    strcat(propstr, ":");
    for (i = 0; i < namelen - strlen(*iter); i++)
           strcat(propstr, " ");
    strcat(propstr, g_udev_device_get_property(device, *iter));
    g_debug("%s", propstr);
  }  
}

/*static gboolean
udev_mgr_compare_models (gconstpointer data1,
                         gconstpointer data2)
{
  return FALSE;
}*/

UdevMgr* udev_mgr_new (DbusmenuMenuitem* scanner, 
                       DbusmenuMenuitem* webcam)
{
  UdevMgr* mgr = g_object_new (UDEV_TYPE_MGR, NULL);
  mgr->scanner_item = scanner;
  mgr->webcam_item = webcam;
  return mgr;
}
