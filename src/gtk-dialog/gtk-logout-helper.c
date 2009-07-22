
#include <gtk/gtk.h>
#include "logout-dialog.h"

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	GtkWidget * dialog = logout_dialog_new(LOGOUT_DIALOG_LOGOUT);
	gtk_dialog_run(GTK_DIALOG(dialog));

	return 0;
}
