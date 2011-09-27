/*
The Dbus object on the bus for the indicator.

Copyright 2010 Canonical Ltd.

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

#include <gio/gio.h>

#include "session-dbus.h"
#include "dbus-shared-names.h"

static GVariant * get_users_real_name (SessionDbus * service);
static void bus_get_cb (GObject * object, GAsyncResult * res, gpointer user_data);
static void bus_method_call (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * method, GVariant * params, GDBusMethodInvocation * invocation, gpointer user_data);

#include "gen-session-dbus.xml.h"

typedef struct _SessionDbusPrivate SessionDbusPrivate;
struct _SessionDbusPrivate {
	gchar * name;
  gboolean user_menu_is_visible;
	GDBusConnection * bus;
	GCancellable * bus_cancel;
	guint dbus_registration;
};

/* GDBus Stuff */
static GDBusNodeInfo *      node_info = NULL;
static GDBusInterfaceInfo * interface_info = NULL;
static GDBusInterfaceVTable interface_table = {
       method_call:    bus_method_call,
       get_property:   NULL, /* No properties */
       set_property:   NULL  /* No properties */
};

#define SESSION_DBUS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), SESSION_DBUS_TYPE, SessionDbusPrivate))

static void session_dbus_class_init (SessionDbusClass *klass);
static void session_dbus_init       (SessionDbus *self);
static void session_dbus_dispose    (GObject *object);
static void session_dbus_finalize   (GObject *object);

G_DEFINE_TYPE (SessionDbus, session_dbus, G_TYPE_OBJECT);

static void
session_dbus_class_init (SessionDbusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (SessionDbusPrivate));

	object_class->dispose = session_dbus_dispose;
	object_class->finalize = session_dbus_finalize;

	/* Setting up the DBus interfaces */
	if (node_info == NULL) {
		GError * error = NULL;

		node_info = g_dbus_node_info_new_for_xml(_session_dbus, &error);
		if (error != NULL) {
			g_error("Unable to parse Session Service Interface description: %s", error->message);
			g_error_free(error);
		}
	}

	if (interface_info == NULL) {
		interface_info = g_dbus_node_info_lookup_interface(node_info, INDICATOR_SESSION_SERVICE_DBUS_IFACE);

		if (interface_info == NULL) {
			g_error("Unable to find interface '" INDICATOR_SESSION_SERVICE_DBUS_IFACE "'");
		}
	}

	return;
}

static void
session_dbus_init (SessionDbus *self)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(self);

	priv->name = NULL;
	priv->bus = NULL;
	priv->bus_cancel = NULL;
	priv->dbus_registration = 0;
  priv->user_menu_is_visible = FALSE;

	priv->bus_cancel = g_cancellable_new();
	g_bus_get(G_BUS_TYPE_SESSION,
	          priv->bus_cancel,
	          bus_get_cb,
	          self);

	return;
}

static void
bus_get_cb (GObject * object, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;
	GDBusConnection * connection = g_bus_get_finish(res, &error);

	if (error != NULL) {
		g_error("OMG! Unable to get a connection to DBus: %s", error->message);
		g_error_free(error);
		return;
	}

	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(user_data);

	g_warn_if_fail(priv->bus == NULL);
	priv->bus = connection;

	if (priv->bus_cancel != NULL) {
		g_object_unref(priv->bus_cancel);
		priv->bus_cancel = NULL;
	}

	/* Now register our object on our new connection */
	priv->dbus_registration = g_dbus_connection_register_object(priv->bus,
	                                                            INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
	                                                            interface_info,
	                                                            &interface_table,
	                                                            user_data,
	                                                            NULL,
	                                                            &error);

	if (error != NULL) {
		g_error("Unable to register the object to DBus: %s", error->message);
		g_error_free(error);
		return;
	}

	return;	
}

/* A method has been called from our dbus inteface.  Figure out what it
   is and dispatch it. */
