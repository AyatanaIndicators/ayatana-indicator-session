/*
A dialog to ask the user about the various logout options that
are available.

Copyright 2010 Canonical Ltd.

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

#ifndef __LOGOUT_DIALOG_H__
#define __LOGOUT_DIALOG_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOGOUT_DIALOG_TYPE            (logout_dialog_get_type ())
#define LOGOUT_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOGOUT_DIALOG_TYPE, LogoutDialog))
#define LOGOUT_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LOGOUT_DIALOG_TYPE, LogoutDialogClass))
#define IS_LOGOUT_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOGOUT_DIALOG_TYPE))
#define IS_LOGOUT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LOGOUT_DIALOG_TYPE))
#define LOGOUT_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LOGOUT_DIALOG_TYPE, LogoutDialogClass))

typedef enum _LogoutDialogType LogoutDialogType;
enum _LogoutDialogType {
	LOGOUT_DIALOG_TYPE_LOG_OUT,
	LOGOUT_DIALOG_TYPE_RESTART,
	LOGOUT_DIALOG_TYPE_SHUTDOWN,
	LOGOUT_DIALOG_TYPE_CNT
};

typedef struct _LogoutDialog      LogoutDialog;
typedef struct _LogoutDialogClass LogoutDialogClass;

struct _LogoutDialogClass {
	GtkMessageDialogClass parent_class;
};

struct _LogoutDialog {
	GtkMessageDialog parent;
};

GType logout_dialog_get_type (void);
LogoutDialog * logout_dialog_new (LogoutDialogType type);

G_END_DECLS

#endif
