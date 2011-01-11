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
	GCancellable * service_proxy_cancel;
	GDBusProxy * service_proxy;
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
static gboolean new_user_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);
static void icon_changed (IndicatorSession * session, const gchar * icon_name);
static void service_connection_cb (IndicatorServiceManager * sm, gboolean connected, gpointer user_data);
static gboolean build_restart_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client);
static void receive_signal (GDBusProxy * proxy, gchar * sender_name, gchar * signal_name, GVariant * parameters, gpointer user_data);
static void service_proxy_cb (GObject * object, GAsyncResult * res, gpointer user_data);

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
	self->service_proxy_cancel = NULL;
	self->service_proxy = NULL;

	/* Now let's fire these guys up. */
	self->service = indicator_service_manager_new_version(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_VERSION);
	g_signal_connect(G_OBJECT(self->service), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(service_connection_cb), self);

	self->status_image = indicator_image_helper(ICON_DEFAULT);
	self->menu = dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME, INDICATOR_SESSION_DBUS_OBJECT);

	DbusmenuClient * client = DBUSMENU_CLIENT(dbusmenu_gtkmenu_get_client(self->menu));
	dbusmenu_client_add_type_handler(client, MENU_SWITCH_TYPE, build_menu_switch);
	dbusmenu_client_add_type_handler(client, USER_ITEM_TYPE, new_user_item);
	dbusmenu_client_add_type_handler(client, RESTART_ITEM_TYPE, build_restart_item);

	GtkAccelGroup * agroup = gtk_accel_group_new();
	dbusmenu_gtkclient_set_accel_group(DBUSMENU_GTKCLIENT(client), agroup);

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

	return;
}

/* Callback from trying to create the proxy for the serivce, this
   could include starting the service.  Sometime it'll fail and
   we'll try to start that dang service again! */
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
		g_error("Could not grab DBus proxy for %s: %s", INDICATOR_SESSION_DBUS_NAME, error->message);
		g_error_free(error);
		return;
	}

	/* Okay, we're good to grab the proxy at this point, we're
	sure that it's ours. */
	self->service_proxy = proxy;

	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);

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

	if (self->service_proxy_cancel != NULL) {
		g_object_unref(self->service_proxy_cancel);
		self->service_proxy_cancel = NULL;
	}

	if (self->service_proxy != NULL) {
		g_object_unref(self->service_proxy);
		self->service_proxy = NULL;
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

static void
icon_name_get_cb (GObject * obj, GAsyncResult * res, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);
	GError * error = NULL;
	gchar * name;
	GVariant * result;

	result = g_dbus_proxy_call_finish(self->service_proxy, res, &error);

	if (error != NULL) {
		return;
	}

	g_variant_get(result, "(&s)", &name);

	if (name == NULL || name[0] == '\0') {
		return;
	}

	indicator_image_helper_update(self->status_image, name);
	return;
}

static void
service_connection_cb (IndicatorServiceManager * sm, gboolean connected, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);

	if (connected) {
		g_dbus_proxy_call(self->service_proxy, "GetIcon", NULL,
		                  G_DBUS_CALL_FLAGS_NONE, -1, NULL,
		                  icon_name_get_cb, user_data);
	} else {
		indicator_image_helper_update(self->status_image, ICON_DEFAULT);
	}

	return;
}

static GtkLabel *
get_label (IndicatorObject * io)
{
	return NULL;
}

static void
icon_changed (IndicatorSession * session, const gchar * icon_name)
{
	indicator_image_helper_update(session->status_image, icon_name);
	return;
}

/* Receives all signals from the service, routed to the appropriate functions */
static void
receive_signal (GDBusProxy * proxy, gchar * sender_name, gchar * signal_name,
                GVariant * parameters, gpointer user_data)
{
	IndicatorSession * self = INDICATOR_SESSION(user_data);

	if (g_strcmp0(signal_name, "IconUpdated") == 0) {
		const gchar *name;
		g_variant_get (parameters, "(&s)", &name);
		icon_changed(self, name);
	}

	return;
}

static GtkImage *
get_icon (IndicatorObject * io)
{
	gtk_widget_show(GTK_WIDGET(INDICATOR_SESSION(io)->status_image));
	return INDICATOR_SESSION(io)->status_image;
}

static void
user_property_change (DbusmenuMenuitem * item, const gchar * property, GVariant * variant, gpointer user_data)
{
	if (g_strcmp0(property, USER_ITEM_PROP_LOGGED_IN) == 0) {
		if (g_variant_get_boolean(variant)) {
			gtk_widget_show(GTK_WIDGET(user_data));
		} else {
			gtk_widget_hide(GTK_WIDGET(user_data));
		}
	}
	return;
}

