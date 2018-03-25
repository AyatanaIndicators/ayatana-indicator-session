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

if(URLDISPATCHER_FOUND)
    add_definitions( -DHAS_URLDISPATCHER )
endif()

set (SERVICE_EXEC "ayatana-indicator-session-service")
set_property (SOURCE main.c
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")
add_executable (${SERVICE_EXEC} main.c)
target_link_libraries (${SERVICE_EXEC} libayatanaindicatorsessionservice backenddbus ${SERVICE_LIBRARIES} ${GCOV_LIBS} ${URLDISPATCHER_LIBRARIES})
install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})

# common properties
set_property (TARGET libayatanaindicatorsessionservice ${SERVICE_EXEC}
              APPEND_STRING PROPERTY COMPILE_FLAGS
              " -g ${CC_WARNING_ARGS} ${GCOV_FLAGS}")
