cmake_minimum_required(VERSION 3.14)

project(ultim8 VERSION 0.0.1)

option(ULTIM8_BUILD_TESTS "build tests" OFF)

add_subdirectory("thirdparty" EXCLUDE_FROM_ALL)
add_subdirectory("src")

if(${ULTIM8_BUILD_TESTS})
  enable_testing()
  add_subdirectory("test" EXCLUDE_FROM_ALL)
endif()

add_subdirectory("etc")