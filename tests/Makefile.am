TESTS =
CLEANFILES =
BUILT_SOURCES =
check_PROGRAMS =

###
###
###

# stock UMB tests on user-visible strings
include $(srcdir)/Makefile.am.strings

check_LIBRARIES = libgtest.a
nodist_libgtest_a_SOURCES = \
  $(GTEST_SOURCE)/src/gtest-all.cc \
  $(GTEST_SOURCE)/src/gtest_main.cc

AM_CPPFLAGS = $(GTEST_CPPFLAGS) -I${top_srcdir}/src -Wall -Werror
AM_CXXFLAGS = $(GTEST_CXXFLAGS)

BUILT_SOURCES += gschemas.compiled
CLEANFILES += gschemas.compiled
gschemas.compiled: Makefile
        @glib-compile-schemas --targetdir=$(abs_builddir) $(top_builddir)/data

###
###
###

TESTS += test-service
check_PROGRAMS += test-service
test_service_SOURCES = test-service.cc
test_service_LDADD = \
	$(TEST_SERVICE_LIBS) \
	libgtest.a
test_service_CPPFLAGS = \
	-DINDICATOR_SERVICE_DIR="\"$(abs_builddir)\"" \
	-DINDICATOR_SERVICE_PATH="\"$(top_builddir)/src/indicator-session-service\"" \
	$(TEST_SERVICE_CFLAGS) \
	$(AM_CPPFLAGS)


