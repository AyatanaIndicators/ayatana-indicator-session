/*
 * libgksuui -- Gtk+ widget and convenience functions for requesting passwords
 * Copyright (C) 2004 Gustavo Noronha Silva
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301 USA
 */

#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <X11/XKBlib.h>

#include "logout-dialog.h"
#include "ck-pk-helper.h"

enum {
	PROP_ZERO,
	PROP_ACTION
};


static void
logout_dialog_class_init (LogoutDialogClass *klass);

static void
logout_dialog_init (LogoutDialog *logout_dialog);

static void
set_property (GObject * object, guint param_id, const GValue * value, GParamSpec *pspec);

static void
get_property (GObject * object, guint param_id, GValue * value, GParamSpec *pspec);

static gboolean
timer_cb (gpointer data);

static void
show_cb (GtkWidget * widget, gpointer data);

static void
check_restart (LogoutDialog * dialog);

static gchar*
get_plural_string (LogoutDialog * dialog);

static const gchar * title_strings[LOGOUT_DIALOG_ACTION_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	NC_("title", "Log Out"),
	/* LOGOUT_DIALOG_RESTART, */	NC_("title", "Restart"),
	/* LOGOUT_DIALOG_SHUTDOWN, */	NC_("title", "Shut Down")
};

static const gchar * button_strings[LOGOUT_DIALOG_ACTION_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	NC_("button", "Log Out"),
	/* LOGOUT_DIALOG_RESTART, */	NC_("button", "Restart"),
	/* LOGOUT_DIALOG_SHUTDOWN, */	NC_("button", "Shut Down")
};

static const gchar * restart_auth = N_("Restart...");

static const gchar * body_logout_update = N_("You recently installed updates which will only take effect after a restart.  Restart to apply software updates.");

static const gchar * icon_strings[LOGOUT_DIALOG_ACTION_CNT] = {
	/* LOGOUT_DIALOG_LOGOUT, */ 	"system-log-out",
	/* LOGOUT_DIALOG_RESTART, */	"system-restart",
	/* LOGOUT_DIALOG_SHUTDOWN, */	"system-shutdown"
};

GType
logout_dialog_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    {
      static const GTypeInfo info =
	{
	  sizeof (LogoutDialogClass), /* size of class */
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) logout_dialog_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (LogoutDialog), /* size of object */
	  0, /* n_preallocs */
	  (GInstanceInitFunc) logout_dialog_init /* instance_init */
	};
      type = g_type_register_static (gtk_dialog_get_type (),
				     "LogoutDialogType",
				     &info, 0);
    }

  return type;
}

static gchar*
get_plural_string (LogoutDialog * dialog)
{
  static gchar *plural_string = "";

  switch (dialog->action)
    {
    case LOGOUT_DIALOG_LOGOUT:
      plural_string = ngettext("You will be logged out in %d second.",
			       "You will be logged out in %d seconds.", 
			       dialog->timeout);
      break;
    case LOGOUT_DIALOG_RESTART:
      plural_string = ngettext("The computer will restart in %d second.",
			       "The computer will restart in %d seconds.", 
			       dialog->timeout);
      break;
    case LOGOUT_DIALOG_SHUTDOWN:
      plural_string = ngettext("The computer will be shut down in %d second.",
			       "The computer will be shut down in %d seconds.",
			       dialog->timeout);
      break;
	default:
	  break;
    }
  
  return plural_string;
}

