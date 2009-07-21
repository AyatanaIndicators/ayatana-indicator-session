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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __LOGOUT_DIALOG_H__
#define __LOGOUT_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOGOUT_TYPE_DIALOG (logout_dialog_get_type ())
#define LOGOUT_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), LOGOUT_TYPE_DIALOG, LogoutDialog))
#define LOGOUT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), LOGOUT_TYPE_DIALOG, LogoutDialogClass))
#define LOGOUT_IS_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), LOGOUT_TYPE_DIALOG))
#define LOGOUT_IS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LOGOUT_TYPE_CONTEXT))
#define LOGOUT_DIALOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), LOGOUT_TYPE_DIALOG, LogoutDialogClass))

typedef struct _LogoutDialogClass LogoutDialogClass;
typedef struct _LogoutDialog LogoutDialog;
typedef enum _LogoutDialogAction LogoutDialogAction;

enum _LogoutDialogAction {
	LOGOUT_DIALOG_LOGOUT,
	LOGOUT_DIALOG_RESTART,
	LOGOUT_DIALOG_SHUTDOWN,
	LOGOUT_DIALOG_ACTION_CNT
};

struct _LogoutDialogClass
{
  GtkDialogClass parent_class;
};

/**
 * LogoutDialog:
 * @dialog: parent widget
 * @main_vbox: GtkDialog's vbox
 * @hbox: box to separate the image of the right-side widgets
 * @image: the authorization image, left-side widget
 * @entry_vbox: right-side widgets container
 * @label: message describing what is required from the user,
 * right-side widget
 * @entry: place to type the password in, right-side widget
 * @ok_button: OK button of the dialog
 * @cancel_button: Cancel button of the dialog
 *
 * Convenience widget based on #GtkDialog to request a password.
 */
struct _LogoutDialog
{
  GtkDialog dialog;

  GtkWidget *main_vbox;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *ok_button;
  GtkWidget *cancel_button;
  GtkWidget *restart_button;
  GtkWidget *vbox_text;
  GtkWidget *message;
  GtkWidget *timeout_text;

  LogoutDialogAction action;

  /* private */
  gchar *         timeout_result;
  guint           timeout;
  guint           timerfunc;
};

GType
logout_dialog_get_type (void);

GtkWidget*
logout_dialog_new (LogoutDialogAction action);

LogoutDialogAction
logout_dialog_get_action (LogoutDialog * widget);

G_END_DECLS

#endif
