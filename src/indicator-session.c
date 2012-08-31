/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using its applet interface.

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

#include "shared-names.h"
#include "user-widget.h"

#define INDICATOR_SESSION_TYPE            (indicator_session_get_type ())
#define INDICATOR_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INDICATOR_SESSION_TYPE, IndicatorSession))
#define INDICATOR_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INDICATOR_SESSION_TYPE, IndicatorSessionClass))
#define IS_INDICATOR_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INDICATOR_SESSION_TYPE))
#define IS_INDICATOR_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INDICATOR_SESSION_TYPE))
#define INDICATOR_SESSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INDICATOR_SESSION_TYPE, IndicatorSessionClass))

typedef struct _IndicatorSession      IndicatorSession;
typedef struct _IndicatorSessionClass IndicatorSessionClass;

struct _IndicatorSessionClass
{
  IndicatorObjectClass parent_class;
};

struct _IndicatorSession
{
  IndicatorObject parent;
  IndicatorServiceManager * service;
  IndicatorObjectEntry entry;
  GCancellable * service_proxy_cancel;
  GDBusProxy * service_proxy;
  GSettings * settings;
  DbusmenuClient * menu_client;
};

static gboolean greeter_mode;

GType indicator_session_get_type (void);

/* Indicator stuff */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_SESSION_TYPE)

/* Prototypes */
static gboolean new_user_item (DbusmenuMenuitem * newitem,
                               DbusmenuMenuitem * parent,
                               DbusmenuClient * client,
                               gpointer user_data);
static gboolean build_restart_item (DbusmenuMenuitem * newitem,
                                    DbusmenuMenuitem * parent,
                                    DbusmenuClient * client,
                                    gpointer user_data);
static void on_menu_layout_updated (DbusmenuClient * client, IndicatorSession * session);
static void indicator_session_update_icon_and_a11y (IndicatorSession * self);
static void indicator_session_update_users_label (IndicatorSession* self,
                                                  const gchar* name);
static void service_connection_cb (IndicatorServiceManager * sm, gboolean connected, gpointer user_data);
static void receive_signal (GDBusProxy * proxy, gchar * sender_name, gchar * signal_name, GVariant * parameters, gpointer user_data);
static void service_proxy_cb (GObject * object, GAsyncResult * res, gpointer user_data);
static void user_real_name_get_cb (GObject * obj, GAsyncResult * res, gpointer user_data);

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
  self->settings = g_settings_new ("com.canonical.indicator.session");

  /* Now let's fire these guys up. */
  self->service = indicator_service_manager_new_version(INDICATOR_SESSION_DBUS_NAME,
                                                        INDICATOR_SESSION_DBUS_VERSION);
  g_signal_connect (G_OBJECT(self->service),
                    INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE,
                    G_CALLBACK(service_connection_cb), self);

  greeter_mode = !g_strcmp0(g_getenv("INDICATOR_GREETER_MODE"), "1");

  self->entry.name_hint = PACKAGE;
  self->entry.label = GTK_LABEL (gtk_label_new ("User Name"));
  self->entry.image = GTK_IMAGE (gtk_image_new());
  self->entry.menu = GTK_MENU (dbusmenu_gtkmenu_new(INDICATOR_SESSION_DBUS_NAME,
                                                    INDICATOR_SESSION_DBUS_OBJECT));
  indicator_session_update_icon_and_a11y (self);
  g_settings_bind (self->settings, "show-real-name-on-panel",
                   self->entry.label, "visible",
                   G_SETTINGS_BIND_GET);

  /* show-real-name-on-panel affects the a11y string */
  g_signal_connect_swapped (self->settings,
                            "notify::show-real-name-on-panel",
                            G_CALLBACK(indicator_session_update_icon_and_a11y),
                            self);

  gtk_widget_show (GTK_WIDGET(self->entry.menu));
  gtk_widget_show (GTK_WIDGET(self->entry.image));
  g_object_ref_sink (self->entry.menu);
  g_object_ref_sink (self->entry.image);

  // set up the handlers
  self->menu_client = DBUSMENU_CLIENT(dbusmenu_gtkmenu_get_client(DBUSMENU_GTKMENU(self->entry.menu)));
  g_signal_connect (self->menu_client, "layout-updated",
                    G_CALLBACK(on_menu_layout_updated), self);

  dbusmenu_client_add_type_handler (self->menu_client,
                                    USER_ITEM_TYPE,
                                    new_user_item);
  dbusmenu_client_add_type_handler (self->menu_client,
                                    RESTART_ITEM_TYPE,
                                    build_restart_item);
  dbusmenu_gtkclient_set_accel_group (DBUSMENU_GTKCLIENT(self->menu_client),
                                      gtk_accel_group_new());
}

