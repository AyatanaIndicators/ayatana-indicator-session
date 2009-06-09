
AC_INIT(src/indicator-sus.c)

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-sus, 0.1dev)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

###########################
# Dependencies 
###########################

GTK_REQUIRED_VERSION=2.12
INDICATOR_REQUIRED_VERSION=0.2.0
DBUSMENUGTK_REQUIRED_VERSION=0.0.0

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

###########################
# Indicator Info
###########################

INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator`
AC_SUBST(INDICATORDIR)

###########################
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])

###########################
# Results
###########################

AC_MSG_NOTICE([

SUS Indicator Configuration:

	Prefix:        $prefix
])
