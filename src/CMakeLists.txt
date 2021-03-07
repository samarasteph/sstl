configure_file(Config.h.in Config.h)
set(BINARY ${CMAKE_PROJECT_NAME})

add_subdirectory(tst)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

#target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${BINARY} Allocator.cpp rwlock.cpp timer_wrapper.cc)

