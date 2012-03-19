/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include <libdbusmenu-gtk/menu.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>
#include <libindicator/indicator-image-helper.h>

#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"
#include "user-widget.h"

#define INDICATOR_SESSION_TYPE            (indicator_session_get_type ())
#define INDICATOR_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INDICATOR_SESSION_TYPE, IndicatorSession))
#define INDICATOR_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INDICATOR_SESSION_TYPE, IndicatorSessionClass))
#define IS_INDICATOR_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INDICATOR_SESSION_TYPE))
#define IS_INDICATOR_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INDICATOR_SESSION_TYPE))
#define INDICATOR_SESSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INDICATOR_SESSION_TYPE, IndicatorSessionClass))

typedef struct _IndicatorSession      IndicatorSession;
typedef struct _IndicatorSessionClass IndicatorSessionClass;

struct _IndicatorSessionClass {
	IndicatorObjectClass parent_class;
};

struct _IndicatorSession {
	IndicatorObject parent;
	IndicatorServiceManager * service;
  IndicatorObjectEntry users;
  IndicatorObjectEntry devices;
  gboolean show_users_entry;
	GCancellable * service_proxy_cancel;
	GDBusProxy * service_proxy;
};

static gboolean greeter_mode;

GType indicator_session_get_type (void);

/* Indicator stuff */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_SESSION_TYPE)

/* Prototypes */
static gboolean build_menu_switch (DbusmenuMenuitem * newitem,
                                   DbusmenuMenuitem * parent,
                                   DbusmenuClient * client,
                                   gpointer user_data);
static gboolean new_user_item (DbusmenuMenuitem * newitem,
                               DbusmenuMenuitem * parent,
                               DbusmenuClient * client,
                               gpointer user_data);
static gboolean build_restart_item (DbusmenuMenuitem * newitem,
                                    DbusmenuMenuitem * parent,
                                    DbusmenuClient * client,
                                    gpointer user_data);
static void indicator_session_update_users_label (IndicatorSession* self,
                                                  const gchar* name);
static void service_connection_cb (IndicatorServiceManager * sm, gboolean connected, gpointer user_data);
static void receive_signal (GDBusProxy * proxy, gchar * sender_name, gchar * signal_name, GVariant * parameters, gpointer user_data);
static void service_proxy_cb (GObject * object, GAsyncResult * res, gpointer user_data);
static void user_real_name_get_cb (GObject * obj, GAsyncResult * res, gpointer user_data);
static void user_menu_visibility_get_cb (GObject* obj, GAsyncResult* res, gpointer user_data);

static void indicator_session_class_init (IndicatorSessionClass *klass);
static void indicator_session_init       (IndicatorSession *self);
static void indicator_session_dispose    (GObject *object);
static void indicator_session_finalize   (GObject *object);
static GList* indicator_session_get_entries (IndicatorObject* obj);
static guint indicator_session_get_location (IndicatorObject * io,
                                             IndicatorObjectEntry * entry);
                                             
G_DEFINE_TYPE (IndicatorSession, indicator_session, INDICATOR_OBJECT_TYPE);

static void 
indicator_session_class_init (IndicatorSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = indicator_session_dispose;
	object_class->finalize = indicator_session_finalize;

	IndicatorObjectClass * io_class = INDICATOR_OBJECT_CLASS(klass);
	io_class->get_entries = indicator_session_get_entries;
	io_class->get_location = indicator_session_get_location;
	return;
}

