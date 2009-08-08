SUBDIRS = gtk-dialog

libexec_PROGRAMS = indicator-status-service indicator-users-service indicator-session-service

###################
# Indicator Stuff
###################

status_users_sessionlibdir = $(INDICATORDIR)
status_users_sessionlib_LTLIBRARIES = libstatus-users-session.la
libstatus_users_session_la_SOURCES = \
	indicator-sus.c \
	dbus-shared-names.h \
	status-service-client.h
libstatus_users_session_la_CFLAGS = $(APPLET_CFLAGS) -Wall -Werror
libstatus_users_session_la_LIBADD = $(APPLET_LIBS)
libstatus_users_session_la_LDFLAGS = -module -avoid-version

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
	status-provider-pidgin.c
indicator_status_service_CFLAGS = $(STATUSSERVICE_CFLAGS) -Wall -Werror
indicator_status_service_LDADD = $(STATUSSERVICE_LIBS)

status-service-client.h: status-service.xml
	dbus-binding-tool \
		--prefix=_status_service_client \
		--mode=glib-client \
		--output=status-service-client.h \
		$(srcdir)/status-service.xml

status-service-server.h: status-service.xml
	dbus-binding-tool \
		--prefix=_status_service_server \
		--mode=glib-server \
		--output=status-service-server.h \
		$(srcdir)/status-service.xml

###############
# Users Stuff
###############

indicator_users_service_SOURCES = users-service.c
indicator_users_service_CFLAGS = $(USERSSERVICE_CFLAGS) -Wall -Werror
indicator_users_service_LDADD = $(USERSSERVICE_LIBS)

#################
# Session Stuff
#################

indicator_session_service_SOURCES = session-service.c
indicator_session_service_CFLAGS = $(SESSIONSERVICE_CFLAGS) -DLIBEXECDIR=\"$(libexecdir)\" -Wall -Werror
indicator_session_service_LDADD = $(SESSIONSERVICE_LIBS)

###############
# Other Stuff
###############

BUILT_SOURCES = \
	status-service-client.h \
	status-service-server.h

EXTRA_DIST = \
	status-service.xml
