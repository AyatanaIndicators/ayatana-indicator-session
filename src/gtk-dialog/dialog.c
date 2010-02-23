#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dialog.h"

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

G_DEFINE_TYPE (LogoutDialog, logout_dialog, GTK_TYPE_DIALOG);

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