static void
indicator_session_init (IndicatorSession *self)
{
	self->service = NULL;
	self->service_proxy_cancel = NULL;
	self->service_proxy = NULL;
  self->show_users_entry = FALSE;
  
	/* Now let's fire these guys up. */
	self->service = indicator_service_manager_new_version(INDICATOR_SESSION_DBUS_NAME,
                                                        INDICATOR_SESSION_DBUS_VERSION);
	g_signal_connect(G_OBJECT(self->service),
                   INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE,
                   G_CALLBACK(service_connection_cb), self);

  GtkWidget* avatar_icon = NULL;
  // users
  self->users.name_hint = PACKAGE"-users";
  self->users.menu =  GTK_MENU (dbusmenu_gtkmenu_new (INDICATOR_USERS_DBUS_NAME,
                                                      INDICATOR_USERS_DBUS_OBJECT));
  // Set the image to the default avator image
  GdkPixbuf* pixbuf  = NULL; 
  GError* error = NULL;
  pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                     "avatar-default",
                                     17,
                                     GTK_ICON_LOOKUP_FORCE_SIZE,
                                     &error);
  
  // I think the avatar image is available always but just in case have a fallback
  if (error != NULL) {
    g_warning ("Could not load the default avatar image for some reason");
    self->users.image = indicator_image_helper (USER_ITEM_ICON_DEFAULT);
  }
  else{
    avatar_icon = gtk_image_new ();
    gtk_image_set_from_pixbuf (GTK_IMAGE (avatar_icon), pixbuf);
    self->users.image = GTK_IMAGE (avatar_icon);
    g_object_unref (pixbuf);
    g_error_free (error);
  }
                                                      
  self->users.label = GTK_LABEL (gtk_label_new (NULL));
  self->users.accessible_desc = _("User Menu");

  const gchar *greeter_var;
  greeter_var = g_getenv("INDICATOR_GREETER_MODE");
  greeter_mode = g_strcmp0(greeter_var, "1") == 0;

  // devices
  self->devices.name_hint = PACKAGE"-devices";
  self->devices.accessible_desc = _("Device Menu");
  self->devices.menu = GTK_MENU (dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME,
                                                      INDICATOR_SESSION_DBUS_OBJECT));
  if (greeter_mode){
    self->devices.image = indicator_image_helper (GREETER_ICON_DEFAULT);
  }
  else{
    self->devices.image = indicator_image_helper (ICON_DEFAULT);
  }
  
  gtk_widget_show (GTK_WIDGET(self->devices.menu));
  gtk_widget_show (GTK_WIDGET(self->devices.image));
  gtk_widget_show (GTK_WIDGET(self->users.image));
  gtk_widget_show (GTK_WIDGET(self->users.menu));
  
  g_object_ref_sink (self->users.menu);
  g_object_ref_sink (self->users.image);
  g_object_ref_sink (self->devices.menu);
  g_object_ref_sink (self->devices.image);
  
  // Setup the handlers for users
	DbusmenuClient * users_client = DBUSMENU_CLIENT(dbusmenu_gtkmenu_get_client(DBUSMENU_GTKMENU(self->users.menu)));
	dbusmenu_client_add_type_handler (users_client,
                                    USER_ITEM_TYPE,
                                    new_user_item);
	dbusmenu_client_add_type_handler_full (users_client,
                                         MENU_SWITCH_TYPE,
                                         build_menu_switch,
                                         self, NULL);
  
  // Setup the handlers for devices
	DbusmenuClient * devices_client = DBUSMENU_CLIENT(dbusmenu_gtkmenu_get_client(DBUSMENU_GTKMENU(self->devices.menu)));
	dbusmenu_client_add_type_handler (devices_client,
                                    RESTART_ITEM_TYPE,
                                    build_restart_item);
  
	GtkAccelGroup * agroup = gtk_accel_group_new();
	dbusmenu_gtkclient_set_accel_group(DBUSMENU_GTKCLIENT(devices_client), agroup);
	return;
}

static void
indicator_session_dispose (GObject *object)
{
	IndicatorSession * self = INDICATOR_SESSION(object);

	if (self->service != NULL) {
		g_object_unref(G_OBJECT(self->service));
		self->service = NULL;
	}

	if (self->service_proxy != NULL) {
		g_object_unref(self->service_proxy);
		self->service_proxy = NULL;
	}

	if (self->service_proxy_cancel != NULL) {
		g_cancellable_cancel(self->service_proxy_cancel);
		g_object_unref(self->service_proxy_cancel);
		self->service_proxy_cancel = NULL;
	}
  
  if (self->users.menu != NULL) {
    g_object_unref (self->users.menu);
  }
  
  if (self->devices.menu != NULL) {
    g_object_unref (self->devices.menu);
  }

	G_OBJECT_CLASS (indicator_session_parent_class)->dispose (object);
	return;
}

static void
indicator_session_finalize (GObject *object)
{

	G_OBJECT_CLASS (indicator_session_parent_class)->finalize (object);
	return;
}

static GList*
indicator_session_get_entries (IndicatorObject* obj)
{
	g_return_val_if_fail(IS_INDICATOR_SESSION(obj), NULL);
  IndicatorSession* self = INDICATOR_SESSION (obj);
  
  g_debug ("get entries");
	GList * retval = NULL;
  // Only show the users menu if we have more than one
  if (self->show_users_entry == TRUE){
    retval = g_list_prepend (retval, &self->users);
  }
  retval = g_list_prepend (retval, &self->devices);

	if (retval != NULL) {
		retval = g_list_reverse(retval);
	}
	return retval;  
}

