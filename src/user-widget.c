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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libindicator/indicator-image-helper.h>
#include "user-widget.h"
#include "dbus-shared-names.h"


typedef struct _UserWidgetPrivate UserWidgetPrivate;

struct _UserWidgetPrivate
{
  DbusmenuMenuitem* twin_item;
  GtkWidget* user_image;
  GtkWidget* user_name;
  GtkWidget* container;
  GtkWidget* tick_icon;
  gboolean logged_in;
  gboolean sessions_active;
};

#define USER_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), USER_WIDGET_TYPE, UserWidgetPrivate))

/* Prototypes */
static void user_widget_class_init    (UserWidgetClass *klass);
static void user_widget_init          (UserWidget *self);
static void user_widget_dispose       (GObject *object);
static void user_widget_finalize      (GObject *object);

static void user_widget_set_twin_item (UserWidget* self,
                                       DbusmenuMenuitem* twin_item);
// keyevent consumers
static gboolean user_widget_button_release_event (GtkWidget *menuitem, 
                                                  GdkEventButton *event);
// Dbusmenuitem properties update callback
static void user_widget_property_update (DbusmenuMenuitem* item,
                                         gchar* property, 
                                         GVariant* value,
                                         gpointer userdata);
#if GTK_CHECK_VERSION(3, 0, 0)
static gboolean user_widget_primitive_draw_cb_gtk_3 (GtkWidget *image,
                                                         cairo_t* cr,
                                                         gpointer user_data);
#else
static gboolean user_widget_primitive_draw_cb (GtkWidget *image,
                                                   GdkEventExpose *event,
                                                   gpointer user_data);
#endif

G_DEFINE_TYPE (UserWidget, user_widget, GTK_TYPE_MENU_ITEM);

static void
user_widget_class_init (UserWidgetClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->button_release_event = user_widget_button_release_event;
  
  g_type_class_add_private (klass, sizeof (UserWidgetPrivate));

  gobject_class->dispose = user_widget_dispose;
  gobject_class->finalize = user_widget_finalize;
}

static void
user_widget_init (UserWidget *self)
{
  UserWidgetPrivate * priv = USER_WIDGET_GET_PRIVATE(self);
  
	gint padding = 0;
	gtk_widget_style_get (GTK_WIDGET(self),
                        "horizontal-padding",
                        &padding,
                        NULL);
  
  priv->user_image = NULL;
  priv->user_name  = NULL;
  priv->logged_in = FALSE;
  priv->sessions_active = FALSE;
  priv->container = NULL;
  priv->tick_icon = NULL;
  
  // Create the UI elements.  
  priv->user_image = gtk_image_new ();
  
  // Just for now set the image to the default avator image
  GdkPixbuf* pixbuf  = NULL; 
  GError* error = NULL;
  pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                     "avatar-default",
                                     32,
                                     GTK_ICON_LOOKUP_FORCE_SIZE,
                                     &error);
  
  if (pixbuf == NULL || error != NULL) {
    g_warning ("Could not load the default avatar image for some reason");
  }
  else{
    gtk_image_set_from_pixbuf (GTK_IMAGE(priv->user_image), pixbuf);
    g_object_unref (pixbuf);
  }

  priv->user_name = gtk_label_new ("");
  priv->container = gtk_hbox_new (FALSE, 0);
  // TODO: 
  // Delete tick icon and draw primitively.
	priv->tick_icon = gtk_image_new_from_icon_name ("account-logged-in",
                                                   GTK_ICON_SIZE_MENU);
 	gtk_misc_set_alignment(GTK_MISC(priv->tick_icon), 1.0, 0.5);
  
  // Pack it together
  gtk_box_pack_start (GTK_BOX (priv->container),
                      priv->user_image,
                      FALSE,
                      FALSE,
                      0);   
  gtk_box_pack_start (GTK_BOX (priv->container),
                      priv->user_name,
                      FALSE,
                      FALSE,
                      3);                       
	gtk_box_pack_start (GTK_BOX(priv->container),
                      priv->tick_icon,
                      FALSE,
                      FALSE, 5);
  
  gtk_widget_show_all (priv->container);
  gtk_container_add (GTK_CONTAINER (self), priv->container);  
  
  // Fetch the drawing context.
  #if GTK_CHECK_VERSION(3, 0, 0) 
  g_signal_connect_after (GTK_WIDGET(self), "draw", 
                          G_CALLBACK(user_widget_primitive_draw_cb_gtk_3),
                          GTK_WIDGET(self));
  #else
  g_signal_connect_after (GTK_WIDGET(self), "expose-event", 
                          G_CALLBACK(user_widget_primitive_draw_cb),
                          GTK_WIDGET(self));  
  #endif  
}

static void
user_widget_dispose (GObject *object)
{
  //UserWidgetPrivate * priv = USER_WIDGET_GET_PRIVATE(USER_WIDGET(object)); 

  G_OBJECT_CLASS (user_widget_parent_class)->dispose (object);
}

// TODO tidy up image and name
static void
user_widget_finalize (GObject *object)
{
  G_OBJECT_CLASS (user_widget_parent_class)->finalize (object);
}

/**
 * We override the expose method to enable primitive drawing of the 
 * empty album art image and rounded rectangles on the album art.
 */

