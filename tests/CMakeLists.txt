set( TEST test_pt_os)

project(${TEST} CXX)

file( GLOB TEST_SRC "*.cpp")
add_executable( ${TEST} ${TEST_SRC})

target_link_libraries(${TEST}
    pt_os_lib
    protothreads
)

include_directories(${PROJECT_SOURCE_DIR} ${OS_LIB_DIR})

target_include_directories(${TEST} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${OS_LIB_DIR})

add_test( NAME ${TEST} COMMAND ./${TEST})
