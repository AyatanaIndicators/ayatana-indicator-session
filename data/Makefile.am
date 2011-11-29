SUBDIRS = \
	icons \
	extra-sessions

dbus_servicesdir = $(DBUSSERVICEDIR)
service_in_files =  indicator-session.service.in
dbus_services_DATA = $(service_in_files:.service.in=.service)

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

@INTLTOOL_SCHEMAS_RULE@
@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@
gsettings_SCHEMAS = com.canonical.indicator.session.gschema.xml

convertdir = $(datadir)/GConf/gsettings
dist_convert_DATA = indicator-session.convert

EXTRA_DIST =					\
	$(service_in_files)			\
	$(gsettings_SCHEMAS:.xml=.xml.in)

CLEANFILES =					\
	$(dbus_services_DATA)			\
	$(gsettings_SCHEMAS)

if BUILD_GTKLOGOUTHELPER
@INTLTOOL_DESKTOP_RULE@

%.desktop.in: %.desktop.in.in
	sed \
		-e "s|\@libexecdir\@|$(libexecdir)|" \
		-e "s|\@bindir\@|$(bindir)|" \
		$< > $@

logout_helper_desktop_in_in_files = \
	indicator-session-lock-screen.desktop.in.in \
	indicator-session-logout.desktop.in.in \
	indicator-session-restart.desktop.in.in \
	indicator-session-shutdown.desktop.in.in

logout_helperdir = $(datadir)/applications
logout_helper_desktop_files = \
	$(logout_helper_desktop_in_in_files:.desktop.in.in=.desktop)
logout_helper_DATA = $(logout_helper_desktop_files)

EXTRA_DIST +=					\
	$(logout_helper_desktop_in_in_files)

CLEANFILES +=					\
	$(logout_helper_desktop_files)
endif
