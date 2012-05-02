
AM_CPPFLAGS = \
	$(GTEST_CPPFLAGS) \
	$(XORG_GTEST_CPPFLAGS) \
	$(INDICATOR_CFLAGS) \
	-I${top_srcdir}/src \
	-Wall -Werror
AM_CXXFLAGS = $(GTEST_CXXFLAGS)

TESTS = test-service
check_PROGRAMS = test-service
test_service_SOURCES = test-service.cc
test_service_LDADD = $(TEST_SERVICE_LIBS) libgtest.a $(XORG_GTEST_MAIN_LIBS) $(X11_LIBS)
test_service_CPPFLAGS = \
	$(TEST_SERVICE_CFLAGS) \
	$(AM_CPPFLAGS) \
	-DINDICATOR_SERVICE_PATH="\"$(top_builddir)/src/indicator-session-service\""

check_LIBRARIES = libgtest.a
nodist_libgtest_a_SOURCES = \
	$(XORG_GTEST_SOURCE)/src/xorg-gtest-all.cpp \
	$(GTEST_SOURCE)/src/gtest-all.cc \
	$(XORG_GTEST_SOURCE)/src/xorg-gtest_main.cpp

