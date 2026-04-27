cmake_minimum_required(VERSION 4.2)
project(protolib VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(protolib
        include/protolib/client/UDP.cpp
        include/protolib/server/UDP.cpp
)

target_include_directories(protolib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (WIN32)
    target_link_libraries(protolib PUBLIC ws2_32)
endif ()

add_executable(udp_server test/udp_server.cpp)
target_link_libraries(udp_server PRIVATE protolib)

add_executable(udp_client test/udp_client.cpp)
target_link_libraries(udp_client PRIVATE protolib)
