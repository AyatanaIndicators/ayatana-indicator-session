# schema
configure_file (com.canonical.indicator.session.gschema.xml.in
                com.canonical.indicator.session.gschema.xml)
include (GSettings)
add_schemas(clinica-schemas ${CMAKE_SOURCE_DIR}/data/com.canonical.indicator.session.gschema.xml)

# .service file
set(SERVICE_NAME "indicator-session.service")
set(SERVICE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME}")
set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/indicator-session")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_NAME}.in" "${SERVICE_PATH}")
install (FILES ${SERVICE_PATH}
         DESTINATION ${DBUSSERVICEDIR})

# .indicator file
install(FILES com.canonical.indicator.session
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/unity/indicators)

# FIXME install files
#
#install(FILES
#datetime-dialog.ui
#DESTINATION share/indicatore-datetime # FIXME this too.
#)
