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
	LOGOUT_DIALOG_TYPE_SHUTDOWN
};

typedef struct _LogoutDialog      LogoutDialog;
typedef struct _LogoutDialogClass LogoutDialogClass;

struct _LogoutDialogClass {
	GtkDialogClass parent_class;
};

struct _LogoutDialog {
	GtkDialog parent;
};

GType logout_dialog_get_type (void);
LogoutDialog * logout_dialog_new (LogoutDialogType type);

G_END_DECLS

#endif
