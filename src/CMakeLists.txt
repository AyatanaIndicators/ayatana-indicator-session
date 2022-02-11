add_subdirectory (backend-dbus)

add_library (libayatanaindicatorsessionservice STATIC
             actions.c
             actions.h
             guest.c
             guest.h
             recoverable-problem.c
             recoverable-problem.h
             service.c
             service.h
             users.c
             users.h
             utils.c
             utils.h)
include_directories(${SERVICE_INCLUDE_DIRS})
link_directories(${SERVICE_LIBRARY_DIRS})

set (SERVICE_EXEC "ayatana-indicator-session-service")
set_property (SOURCE main.c
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")
add_executable (${SERVICE_EXEC} main.c)
target_link_libraries (${SERVICE_EXEC} libayatanaindicatorsessionservice backenddbus ${SERVICE_LIBRARIES})
install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME})
