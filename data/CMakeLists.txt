##
##  GSettings schema
##

set (SCHEMA_NAME "com.canonical.indicator.session.gschema.xml")
set (SCHEMA_PATH "${CMAKE_CURRENT_BINARY_DIR}/${SCHEMA_NAME}")
set (SCHEMA_PATH_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_NAME}.in")

# generate the .xml file using intltool
set (ENV{LC_ALL} "C") 
execute_process (COMMAND intltool-merge -x -u --no-translations ${SCHEMA_PATH_IN} ${SCHEMA_PATH})

# let UseGSettings do the rest
add_schema (${SCHEMA_PATH})


##
##  DBus Service File
##

set (SERVICE_NAME "${CMAKE_PROJECT_NAME}.service")
set (SERVICE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME}")
set (SERVICE_PATH_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_NAME}.in")

# build it
set (pkglibexecdir "${PROJECT_LIBEXECDIR}")
configure_file ("${SERVICE_PATH_IN}" "${SERVICE_PATH}")

# install it
install (FILES ${SERVICE_PATH} DESTINATION ${DBUSSERVICEDIR})


##
##  Unity Indicator File
##

set (UNITY_INDICATOR_NAME "com.canonical.indicator.session")
set (UNITY_INDICATOR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_INDICATOR_NAME}")
set (UNITY_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share/unity")

install (FILES ${UNITY_INDICATOR_PATH} DESTINATION "${UNITY_INSTALL_PREFIX}/indicators")


##
##  Icons
##

if(${LOCAL_INSTALL})
  set (ICON_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/libindicator/icons/")
else()
  EXEC_PROGRAM(${PKG_CONFIG_EXECUTABLE} ARGS indicator3-0.4 --variable iconsdir OUTPUT_VARIABLE ICON_PREFIX)
endif()
set (ICON_DIR "${ICON_PREFIX}/hicolor")
message ("Installing icons to ${ICON_DIR}")
install (DIRECTORY icons
         DESTINATION ${ICON_DIR}
         FILES_MATCHING PATTERN "*.png" PATTERN "*.svg")
