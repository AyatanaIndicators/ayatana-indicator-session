SUBDIRS = \
	icons \
	extra-sessions

upstart_jobsdir = $(datadir)/upstart/sessions/
upstart_jobs_in_files = indicator-session.conf.in
upstart_jobs_DATA = $(upstart_jobs_in_files:.conf.in=.conf)

%.conf: %.conf.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

@INTLTOOL_SCHEMAS_RULE@
@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@
gsettings_SCHEMAS = com.canonical.indicator.session.gschema.xml

convertdir = $(datadir)/GConf/gsettings
dist_convert_DATA = indicator-session.convert

EXTRA_DIST =					\
	$(upstart_jobs_in_files)	\
	$(gsettings_SCHEMAS:.xml=.xml.in)

CLEANFILES =					\
	$(upstart_jobs_DATA)		\
	$(gsettings_SCHEMAS)
