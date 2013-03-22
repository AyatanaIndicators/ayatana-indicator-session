project (indicator-session C CXX)
set (PROJECT_VERSION "13.10.1")
cmake_minimum_required (VERSION 2.8.9)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option (full_warnings "Full compiler warnings." ON)
option (enable_tests "Build the package's automatic tests." ON)
option (enable_lcov "Generate lcov code coverage reports." ON)

set (ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
set (CC_WARNING_ARGS " -Wall -pedantic -Wextra -Wno-missing-field-initializers")

#
# Check for glib/gio
#

include (FindPkgConfig)
pkg_check_modules (SERVICE REQUIRED
                   glib-2.0>=2.35.4
                   gio-unix-2.0>=2.25.11)
include_directories (${SERVICE_INCLUDE_DIRS})

#
# Testing / Coverage
#

if (${enable_tests})
  set (GTEST_SOURCE_DIR /usr/src/gtest/src)
  set (GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/..)
  set (GTEST_LIBS -lpthread)
  enable_testing ()
  if (${enable_lcov})
    include(cmake/GCov.cmake)
  endif ()
endif ()

#
# Custom Targets 
#

add_custom_target (dist
                   COMMAND bzr export --root=${ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target (clean-coverage
                   COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' | xargs rm -f)

add_custom_target (cppcheck COMMAND cppcheck --enable=all -q --error-exitcode=2 --inline-suppr
                   ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/test)

#
# 
#


add_definitions (-DGETTEXT_PACKAGE=\"indicator-session\")
add_definitions (-DPACKAGE_NAME=\"indicator-session\")
add_definitions (-DGNOMELOCALEDIR=\"@CMAKE_INSTALL_PREFIX@/share/locale\")

#
#  Directories
#

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)
add_subdirectory(data)
if (${enable_tests})
  add_subdirectory(tests)
endif ()
