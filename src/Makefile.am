SUBDIRS = gtk-dialog

libexec_PROGRAMS = indicator-session-service

###################
# Indicator Stuff
###################

sessionlibdir = $(INDICATORDIR)
sessionlib_LTLIBRARIES = libsession.la
libsession_la_SOURCES = \
	indicator-session.c \
	dbus-shared-names.h \
	users-service-client.h
libsession_la_CFLAGS = $(APPLET_CFLAGS) -Wall -Werror
libsession_la_LIBADD = $(APPLET_LIBS)
libsession_la_LDFLAGS = -module -avoid-version


users-service-client.h: $(srcdir)/users-service.xml
	dbus-binding-tool \
		--prefix=_users_service_client \
		--mode=glib-client \
		--output=users-service-client.h \
		$(srcdir)/users-service.xml

users-service-marshal.h: $(srcdir)/users-service.list
	glib-genmarshal --header \
		--prefix=_users_service_marshal $(srcdir)/users-service.list \
		> users-service-marshal.h

users-service-marshal.c: $(srcdir)/users-service.list
	glib-genmarshal --body \
		--prefix=_users_service_marshal $(srcdir)/users-service.list \
		> users-service-marshal.c

#################
# Session Stuff
#################

indicator_session_service_SOURCES = \
	lock-helper.c \
	lock-helper.h \
	session-service.c \
	gtk-dialog/gconf-helper.c \
	users-service-dbus.h \
	users-service-dbus.c \
	users-service-marshal.c
indicator_session_service_CFLAGS = $(SESSIONSERVICE_CFLAGS) $(GCONF_CFLAGS) -DLIBEXECDIR=\"$(libexecdir)\" -Wall -Werror
indicator_session_service_LDADD = $(SESSIONSERVICE_LIBS) $(GCONF_LIBS)

###############
# Other Stuff
###############

BUILT_SOURCES = \
	users-service-client.h \
	users-service-marshal.h \
	users-service-marshal.c

EXTRA_DIST = \
	users-service.xml \
	users-service.list

CLEANFILES = \
	$(BUILT_SOURCES)
