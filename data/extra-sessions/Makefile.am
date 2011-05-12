
###################################
# The files that are actually here
###################################

extra_guests_desktop_in_in_files = \
	classic-desktop.desktop.in.in
extra_guests_sh_files = \
	classic-desktop.sh

###################################
# Rules to make things work
###################################

extra_guestsdir = $(datadir)/indicators/session/applications/
extra_guests_desktop_files = \
	$(extra_guests_desktop_in_in_files:.desktop.in.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

%.desktop.in: %.desktop.in.in
	sed -e "s|\@guestdir\@|$(extra_guestsdir)|" $< > $@

extra_guests_DATA = \
	$(extra_guests_desktop_files) \
	$(extra_guests_sh_files)

EXTRA_DIST = \
	$(extra_guests_desktop_in_in_files) \
	$(extra_guests_sh_files)

CLEANFILES = \
	$(extra_guests_desktop_files)
