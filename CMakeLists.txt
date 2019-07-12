cmake_minimum_required(VERSION 3.0)
project(i1i2i3_phone CXX)

#set(CMAKE_CXX_COMPILER "/usr/bin/clang-7")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-8")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-g3 -O0 -Wall -Wextra")

include_directories("${CMAKE_CURRENT_LIST_DIR}/cereal/include")

file(GLOB client_src ${CMAKE_CURRENT_LIST_DIR}/client/*.cpp)
add_executable(client ${client_src})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_LIST_DIR}/common)
target_link_libraries(client PRIVATE pthread)

file(GLOB server_src ${CMAKE_CURRENT_LIST_DIR}/server/*.cpp)
file(GLOB server_src_impl ${CMAKE_CURRENT_LIST_DIR}/server/**/*.cpp)
add_executable(server ${server_src} ${server_src_impl})
target_include_directories(server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/common)
target_link_libraries(server PRIVATE pthread)

# test
file(GLOB test_srcs ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
foreach (test_src ${test_srcs})
    get_filename_component(target ${test_src} NAME_WE)
    add_executable(${target} ${test_src})
    target_link_libraries(${target} PRIVATE boost_system pthread)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/common)
endforeach ()


