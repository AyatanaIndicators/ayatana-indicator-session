# build desktopmock
add_library (desktopmock STATIC
             mock-accounts.cc
             mock-accounts.h
             mock-login1-manager.cc
             mock-login1-manager.h
             mock-login1-seat.cc
             mock-login1-seat.h
             mock-display-manager-seat.cc
             mock-display-manager-seat.h
             mock-end-session-dialog.cc
             mock-end-session-dialog.h
             mock-object.cc
             mock-object.h
             mock-screen-saver.cc
             mock-screen-saver.h
             mock-unity-session.cc
             mock-unity-session.h
             mock-session-manager.cc
             mock-session-manager.h
             mock-user.cc
             mock-user.h
             mock-webcredentials.cc
             mock-webcredentials.h)

add_dependencies(desktopmock backenddbus)

include_directories (${SERVICE_INCLUDE_DIRS})
include_directories (${CMAKE_SOURCE_DIR}/src)
include_directories (${CMAKE_BINARY_DIR}/src)
include_directories (${CMAKE_SOURCE_DIR}/tests)

# test the Actions class
add_executable (test-actions
                test-actions.cc)
target_link_options(test-actions PRIVATE -no-pie)
add_test (test-actions test-actions)
target_link_libraries (test-actions desktopmock backenddbus libayatanaindicatorsessionservice ${SERVICE_LDFLAGS} ${GTEST_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARIES})

# test the Guest class
add_executable (test-guest
                test-guest.cc)
target_link_options(test-guest PRIVATE -no-pie)
add_test (test-guest test-guest)
target_link_libraries (test-guest desktopmock backenddbus libayatanaindicatorsessionservice ${SERVICE_LDFLAGS} ${GTEST_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARIES})

# test the Users class
add_executable (test-users
                test-users.cc)
target_link_options(test-users PRIVATE -no-pie)
add_test (test-users test-users)
target_link_libraries (test-users desktopmock backenddbus libayatanaindicatorsessionservice ${SERVICE_LDFLAGS} ${GTEST_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARIES})
