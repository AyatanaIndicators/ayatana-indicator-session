CLEANFILES =
EXTRA_DIST =

libexec_PROGRAMS = \
	indicator-session-service

if BUILD_GTKLOGOUTHELPER
libexec_PROGRAMS += \
	gtk-logout-helper
endif

###################
# Indicator Stuff
###################

CLEANFILES += .libs/*.gcda .libs/*.gcno *.gcda *.gcno

sessionlibdir = $(INDICATORDIR)
sessionlib_LTLIBRARIES = libsession.la
libsession_la_SOURCES = \
	indicator-session.c \
	gen-session-dbus.xml.h \
	shared-names.h \
	user-widget.c \
	user-widget.h
libsession_la_CFLAGS = \
	$(APPLET_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall -Wunused \
	-DG_LOG_DOMAIN=\"Indicator-Session\"
libsession_la_LIBADD = $(APPLET_LIBS)
libsession_la_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	-module -avoid-version

dbus_display_manager_sources = \
	dbus-display-manager.c \
	dbus-display-manager.h

$(dbus_display_manager_sources): display-manager.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-display-manager \
	    $^

dbus_consolekit_manager_sources = \
	dbus-consolekit-manager.c \
	dbus-consolekit-manager.h

$(dbus_consolekit_manager_sources): org.freedesktop.ConsoleKit.Manager.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-consolekit-manager \
	    $^

dbus_consolekit_seat_sources = \
	dbus-consolekit-seat.c \
	dbus-consolekit-seat.h

$(dbus_consolekit_seat_sources): org.freedesktop.ConsoleKit.Seat.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-consolekit-seat \
	    $^

dbus_consolekit_session_sources = \
	dbus-consolekit-session.c \
	dbus-consolekit-session.h

$(dbus_consolekit_session_sources): org.freedesktop.ConsoleKit.Session.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-consolekit-session \
	    $^

dbus_accounts_sources = \
	dbus-accounts.c \
	dbus-accounts.h

$(dbus_accounts_sources): org.freedesktop.Accounts.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-accounts \
	    $^
	
dbus_user_sources = \
	dbus-user.c \
	dbus-user.h

$(dbus_user_sources): org.freedesktop.Accounts.User.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-user \
	    $^
	
dbus_upower_sources = \
        dbus-upower.c \
        dbus-upower.h

$(dbus_upower_sources): upower.xml
	gdbus-codegen \
	    --interface-prefix org.freedesktop \
	    --generate-c-code dbus-upower \
	    --c-namespace DBus \
	    $^

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
	$(dbus_accounts_sources) \
	$(dbus_consolekit_manager_sources) \
	$(dbus_consolekit_seat_sources) \
	$(dbus_consolekit_session_sources) \
	$(dbus_display_manager_sources) \
	$(dbus_upower_sources) \
	$(dbus_user_sources) \
	session-service.c \
	session-dbus.c \
	session-dbus.h \
	gen-session-dbus.xml.c \
	users-service-dbus.h \
	users-service-dbus.c \
	session-menu-mgr.h \
	session-menu-mgr.c \
	online-accounts-mgr.c \
	online-accounts-mgr.h

indicator_session_service_CFLAGS = \
	$(SESSIONSERVICE_CFLAGS) \
	$(GCONF_CFLAGS) \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-Wall \
	-DG_LOG_DOMAIN=\"Indicator-Session\" \
	$(COVERAGE_CFLAGS)
indicator_session_service_LDADD = \
	$(SESSIONSERVICE_LIBS) \
	$(GCONF_LIBS)
indicator_session_service_LDFLAGS = \
	$(COVERAGE_LDFLAGS)

#################
# GTK Logout Stuff
#################

if BUILD_GTKLOGOUTHELPER
gtk_logout_helper_SOURCES = \
	$(dbus_consolekit_manager_sources) \
	gtk-logout-helper.c \
	dialog.c \
	dialog.h

gtk_logout_helper_CFLAGS = \
	$(SESSIONSERVICE_CFLAGS) \
	$(GTKLOGOUTHELPER_CFLAGS) \
	$(GCONF_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall \
	-DINDICATOR_ICONS_DIR="\"$(INDICATORICONSDIR)\""

gtk_logout_helper_LDADD = \
	$(SESSIONSERVICE_LIBS) \
	$(GTKLOGOUTHELPER_LIBS) \
	$(GCONF_LIBS)

gtk_logout_helper_LDFLAGS = \
	$(COVERAGE_LDFLAGS)
endif


###############
# Other Stuff
###############

BUILT_SOURCES = \
	$(dbus_accounts_sources) \
	$(dbus_consolekit_manager_sources) \
	$(dbus_consolekit_seat_sources) \
	$(dbus_consolekit_session_sources) \
	$(dbus_display_manager_sources) \
	$(dbus_upower_sources) \
	$(dbus_user_sources) \
	gen-session-dbus.xml.c \
	gen-session-dbus.xml.h

EXTRA_DIST += \
	display-manager.xml \
	org.freedesktop.Accounts.User.xml \
	org.freedesktop.Accounts.xml \
	org.freedesktop.ConsoleKit.Manager.xml \
	org.freedesktop.ConsoleKit.Seat.xml \
	org.freedesktop.ConsoleKit.Session.xml \
	session-dbus.xml \
	upower.xml

CLEANFILES += $(BUILT_SOURCES)
