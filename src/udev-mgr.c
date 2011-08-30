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

#include <glib/gi18n.h>

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
static void udev_mgr_handle_webcam (UdevMgr* self,
                                    GUdevDevice* device,
                                    UdevMgrDeviceAction action);                                                      
static void udev_mgr_handle_scsi_device (UdevMgr* self,
                                         GUdevDevice* device,
                                         UdevMgrDeviceAction action);

static void udev_mgr_cleanup_lists(gpointer data, gpointer self);
static void udev_mgr_cleanup_entries(gpointer data, gpointer self);


static void debug_device (UdevMgr* self,
                          GUdevDevice* device,
                          UdevMgrDeviceAction action);
                          
static gchar* format_device_name (UdevMgr* self,
                                  const gchar* brand,
                                  const gchar* generic,
                                  const gchar* branded) G_GNUC_WARN_UNUSED_RESULT;
struct _UdevMgr
{
	GObject parent_instance;
  DbusmenuMenuitem* scanner_item;
  DbusmenuMenuitem* webcam_item;  
  GUdevClient* client;  
  GHashTable* supported_usb_scanners;
  GHashTable* supported_scsi_scanners;
  GHashTable* scanners_present;
  GHashTable* webcams_present;
};

const char *subsystems[3] = {"usb", "scsi", "video4linux"};
const gchar* usb_subsystem = "usb";
const gchar* scsi_subsystem = "scsi";
const gchar* video4linux_subsystem = "video4linux";


G_DEFINE_TYPE (UdevMgr, udev_mgr, G_TYPE_OBJECT);

static void
udev_mgr_init (UdevMgr* self)
{
  self->client = NULL;
  self->supported_usb_scanners = NULL;
  self->scanners_present = NULL;
  self->webcams_present = NULL;
  self->client = g_udev_client_new (subsystems);  
  self->supported_usb_scanners = g_hash_table_new_full (g_str_hash,
                                                        g_str_equal,
                                                        g_free,
                                                        (GDestroyNotify)udev_mgr_cleanup_lists);
  self->supported_scsi_scanners = g_hash_table_new_full (g_str_hash,
                                                         g_str_equal,
                                                         g_free,
                                                         (GDestroyNotify)udev_mgr_cleanup_lists);
  self->scanners_present = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  g_free);
  self->webcams_present = g_hash_table_new_full (g_str_hash,
                                                 g_str_equal,
                                                 g_free,
                                                 g_free);
  
  // load into memory all supported scanners ...
  populate_usb_scanners (self->supported_usb_scanners);
  populate_scsi_scanners (self->supported_scsi_scanners);
  g_signal_connect (G_OBJECT (self->client),
                   "uevent",
                    G_CALLBACK (udev_mgr_uevent_cb),
                    self);
}

static void
udev_mgr_cleanup_lists(gpointer data, gpointer self)
{
  GList* scanners = (GList*)data;
  g_list_foreach (scanners, udev_mgr_cleanup_entries, NULL);  
  g_list_free(scanners);
}

static void
udev_mgr_cleanup_entries(gpointer data, gpointer self)
{
  gchar* entry = (gchar*)data;
  g_free(entry);
}

static void
udev_mgr_finalize (GObject *object)
{
  UdevMgr* self = UDEV_MGR (object);
  g_hash_table_destroy (self->supported_scsi_scanners);  
  g_hash_table_destroy (self->supported_usb_scanners);  
  g_hash_table_destroy (self->scanners_present);  
  g_hash_table_destroy (self->webcams_present);  
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

  const gchar* subsystem = NULL; 
  subsystem = g_udev_device_get_subsystem (device);

  if (g_strcmp0 (subsystem, "usb") == 0){
    udev_mgr_check_if_usb_device_is_supported (self, device, ADD);
  }
  else if (g_strcmp0 (subsystem, "video4linux") == 0){
    udev_mgr_handle_webcam (self, device, ADD);    
  }
  else if (g_strcmp0 (subsystem, "scsi") == 0){
    udev_mgr_handle_scsi_device (self, device, ADD);    
  }
  
  g_object_unref (device);
}


static void udev_mgr_update_menuitems (UdevMgr* self)
{
  dbusmenu_menuitem_property_set_bool (self->scanner_item,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       g_hash_table_size (self->scanners_present) > 0);

  dbusmenu_menuitem_property_set_bool (self->webcam_item,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       g_hash_table_size (self->webcams_present) > 0);

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
  subsystem = g_udev_device_get_subsystem (device);

  if (g_strcmp0 (subsystem, "usb") == 0){
    udev_mgr_check_if_usb_device_is_supported (self,
                                               device,
                                               udev_mgr_action);
  }
  else if (g_strcmp0 (subsystem, "video4linux") == 0){
    udev_mgr_handle_webcam (self, device, udev_mgr_action);
  }
  else if (g_strcmp0 (subsystem, "scsi") == 0){
    udev_mgr_handle_scsi_device (self, device, udev_mgr_action);    
  }
}


