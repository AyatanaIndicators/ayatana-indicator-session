
AC_INIT(src/indicator-session.c)

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-session, 0.3.96)

AM_MAINTAINER_MODE

GLIB_GSETTINGS

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
GTK3_REQUIRED_VERSION=3.0
INDICATOR_REQUIRED_VERSION=0.3.19
DBUSMENUGTK_REQUIRED_VERSION=0.5.90
POLKIT_REQUIRED_VERSION=0.92

AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk],
    [Which version of gtk to use for the indicator @<:@default=3@:>@])],
  [],
  [with_gtk=3])

AS_IF([test "x$with_gtk" = x3],
        [PKG_CHECK_MODULES(APPLET, gtk+-3.0 >= $GTK3_REQUIRED_VERSION
                                   indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                   dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION)
        ],
      [test "x$with_gtk" = x2],
         [PKG_CHECK_MODULES(APPLET, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                    indicator-0.4 >= $INDICATOR_REQUIRED_VERSION
                                    dbusmenu-gtk-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION)
        ],
      [AC_MSG_FAILURE([Value for --with-indicator-gtk was neither 2 nor 3])]
)

AC_SUBST(APPLET_CFLAGS)
AC_SUBST(APPLET_LIBS)

DBUSMENUGLIB_REQUIRED_VERSION=0.1.1

AS_IF([test "x$with_gtk" = x3],
    [PKG_CHECK_MODULES(SESSIONSERVICE, dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                       dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION
                                       dbus-glib-1
                                       gio-unix-2.0
                                       indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                       packagekit-glib2)
    ],
    [test "x$with_gtk" = x2],
    [PKG_CHECK_MODULES(SESSIONSERVICE, dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                       dbusmenu-gtk-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION
                                       dbus-glib-1
                                       gio-unix-2.0
                                       indicator-0.4 >= $INDICATOR_REQUIRED_VERSION
                                       packagekit-glib2)
    ]
)

AC_SUBST(SESSIONERVICE_CFLAGS)
AC_SUBST(SESSIONERVICE_LIBS)

PKG_CHECK_MODULES(GUDEV, gudev-1.0, has_gudev=true, has_gudev=false)
AC_SUBST(GUDEV_CFLAGS)
AC_SUBST(GUDEV_LIBS)

AM_CONDITIONAL([USE_GTK3], [test "x$with_gtk" = "x3"])
AS_IF([test "x$with_gtk" = x3], [
       AC_DEFINE(HAVE_GTK3, 1, [whether gtk3 is available])
     ])

###########################
# GTK Logout Helper
###########################

AC_ARG_ENABLE([gtklogouthelper],
            AS_HELP_STRING([--enable-gtklogouthelper], [enable GTK Logout Helper]),,
            enable_gtklogouthelper=auto)

if test x"$enable_gtklogouthelper" != x"no" ; then
   AS_IF([test "x$with_gtk" = x3],
       [PKG_CHECK_MODULES(GTKLOGOUTHELPER, gtk+-3.0 >= $GTK3_REQUIRED_VERSION
                                           polkit-gobject-1 >= $POLKIT_REQUIRED_VERSION,
	                                   [have_gtklogouthelper=yes],
	                                   [have_gtklogouthelper=no])
       ],
       [test "x$with_gtk" = x2],
       [PKG_CHECK_MODULES(GTKLOGOUTHELPER, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                           polkit-gobject-1 >= $POLKIT_REQUIRED_VERSION,
	                                   [have_gtklogouthelper=yes],
	                                   [have_gtklogouthelper=no])
       ]
   )
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
# APT support
###########################
AC_ARG_ENABLE([apt],
	AC_HELP_STRING([--disable-apt], [disable APT support]),,
	[enable_apt=yes])
AM_CONDITIONAL([BUILD_APT], [test "x$enable_apt" != "xno"])

if test "x$enable_apt" != "xno"; then
   AC_DEFINE(HAVE_APT, 1, [Define to 1 to enable APT support])
fi

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

AM_CONDITIONAL([HAS_GUDEV], [test "x$has_gudev" = "xtrue"])

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
	Indicator GTK:      $with_gtk
	Logout Helper:      $have_gtklogouthelper
	APT support:        $enable_apt
	Coverage reporting: $use_gcov
])
