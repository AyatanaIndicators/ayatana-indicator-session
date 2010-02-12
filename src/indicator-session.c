/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menu.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

#include "dbus-shared-names.h"
#include "dbusmenu-shared.h"

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
	GtkImage * status_image;
	DbusmenuGtkMenu * menu;
};

GType indicator_session_get_type (void);

/* Indicator stuff */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_SESSION_TYPE)

/* Prototypes */
static GtkLabel * get_label (IndicatorObject * io);
static GtkImage * get_icon (IndicatorObject * io);
static GtkMenu * get_menu (IndicatorObject * io);
static gboolean build_menu_switch (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);

static void indicator_session_class_init (IndicatorSessionClass *klass);
static void indicator_session_init       (IndicatorSession *self);
static void indicator_session_dispose    (GObject *object);
static void indicator_session_finalize   (GObject *object);

G_DEFINE_TYPE (IndicatorSession, indicator_session, INDICATOR_OBJECT_TYPE);

static void
indicator_session_class_init (IndicatorSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = indicator_session_dispose;
	object_class->finalize = indicator_session_finalize;

	IndicatorObjectClass * io_class = INDICATOR_OBJECT_CLASS(klass);
	io_class->get_label = get_label;
	io_class->get_image = get_icon;
	io_class->get_menu = get_menu;

	return;
}

static void
indicator_session_init (IndicatorSession *self)
{
	/* Set good defaults */
	self->service = NULL;

	/* Now let's fire these guys up. */
	self->service = indicator_service_manager_new_version(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_VERSION);

	self->status_image = GTK_IMAGE(gtk_image_new_from_icon_name("system-shutdown-panel", GTK_ICON_SIZE_MENU));
	self->menu = dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT);

	DbusmenuClient * client = DBUSMENU_CLIENT(dbusmenu_gtkmenu_get_client(self->menu));
	dbusmenu_client_add_type_handler(client, MENU_SWITCH_TYPE, build_menu_switch);

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

	G_OBJECT_CLASS (indicator_session_parent_class)->dispose (object);
	return;
}

static void
indicator_session_finalize (GObject *object)
{

	G_OBJECT_CLASS (indicator_session_parent_class)->finalize (object);
	return;
}

static GtkLabel *
get_label (IndicatorObject * io)
{
	return NULL;
}

static GtkImage *
get_icon (IndicatorObject * io)
{
	gtk_widget_show(GTK_WIDGET(INDICATOR_SESSION(io)->status_image));
	return INDICATOR_SESSION(io)->status_image;
}

/* Indicator based function to get the menu for the whole
   applet.  This starts up asking for the parts of the menu
   from the various services. */
static GtkMenu *
get_menu (IndicatorObject * io)
{
	return GTK_MENU(INDICATOR_SESSION(io)->menu);
}

static void
switch_property_change (DbusmenuMenuitem * item, const gchar * property, const GValue * value, gpointer user_data)
{
	if (g_strcmp0(property, MENU_SWITCH_USER) != 0) {
		return;
	}
	
	GtkMenuItem * gmi = dbusmenu_gtkclient_menuitem_get(DBUSMENU_GTKCLIENT(user_data), item);
	gchar * finalstring = NULL;
	gboolean set_ellipsize = FALSE;

	/* If there's a NULL string of some type, then we want to
	   go back to our old 'Switch User' which isn't great but
	   eh, this error condition should never happen. */
	if (value == NULL || g_value_get_string(value) == NULL || g_value_get_string(value)[0] == '\0') {
		finalstring = _("Switch User...");
		set_ellipsize = FALSE;
	}

	if (finalstring == NULL) {
		const gchar * username = g_value_get_string(value);
		GtkStyle * style = gtk_widget_get_style(GTK_WIDGET(gmi)); /* TODO: Switch to menuitem label */

		PangoLayout * layout = pango_layout_new(gtk_widget_get_pango_context(GTK_WIDGET(gmi)));
		pango_layout_set_text(layout, username, -1);
		pango_layout_set_font_description(layout, style->font_desc);

		gint width;
		pango_layout_get_pixel_size(layout, &width, NULL);
		g_debug("Username width %dpx", width);

		gint point = pango_font_description_get_size(style->font_desc);
		g_debug("Font size %f pt", (gfloat)point / PANGO_SCALE);

		gdouble dpi = gdk_screen_get_resolution(gdk_screen_get_default());
		g_debug("Screen DPI %f", dpi);

		gdouble pixels_per_em = ((point * dpi) / 72.0f) / PANGO_SCALE;
		gdouble ems = width / pixels_per_em;
		g_debug("Username width %fem", ems);

		/* TODO: We need some way to remove the elipsis from appearing
		         twice in the label.  Not sure how to do that yet. */
		finalstring = g_strdup_printf(_("Switch from %s..."), username);
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

/* Callback for when the style changes so we can reevaluate the
   size of the user name with the potentially new font. */
static void
switch_style_set (GtkWidget * widget, GtkStyle * prev_style, gpointer user_data)
{
	DbusmenuGtkClient * client = DBUSMENU_GTKCLIENT(user_data);
	DbusmenuMenuitem * mi = DBUSMENU_MENUITEM(g_object_get_data(G_OBJECT(widget), dbusmenu_item_data));

	switch_property_change(mi, MENU_SWITCH_USER, dbusmenu_menuitem_property_get_value(mi, MENU_SWITCH_USER), client);
	return;
}


/* This function checks to see if the user name is short enough
   to not need ellipsing itself, or if, it will get ellipsed by
   the standard label processor. */
static gboolean
build_menu_switch (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());
	if (gmi == NULL) {
		return FALSE;
	}
	g_object_set_data(G_OBJECT(gmi), dbusmenu_item_data, newitem);

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(switch_property_change), client);
	g_signal_connect(G_OBJECT(gmi), "style-set", G_CALLBACK(switch_style_set), client);
	switch_property_change(newitem, MENU_SWITCH_USER, dbusmenu_menuitem_property_get_value(newitem, MENU_SWITCH_USER), client);

	return TRUE;
}
