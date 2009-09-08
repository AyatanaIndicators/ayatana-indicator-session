
libexec_PROGRAMS = gtk-logout-helper

gtk_logout_helper_SOURCES = \
	gtk-logout-helper.c \
	ck-pk-helper.c \
	ck-pk-helper.h \
	gconf-helper.c \
	gconf-helper.h \
	logout-dialog.c \
	logout-dialog.h

gtk_logout_helper_CFLAGS = $(SESSIONSERVICE_CFLAGS) $(GTKLOGOUTHELPER_CFLAGS) $(GCONF_CFLAGS) -Wall -Werror
gtk_logout_helper_LDADD =  $(SESSIONSERVICE_LIBS) $(GTKLOGOUTHELPER_LIBS) $(GCONF_LIBS)

