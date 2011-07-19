
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
	gen-session-dbus.xml.h \
	dbus-shared-names.h \
	dbusmenu-shared.h \
	user-widget.c \
	user-widget.h \
	accounts-service-client.h \
	accounts-service-user-client.h
libsession_la_CFLAGS = \
	$(APPLET_CFLAGS) \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"Indicator-Session\"
libsession_la_LIBADD = $(APPLET_LIBS)
libsession_la_LDFLAGS = -module -avoid-version

consolekit-manager-client.h: $(srcdir)/consolekit-manager.xml
	dbus-binding-tool \
		--prefix=_consolekit_manager_client \
		--mode=glib-client \
		--output=consolekit-manager-client.h \
		$(srcdir)/consolekit-manager.xml

consolekit-session-client.h: $(srcdir)/consolekit-session.xml
	dbus-binding-tool \
		--prefix=_consolekit_session_client \
		--mode=glib-client \
		--output=consolekit-session-client.h \
		$(srcdir)/consolekit-session.xml

display-manager-client.h: $(srcdir)/display-manager.xml
	dbus-binding-tool \
		--prefix=_gdm_local_display_factory_client \
		--mode=glib-client \
		--output=display-manager-client.h \
		$(srcdir)/display-manager.xml

accounts-service-client.h: $(srcdir)/accounts-service.xml
	dbus-binding-tool \
		--prefix=_accounts_service_client \
		--mode=glib-client \
		--output=accounts-service-client.h \
		$(srcdir)/accounts-service.xml

accounts-service-user-client.h: $(srcdir)/accounts-service-user.xml
	dbus-binding-tool \
		--prefix=_accounts_service_user_client \
		--mode=glib-client \
		--output=accounts-service-user-client.h \
		$(srcdir)/accounts-service-user.xml

upower-client.h: $(srcdir)/upower.xml
	dbus-binding-tool \
		--prefix=_upower_client \
		--mode=glib-client \
		--output=upower-client.h \
		$(srcdir)/upower.xml

gen-%.xml.c: %.xml
	@echo "Building $@ from $<"
	@echo "const char * _$(subst -,_,$(subst .,_,$(basename $(notdir $<)))) = " > $@
	@sed -e "s:\":\\\\\":g" -e s:^:\": -e s:\$$:\\\\n\": $< >> $@
	@echo ";" >> $@

gen-%.xml.h: %.xml
	@echo "Building $@ from $<"
	@echo "extern const char * _$(subst -,_,$(subst .,_,$(basename $(notdir $<))));" > $@

#################
# Session Stuff
#################

indicator_session_service_SOURCES = \
	lock-helper.c \
	lock-helper.h \
	session-service.c \
	session-dbus.c \
	session-dbus.h \
	gen-session-dbus.xml.c \
	dbusmenu-shared.h \
	gconf-helper.c \
	users-service-dbus.h \
	users-service-dbus.c \
	user-menu-mgr.h \
	user-menu-mgr.c \	
	device-menu-mgr.h \
	device-menu-mgr.c \
	apt-watcher.h \
	apt-watcher.c \
	udev-mgr.h \
	udev-mgr.c
indicator_session_service_CFLAGS = \
	$(SESSIONSERVICE_CFLAGS) \
	$(GCONF_CFLAGS) \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"Indicator-Session\"
indicator_session_service_LDADD = \
	$(SESSIONSERVICE_LIBS) \
	$(GCONF_LIBS)

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
	consolekit-session-client.h \
	display-manager-client.h \
	gen-session-dbus.xml.c \
	gen-session-dbus.xml.h \
	upower-client.h \
	accounts-service-client.h \
	accounts-service-user-client.h

EXTRA_DIST = \
	consolekit-manager.xml \
	consolekit-session.xml \
	display-manager.xml \
	session-dbus.xml \
	upower.xml \
	accounts-service.xml \
	accounts-service-user.xml

CLEANFILES = \
	$(BUILT_SOURCES)
