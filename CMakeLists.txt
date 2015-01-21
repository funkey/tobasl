cmake_minimum_required(VERSION 2.6)
include(${CMAKE_SOURCE_DIR}/cmake/include/define_module.cmake)

#######################
# project information #
#######################

project(multi2cut)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

#####################
# compiler settings #
#####################

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fomit-frame-pointer -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fPIC")
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release or Debug" FORCE)
endif()

#######################
# project directories #
#######################

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(modules)
add_subdirectory(mergetree)
add_subdirectory(inference)
add_subdirectory(slices)
add_subdirectory(features)
add_subdirectory(io)
add_subdirectory(binaries)

###############
# config file #
###############

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
)
