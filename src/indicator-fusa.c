
#include <gtk/gtk.h>

GtkWidget *
get_menu_item (void)
{
	GtkWidget * mainmenu = gtk_menu_item_new();

	GtkWidget * hbox = gtk_hbox_new(FALSE, 3);

	GtkWidget * label = gtk_label_new("Ted Gould");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 3);
	gtk_widget_show(label);

	GtkWidget * icon = gtk_image_new_from_icon_name("gnome-logout",
	                                                GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
	gtk_widget_show(icon);

	gtk_container_add(GTK_CONTAINER(mainmenu), hbox);
	gtk_widget_show(hbox);

	gtk_widget_show(mainmenu);
	return mainmenu;
}