#if GTK_CHECK_VERSION(3, 0, 0)  

// Draw the radio dot and/or green check mark
// TODO handle drawing of green check mark
static gboolean
user_widget_primitive_draw_cb_gtk_3 (GtkWidget *widget,
                                     cairo_t* cr,
                                     gpointer user_data)
{
	
  g_return_val_if_fail(IS_USER_WIDGET(user_data), FALSE);
  UserWidget* meta = USER_WIDGET(user_data);
  UserWidgetPrivate * priv = USER_WIDGET_GET_PRIVATE(meta);  

  // Draw dot only when user is the current user.
  if (!dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                            USER_ITEM_PROP_IS_CURRENT_USER)){
    return FALSE;                                           
  }
  
  
  GtkStyle *style;
  gdouble x, y;
  gdouble offset = 15.0;
  
  style = gtk_widget_get_style (widget);
  
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);
  x = allocation.x + 13;        
  y = offset;
  
  cairo_arc (cr, x, y, 3.0, 0.0, 2 * G_PI);;
  
  cairo_set_source_rgb (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
                            style->fg[gtk_widget_get_state(widget)].green/65535.0,
                            style->fg[gtk_widget_get_state(widget)].blue/65535.0);
  cairo_fill (cr);                                             

  return FALSE;  
}

// GTK 2 Expose handler
#else

// Draw the triangle if the player is running ...
static gboolean
user_widget_primitive_draw_cb (GtkWidget *widget,
                               GdkEventExpose *event,
                               gpointer user_data)
{
  /*
  g_return_val_if_fail(IS_USER_WIDGET(user_data), FALSE);
  UserWidget* meta = USER_WIDGET(user_data);
  UserWidgetPrivate * priv = USER_WIDGET_GET_PRIVATE(meta);  

  GtkStyle *style;
  cairo_t *cr;
  int x, y, arrow_width, arrow_height;               

  gint offset = 3;
  arrow_width = 5; 
  arrow_height = 9;
  
  style = gtk_widget_get_style (widget);

  cr = (cairo_t*) gdk_cairo_create (gtk_widget_get_window (widget));  
  
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);
  x = allocation.x;
  y = allocation.y;
    
  // Draw player icon  
  if (priv->icon_buf != NULL){  
    gdk_cairo_set_source_pixbuf (cr,
                                 priv->icon_buf,
                                 x + arrow_width + 1,
                                 y + offset);
    cairo_paint (cr);
  }
    
  // Draw triangle but only if the player is running.
  if (dbusmenu_menuitem_property_get_bool (priv->twin_item,
                                             DBUSMENU_METADATA_MENUITEM_PLAYER_RUNNING)){
    y += (double)arrow_height/2.0 + offset;
    cairo_set_line_width (cr, 1.0);

    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x, y + arrow_height);
    cairo_line_to (cr, x + arrow_width, y + (double)arrow_height/2.0);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, style->fg[gtk_widget_get_state(widget)].red/65535.0,
                              style->fg[gtk_widget_get_state(widget)].green/65535.0,
                              style->fg[gtk_widget_get_state(widget)].blue/65535.0);
    cairo_fill (cr);                                             
  }
  
  cairo_destroy (cr);*/
  return FALSE;  
}
#endif


/* Suppress/consume keyevents */
static gboolean
user_widget_button_release_event (GtkWidget *menuitem, 
                                      GdkEventButton *event)
{
  return FALSE;
}

static void 
user_widget_property_update (DbusmenuMenuitem* item, gchar* property, 
                             GVariant* value, gpointer userdata)
{
  g_return_if_fail (IS_USER_WIDGET (userdata)); 
  //gtk_widget_queue_redraw (GTK_WIDGET(userdata));
}


static void
user_widget_set_twin_item (UserWidget* self,
                           DbusmenuMenuitem* twin_item)
{
  UserWidgetPrivate* priv = USER_WIDGET_GET_PRIVATE(self);
  priv->twin_item = twin_item;
  g_signal_connect( G_OBJECT(priv->twin_item), "property-changed", 
                    G_CALLBACK(user_widget_property_update), self);
 
  const gchar * icon_name = dbusmenu_menuitem_property_get (twin_item,
                                                            USER_ITEM_PROP_ICON);
  gtk_label_set_label (GTK_LABEL (priv->user_name),
                       dbusmenu_menuitem_property_get (twin_item, USER_ITEM_PROP_NAME));

	//if (dbusmenu_menuitem_property_get_bool (twin_item, USER_ITEM_PROP_LOGGED_IN)) {
	//	gtk_widget_show(priv->tick_icon);
	//} else {
  gtk_widget_show(priv->tick_icon);
	//}

	g_debug("Using user icon for '%s' from file: %s",
          dbusmenu_menuitem_property_get(twin_item, USER_ITEM_PROP_NAME), icon_name);

}

 /**
 * transport_new:
 * @returns: a new #UserWidget.
 **/
GtkWidget* 
user_widget_new(DbusmenuMenuitem *item)
{
  GtkWidget* widget =  g_object_new(USER_WIDGET_TYPE, NULL);
  user_widget_set_twin_item ( USER_WIDGET(widget), item );
  return widget;                  
}
