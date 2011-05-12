
extra_guests_desktop_in_files = \
	classic-desktop.desktop.in
extra_guests_sh_files = \
	classic-desktop.sh

extra_guestsdir = $(datadir)/indicators/session/applications/
extra_guests_desktop_files = \
	$(extra_guests_desktop_in_files:.desktop.in=.desktop)

%.desktop: %.desktop.in
	sed -e "s|\@guestdir\@|$(extra_guestsdir)|" $< > $@

extra_guests_DATA = \
	$(extra_guests_desktop_files) \
	$(extra_guests_sh_files)

EXTRA_DIST = \
	$(extra_guests_desktop_in_files) \
	$(extra_guests_sh_files)

CLEANFILES = \
	$(extra_guests_desktop_files)
