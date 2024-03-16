cmake_minimum_required(VERSION 3.16)

add_subdirectory("protothreads")
add_subdirectory("etl")

# cd ./build && make test
enable_testing()

if (DEBUG)
    SET(CMAKE_BUILD_TYPE "Debug")
    message(STATUS "Build with debug mode")
else()
    SET(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build with release mode as default")
endif()

set( LIB_NAME pt_os_lib )

file( GLOB SRCS 
  "*.cpp")

add_library(${LIB_NAME} STATIC ${SRCS})

include_directories(${PROJECT_SOURCE_DIR})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIB_NAME} protothreads)

set( OS_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE )

add_subdirectory( "./tests" )