static void
indicator_session_dispose (GObject *object)
{
  IndicatorSession * self = INDICATOR_SESSION(object);

  g_clear_object (&self->settings);
  g_clear_object (&self->service);
  g_clear_object (&self->service_proxy);

  if (self->service_proxy_cancel != NULL)
    {
      g_cancellable_cancel(self->service_proxy_cancel);
      g_clear_object (&self->service_proxy_cancel);
    }

  g_clear_object (&self->entry.menu);

  G_OBJECT_CLASS (indicator_session_parent_class)->dispose (object);
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
  return g_list_append (NULL, &self->entry);
}

static guint
indicator_session_get_location (IndicatorObject * io,
                                IndicatorObjectEntry * entry)
{
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

	g_clear_object (&self->service_proxy_cancel);

	if (error != NULL) {
		g_warning("Could not grab DBus proxy for %s: %s", INDICATOR_SESSION_DBUS_NAME, error->message);
		g_error_free(error);
		return;
	}

	/* Okay, we're good to grab the proxy at this point, we're
	sure that it's ours. */
	self->service_proxy = proxy;

	g_signal_connect(proxy, "g-signal", G_CALLBACK(receive_signal), self);

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
               DbusmenuClient   * client,
               gpointer           user_data)
{
  g_return_val_if_fail (DBUSMENU_IS_MENUITEM(newitem), FALSE);
  g_return_val_if_fail (DBUSMENU_IS_GTKCLIENT(client), FALSE);

  GtkWidget * user_item = user_widget_new (newitem);

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
  GVariant * result = g_dbus_proxy_call_finish(self->service_proxy, res, &error);

  if (error != NULL)
    {
      g_warning ("Unable to complete real name dbus query: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      const gchar * username = NULL;
      g_variant_get (result, "(&s)", &username);
      indicator_session_update_users_label (self, username);
      g_variant_unref (result);
    }
}

/* Receives all signals from the service, routed to the appropriate functions */
static void
receive_signal (GDBusProxy * proxy,
                gchar      * sender_name,
                gchar      * signal_name,
                GVariant   * parameters,
                gpointer     user_data)
{
  IndicatorSession * self = INDICATOR_SESSION(user_data);

  if (!g_strcmp0(signal_name, "UserRealNameUpdated"))
    {
      const gchar * username = NULL;
      g_variant_get (parameters, "(&s)", &username);
      indicator_session_update_users_label (self, username);	
    }
}




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
indicator_session_update_users_label (IndicatorSession * self,
                                      const gchar      * name)
{
  gtk_label_set_text (self->entry.label, name ? name : "");
}

/***
**** Disposition
***/

enum
{
  DISPOSITION_NORMAL,
  DISPOSITION_INFO,
  DISPOSITION_WARNING,
  DISPOSITION_ALERT
};
  
static void
indicator_session_update_a11y_from_disposition (IndicatorSession * indicator,
                                                int                disposition)
{
  gchar * a11y;
  const gchar * username = gtk_label_get_text (indicator->entry.label);
  const gboolean need_attn = disposition != DISPOSITION_NORMAL;
  const gboolean show_name = g_settings_get_boolean (indicator->settings,
                                                     "show-real-name-on-panel");

  if (show_name && need_attn)
    a11y = g_strdup_printf (_("System %s (Attention Required)"), username);
  else if (show_name)
    a11y = g_strdup_printf (_("System %s"), username);
  else if (need_attn)
    a11y = g_strdup  (_("System (Attention Required)"));
  else
    a11y = g_strdup (_("System"));

  g_debug (G_STRLOC" setting a11y to \"%s\"", a11y);
  g_clear_pointer (&indicator->entry.accessible_desc, g_free);
  indicator->entry.accessible_desc = a11y;
  g_signal_emit (indicator,
                 INDICATOR_OBJECT_SIGNAL_ACCESSIBLE_DESC_UPDATE_ID,
                 0,
                 &indicator->entry);
}

static void
indicator_session_update_icon_from_disposition (IndicatorSession * indicator,
                                                int                disposition)
{
  const gchar * icon;

  if (greeter_mode)
    {
      if (disposition == DISPOSITION_NORMAL)
        icon = GREETER_ICON_DEFAULT;
      else
        icon = GREETER_ICON_RESTART;
    }
  else
    {
      if (disposition == DISPOSITION_NORMAL)
        icon = ICON_DEFAULT;
      else if (disposition == DISPOSITION_INFO)
        icon = ICON_INFO;
      else
        icon = ICON_ALERT;
    }

  g_debug (G_STRLOC" setting icon to \"%s\"", icon);
  gtk_image_set_from_icon_name (GTK_IMAGE(indicator->entry.image),
                                icon,
                                GTK_ICON_SIZE_BUTTON);
}
  
static int
calculate_disposition (IndicatorSession * indicator)
{
  GList * l;
  DbusmenuMenuitem * root = dbusmenu_client_get_root (indicator->menu_client);
  GList * children = dbusmenu_menuitem_get_children (root);
  int ret = DISPOSITION_NORMAL;

  for (l=children; l!=NULL; l=l->next)
    {
      int val;
      const gchar * key = DBUSMENU_MENUITEM_PROP_DISPOSITION;
      const gchar * val_str = dbusmenu_menuitem_property_get (l->data, key);

      if (!g_strcmp0 (val_str, DBUSMENU_MENUITEM_DISPOSITION_ALERT))
        val = DISPOSITION_ALERT;
      else if (!g_strcmp0 (val_str, DBUSMENU_MENUITEM_DISPOSITION_WARNING))
        val = DISPOSITION_WARNING;
      else if (!g_strcmp0 (val_str, DBUSMENU_MENUITEM_DISPOSITION_INFORMATIVE))
        val = DISPOSITION_INFO;
      else
        val = DISPOSITION_NORMAL;

      if (ret < val)
        ret = val;
    }

  return ret;
}

static void
indicator_session_update_icon_and_a11y (IndicatorSession * indicator)
{
  const int disposition = calculate_disposition (indicator);
  indicator_session_update_a11y_from_disposition (indicator, disposition);
  indicator_session_update_icon_from_disposition (indicator, disposition);
}

static void
on_menuitem_property_changed (DbusmenuMenuitem * mi,
                              gchar            * property,
                              GValue           * value,
                              gpointer           indicator)
{
  if (!g_strcmp0 (property, DBUSMENU_MENUITEM_PROP_DISPOSITION))
    indicator_session_update_icon_and_a11y (indicator);
}

static void
on_menu_layout_updated (DbusmenuClient * client, IndicatorSession * session)
{
  GList * l;
  DbusmenuMenuitem * root = dbusmenu_client_get_root (client);
  GList * children = dbusmenu_menuitem_get_children (root);
  static GQuark tag = 0;

  if (G_UNLIKELY (tag == 0))
    {
      tag = g_quark_from_static_string ("x-tagged-by-indicator-session");
    }

  for (l=children; l!=NULL; l=l->next)
    {
      if (g_object_get_qdata (l->data, tag) == NULL)
        {
          g_object_set_qdata (l->data, tag, GINT_TO_POINTER(1));
          g_signal_connect (l->data, "property-changed", G_CALLBACK(on_menuitem_property_changed), session);
        }
    }
}