static void
bus_method_call (GDBusConnection * connection, const gchar * sender,
                 const gchar * path, const gchar * interface,
                 const gchar * method, GVariant * params,
                 GDBusMethodInvocation * invocation, gpointer user_data)
{
	SessionDbus * service = SESSION_DBUS (user_data);
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE (service);

	GVariant * retval = NULL;

	if (g_strcmp0(method, "GetUserRealName") == 0) {
		retval = get_users_real_name (service);
	}
  else if (g_strcmp0 (method, "GetUserMenuVisibility") == 0){
    retval =  g_variant_new ("(b)", priv->user_menu_is_visible);
  }
  else {
    g_warning("Calling method '%s' on the indicator service and it's unknown", method);
	}
	g_dbus_method_invocation_return_value(invocation, retval);
	return;
}

static void
session_dbus_dispose (GObject *object)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(object);

	if (priv->dbus_registration != 0) {
		g_dbus_connection_unregister_object(priv->bus, priv->dbus_registration);
		/* Don't care if it fails, there's nothing we can do */
		priv->dbus_registration = 0;
	}

	if (priv->bus != NULL) {
		g_object_unref(priv->bus);
		priv->bus = NULL;
	}

	if (priv->bus_cancel != NULL) {
		g_cancellable_cancel(priv->bus_cancel);
		g_object_unref(priv->bus_cancel);
		priv->bus_cancel = NULL;
	}

	G_OBJECT_CLASS (session_dbus_parent_class)->dispose (object);
	return;
}

static void
session_dbus_finalize (GObject *object)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(object);

	if (priv->name != NULL) {
		g_free(priv->name);
		priv->name = NULL;
	}

	G_OBJECT_CLASS (session_dbus_parent_class)->finalize (object);
	return;
}

static GVariant *
get_users_real_name (SessionDbus * service)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(service);
	return g_variant_new ("(s)", priv->name);
}

SessionDbus *
session_dbus_new (void)
{
	return SESSION_DBUS(g_object_new(SESSION_DBUS_TYPE, NULL));
}

void
session_dbus_set_name (SessionDbus * session, const gchar * name)
{
}

void
session_dbus_set_users_real_name (SessionDbus * session, const gchar * name)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(session);
	GError * error = NULL;
	if (priv->name != NULL) {
		g_free(priv->name);
		priv->name = NULL;
	}
    
	priv->name = g_strdup(name);

	if (priv->bus != NULL) {
		g_dbus_connection_emit_signal (priv->bus,
                                   NULL,
                                   INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
                                   INDICATOR_SESSION_SERVICE_DBUS_IFACE,
                                   "UserRealNameUpdated",
                                   g_variant_new ("(s)", priv->name, NULL),
                                   &error);

		if (error != NULL) {
			g_warning("Unable to send UserRealNameUpdated signal: %s", error->message);
			g_error_free(error);
			return;
		}
	}
	return;
}

void 
session_dbus_set_user_menu_visibility (SessionDbus* session,
                                       gboolean visible)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(session);
	GError * error = NULL;
    
	priv->user_menu_is_visible = visible;

	if (priv->bus != NULL) {
		g_dbus_connection_emit_signal (priv->bus,
                                   NULL,
                                   INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
                                   INDICATOR_SESSION_SERVICE_DBUS_IFACE,
                                   "UserMenuIsVisible",
                                   g_variant_new ("(b)", priv->user_menu_is_visible),
                                   &error);

		if (error != NULL) {
			g_warning("Unable to send UserMenuIsVisible signal: %s", error->message);
			g_error_free(error);
		}
	}  
}

void session_dbus_restart_required (SessionDbus* session)
{
	SessionDbusPrivate * priv = SESSION_DBUS_GET_PRIVATE(session);
	GError * error = NULL;
    
	if (priv->bus != NULL) {
    g_debug("About to send RebootRequired signal");

		g_dbus_connection_emit_signal (priv->bus,
                                   NULL,
                                   INDICATOR_SESSION_SERVICE_DBUS_OBJECT,
                                   INDICATOR_SESSION_SERVICE_DBUS_IFACE,
                                   "RestartRequired",
                                   NULL,
                                   &error);

		if (error != NULL) {
			g_warning("Unable to send reboot-required signal: %s", error->message);
			g_error_free(error);
		}
	}  
  
}