/* Builds an item with a hip little logged in icon. */
static gboolean
new_user_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_menu_item_new());
	gint padding = 0;
	gtk_widget_style_get(GTK_WIDGET(gmi), "horizontal-padding", &padding, NULL);
	GtkWidget * hbox = gtk_hbox_new(FALSE, padding);

	GtkWidget * usericon = NULL;
	const gchar * icon_name = dbusmenu_menuitem_property_get(newitem, USER_ITEM_PROP_ICON);
	g_debug("Using user icon for '%s' from file: %s", dbusmenu_menuitem_property_get(newitem, USER_ITEM_PROP_NAME), icon_name);
	if (icon_name != NULL && icon_name[0] != '\0') {
		if (g_strcmp0(icon_name, USER_ITEM_ICON_DEFAULT) != 0 && g_file_test(icon_name, G_FILE_TEST_EXISTS)) {
			gint width, height;
			gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

			GError * error = NULL;
			GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file_at_size(icon_name, width, height, &error);

			if (error == NULL) {
				usericon = gtk_image_new_from_pixbuf(pixbuf);
				g_object_unref(pixbuf);
			} else {
				g_warning("Unable to load user icon '%s': %s", icon_name, error->message);
				g_error_free(error);
			}
		}

		if (usericon == NULL) {
			GIcon * gicon = g_themed_icon_new_with_default_fallbacks("stock_person-panel");
			usericon = gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_MENU);
			g_object_unref(gicon);
		}
	}
	if (usericon != NULL) {
		gtk_misc_set_alignment(GTK_MISC(usericon), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(hbox), usericon, FALSE, FALSE, 0);
		gtk_widget_show(usericon);
	}

	GtkWidget * label = gtk_label_new(dbusmenu_menuitem_property_get(newitem, USER_ITEM_PROP_NAME));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	GtkWidget * icon = gtk_image_new_from_icon_name("account-logged-in", GTK_ICON_SIZE_MENU);
	gtk_misc_set_alignment(GTK_MISC(icon), 1.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
	if (dbusmenu_menuitem_property_get_bool(newitem, USER_ITEM_PROP_LOGGED_IN)) {
		gtk_widget_show(icon);
	} else {
		gtk_widget_hide(icon);
	}

	gtk_container_add(GTK_CONTAINER(gmi), hbox);
	gtk_widget_show(hbox);

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(user_property_change), icon);

	return TRUE;
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
switch_property_change (DbusmenuMenuitem * item, const gchar * property, GVariant * variant, gpointer user_data)
{
	if (g_strcmp0(property, MENU_SWITCH_USER) != 0) {
		return;
	}
	
	GtkMenuItem * gmi = dbusmenu_gtkclient_menuitem_get(DBUSMENU_GTKCLIENT(user_data), item);
	gchar * finalstring = NULL;
	gboolean set_ellipsize = FALSE;
	gboolean no_name_in_lang = FALSE;

	/* TRANSLATORS: Translate the '1' below into anything other than
	   '1' if "Switch From %s..." doesn't make sense in your language.
	   Instead, the string "Switch User..." will be used. */
	const gchar * translate = C_("session_menu:switchfrom", "1");
	if (g_strcmp0(translate, "1") != 0) {
		no_name_in_lang = TRUE;
	}

	/* If there's a NULL string of some type, then we want to
	   go back to our old 'Switch User' which isn't great but
	   eh, this error condition should never happen. */
	if (variant == NULL || g_variant_get_string(variant, NULL) == NULL || g_variant_get_string(variant, NULL)[0] == '\0' || no_name_in_lang) {
		finalstring = _("Switch User...");
		set_ellipsize = FALSE;
	}

	if (finalstring == NULL) {
		const gchar * username = g_variant_get_string(variant, NULL);
		GtkStyle * style = gtk_widget_get_style(GTK_WIDGET(gmi));

		PangoLayout * layout = pango_layout_new(gtk_widget_get_pango_context(GTK_WIDGET(gmi)));
		pango_layout_set_text(layout, username, -1);
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

		/* TODO: We need some way to remove the elipsis from appearing
		         twice in the label.  Not sure how to do that yet. */
		finalstring = g_strdup_printf(_("Switch From %s..."), username);
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

/* IF the label or icon changes we need to grab that and update
   the menu item */
static void
restart_property_change (DbusmenuMenuitem * item, const gchar * property, GVariant * variant, gpointer user_data)
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

/* Builds the restart item which is a more traditional GTK image
   menu item that puts the graphic into the gutter. */
static gboolean
build_restart_item (DbusmenuMenuitem * newitem, DbusmenuMenuitem * parent, DbusmenuClient * client)
{
	GtkMenuItem * gmi = GTK_MENU_ITEM(gtk_image_menu_item_new());
	if (gmi == NULL) {
		return FALSE;
	}

	dbusmenu_gtkclient_newitem_base(DBUSMENU_GTKCLIENT(client), newitem, gmi, parent);

	g_signal_connect(G_OBJECT(newitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(restart_property_change), client);

	/* Grab the inital variants and put them into the item */
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


/* Callback for when the style changes so we can reevaluate the
   size of the user name with the potentially new font. */
static void
switch_style_set (GtkWidget * widget, GtkStyle * prev_style, gpointer user_data)
{
	DbusmenuGtkClient * client = DBUSMENU_GTKCLIENT(user_data);
	DbusmenuMenuitem * mi = DBUSMENU_MENUITEM(g_object_get_data(G_OBJECT(widget), dbusmenu_item_data));

	switch_property_change(mi, MENU_SWITCH_USER, dbusmenu_menuitem_property_get_variant(mi, MENU_SWITCH_USER), client);
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
	switch_property_change(newitem, MENU_SWITCH_USER, dbusmenu_menuitem_property_get_variant(newitem, MENU_SWITCH_USER), client);

	return TRUE;
}