static void
logout_dialog_class_init (LogoutDialogClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  g_object_class_install_property(gobject_class, PROP_ACTION,
                                  g_param_spec_int("action", NULL, NULL,
                                                   LOGOUT_DIALOG_LOGOUT, LOGOUT_DIALOG_SHUTDOWN,
                                                   LOGOUT_DIALOG_LOGOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  return;
}

static void
set_property (GObject * object, guint param_id, const GValue * value, GParamSpec *pspec)
{
	g_return_if_fail(param_id == PROP_ACTION);

	LogoutDialog * dialog = LOGOUT_DIALOG(object);
	dialog->action = (LogoutDialogAction)g_value_get_int(value);

	gtk_image_set_from_icon_name(GTK_IMAGE(dialog->image), icon_strings[dialog->action], GTK_ICON_SIZE_DIALOG);
	gtk_window_set_title (GTK_WINDOW(dialog), _(title_strings[dialog->action]));
	gtk_widget_hide(dialog->message);
	gtk_button_set_label(GTK_BUTTON(dialog->ok_button), _(button_strings[dialog->action]));

	gchar * timeouttxt = g_strdup_printf(get_plural_string(dialog), dialog->timeout);
	gtk_label_set_text(GTK_LABEL(dialog->timeout_text), timeouttxt);
	g_free(timeouttxt);

	check_restart(dialog);

	return;
}

static void
get_property (GObject * object, guint param_id, GValue * value, GParamSpec *pspec)
{
	g_return_if_fail(param_id == PROP_ACTION);
	g_value_set_int(value, LOGOUT_DIALOG(object)->action);
}

static gboolean
timer_cb (gpointer data)
{
	LogoutDialog * dialog = LOGOUT_DIALOG(data);

	if (dialog->timeout == 0) {
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		dialog->timerfunc = 0;
		return FALSE;
	} else {
		dialog->timeout--;

		gchar * timeouttxt = g_strdup_printf(get_plural_string(dialog), dialog->timeout);
		gtk_label_set_text(GTK_LABEL(dialog->timeout_text), timeouttxt);
		g_free(timeouttxt);
	}

	return TRUE;
}

static void
show_cb (GtkWidget * widget, gpointer data)
{
	LogoutDialog * dialog = LOGOUT_DIALOG(widget);

	if (dialog->timerfunc != 0) {
		g_source_remove(dialog->timerfunc);
		dialog->timerfunc = 0;
	}

	dialog->timerfunc = g_timeout_add_seconds(1, timer_cb, dialog);
	return;
}

static void
check_restart (LogoutDialog * dialog)
{
	if (dialog->action != LOGOUT_DIALOG_LOGOUT) {
		return;
	}

	if (g_file_test("/var/run/reboot-required", G_FILE_TEST_EXISTS)) {
		if (pk_can_do_action("org.freedesktop.consolekit.system.restart", NULL) ||
			pk_can_do_action("org.freedesktop.consolekit.system.restart-multiple-users", NULL)) {

			gtk_label_set_text(GTK_LABEL(dialog->message), _(body_logout_update));
			gtk_widget_show(dialog->message);
			if (pk_require_auth(LOGOUT_DIALOG_RESTART)) {
			  gtk_button_set_label(GTK_BUTTON(dialog->restart_button), _(restart_auth));
			} else {
			  gtk_button_set_label(GTK_BUTTON(dialog->restart_button), _(button_strings[LOGOUT_DIALOG_RESTART]));
			}
			gtk_widget_show(dialog->restart_button);
		}
	}

	return;
}

static gboolean
focus_out_cb (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
  gtk_window_present (GTK_WINDOW(widget));
  return TRUE;
}

static void
logout_dialog_init (LogoutDialog *logout_dialog)
{
  GtkDialog *dialog;
  gint      border_width = 6;

  logout_dialog->timeout = 60;
  logout_dialog->timerfunc = 0;

  /* dialog window */
  dialog = GTK_DIALOG(logout_dialog);

  /* make sure that our window will always have the focus */
  g_signal_connect (G_OBJECT(dialog), "focus-out-event",
		    G_CALLBACK(focus_out_cb), NULL);

  logout_dialog->main_vbox = dialog->vbox;

  gtk_window_set_title (GTK_WINDOW(logout_dialog), "");
  gtk_dialog_set_has_separator (GTK_DIALOG(logout_dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(logout_dialog), border_width);
  gtk_box_set_spacing (GTK_BOX(logout_dialog->main_vbox), 12);
  gtk_window_set_resizable (GTK_WINDOW(logout_dialog), FALSE);

  gtk_window_stick(GTK_WINDOW(logout_dialog));
  gtk_window_set_keep_above(GTK_WINDOW(logout_dialog), TRUE);
  gtk_widget_realize(GTK_WIDGET(logout_dialog));
  /* remove superfluous window buttons */
  gdk_window_set_functions (GTK_WIDGET(logout_dialog)->window, 0);
  gdk_window_set_decorations (GTK_WIDGET(logout_dialog)->window, GDK_DECOR_BORDER | GDK_DECOR_TITLE);

  /* center window */
  gtk_window_set_position (GTK_WINDOW(logout_dialog), GTK_WIN_POS_CENTER);

  /* the action buttons */
  /*  the cancel button  */
  logout_dialog->restart_button = gtk_dialog_add_button (dialog,
						      GTK_STOCK_HELP,
						      GTK_RESPONSE_HELP);
  gtk_button_set_label(GTK_BUTTON(logout_dialog->restart_button), _(button_strings[LOGOUT_DIALOG_RESTART]));
  gtk_widget_hide(logout_dialog->restart_button);

  /*  the cancel button  */
  logout_dialog->cancel_button = gtk_dialog_add_button (dialog,
						      GTK_STOCK_CANCEL,
						      GTK_RESPONSE_CANCEL);
  /*  the ok button  */
  logout_dialog->ok_button = gtk_dialog_add_button (dialog,
						  GTK_STOCK_OK,
						  GTK_RESPONSE_OK);
  gtk_widget_grab_default (logout_dialog->ok_button);

  /* Window Title and Icon */
  gtk_window_set_title (GTK_WINDOW(logout_dialog), _(title_strings[logout_dialog->action]));
  gtk_window_set_icon_name (GTK_WINDOW(logout_dialog), icon_strings[logout_dialog->action]);

  /* hbox */
  logout_dialog->hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER(logout_dialog->hbox), 6);
  gtk_box_pack_start (GTK_BOX(logout_dialog->main_vbox),
		      logout_dialog->hbox, FALSE, FALSE, 0);
  gtk_widget_show (logout_dialog->hbox);

  /* image */
  logout_dialog->image =
    gtk_image_new_from_icon_name (icon_strings[logout_dialog->action],
			      GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC(logout_dialog->image), 0.5, 0);
  gtk_box_pack_start (GTK_BOX(logout_dialog->hbox), logout_dialog->image,
		      FALSE, FALSE, 0);
  gtk_widget_show (logout_dialog->image);

  /* vbox for text */
  logout_dialog->vbox_text = gtk_vbox_new(FALSE, 12);
  gtk_box_pack_start(GTK_BOX(logout_dialog->hbox), logout_dialog->vbox_text, TRUE, TRUE, 0);
  gtk_widget_show(logout_dialog->vbox_text);

  /* Message */
  logout_dialog->message = gtk_label_new("");
  gtk_label_set_line_wrap(GTK_LABEL(logout_dialog->message), TRUE);
  gtk_label_set_single_line_mode(GTK_LABEL(logout_dialog->message), FALSE);
  gtk_label_set_selectable(GTK_LABEL(logout_dialog->message), TRUE);
  gtk_misc_set_alignment (GTK_MISC(logout_dialog->message), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(logout_dialog->vbox_text), logout_dialog->message, TRUE, TRUE, 0);
  gtk_widget_show(logout_dialog->message);

  /* timeout */
  logout_dialog->timeout_text = gtk_label_new("");
  gtk_label_set_line_wrap(GTK_LABEL(logout_dialog->timeout_text), TRUE);
  gtk_label_set_single_line_mode(GTK_LABEL(logout_dialog->timeout_text), FALSE);
  gtk_label_set_selectable(GTK_LABEL(logout_dialog->timeout_text), FALSE);
  gtk_misc_set_alignment (GTK_MISC(logout_dialog->timeout_text), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(logout_dialog->vbox_text), logout_dialog->timeout_text, TRUE, TRUE, 0);
  gtk_widget_show(logout_dialog->timeout_text);

  g_signal_connect(G_OBJECT(logout_dialog), "show", G_CALLBACK(show_cb), logout_dialog);

  return;
}

/**
 * logout_dialog_new:
 *
 * Creates a new #LogoutDialog.
 *
 * Returns: the new #LogoutDialog
 */
GtkWidget*
logout_dialog_new (LogoutDialogAction action)
{
  LogoutDialog * dialog = g_object_new (LOGOUT_TYPE_DIALOG, "action", action, NULL);
  return GTK_WIDGET(dialog);
}

LogoutDialogAction
logout_dialog_get_action (LogoutDialog * dialog)
{
	return dialog->action;
}

