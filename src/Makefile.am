
libexec_PROGRAMS = indicator-status-service indicator-users-service indicator-session-service

status_users_sessionlibdir = $(INDICATORDIR)
status_users_sessionlib_LTLIBRARIES = libstatus-users-session.la
libstatus_users_session_la_SOURCES = indicator-sus.c dbus-shared-names.h
libstatus_users_session_la_CFLAGS = $(APPLET_CFLAGS) -Wall -Werror
libstatus_users_session_la_LIBADD = $(APPLET_LIBS)

indicator_status_service_SOURCES = status-service.c
indicator_status_service_CFLAGS = $(STATUSSERVICE_CFLAGS)
indicator_status_service_LDADD = $(STATUSSERVICE_LIBS)

indicator_users_service_SOURCES = users-service.c
indicator_users_service_CFLAGS = $(USERSSERVICE_CFLAGS)
indicator_users_service_LDADD = $(USERSSERVICE_LIBS)

indicator_session_service_SOURCES = session-service.c
indicator_session_service_CFLAGS = $(SESSIONSERVICE_CFLAGS)
indicator_session_service_LDADD = $(SESSIONSERVICE_LIBS)