static void
udev_mgr_handle_webcam (UdevMgr* self,
                        GUdevDevice* device,
                        UdevMgrDeviceAction action)
{
  if (FALSE)
    debug_device (self, device, action);    
  
  const gchar* vendor;
  const gchar* product;
  
  vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
  product = g_udev_device_get_property (device, "ID_MODEL_ID");

  if (!vendor || !product) {
    return;
  }  

  if (action == REMOVE){
    if (g_hash_table_lookup (self->webcams_present, product) == NULL){
      g_warning ("Got a remove event on a webcam device but we don't have that device in our webcam cache");
      return;                     
    }
    g_hash_table_remove (self->webcams_present,
                         product);
    dbusmenu_menuitem_property_set (self->webcam_item,
                                    DBUSMENU_MENUITEM_PROP_LABEL,
                                    _("Webcam"));                            
  }
  else {
    if (g_hash_table_lookup (self->webcams_present, product) != NULL){
      g_warning ("Got an ADD event on a webcam device but we already have that device in our webcam cache");
      return;                     
    }
    
    const gchar* manufacturer = NULL;        
    manufacturer = g_udev_device_get_property (device, "ID_VENDOR");
    
    if (manufacturer != NULL){
	  gchar * label = format_device_name(self, manufacturer, _("Webcam"), _("%s Webcam"));
      dbusmenu_menuitem_property_set (self->webcam_item,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      label);
	  g_free(label);
    }
    
    g_hash_table_insert (self->webcams_present,
                         g_strdup (product),
                         g_strdup (vendor));                               
  }
  udev_mgr_update_menuitems (self);  
}                        

static void
debug_device (UdevMgr* self,
              GUdevDevice* device,
              UdevMgrDeviceAction action)
{
  const gchar* vendor;
  const gchar* product;
  const gchar* number;
  const gchar* name;
  const gchar* manufacturer;
  
	vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
	manufacturer = g_udev_device_get_property (device, "ID_VENDOR");
	product = g_udev_device_get_property (device, "ID_MODEL_ID");
  number = g_udev_device_get_number (device);
  name = g_udev_device_get_name (device);
  
  g_debug ("%s device vendor id %s , product id of %s , number of %s and name of %s",
           g_strdup(manufacturer),
           g_strdup(vendor),
           g_strdup(product),
           g_strdup(number),
           g_strdup(name));
         
  /*const gchar *const *list;
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
  }*/ 
}
// TODO SCSI is not dynamic right ?
// i.e. just need to handle startup scan.
static void udev_mgr_handle_scsi_device (UdevMgr* self,
                                         GUdevDevice* device,
                                         UdevMgrDeviceAction action)
{
  const gchar* type = NULL;
	type = g_udev_device_get_property (device, "TYPE");
  
  if (!type) {
    return;
  }  
  
  // apparently anything thats type 6 and SCSI is a Scanner
  if (g_strcmp0 (type, "6") == 0 && action == ADD){
    
    const gchar* manufacturer = NULL;        
    manufacturer = g_udev_device_get_property (device, "ID_VENDOR");
    
    if (manufacturer != NULL){
	    gchar * label = format_device_name(self, manufacturer, _("Scanner"), _("%s Scanner"));
      dbusmenu_menuitem_property_set (self->scanner_item,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      label);
	    g_free(label);
    }
    
    gchar* random_scanner_name = 	g_strdup_printf("%p--scanner", self);
    g_hash_table_insert (self->scanners_present,
                         random_scanner_name,
                         g_strdup("Scanner")); 
    udev_mgr_update_menuitems (self);    
    return;                         
  }

  // We only care about type 3 for the special cases below
  if (g_strcmp0 (type, "3") != 0){
    return;
  }

  const gchar* vendor = NULL;
	vendor = g_udev_device_get_property (device, "VENDOR");
  
  if (vendor == NULL)
    return;

  GList* vendor_list = NULL;
  vendor_list = g_hash_table_lookup (self->supported_scsi_scanners,
                                     (gpointer)vendor);
  if (vendor_list == NULL)
    return;

  const gchar* model_id = NULL;
	model_id = g_udev_device_get_property (device, "MODEL");
  
  if (model_id == NULL)
    return;

  GList* model_entry = NULL;
  model_entry = g_list_find_custom (vendor_list,
                                    model_id,
                                    (GCompareFunc)g_strcmp0);
    
  if (model_entry != NULL){
    if (action == REMOVE){
      if (g_hash_table_lookup (self->scanners_present, g_strdup(vendor)) == NULL){
        g_warning ("Got an REMOVE event on a scanner device but we dont have that device in our scanners cache");
      }
      else{
        g_hash_table_remove (self->scanners_present, vendor);
        dbusmenu_menuitem_property_set (self->scanner_item,
                                        DBUSMENU_MENUITEM_PROP_LABEL,
                                        _("Scanner"));
        
      }
    }
    else{      
      if (g_hash_table_lookup (self->scanners_present, g_strdup(vendor)) != NULL){
        g_warning ("Got an ADD event on a scanner device but we already have that device in our scanners cache");
      }
      else{
        const gchar* manufacturer = NULL;        
        manufacturer = g_udev_device_get_property (device, "ID_VENDOR");
        
        if (manufacturer != NULL){
          gchar * label = format_device_name(self, manufacturer, _("Scanner"), _("%s Scanner"));
          dbusmenu_menuitem_property_set (self->scanner_item,
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);
          g_free(label);
        }
        g_hash_table_insert (self->scanners_present,
                             g_strdup(vendor),
                             g_strdup(model_id)); 
      }
    }
    udev_mgr_update_menuitems (self);
  }
}                                         

