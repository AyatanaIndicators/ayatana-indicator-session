
dbus_servicesdir = $(DBUSSERVICEDIR)
service_in_files =  indicator-status.service.in \
                    indicator-users.service.in \
                    indicator-session.service.in
dbus_services_DATA = $(service_in_files:.service.in=.service)

$(dbus_services_DATA): $(service_in_files) Makefile
	@sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

EXTRA_DIST = $(service_in_files)

