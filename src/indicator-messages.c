
#include <gtk/gtk.h>

GtkWidget *
get_menu_item (void)
{
	GtkWidget * main = gtk_menu_item_new_with_label("Message Me");

	return main;
}

