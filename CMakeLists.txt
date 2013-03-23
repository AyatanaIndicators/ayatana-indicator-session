project (indicator-session C CXX)
cmake_minimum_required (VERSION 2.8.9)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_VERSION "13.10.1")
set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${CMAKE_PROJECT_NAME})

option (enable_tests "Build the package's automatic tests." ON)
option (full_warnings "Full compiler warnings." ON)
option (enable_lcov "Generate lcov code coverage reports." ON)


find_package(PkgConfig REQUIRED)
include (FindPkgConfig)
include (GdbusCodegen)
include (GSettings)

add_definitions( -DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}" )
add_definitions (-DGNOMELOCALEDIR=\"@CMAKE_INSTALL_PREFIX@/share/locale\")

pkg_check_modules (SERVICE REQUIRED
                   glib-2.0>=2.35.4
                   gio-unix-2.0>=2.25.11)
include_directories (${SERVICE_INCLUDE_DIRS})

set (CC_WARNING_ARGS " -Wall -pedantic -Wextra -Wno-missing-field-initializers")



set (ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target (dist
                   COMMAND bzr export --root=${ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target (clean-coverage
                   COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' | xargs rm -f)

add_custom_target (cppcheck COMMAND cppcheck --enable=all -q --error-exitcode=2 --inline-suppr
                   ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/tests)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)

if (${enable_tests})
  set (GTEST_SOURCE_DIR /usr/src/gtest/src)
  set (GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/..)
  set (GTEST_LIBS -lpthread)
  enable_testing ()
  if (${enable_lcov})
    include(cmake/GCov.cmake)
  endif ()
  add_subdirectory(tests)
endif ()
