add_subdirectory (backend-dbus)

add_library (libindicatorsessionservice STATIC
             actions.c
             actions.h
             guest.c
             guest.h
             service.c
             service.h
             users.c
             users.h)
set_target_properties (libindicatorsessionservice PROPERTIES COMPILE_FLAGS " -g ${CC_WARNING_ARGS} ${GCOV_FLAGS}")

include_directories(${SERVICE_INCLUDE_DIRS})
link_directories(${SERVICE_LIBRARY_DIRS})

set (SERVICE_EXEC "indicator-session-service")
add_executable (${SERVICE_EXEC} main.c)
target_link_libraries (${SERVICE_EXEC} libindicatorsessionservice backenddbus ${SERVICE_LIBRARIES} ${GCOV_LIBS})
install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${PROJECT_LIBEXECDIR})