static guint
indicator_session_get_location (IndicatorObject * io,
                                IndicatorObjectEntry * entry)
{  
	IndicatorSession * self = INDICATOR_SESSION (io);
  if (entry == &self->users){
    return 0;
  }
  else if (entry == &self->devices){
    return 1;
  }
  g_warning ("IOEntry handed to us to position but we don't own it!");
  return 0;
}

/* callback for the service manager state of being */
static void
service_connection_cb (IndicatorServiceManager * sm, gboolean connected, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION (user_data);

	if (connected) {
    if (self->service_proxy != NULL){
      // Its a reconnect !
      // Fetch synchronisation data and return (proxy is still legit)
      g_dbus_proxy_call (self->service_proxy,
                         "GetUserMenuVisibility",
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         user_menu_visibility_get_cb,
                         user_data);                               
      g_dbus_proxy_call (self->service_proxy,
                         "GetUserRealName",
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         user_real_name_get_cb,
                         user_data);      
      return;
    }
    
	  self->service_proxy_cancel = g_cancellable_new();
	  g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                              G_DBUS_PROXY_FLAGS_NONE,
                              NULL,
                              INDICATOR_SESSION_DBUS_NAME,
                              INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
                              INDICATOR_SESSION_SERVICE_DBUS_IFACE,
                              self->service_proxy_cancel,
                              service_proxy_cb,
                              self);
  }    
	return;
}


static void
service_proxy_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	IndicatorSession * self = INDICATOR_SESSION(user_data);
	g_return_if_fail(self != NULL);

	GDBusProxy * proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

	if (self->service_proxy_cancel != NULL) {
		g_object_unref(self->service_proxy_cancel);
		self->service_proxy_cancel = NULL;
	}

	if (error != NULL) {
		g_warning("Could not grab DBus proxy for %s: %s", INDICATOR_SESSION_DBUS_NAME, error->message);
		g_error_free(error);
		return;
	}

	/* Okay, we're good to grab the proxy at this point, we're
	sure that it's ours. */
	self->service_proxy = proxy;

	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);
  
  // Figure out whether we should show the user menu at all.
  g_dbus_proxy_call (self->service_proxy,
                     "GetUserMenuVisibility",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     user_menu_visibility_get_cb,
                     user_data);                               
  
  // Fetch the user's real name for the user entry label
  g_dbus_proxy_call (self->service_proxy,
                     "GetUserRealName",
                     NULL,
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     user_real_name_get_cb,
                     user_data);
	return;
}


static gboolean
new_user_item (DbusmenuMenuitem * newitem,
               DbusmenuMenuitem * parent,
               DbusmenuClient * client,
               gpointer user_data)
{
  

  GtkWidget* user_item = NULL;

  g_return_val_if_fail(DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail(DBUSMENU_IS_GTKCLIENT(client), FALSE);

  user_item = user_widget_new(newitem);

  GtkMenuItem *user_widget = GTK_MENU_ITEM(user_item);

  dbusmenu_gtkclient_newitem_base (DBUSMENU_GTKCLIENT(client),
                                   newitem,
                                   user_widget,
                                   parent);

  g_debug ("%s (\"%s\")", __func__,
           dbusmenu_menuitem_property_get (newitem,
                                           USER_ITEM_PROP_NAME));
  gtk_widget_show_all (user_item);

  return TRUE;
}


static void
user_real_name_get_cb (GObject * obj, GAsyncResult * res, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);
	GError * error = NULL;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->service_proxy, res, &error);

	if (error != NULL) {
    g_warning ("unable to complete real name dbus query");
    g_error_free (error);
		return;
	}
  
  const gchar* username = NULL;
  g_variant_get (result, "(&s)", &username);
  indicator_session_update_users_label (self, username);
	return;
}

