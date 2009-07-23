
#include <glib.h>
#include <gtk/gtk.h>
#include "logout-dialog.h"

static LogoutDialogAction type = LOGOUT_DIALOG_LOGOUT;

static gboolean
option_cb (const gchar * arg, const gchar * value, gpointer data, GError * error)
{
	type = GPOINTER_TO_INT(data);
	return TRUE;
}

static GOptionEntry options[] = {
	{"logout",     'l',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_cb, "Log out of the current session",     GINT_TO_POINTER(LOGOUT_DIALOG_LOGOUT)},
	{"shutdown",   's',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_cb, "Shutdown the entire system",         GINT_TO_POINTER(LOGOUT_DIALOG_RESTART)},
	{"restart",    'r',  G_OPTION_FLAG_NO_ARG,  G_OPTION_ARG_CALLBACK,  option_cb, "Restart the system",                 GINT_TO_POINTER(LOGOUT_DIALOG_SHUTDOWN)},

	{NULL}
};

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	GError * error = NULL;
	GOptionContext * context = g_option_context_new(" - logout of the current session");
	g_option_context_add_main_entries(context, options, "gtk-logout-helper");
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_set_help_enabled(context, TRUE);

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_debug("Option parsing failed: %s", error->message);
		g_error_free(error);
		return 1;
	}

	GtkWidget * dialog = logout_dialog_new(LOGOUT_DIALOG_LOGOUT);
	gtk_dialog_run(GTK_DIALOG(dialog));

	return 0;
}
