
libexec_PROGRAMS = gtk-logout-helper

gtk_logout_helper_SOURCES = \
	gtk-logout-helper.c \
	gconf-helper.c \
	gconf-helper.h \
	dialog.c \
	dialog.h

gtk_logout_helper_CFLAGS = $(SESSIONSERVICE_CFLAGS) $(GTKLOGOUTHELPER_CFLAGS) $(GCONF_CFLAGS) -Wall -Werror -DINDICATOR_ICONS_DIR="\"$(INDICATORICONSDIR)\""
gtk_logout_helper_LDADD =  $(SESSIONSERVICE_LIBS) $(GTKLOGOUTHELPER_LIBS) $(GCONF_LIBS)