static void 
user_menu_visibility_get_cb (GObject* obj, GAsyncResult* res, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);
	GError * error = NULL;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->service_proxy, res, &error);

	if (error != NULL) {
    g_warning ("unable to complete real name dbus query");
    g_error_free (error);
		return;
	}
  gboolean update;
  g_variant_get (result, "(b)", &update);
  
  // If it is what we had before no need to do anything...
  if (self->show_users_entry == update){
    return;
  }  
  //Otherwise
  self->show_users_entry = update;

  if (self->show_users_entry == TRUE){
    g_signal_emit_by_name ((gpointer)self,
                           "entry-added",
                           &self->users);   
  }
  else{
    g_signal_emit_by_name ((gpointer)self,
                           "entry-removed",
                           &self->users);       
  }
}

/* Receives all signals from the service, routed to the appropriate functions */
static void
receive_signal (GDBusProxy * proxy,
                gchar * sender_name,
                gchar * signal_name,
                GVariant * parameters,
                gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);

	if (g_strcmp0(signal_name, "UserRealNameUpdated") == 0) {
    const gchar* username = NULL;
    g_variant_get (parameters, "(&s)", &username);
    indicator_session_update_users_label (self, username);	
  }
  else if (g_strcmp0(signal_name, "UserMenuIsVisible") == 0) {
    gboolean update;
    g_variant_get (parameters, "(b)", &update);
    
    // If it is what we had before no need to do anything...
    if (self->show_users_entry == update){
      return;
    }
    
    //Otherwise
    self->show_users_entry = update;
    
    if (self->show_users_entry == TRUE){
      g_signal_emit_by_name ((gpointer)self,
                             "entry-added",
                             &self->users);
                             
    }   
    else{
      g_signal_emit_by_name ((gpointer)self,
                             "entry-removed",
                             &self->users);       
    }
  }
  else if (g_strcmp0(signal_name, "RestartRequired") == 0) {    
    if (greeter_mode == TRUE){
      indicator_image_helper_update(self->devices.image, GREETER_ICON_RESTART);
    }
    else{
      g_debug ("reboot required");
      indicator_image_helper_update(self->devices.image, ICON_RESTART);
    }
    self->devices.accessible_desc = _("Device Menu (reboot required)");
    g_signal_emit(G_OBJECT(self), INDICATOR_OBJECT_SIGNAL_ACCESSIBLE_DESC_UPDATE_ID, 0, &(self->devices));
  }  
}



static void
switch_property_change (DbusmenuMenuitem * item,
                        const gchar * property,
                        GVariant * variant,
                        gpointer user_data)
{
  if (g_strcmp0 (property, MENU_SWITCH_USER) != 0) {
    return;
  }
	
  GtkMenuItem * gmi = dbusmenu_gtkclient_menuitem_get(DBUSMENU_GTKCLIENT(user_data), item);
  gchar * finalstring = NULL;
  gboolean set_ellipsize = FALSE;
  gboolean no_name_in_lang = FALSE;

  const gchar * translate = C_("session_menu:switchfrom", "1");
  if (g_strcmp0(translate, "1") != 0) {
    no_name_in_lang = TRUE;
  }
  
  GSettings* settings = g_settings_new ("com.canonical.indicator.session");
  gboolean use_username = g_settings_get_boolean (settings,
                                                  "use-username-in-switch-item");    
  g_object_unref (settings);

  if (variant == NULL || g_variant_get_string(variant, NULL) == NULL ||
      g_variant_get_string(variant, NULL)[0] == '\0' || no_name_in_lang 
      || use_username == FALSE) {
    finalstring = _("Switch User Account…");
    set_ellipsize = FALSE;
  }

  if (finalstring == NULL) {
    const gchar * username = g_variant_get_string(variant, NULL);
    GtkStyle * style = gtk_widget_get_style(GTK_WIDGET(gmi));

    PangoLayout * layout = pango_layout_new(gtk_widget_get_pango_context(GTK_WIDGET(gmi)));
    pango_layout_set_text (layout, username, -1);
    pango_layout_set_font_description(layout, style->font_desc);

    gint width;
    pango_layout_get_pixel_size(layout, &width, NULL);
    g_object_unref(layout);
    g_debug("Username width %dpx", width);

    gint point = pango_font_description_get_size(style->font_desc);
    g_debug("Font size %f pt", (gfloat)point / PANGO_SCALE);

    gdouble dpi = gdk_screen_get_resolution(gdk_screen_get_default());
    g_debug("Screen DPI %f", dpi);

    gdouble pixels_per_em = ((point * dpi) / 72.0f) / PANGO_SCALE;
    gdouble ems = width / pixels_per_em;
    g_debug("Username width %fem", ems);

    finalstring = g_strdup_printf(_("Switch From %s…"), username);
    if (ems >= 20.0f) {
      set_ellipsize = TRUE;
    } else {
      set_ellipsize = FALSE;
    }
    
  }
  gtk_menu_item_set_label(gmi, finalstring);

  GtkLabel * label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(gmi)));
  if (label != NULL) {
    if (set_ellipsize) {
      gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_END);
    } else {
      gtk_label_set_ellipsize(label, PANGO_ELLIPSIZE_NONE);
    }
  }
	return;
}

