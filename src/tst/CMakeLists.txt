enable_testing()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(BINARY ${CMAKE_PROJECT_NAME}_test)
# set GTEST_ROOT
find_package(GTest REQUIRED)
find_file(ALLOCATOR_CPP NAMES Allocator.cpp PATHS ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CPP_SRC test_allocator.cpp test_double_chained_list.cpp test_mutual_timer.cpp test_rwlock.cpp test_observable.cpp)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(${BINARY} ${ALLOCATOR_CPP} ${CPP_SRC})


target_link_libraries(${BINARY} ${CMAKE_PROJECT_NAME} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
add_test(NAME  ${BINARY} COMMAND ${BINARY})

set_tests_properties( ${BINARY}
    PROPERTIES
    PASS_REGULAR_EXPRESSION "OK")

set_tests_properties( ${BINARY}
    PROPERTIES
    FAIL_REGULAR_EXPRESSION "FAILED")