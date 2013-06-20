##
##  GSettings schema
##

set (SCHEMA_NAME "com.canonical.indicator.session.gschema.xml")
set (SCHEMA_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SCHEMA_NAME}")
set (SCHEMA_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME}.in")

# generate the .xml file using intltool
set (ENV{LC_ALL} "C") 
execute_process (COMMAND intltool-merge -quiet --xml-style --utf8 --no-translations "${SCHEMA_FILE_IN}" "${SCHEMA_FILE}")

# let UseGSettings do the rest
add_schema (${SCHEMA_FILE})


##
##  DBus Service File
##

# where to install
if (${LOCAL_INSTALL})
  set (DBUS_SERVICE_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/services")
else ()
  EXEC_PROGRAM (${PKG_CONFIG_EXECUTABLE} ARGS dbus-1 --variable session_bus_services_dir OUTPUT_VARIABLE DBUS_SERVICE_DIR)
endif ()
message (STATUS "${DBUS_SERVICE_DIR} is the DBus Service File install dir")

set (SERVICE_NAME "${CMAKE_PROJECT_NAME}.service")
set (SERVICE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME}")
set (SERVICE_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_NAME}.in")

# build it
set (pkglibexecdir "${PROJECT_LIBEXECDIR}")
configure_file ("${SERVICE_FILE_IN}" "${SERVICE_FILE}")

# install it
install (FILES "${SERVICE_FILE}"
         DESTINATION "${DBUS_SERVICE_DIR}")


##
##  Unity Indicator File
##

# where to install
set (UNITY_INDICATOR_DIR "${CMAKE_INSTALL_PREFIX}/share/unity/indicators")
message (STATUS "${UNITY_INDICATOR_DIR} is the Unity Indicator install dir")

set (UNITY_INDICATOR_NAME "com.canonical.indicator.session")
set (UNITY_INDICATOR_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_INDICATOR_NAME}")

install (FILES "${UNITY_INDICATOR_FILE}"
         DESTINATION "${UNITY_INDICATOR_DIR}")


##
##  Icons
##

# where to install
if (${LOCAL_INSTALL})
  set (ICON_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/libindicator/icons")
else ()
  EXEC_PROGRAM (${PKG_CONFIG_EXECUTABLE} ARGS indicator3-0.4 --variable iconsdir OUTPUT_VARIABLE ICON_PREFIX)
endif ()
set (ICON_DIR "${ICON_PREFIX}/hicolor")
message (STATUS "${ICON_DIR} is the Icon install dir")

install (DIRECTORY icons
         DESTINATION "${ICON_DIR}"
         FILES_MATCHING PATTERN "*.png" PATTERN "*.svg")

