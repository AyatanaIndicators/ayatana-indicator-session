SUBDIRS = \
	icons \
	extra-sessions

dbus_servicesdir = $(DBUSSERVICEDIR)
service_in_files =  indicator-session.service.in
dbus_services_DATA = $(service_in_files:.service.in=.service)

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST =					\
	$(service_in_files)			\
	$(schema_in_files)

CLEANFILES =					\
	$(dbus_services_DATA)			\
	$(schema_DATA)

