# schema
configure_file (com.canonical.indicator.session.gschema.xml.in
                com.canonical.indicator.session.gschema.xml)
include (GSettings)
add_schemas(clinica-schemas ${CMAKE_SOURCE_DIR}/data/com.canonical.indicator.session.gschema.xml)

# .service file
include (GNUInstallDirs)
configure_file (indicator-session.service.in indicator-session.service)

# .indicator file
install(FILES com.canonical.indicator.session
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/unity/indicators)

# FIXME install files
#install(FILES
#${CMAKE_CURRENT_BINARY_DIR}/indicator-datetime.service
#DESTINATION share/indicator-datetime # FIXME to be the real install dir.
#)
#
#install(FILES
#datetime-dialog.ui
#DESTINATION share/indicatore-datetime # FIXME this too.
#)