static void
udev_mgr_check_if_usb_device_is_supported (UdevMgr* self, 
                                           GUdevDevice *device,
                                           UdevMgrDeviceAction action)
{
  const gchar* vendor = NULL;
  //debug_device (self, device, action);    
  
	vendor = g_udev_device_get_property (device, "ID_VENDOR_ID");
  
  if (vendor == NULL)
    return;

  //g_debug ("vendor = %s", vendor);
  
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
      if (g_hash_table_lookup (self->scanners_present, g_strdup(vendor)) == NULL){
        g_warning ("Got an REMOVE event on a scanner device but we dont have that device in our scanners cache");
      }
      else{
        g_hash_table_remove (self->scanners_present, vendor);
        dbusmenu_menuitem_property_set (self->scanner_item, 
                                        DBUSMENU_MENUITEM_PROP_LABEL,
                                        _("Scanner"));        
      }
    }
    else{      
      if (g_hash_table_lookup (self->scanners_present, g_strdup(vendor)) != NULL){
        g_warning ("Got an ADD event on a scanner device but we already have that device in our scanners cache");
      }
      else{
        const gchar* manufacturer = NULL;
        
        manufacturer = g_udev_device_get_property (device, "ID_VENDOR");
        if (manufacturer != NULL){
          gchar * label = format_device_name(self, manufacturer, _("Scanner"), _("%s Scanner"));
          dbusmenu_menuitem_property_set (self->scanner_item, 
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);
          g_free(label);
        }
                                        
        g_hash_table_insert (self->scanners_present,
                             g_strdup(vendor),
                             g_strdup(model_id)); 
      }
    }
    udev_mgr_update_menuitems (self);
  }
}

UdevMgr* udev_mgr_new (DbusmenuMenuitem* scanner, 
                       DbusmenuMenuitem* webcam)
{
  UdevMgr* mgr = g_object_new (UDEV_TYPE_MGR, NULL);
  mgr->scanner_item = scanner;
  mgr->webcam_item = webcam;
  
  // Check for USB devices
  GList* usb_devices_available = NULL;
  usb_devices_available  = g_udev_client_query_by_subsystem (mgr->client,
                                                             usb_subsystem);
  if (usb_devices_available != NULL){
    g_list_foreach (usb_devices_available,
                    udevice_mgr_device_list_iterator,
                    mgr);
                    
    g_list_free (usb_devices_available);
  }  
  // Check for webcams
  GList* video_devices_available  = NULL;
  video_devices_available  = g_udev_client_query_by_subsystem (mgr->client,
                                                               video4linux_subsystem);
  if (video_devices_available != NULL){
    g_list_foreach (video_devices_available,
                    udevice_mgr_device_list_iterator,
                    mgr);
    
    g_list_free (video_devices_available);
  }  
  // Check for SCSI devices
  GList* scsi_devices_available = NULL;
  scsi_devices_available  = g_udev_client_query_by_subsystem (mgr->client,
                                                              scsi_subsystem);
  if (scsi_devices_available != NULL){
    g_list_foreach (scsi_devices_available,
                    udevice_mgr_device_list_iterator,
                    mgr);                    
    g_list_free (scsi_devices_available);
  }
  return mgr;
}

static gchar* format_device_name (UdevMgr* self,
                                  const gchar* brand,
                                  const gchar* generic,
                                  const gchar* branded)
{
  // We don't want to accommodate long names
  if (strlen(brand) > 7)
    return g_strdup(generic);

  gint i = 0;

  // If it contains something other than an alphabetic entry ignore it.
  for(i = 0; i < sizeof(brand); i++){
    if ( !g_ascii_isalpha (brand[i]) )
      return g_strdup(generic);
  }

  gchar* lowered = g_ascii_strdown (brand, -1);
  lowered[0] = g_ascii_toupper (lowered[0]);
  gchar* label = g_strdup_printf(branded, lowered);
  g_free (lowered);  
  return label;
}
