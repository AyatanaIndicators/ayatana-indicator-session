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

add_executable (indicator-session-service main.c)
target_link_libraries (indicator-session-service libindicatorsessionservice backenddbus ${SERVICE_LIBRARIES} ${GCOV_LIBS})

