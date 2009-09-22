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

#include "libempathy/empathy-account-manager.h"

#include "status-provider.h"
#include "status-provider-mc5.h"
#include "status-provider-mc5-marshal.h"

#include <dbus/dbus-glib.h>

static gchar * sp_to_mc_map[] = {
	/* STATUS_PROVIDER_STATUS_ONLINE,  */  "available",
	/* STATUS_PROVIDER_STATUS_AWAY,    */  "away",
	/* STATUS_PROVIDER_STATUS_DND      */  "busy",
	/* STATUS_PROVIDER_STATUS_INVISIBLE*/  "invisible",
	/* STATUS_PROVIDER_STATUS_OFFLINE  */  "offline",
	/* STATUS_PROVIDER_STATUS_DISCONNECTED*/NULL
};

static TpConnectionPresenceType sp_to_tp_map[] = {
	/* STATUS_PROVIDER_STATUS_ONLINE,  */    TP_CONNECTION_PRESENCE_TYPE_AVAILABLE,
	/* STATUS_PROVIDER_STATUS_AWAY,    */    TP_CONNECTION_PRESENCE_TYPE_AWAY,
	/* STATUS_PROVIDER_STATUS_DND      */    TP_CONNECTION_PRESENCE_TYPE_BUSY,
	/* STATUS_PROVIDER_STATUS_INVISIBLE*/    TP_CONNECTION_PRESENCE_TYPE_HIDDEN,
	/* STATUS_PROVIDER_STATUS_OFFLINE  */    TP_CONNECTION_PRESENCE_TYPE_OFFLINE,
	/* STATUS_PROVIDER_STATUS_DISCONNECTED*/ TP_CONNECTION_PRESENCE_TYPE_UNSET
};

typedef struct _StatusProviderMC5Private StatusProviderMC5Private;
struct _StatusProviderMC5Private {
	EmpathyAccountManager * manager;
	StatusProviderStatus status;
};

#define STATUS_PROVIDER_MC5_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), STATUS_PROVIDER_MC5_TYPE, StatusProviderMC5Private))

/* Prototypes */
/* GObject stuff */
static void status_provider_mc5_class_init (StatusProviderMC5Class *klass);
static void status_provider_mc5_init       (StatusProviderMC5 *self);
static void status_provider_mc5_dispose    (GObject *object);
static void status_provider_mc5_finalize   (GObject *object);
/* Internal Funcs */
static void set_status (StatusProvider * sp, StatusProviderStatus status);
static StatusProviderStatus get_status (StatusProvider * sp);

G_DEFINE_TYPE (StatusProviderMC5, status_provider_mc5, STATUS_PROVIDER_TYPE);

static void
status_provider_mc5_class_init (StatusProviderMC5Class *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (StatusProviderMC5Private));

	object_class->dispose = status_provider_mc5_dispose;
	object_class->finalize = status_provider_mc5_finalize;

	StatusProviderClass * spclass = STATUS_PROVIDER_CLASS(klass);

	spclass->set_status = set_status;
	spclass->get_status = get_status;

	return;
}


static void
status_provider_mc5_init (StatusProviderMC5 *self)
{
	StatusProviderMC5Private * priv = STATUS_PROVIDER_MC5_GET_PRIVATE(self);

	priv->status = STATUS_PROVIDER_STATUS_OFFLINE;
	priv->manager = EMPATHY_ACCOUNT_MANAGER(g_object_new(EMPATHY_TYPE_ACCOUNT_MANAGER, NULL));

	return;
}

static void
status_provider_mc5_dispose (GObject *object)
{
	StatusProviderMC5Private * priv = STATUS_PROVIDER_MC5_GET_PRIVATE(object);

	if (priv->manager != NULL) {
		g_object_unref(priv->manager);
		priv->manager = NULL;
	}

	G_OBJECT_CLASS (status_provider_mc5_parent_class)->dispose (object);
	return;
}

static void
status_provider_mc5_finalize (GObject *object)
{

	G_OBJECT_CLASS (status_provider_mc5_parent_class)->finalize (object);
	return;
}

/**
	status_provider_mc5_new:

	Creates a new #StatusProviderMC5 object.  No parameters or anything
	like that.  Just a convience function.

	Return value: A new instance of #StatusProviderMC5
*/
StatusProvider *
status_provider_mc5_new (void)
{
	return STATUS_PROVIDER(g_object_new(STATUS_PROVIDER_MC5_TYPE, NULL));
}

static void
set_status (StatusProvider * sp, StatusProviderStatus status)
{
	StatusProviderMC5Private * priv = STATUS_PROVIDER_MC5_GET_PRIVATE(sp);
	if (priv->manager == NULL) {
		priv->status = STATUS_PROVIDER_STATUS_DISCONNECTED;
		return;
	}

	priv->status = status;

	empathy_account_manager_request_global_presence(priv->manager, sp_to_tp_map[status], sp_to_mc_map[status], "");

	return;
}

static StatusProviderStatus
get_status (StatusProvider * sp)
{
	g_return_val_if_fail(IS_STATUS_PROVIDER_MC5(sp), STATUS_PROVIDER_STATUS_DISCONNECTED);
	StatusProviderMC5Private * priv = STATUS_PROVIDER_MC5_GET_PRIVATE(sp);

	if (priv->manager == NULL) {
		return STATUS_PROVIDER_STATUS_DISCONNECTED;
	}

	return priv->status;
}

