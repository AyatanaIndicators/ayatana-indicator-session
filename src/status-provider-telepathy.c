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

#include "status-provider.h"
#include "status-provider-telepathy.h"
#include "status-provider-telepathy-marshal.h"

#include <dbus/dbus-glib.h>


typedef struct _StatusProviderTelepathyPrivate StatusProviderTelepathyPrivate;
struct _StatusProviderTelepathyPrivate {
	DBusGProxy * proxy;
};

#define STATUS_PROVIDER_TELEPATHY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_PROVIDER_TELEPATHY_TYPE, StatusProviderTelepathyPrivate))

/* Prototypes */
/* GObject stuff */
static void status_provider_telepathy_class_init (StatusProviderTelepathyClass *klass);
static void status_provider_telepathy_init       (StatusProviderTelepathy *self);
static void status_provider_telepathy_dispose    (GObject *object);
static void status_provider_telepathy_finalize   (GObject *object);

G_DEFINE_TYPE (StatusProviderTelepathy, status_provider_telepathy, STATUS_PROVIDER_TYPE);

static void
status_provider_telepathy_class_init (StatusProviderTelepathyClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (StatusProviderTelepathyPrivate));

	object_class->dispose = status_provider_telepathy_dispose;
	object_class->finalize = status_provider_telepathy_finalize;

	//StatusProviderClass * spclass = STATUS_PROVIDER_CLASS(klass);

	//spclass->set_status = set_status;
	//spclass->get_status = get_status;

	return;
}


static void
status_provider_telepathy_init (StatusProviderTelepathy *self)
{
	//StatusProviderTelepathyPrivate * priv = STATUS_PROVIDER_TELEPATHY_GET_PRIVATE(self);

	return;
}

static void
status_provider_telepathy_dispose (GObject *object)
{
	StatusProviderTelepathyPrivate * priv = STATUS_PROVIDER_TELEPATHY_GET_PRIVATE(object);

	if (priv->proxy != NULL) {
		g_object_unref(priv->proxy);
		priv->proxy = NULL;
	}

	G_OBJECT_CLASS (status_provider_telepathy_parent_class)->dispose (object);
	return;
}

static void
status_provider_telepathy_finalize (GObject *object)
{

	G_OBJECT_CLASS (status_provider_telepathy_parent_class)->finalize (object);
	return;
}

/**
	status_provider_telepathy_new:

	Creates a new #StatusProviderTelepathy object.  No parameters or anything
	like that.  Just a convience function.

	Return value: A new instance of #StatusProviderTelepathy
*/
StatusProvider *
status_provider_telepathy_new (void)
{
	return STATUS_PROVIDER(g_object_new(STATUS_PROVIDER_TELEPATHY_TYPE, NULL));
}

