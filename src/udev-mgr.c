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
static void udev_mgr_update_menuitems (UdevMgr* self);
static void udev_mgr_check_if_usb_device_is_supported (UdevMgr* self, 
                                                        GUdevDevice *device,
                                                        UdevMgrDeviceAction action);   
                                                                                                                                             
static void udev_mgr_check_if_scsi_device_is_supported (UdevMgr* self, 
                                                        GUdevDevice *device,
                                                        UdevMgrDeviceAction action);

struct _UdevMgr
{
	GObject parent_instance;
  DbusmenuMenuitem* scanner_item;
  DbusmenuMenuitem* webcam_item;  
  GUdevClient* client;  
  GHashTable* supported_usb_scanners;
  GHashTable* scanners_present;
};

const char *subsystems[2] = {"usb", "scsi"};
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
  self->supported_usb_scanners = NULL;
  self->scanners_present = NULL;
    
  self->client = g_udev_client_new (subsystems);  
  self->supported_usb_scanners = g_hash_table_new (g_str_hash, g_str_equal);
  self->scanners_present = g_hash_table_new (g_str_hash, g_str_equal);
  
  populate_usb_scanners(self->supported_usb_scanners);
  g_signal_connect (G_OBJECT (self->client),
                   "uevent",
                    G_CALLBACK (udev_mgr_uevent_cb),
                    self);
}

static void
udev_mgr_finalize (GObject *object)
{
  // TODO tidy up hashtables. 
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
  udev_mgr_check_if_usb_device_is_supported (self, device, ADD);

  /*const gchar* vendor = NULL;
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
      self->scanners_present += 1;    
      g_debug ("WE HAVE A SUCCESSFUL MATCH!");
    }    
  }
  g_debug ("JUST SET SCANNERS TO TRUE");*/                                                 
  g_object_unref (device);
}


static void udev_mgr_update_menuitems (UdevMgr* self)
{
  dbusmenu_menuitem_property_set_bool (self->scanner_item,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       g_hash_table_size (self->scanners_present) > 0);
}

static void udev_mgr_uevent_cb (GUdevClient *client,
                                gchar       *action,
                                GUdevDevice *device,
                                gpointer     user_data)   
{
  g_return_if_fail (UDEV_IS_MGR (user_data));
  UdevMgr* self = UDEV_MGR (user_data);
  g_return_if_fail (device != NULL);
  
  g_debug ("just received a UEVENT with an action :  %s", action);
 
  UdevMgrDeviceAction udev_mgr_action = ADD;

  if (g_strcmp0 (action, "remove") == 0){
    udev_mgr_action = REMOVE;
  }
  const gchar* subsystem = NULL;
  const gchar* device_type = NULL;
  
  device_type = g_udev_device_get_devtype (device);  
  subsystem = g_udev_device_get_subsystem (device);

  g_debug ("And the subsystem is %s", subsystem);  
  g_debug ("And the device_type is %s", device_type);  

  if (g_strcmp0 (subsystem, "usb") == 0){
    if ( g_udev_device_has_property (device, "ID_USB_INTERFACES")){
      const gchar* id_usb_interfaces = NULL;
      id_usb_interfaces = g_udev_device_get_property (device, "ID_USB_INTERFACES");
      
      GError* error = NULL;
      GMatchInfo *match_info = NULL;
      GRegex* webcam_regex = NULL;
      
      webcam_regex = g_regex_new (":0e[0-9]{4}:*", 
                                   0,
                                   0,
                                   &error);  
      // This seems to be case with certain webcams
      //#":ff[f|0]{4}                                        
      if (error != NULL){
        g_debug ("Errors creating the regex : %s", error->message);        
      }                                                                       
      gboolean result = g_regex_match (webcam_regex,
                                       id_usb_interfaces,
                                       0,
                                       & match_info);
  
      g_debug ("we have found the id usb interfaces : %s", id_usb_interfaces); 
      
      if (result == TRUE){
        g_debug ("SUCCESSFUL MATCH");
      }   
      else {
        g_debug ("NO MATCH");
      }
    }
                                          
    udev_mgr_check_if_usb_device_is_supported (self, device, udev_mgr_action);
  }
  else if (g_strcmp0 (subsystem, "scsi") == 0){
    udev_mgr_check_if_scsi_device_is_supported (self, device, udev_mgr_action);    
  }
  
  return;
  
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


static void
udev_mgr_check_if_scsi_device_is_supported (UdevMgr* self, 
                                            GUdevDevice *device,
                                            UdevMgrDeviceAction action)
{
}

static void
udev_mgr_check_if_usb_device_is_supported (UdevMgr* self, 
                                           GUdevDevice *device,
                                           UdevMgrDeviceAction action)
{
  g_debug ("got a uevent");
  const gchar* vendor = NULL;
  
	vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
  
  if (vendor == NULL)
    return;

  g_debug ("vendor = %s", vendor);
  
  GList* vendor_list = NULL;
  vendor_list = g_hash_table_lookup (self->supported_usb_scanners,
                                     (gpointer)vendor);
  if (vendor_list == NULL)
    return;

  const gchar* model_id = NULL;
	model_id = g_udev_device_get_property (device, "ID_MODEL_ID");
  
  if (model_id == NULL)
    return;
  
  GList* model_entry = NULL;
  model_entry = g_list_find_custom(vendor_list, model_id, (GCompareFunc)g_strcmp0);
    
  if (model_entry != NULL){
    if (action == REMOVE){
      // TODO handle the case where its removed
      // remove it if present from the hash and call update_menuitems
    }
    else{      
      // TODO: populate it with the name of the device
      // this will be needed for the menuitem
      g_hash_table_insert (self->scanners_present,
                           g_strdup(vendor),
                           g_strdup(model_id)); 
    }
  }
  // DEBUG purposes.  
  if (model_entry == NULL){
    g_debug ("CANT FIND THE MODEL %s FOR  VENDOR %s", model_id, vendor);
  }
  else{
    g_debug ("WE HAVE A SUCCESSFUL MATCH!");
  }    
}

UdevMgr* udev_mgr_new (DbusmenuMenuitem* scanner, 
                       DbusmenuMenuitem* webcam)
{
  UdevMgr* mgr = g_object_new (UDEV_TYPE_MGR, NULL);
  mgr->scanner_item = scanner;
  mgr->webcam_item = webcam;

  GList* devices_available  = g_udev_client_query_by_subsystem (mgr->client,
                                                                usb_subsystem);
  g_list_foreach (devices_available,
                  udevice_mgr_device_list_iterator,
                  mgr);
  g_list_free (devices_available);
  udev_mgr_update_menuitems (mgr);  
  return mgr;
}
