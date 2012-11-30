AC_INIT([indicator-session],[12.10.4])
AC_CONFIG_SRCDIR([src/session-menu-mgr.c])
AM_INIT_AUTOMAKE([check-news])
AC_CONFIG_HEADERS([config.h])

AM_MAINTAINER_MODE

GLIB_GSETTINGS

IT_PROG_INTLTOOL([0.35.0])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_HEADER_STDC
LT_INIT

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

###########################
# Dependencies 
###########################

GIO_REQUIRED_VERSION=2.33
GLIB_REQUIRED_VERSION=2.33
GTK_REQUIRED_VERSION=3.0
INDICATOR_REQUIRED_VERSION=0.3.19
DBUSMENUGTK_REQUIRED_VERSION=0.5.90
DBUSTEST_REQUIRED_VERSION=0.0.5
DBUSMENUGLIB_REQUIRED_VERSION=0.1.1


PKG_CHECK_MODULES(APPLET, glib-2.0 >= $GLIB_REQUIRED_VERSION
                          gtk+-3.0 >= $GTK_REQUIRED_VERSION
                          indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                          dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION)
AC_SUBST(APPLET_CFLAGS)
AC_SUBST(APPLET_LIBS)


PKG_CHECK_MODULES(SESSIONSERVICE, glib-2.0 >= $GLIB_REQUIRED_VERSION
                                  dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                  dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION
                                  gio-unix-2.0
                                  indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION)
AC_SUBST(SESSIONERVICE_CFLAGS)
AC_SUBST(SESSIONERVICE_LIBS)

AC_SUBST(GUDEV_CFLAGS)
AC_SUBST(GUDEV_LIBS)

###########################
# GTK Logout Helper
###########################

AC_ARG_ENABLE([gtklogouthelper],
              [AS_HELP_STRING([--enable-gtklogouthelper], [enable GTK Logout Helper])],
              [],
              [enable_gtklogouthelper=auto])

if test x"$enable_gtklogouthelper" != x"no" ; then
   PKG_CHECK_MODULES([GTKLOGOUTHELPER],
                     [gtk+-3.0 >= $GTK_REQUIRED_VERSION],
                     [have_gtklogouthelper=yes],
                     [have_gtklogouthelper=no])
   if test x${have_gtklogouthelper} = xyes; then
      AC_DEFINE(HAVE_GTKLOGOUTHELPER, 1, [Define to 1 to enable GTK Logout Helper])
   fi
   if test x${enable_gtklogouthelper} = xyes && test x${have_gtklogouthelper} = xno; then
     AC_MSG_ERROR([GTK Logout Helper configured but polkit-gobject not found])
   fi
else
   have_gtklogouthelper=no
fi
AM_CONDITIONAL(BUILD_GTKLOGOUTHELPER, test x${have_gtklogouthelper} = xyes)

AC_SUBST(GTKLOGOUTHELPER_CFLAGS)
AC_SUBST(GTKLOGOUTHELPER_LIBS)

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
elif test "x$with_gtk" = x2; then
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator-0.4`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator-0.4`
else
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`
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
# Google Test framework
###########################

AC_ARG_ENABLE([tests],
    [AS_HELP_STRING([--disable-tests], [Disable test scripts and tools (default=auto)])],
    [enable_tests=${enableval}],
    [enable_tests=auto])
if test "x$enable_tests" != "xno"; then
    m4_include([m4/gtest.m4])
    CHECK_GTEST
    CHECK_XORG_GTEST
    if test "x$enable_tests" = "xauto"; then
        enable_tests=${have_gtest}
    elif test "x$enable_tests" = "xyes" && test "x$have_gtest" != "xyes"; then
        AC_MSG_ERROR([tests were requested but gtest is not installed.])
    fi
    if test "x$enable_tests" = "xyes"; then
        PKG_CHECK_MODULES([TEST_SERVICE],[glib-2.0 >= $GLIB_REQUIRED_VERSION
                                          gio-2.0 >= $GIO_REQUIRED_VERSION],
                                         [enable_tests="yes"],
                                         [enable_tests="no"])
    fi
fi
AM_CONDITIONAL([BUILD_TESTS],[test "x$enable_tests" = "xyes"])
AC_SUBST([TEST_SERVICE_CFLAGS])
AC_SUBST([TEST_SERVICE_LIBS])

##############################
# Custom Junk
##############################

AC_DEFUN([AC_DEFINE_PATH], [
	test "x$prefix" = xNONE && prefix="$ac_default_prefix"
	test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
	ac_define_path=`eval echo [$]$2`
	ac_define_path=`eval echo [$]ac_define_path`
	$1="$ac_define_path"
	AC_SUBST($1)
	ifelse($3, ,
		AC_DEFINE_UNQUOTED($1, "$ac_define_path"),
		AC_DEFINE_UNQUOTED($1, "$ac_define_path", $3))
])

###########################
# Internationalization
###########################

GETTEXT_PACKAGE=indicator-session
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the default get text domain])
AC_DEFINE_PATH(GNOMELOCALEDIR, "${datadir}/locale", [locale directory])

AM_GLIB_GNU_GETTEXT

###########################
# gcov coverage reporting
###########################

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AM_CONDITIONAL([HAVE_GCOV], [test "x$ac_cv_check_gcov" = xyes])
AM_CONDITIONAL([HAVE_LCOV], [test "x$ac_cv_check_lcov" = xyes])
AM_CONDITIONAL([HAVE_GCOVR], [test "x$ac_cv_check_gcovr" = xyes])
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

###########################
# Files
###########################

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/16x16/actions/Makefile
data/icons/16x16/status/Makefile
data/icons/22x22/Makefile
data/icons/22x22/actions/Makefile
data/icons/22x22/status/Makefile
data/icons/24x24/Makefile
data/icons/24x24/actions/Makefile
data/icons/24x24/status/Makefile
data/icons/32x32/Makefile
data/icons/32x32/actions/Makefile
data/icons/32x32/status/Makefile
data/icons/scalable/Makefile
data/icons/scalable/actions/Makefile
data/icons/scalable/status/Makefile
data/extra-sessions/Makefile
tests/Makefile
tests/indicator-session.service
po/Makefile.in
])

AC_OUTPUT

###########################
# Results
###########################

AC_MSG_NOTICE([

SUS Indicator Configuration:

	Prefix:             $prefix
	Indicator Dir:      $INDICATORDIR
	Logout Helper:      $have_gtklogouthelper
	Unit Tests:         $enable_tests
	Coverage reporting: $use_gcov
])