static const gchar * dbusmenu_item_data = "dbusmenu-item";

static void
restart_property_change (DbusmenuMenuitem * item,
                         const gchar * property,
                         GVariant * variant,
                         gpointer user_data)
{
	DbusmenuGtkClient * client = DBUSMENU_GTKCLIENT(user_data);
	GtkMenuItem * gmi = dbusmenu_gtkclient_menuitem_get(client, item);

	if (g_strcmp0(property, RESTART_ITEM_LABEL) == 0) {
		gtk_menu_item_set_label(gmi, g_variant_get_string(variant, NULL));
	} else if (g_strcmp0(property, RESTART_ITEM_ICON) == 0) {
		GtkWidget * image = gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(gmi));

		GIcon * gicon = g_themed_icon_new_with_default_fallbacks(g_variant_get_string(variant, NULL));
		if (image == NULL) {
			image = gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gmi), image);
		} else {
			gtk_image_set_from_gicon(GTK_IMAGE(image), gicon, GTK_ICON_SIZE_MENU);
		}
		g_object_unref(G_OBJECT(gicon));
	}
	return;
}

static gboolean
build_restart_item (DbusmenuMenuitem * newitem,
                    DbusmenuMenuitem * parent,
                    DbusmenuClient * client,
                    gpointer user_data)
{
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_image_menu_item_new());
	if (gmi == NULL) {
		return FALSE;
	}

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(restart_property_change), client);

	GVariant * variant;
	variant = dbusmenu_menuitem_property_get_variant(newitem, RESTART_ITEM_LABEL);
	if (variant != NULL) {
		restart_property_change(newitem, RESTART_ITEM_LABEL, variant, client);
	}

	variant = dbusmenu_menuitem_property_get_variant(newitem, RESTART_ITEM_ICON);
	if (variant != NULL) {
		restart_property_change(newitem, RESTART_ITEM_ICON, variant, client);
	}

	return TRUE;
}

static void
switch_style_set (GtkWidget * widget,
                  GtkStyle * prev_style,
                  gpointer user_data)
{
	DbusmenuGtkClient * client = DBUSMENU_GTKCLIENT(user_data);
	DbusmenuMenuitem * mi = DBUSMENU_MENUITEM(g_object_get_data(G_OBJECT(widget),
                                                              dbusmenu_item_data));

	switch_property_change (mi,
                          MENU_SWITCH_USER,
                          dbusmenu_menuitem_property_get_variant(mi, MENU_SWITCH_USER),
                          client);
	return;
}

static gboolean
build_menu_switch (DbusmenuMenuitem * newitem,
                   DbusmenuMenuitem * parent,
                   DbusmenuClient * client,
                   gpointer user_data)
{
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());
	if (gmi == NULL) {
		return FALSE;
	}
  	
  g_object_set_data(G_OBJECT(gmi), dbusmenu_item_data, newitem);

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect (G_OBJECT(newitem),
                    DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED,
                    G_CALLBACK(switch_property_change),
                    client);
	g_signal_connect (G_OBJECT(gmi),
                    "style-set",
                    G_CALLBACK(switch_style_set),
                    client);
                    
	switch_property_change (newitem,
                          MENU_SWITCH_USER,
                          dbusmenu_menuitem_property_get_variant(newitem, MENU_SWITCH_USER), client);
  	
  return TRUE;
}

static void
indicator_session_update_users_label (IndicatorSession* self, 
                                      const gchar* name)
{  
  if (name == NULL){
    gtk_widget_hide(GTK_WIDGET(self->users.label));
    return;
  }  

  GSettings* settings = g_settings_new ("com.canonical.indicator.session");
  const gboolean use_name = g_settings_get_boolean (settings, "show-real-name-on-panel");    
  gtk_label_set_text (self->users.label, name);
  gtk_widget_set_visible (GTK_WIDGET(self->users.label), use_name);
  g_object_unref (settings);
}
