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

@INTLTOOL_DESKTOP_RULE@

%.desktop.in: %.desktop.in.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

logout_helper_desktop_in_in_files = \
	indicator-session-shutdown.desktop.in.in

logout_helperdir = $(datadir)/applications
logout_helper_desktop_files = \
	$(logout_helper_desktop_in_in_files:.desktop.in.in=.desktop)

EXTRA_DIST =					\
	$(service_in_files)			\
	$(gsettings_SCHEMAS:.xml=.xml.in) \
	$(logout_helper_desktop_in_in_files)


CLEANFILES =					\
	$(dbus_services_DATA)			\
	$(gsettings_SCHEMAS) \
	$(logout_helper_desktop_files)

