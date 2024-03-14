cmake_minimum_required(VERSION 3.26)

add_subdirectory("protothreads")

# cd ./build && make test
enable_testing()

set( LIB_NAME pt_os_lib )

file( GLOB SRCS 
  "*.cpp")

add_library(${LIB_NAME} STATIC ${SRCS})

include_directories(${PROJECT_SOURCE_DIR})

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${LIB_NAME} protothreads)

set( OS_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE )

add_subdirectory( "./tests" )
