project (ayatana-indicator-session C CXX)
cmake_minimum_required (VERSION 2.8.9)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_VERSION "0.8.2")
set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE "ayatana-indicator-session")

option (ENABLE_TESTS "Build the package's automatic tests." OFF)
option (ENABLE_COVERAGE "Generate lcov code coverage reports." OFF)

if(ENABLE_COVERAGE)
    set(ENABLE_TESTS ON)
    set(CMAKE_BUILD_TYPE "Coverage")
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

##
##  GNU standard installation directories
## 
include (GNUInstallDirs)
set (CMAKE_INSTALL_PKGLIBEXECDIR "${CMAKE_INSTALL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")
set (CMAKE_INSTALL_FULL_PKGLIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")


##
##  Check for prerequisites
##
find_package (PkgConfig REQUIRED)
include (FindPkgConfig)
pkg_check_modules (SERVICE REQUIRED
                   libayatana-common>=0.9.2
                   glib-2.0>=2.36
                   gio-unix-2.0>=2.36)
include_directories (${SERVICE_INCLUDE_DIRS})

set(URL_DISPATCHER_REQUIRED_VERSION 0)
pkg_check_modules(
  URLDISPATCHER
  lomiri-url-dispatcher>=${URL_DISPATCHER_REQUIRED_VERSION}
)
include_directories(${URLDISPATCHER_INCLUDE_DIRS})

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

# testing & coverage
if (${ENABLE_TESTS})

  if (EXISTS /usr/src/googletest/src)
    set (GTEST_SOURCE_DIR /usr/src/googletest/src)
  else ()
    set (GTEST_SOURCE_DIR /usr/src/gtest/src)
  endif ()
  set (GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/..)
  set (GTEST_LIBS -lpthread)
  enable_testing ()
  if (${ENABLE_COVERAGE})
    include(GCov)
  endif ()
endif ()

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)
if (${ENABLE_TESTS})
  add_subdirectory (tests)
endif ()
