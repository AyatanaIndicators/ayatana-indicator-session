
libexec_PROGRAMS = \
	indicator-session-service \
	gtk-logout-helper

###################
# Indicator Stuff
###################

sessionlibdir = $(INDICATORDIR)
sessionlib_LTLIBRARIES = libsession.la
libsession_la_SOURCES = \
	indicator-session.c \
	session-dbus-client.h \
	dbus-shared-names.h \
	dbusmenu-shared.h \
	users-service-client.h
libsession_la_CFLAGS = $(APPLET_CFLAGS) -Wall -Werror
libsession_la_LIBADD = $(APPLET_LIBS)
libsession_la_LDFLAGS = -module -avoid-version

consolekit-manager-client.h: $(srcdir)/consolekit-manager.xml
	dbus-binding-tool \
		--prefix=_consolekit_manager_client \
		--mode=glib-client \
		--output=consolekit-manager-client.h \
		$(srcdir)/consolekit-manager.xml

gdm-local-display-factory-client.h: $(srcdir)/gdm-local-display-factory.xml
	dbus-binding-tool \
		--prefix=_gdm_local_display_factory_client \
		--mode=glib-client \
		--output=gdm-local-display-factory-client.h \
		$(srcdir)/gdm-local-display-factory.xml

users-service-client.h: $(srcdir)/users-service.xml
	dbus-binding-tool \
		--prefix=_users_service_client \
		--mode=glib-client \
		--output=users-service-client.h \
		$(srcdir)/users-service.xml

upower-client.h: $(srcdir)/upower.xml
	dbus-binding-tool \
		--prefix=_upower_client \
		--mode=glib-client \
		--output=upower-client.h \
		$(srcdir)/upower.xml

session-dbus-client.h: $(srcdir)/session-dbus.xml
	dbus-binding-tool \
		--prefix=_session_dbus_client \
		--mode=glib-client \
		--output=session-dbus-client.h \
		$(srcdir)/session-dbus.xml

session-dbus-server.h: $(srcdir)/session-dbus.xml
	dbus-binding-tool \
		--prefix=_session_dbus_server \
		--mode=glib-server \
		--output=session-dbus-server.h \
		$(srcdir)/session-dbus.xml

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
	session-dbus.c \
	session-dbus.h \
	session-dbus-server.h \
	dbusmenu-shared.h \
	gconf-helper.c \
	users-service-dbus.h \
	users-service-dbus.c \
	users-service-marshal.c
indicator_session_service_CFLAGS = $(SESSIONSERVICE_CFLAGS) $(GCONF_CFLAGS) -DLIBEXECDIR=\"$(libexecdir)\" -Wall -Werror
indicator_session_service_LDADD = $(SESSIONSERVICE_LIBS) $(GCONF_LIBS)

#################
# GTK Logout Stuff
#################

gtk_logout_helper_SOURCES = \
	gtk-logout-helper.c \
	gconf-helper.c \
	gconf-helper.h \
	dialog.c \
	dialog.h

gtk_logout_helper_CFLAGS = \
	$(SESSIONSERVICE_CFLAGS) \
	$(GTKLOGOUTHELPER_CFLAGS) \
	$(GCONF_CFLAGS) \
	-Wall -Werror \
	-DINDICATOR_ICONS_DIR="\"$(INDICATORICONSDIR)\""

gtk_logout_helper_LDADD = \
	$(SESSIONSERVICE_LIBS) \
	$(GTKLOGOUTHELPER_LIBS) \
	$(GCONF_LIBS)


###############
# Other Stuff
###############

BUILT_SOURCES = \
	consolekit-manager-client.h \
	gdm-local-display-factory-client.h \
	session-dbus-client.h \
	session-dbus-server.h \
	upower-client.h \
	users-service-client.h \
	users-service-marshal.h \
	users-service-marshal.c

EXTRA_DIST = \
	consolekit-manager.xml \
	gdm-local-display-factory.xml \
	session-dbus.xml \
	upower.xml \
	users-service.xml \
	users-service.list

CLEANFILES = \
	$(BUILT_SOURCES)
