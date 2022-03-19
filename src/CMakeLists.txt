configure_file(Config.h.in Config.h)
set(BINARY ${CMAKE_PROJECT_NAME})
# set BOOSTROOT
find_package(Boost REQUIRED COMPONENTS system)

include_directories(src ${Boost_INCLUDE_DIRS})

add_subdirectory(tst)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

add_library(${BINARY} Allocator.cpp rwlock.cpp timer_wrapper.cc bin_tree.hpp)
