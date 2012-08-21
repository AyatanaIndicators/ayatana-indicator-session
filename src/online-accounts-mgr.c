/*
Copyright 2012 Canonical Ltd.

Authors:
    Alberto Mardegan <alberto.mardegan@canonical.com>

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
#include <glib/gi18n.h>

#include "online-accounts-mgr.h"

#include <libdbusmenu-glib/client.h>

struct _OnlineAccountsMgr
{
  GObject parent_instance;
  GDBusProxy *proxy;
  DbusmenuMenuitem *menu_item;
};

#define ONLINE_ACCOUNTS_OBJECT_PATH "/com/canonical/indicators/webcredentials"
#define ONLINE_ACCOUNTS_BUS_NAME "com.canonical.indicators.webcredentials"
#define ONLINE_ACCOUNTS_INTERFACE ONLINE_ACCOUNTS_BUS_NAME

G_DEFINE_TYPE (OnlineAccountsMgr, online_accounts_mgr, G_TYPE_OBJECT);

static void
update_disposition (OnlineAccountsMgr *self, GVariant *error_status_prop)
{
  gboolean error_status;

  error_status = g_variant_get_boolean (error_status_prop);
  dbusmenu_menuitem_property_set (self->menu_item,
                                  DBUSMENU_MENUITEM_PROP_DISPOSITION,
                                  error_status ?
                                  DBUSMENU_MENUITEM_DISPOSITION_ALERT :
                                  DBUSMENU_MENUITEM_DISPOSITION_NORMAL);
}

static void
on_properties_changed (GDBusProxy *proxy,
                       GVariant *changed_properties,
                       GStrv invalidated_properties,
                       OnlineAccountsMgr *self)
{
  if (g_variant_n_children (changed_properties) > 0) {
    GVariantIter *iter;
    const gchar *key;
    GVariant *value;

    g_variant_get (changed_properties, "a{sv}", &iter);
    while (g_variant_iter_loop (iter, "{&sv}", &key, &value)) {
      if (g_strcmp0 (key, "ErrorStatus") == 0) {
        update_disposition (self, value);
      }
    }
    g_variant_iter_free (iter);
  }
}

static void
on_menu_item_activated (DbusmenuMenuitem *menu_item,
                        guint timestamp,
                        OnlineAccountsMgr *self)
{
  GError *error = NULL;

  if (!g_spawn_command_line_async("gnome-control-center credentials", &error))
  {
    g_warning("Unable to show control center: %s", error->message);
    g_error_free(error);
  }
}

static void
online_accounts_mgr_init (OnlineAccountsMgr *self)
{
  GError *error = NULL;
  GVariant *error_status_prop;

  self->menu_item = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (self->menu_item,
                                  DBUSMENU_MENUITEM_PROP_TYPE,
                                  DBUSMENU_CLIENT_TYPES_DEFAULT);
  dbusmenu_menuitem_property_set (self->menu_item,
                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                  _("Online Accounts\342\200\246"));
  g_signal_connect (self->menu_item,
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (on_menu_item_activated),
                    self);

  self->proxy =
    g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                   G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                   NULL,
                                   ONLINE_ACCOUNTS_BUS_NAME,
                                   ONLINE_ACCOUNTS_OBJECT_PATH,
                                   ONLINE_ACCOUNTS_INTERFACE,
                                   NULL,
                                   &error);
  if (G_UNLIKELY (error != NULL)) {
      g_warning ("Couldn't create online_accounts proxy: %s", error->message);
      g_clear_error (&error);
      return;
  }

  g_signal_connect (self->proxy, "g-properties-changed",
                    G_CALLBACK (on_properties_changed), self);

  error_status_prop =
    g_dbus_proxy_get_cached_property (self->proxy, "ErrorStatus");
  if (error_status_prop != NULL) {
    update_disposition (self, error_status_prop);
    g_variant_unref (error_status_prop);
  }
}

static void
online_accounts_mgr_dispose (GObject *object)
{
  OnlineAccountsMgr *self = ONLINE_ACCOUNTS_MGR (object);

  if (self->proxy != NULL) {
    g_object_unref (self->proxy);
    self->proxy = NULL;
  }

  if (self->menu_item != NULL) {
    g_object_unref (self->menu_item);
    self->menu_item = NULL;
  }

  G_OBJECT_CLASS (online_accounts_mgr_parent_class)->dispose (object);
}

static void
online_accounts_mgr_class_init (OnlineAccountsMgrClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = online_accounts_mgr_dispose;
}

OnlineAccountsMgr *online_accounts_mgr_new ()
{
  return g_object_new (ONLINE_ACCOUNTS_TYPE_MGR, NULL);
}

DbusmenuMenuitem *online_accounts_mgr_get_menu_item (OnlineAccountsMgr *self)
{
  g_return_val_if_fail (ONLINE_ACCOUNTS_IS_MGR (self), NULL);
  return self->menu_item;
}
