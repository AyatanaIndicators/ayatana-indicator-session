SUBDIRS = gtk-dialog

libexec_PROGRAMS = indicator-status-service indicator-users-service indicator-session-service

###################
# Indicator Stuff
###################

sessionlibdir = $(INDICATORDIR)
sessionlib_LTLIBRARIES = libsession.la
libsession_la_SOURCES = \
	indicator-session.c \
	dbus-shared-names.h \
	status-service-client.h
libsession_la_CFLAGS = $(APPLET_CFLAGS) -Wall -Werror
libsession_la_LIBADD = $(APPLET_LIBS)
libsession_la_LDFLAGS = -module -avoid-version

################
# Status Stuff
################

indicator_status_service_SOURCES = \
	status-service.c \
	status-service-dbus.h \
	status-service-dbus.c \
	status-service-server.h \
	status-provider.h \
	status-provider.c \
	status-provider-pidgin.h \
	status-provider-pidgin.c \
	status-provider-pidgin-marshal.h \
	status-provider-pidgin-marshal.c \
	status-provider-telepathy.h \
	status-provider-telepathy.c \
	status-provider-telepathy-marshal.h \
	status-provider-telepathy-marshal.c
indicator_status_service_CFLAGS = $(STATUSSERVICE_CFLAGS) -Wall -Werror
indicator_status_service_LDADD = $(STATUSSERVICE_LIBS)

status-service-client.h: $(srcdir)/status-service.xml
	dbus-binding-tool \
		--prefix=_status_service_client \
		--mode=glib-client \
		--output=status-service-client.h \
		$(srcdir)/status-service.xml

status-service-server.h: $(srcdir)/status-service.xml
	dbus-binding-tool \
		--prefix=_status_service_server \
		--mode=glib-server \
		--output=status-service-server.h \
		$(srcdir)/status-service.xml

status-provider-pidgin-marshal.h: $(srcdir)/status-provider-pidgin.list
	glib-genmarshal --header \
		--prefix=_status_provider_pidgin_marshal $(srcdir)/status-provider-pidgin.list \
		> status-provider-pidgin-marshal.h

status-provider-pidgin-marshal.c: $(srcdir)/status-provider-pidgin.list
	glib-genmarshal --body \
		--prefix=_status_provider_pidgin_marshal $(srcdir)/status-provider-pidgin.list \
		> status-provider-pidgin-marshal.c

status-provider-telepathy-marshal.h: $(srcdir)/status-provider-telepathy.list
	glib-genmarshal --header \
		--prefix=_status_provider_telepathy_marshal $(srcdir)/status-provider-telepathy.list \
		> status-provider-telepathy-marshal.h

status-provider-telepathy-marshal.c: $(srcdir)/status-provider-telepathy.list
	glib-genmarshal --body \
		--prefix=_status_provider_telepathy_marshal $(srcdir)/status-provider-telepathy.list \
		> status-provider-telepathy-marshal.c

###############
# Users Stuff
###############

indicator_users_service_SOURCES = users-service.c
indicator_users_service_CFLAGS = $(USERSSERVICE_CFLAGS) -Wall -Werror
indicator_users_service_LDADD = $(USERSSERVICE_LIBS)

#################
# Session Stuff
#################

indicator_session_service_SOURCES = session-service.c gtk-dialog/gconf-helper.c
indicator_session_service_CFLAGS = $(SESSIONSERVICE_CFLAGS) $(GCONF_CFLAGS) -DLIBEXECDIR=\"$(libexecdir)\" -Wall -Werror
indicator_session_service_LDADD = $(SESSIONSERVICE_LIBS) $(GCONF_LIBS)

###############
# Other Stuff
###############

BUILT_SOURCES = \
	status-service-client.h \
	status-service-server.h \
	status-provider-pidgin-marshal.h \
	status-provider-pidgin-marshal.c \
	status-provider-telepathy-marshal.h \
	status-provider-telepathy-marshal.c

EXTRA_DIST = \
	status-service.xml \
	status-provider-pidgin.list \
	status-provider-telepathy.list

CLEANFILES = \
	$(BUILT_SOURCES)
