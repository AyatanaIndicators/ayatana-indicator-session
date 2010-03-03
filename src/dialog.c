#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>

#include "consolekit-manager-client.h"
#include "dialog.h"

/* Strings */

static const gchar * title_strings[LOGOUT_DIALOG_TYPE_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	NC_("title", "Log Out"),
	/* LOGOUT_DIALOG_RESTART, */	NC_("title", "Restart"),
	/* LOGOUT_DIALOG_SHUTDOWN, */	NC_("title", "Switch Off")
};

static const gchar * body_strings[LOGOUT_DIALOG_TYPE_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	N_("Are you sure you want to close all programs and log out?"),
	/* LOGOUT_DIALOG_RESTART, */	N_("Are you sure you want to close all programs and restart the computer?"),
	/* LOGOUT_DIALOG_SHUTDOWN, */	N_("Are you sure you want to close all programs and shut down the computer?")
};

static const gchar * button_strings[LOGOUT_DIALOG_TYPE_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	NC_("button", "Log Out"),
	/* LOGOUT_DIALOG_RESTART, */	NC_("button", "Restart"),
	/* LOGOUT_DIALOG_SHUTDOWN, */	NC_("button", "Switch Off")
};

/* TRANSLATORS: These strings have an ellipsis so that the user knows
   they are also going to get a password dialog to do the action. */
static const gchar * button_auth_strings[LOGOUT_DIALOG_TYPE_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	NC_("button auth", "Log Out"),
	/* LOGOUT_DIALOG_RESTART, */	NC_("button auth", "Restart..."),
	/* LOGOUT_DIALOG_SHUTDOWN, */	NC_("button auth", "Switch Off...")
};

/*
static const gchar * restart_updates = N_("Restart Instead");
static const gchar * restart_auth = N_("Restart...");
static const gchar * body_logout_update = N_("Some software updates won't apply until the computer next restarts.");
*/

static const gchar * icon_strings[LOGOUT_DIALOG_TYPE_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	"system-log-out",
	/* LOGOUT_DIALOG_RESTART, */	"system-restart",
	/* LOGOUT_DIALOG_SHUTDOWN, */	"system-shutdown"
};



typedef struct _LogoutDialogPrivate LogoutDialogPrivate;
struct _LogoutDialogPrivate {
	guint type;
};

#define LOGOUT_DIALOG_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), LOGOUT_DIALOG_TYPE, LogoutDialogPrivate))

static void logout_dialog_class_init (LogoutDialogClass *klass);
static void logout_dialog_init       (LogoutDialog *self);
static void logout_dialog_dispose    (GObject *object);
static void logout_dialog_finalize   (GObject *object);

G_DEFINE_TYPE (LogoutDialog, logout_dialog, GTK_TYPE_MESSAGE_DIALOG);

static void
logout_dialog_class_init (LogoutDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (LogoutDialogPrivate));

	object_class->dispose = logout_dialog_dispose;
	object_class->finalize = logout_dialog_finalize;

	return;
}

static void
logout_dialog_init (LogoutDialog *self)
{

	return;
}

static void
logout_dialog_dispose (GObject *object)
{


	G_OBJECT_CLASS (logout_dialog_parent_class)->dispose (object);
	return;
}

static void
logout_dialog_finalize (GObject *object)
{


	G_OBJECT_CLASS (logout_dialog_parent_class)->finalize (object);
	return;
}

/* Checks for updates that would signal that a restart is
   required for them to apply */
static gboolean
check_restart_required (void)
{

	return FALSE;
}

/* Checks with console kit to see if we can do what we want */
static gboolean
ck_check_allowed (LogoutDialogType type)
{
	DBusGConnection * system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, NULL);
	g_return_val_if_fail(system_bus != NULL, TRUE);

	DBusGProxy * ck_proxy = dbus_g_proxy_new_for_name (system_bus,
	                                                   "org.freedesktop.ConsoleKit",
	                                                   "/org/freedesktop/ConsoleKit/Manager",
	                                                   "org.freedesktop.ConsoleKit.Manager");
	g_return_val_if_fail(ck_proxy != NULL, TRUE);

	gboolean retval = TRUE;
	switch (type) {
	case LOGOUT_DIALOG_TYPE_RESTART:
		org_freedesktop_ConsoleKit_Manager_can_restart(ck_proxy, &retval, NULL);
		break;
	case LOGOUT_DIALOG_TYPE_SHUTDOWN:
		org_freedesktop_ConsoleKit_Manager_can_stop(ck_proxy, &retval, NULL);
		break;
	default:
		break;
	}

	g_object_unref(ck_proxy);

	return retval;
}

LogoutDialog *
logout_dialog_new (LogoutDialogType type)
{
	GtkWidget * image = gtk_image_new_from_icon_name(icon_strings[type], GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(image);

	LogoutDialog * dialog = LOGOUT_DIALOG(g_object_new(LOGOUT_DIALOG_TYPE,
	                                      /* Window */
	                                      "icon-name", icon_strings[type],
	                                      "modal", TRUE,
	                                      "resizable", FALSE,
	                                      "title", _(title_strings[type]),
	                                      "window-position", GTK_WIN_POS_CENTER_ALWAYS,
	                                      /* Message Dialog */
	                                      "buttons", GTK_BUTTONS_NONE,
	                                      "image", image,
	                                      "message-type", GTK_MESSAGE_OTHER,
	                                      "text", _(body_strings[type]),
	                                      NULL));

	gboolean allowed = FALSE;
	if (type == LOGOUT_DIALOG_TYPE_LOG_OUT) {
		allowed = ck_check_allowed(LOGOUT_DIALOG_TYPE_RESTART);
	} else {
		allowed = ck_check_allowed(type);
	}

	gboolean restart_required = FALSE;
	if (type == LOGOUT_DIALOG_TYPE_LOG_OUT) {
		restart_required = check_restart_required();
	}

	const gchar * button_text;
	if (allowed) {
		button_text = _(button_strings[type]);
	} else {
		button_text = _(button_auth_strings[type]);
	}

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
	                       _("Cancel"), GTK_RESPONSE_CANCEL,
	                       button_text, GTK_RESPONSE_OK,
	                       NULL);

	return dialog;
}
