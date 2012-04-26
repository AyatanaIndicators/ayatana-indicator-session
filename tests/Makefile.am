AM_CPPFLAGS = $(GTEST_CPPFLAGS) $(SESSIONSERVICE_CFLAGS) $(LIBDBUSTEST_CFLAGS) -I${top_srcdir}/src -Wall -Werror
AM_CXXFLAGS = $(GTEST_CXXFLAGS)

check_PROGRAMS = test-service
test_service_SOURCES = test-service.cc
test_service_LDADD = $(SESSIONSERVICE_LIBS) $(LIBDBUSTEST_LIBS) libgtest.a

check_LIBRARIES = libgtest.a
nodist_libgtest_a_SOURCES = \
	$(GTEST_SOURCE)/src/gtest-all.cc \
	$(GTEST_SOURCE)/src/gtest_main.cc

check_SCRIPTS = test-service-runner.sh
test-service-runner.sh: Makefile.am
	@echo "#!/bin/sh" > $@
	@echo $(top_builddir)/tests/test-service $(top_builddir)/src/indicator-session-service$(EXEEXT) 2\>/dev/null >> $@
	@chmod +x $@

TESTS = ${check_SCRIPTS}
