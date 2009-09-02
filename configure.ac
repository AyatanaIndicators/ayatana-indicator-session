
AC_INIT(src/indicator-session.c)

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-session, 0.1)

AM_MAINTAINER_MODE

IT_PROG_INTLTOOL([0.35.0])

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

###########################
# Dependencies 
###########################

GTK_REQUIRED_VERSION=2.12
INDICATOR_REQUIRED_VERSION=0.2.0
DBUSMENUGTK_REQUIRED_VERSION=0.1.0

PKG_CHECK_MODULES(APPLET, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                          indicator >= $INDICATOR_REQUIRED_VERSION
						  dbusmenu-gtk >= $DBUSMENUGTK_REQUIRED_VERSION)
AC_SUBST(APPLET_CFLAGS)
AC_SUBST(APPLET_LIBS)

DBUSMENUGLIB_REQUIRED_VERSION=0.0.0

PKG_CHECK_MODULES(STATUSSERVICE, dbusmenu-glib >= $DBUSMENUGLIB_REQUIRED_VERSION)

AC_SUBST(STATUSERVICE_CFLAGS)
AC_SUBST(STATUSERVICE_LIBS)

PKG_CHECK_MODULES(USERSSERVICE, dbusmenu-glib >= $DBUSMENUGLIB_REQUIRED_VERSION)

AC_SUBST(USERSERVICE_CFLAGS)
AC_SUBST(USERSERVICE_LIBS)

PKG_CHECK_MODULES(SESSIONSERVICE, dbusmenu-glib >= $DBUSMENUGLIB_REQUIRED_VERSION)

AC_SUBST(SESSIONERVICE_CFLAGS)
AC_SUBST(SESSIONERVICE_LIBS)

PKG_CHECK_MODULES(GTKLOGOUTHELPER, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                   polkit-gnome)

AC_SUBST(GTKLOGOUTHELPER_CFLAGS)
AC_SUBST(GTKLOGOUTHELPER_LIBS)

AC_PATH_PROG(GCONFTOOL, gconftool-2)
dnl Specify the gconf configuration source,
dnl default to xml::$(sysconfdir)/gconf/gconf.xml.defaults

AM_GCONF_SOURCE_2

PKG_CHECK_MODULES(GCONF, gconf-2.0 >= 2.0)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

###########################
# Check to see if we're local
###########################

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

###########################
# Indicator Info
###########################

if test "x$with_localinstall" = "xyes"; then
	INDICATORDIR="${libdir}/indicators/2/"
	INDICATORICONSDIR="${datadir}/indicator-applet/icons/"
else
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator`
fi
AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

###########################
# DBus Service Info
###########################

if test "x$with_localinstall" = "xyes"; then
	DBUSSERVICEDIR="${datadir}/dbus-1/services/"
else
	DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
fi
AC_SUBST(DBUSSERVICEDIR)

###########################
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
src/gtk-dialog/Makefile
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/16x16/status/Makefile
data/icons/16x16/actions/Makefile
data/icons/22x22/Makefile
data/icons/22x22/status/Makefile
data/icons/22x22/actions/Makefile
data/icons/24x24/Makefile
data/icons/24x24/status/Makefile
data/icons/24x24/actions/Makefile
data/icons/32x32/Makefile
data/icons/32x32/status/Makefile
data/icons/32x32/actions/Makefile
data/icons/48x48/Makefile
data/icons/48x48/status/Makefile
data/icons/scalable/Makefile
data/icons/scalable/status/Makefile
data/icons/scalable/actions/Makefile
po/Makefile.in
])

###########################
# Results
###########################

AC_MSG_NOTICE([

SUS Indicator Configuration:

	Prefix:        $prefix
])
