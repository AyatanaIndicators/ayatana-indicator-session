project (ayatana-indicator-session C CXX)
cmake_minimum_required (VERSION 3.13)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_VERSION "0.8.2")
set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE "ayatana-indicator-session")

option(ENABLE_TESTS "Enable all tests and checks" OFF)
option(ENABLE_COVERAGE "Enable coverage reports (includes enabling all tests and checks)" OFF)

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

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Weverything")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-c++98-compat -Wno-padded")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-documentation")

else()

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall")

endif()

set (ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target (dist
                   COMMAND bzr export --root=${ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target (cppcheck COMMAND cppcheck --enable=all -q --error-exitcode=2 --inline-suppr
                   ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/tests)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)

if (ENABLE_TESTS)

    include(CTest)
    enable_testing()
    add_subdirectory(tests)

    if (ENABLE_COVERAGE)

        find_package(CoverageReport)
        ENABLE_COVERAGE_REPORT(TARGETS "libayatanaindicatorsessionservice" "ayatana-indicator-session-service" TESTS "test-service" "test-actions" "test-guest" "test-users" FILTER /usr/include ${CMAKE_BINARY_DIR}/*)

    endif()

endif()

# Display config info

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Unit tests: ${ENABLE_TESTS}")
