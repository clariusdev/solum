cmake_minimum_required(VERSION 3.25)

project(solum)

set(OpenIGTLink_BUILD_TESTING OFF)

add_subdirectory(solum)
add_subdirectory(../../openigtlink/repo openigtlink)
