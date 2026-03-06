cmake_minimum_required(VERSION 3.21)
project(tickwire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tickwire STATIC
        src/dummy.cpp
        include/tickwire/socket.hpp
        src/platform/socket_posix.cpp
        src/platform/socket_win32.cpp
        src/socket.cpp
        include/tickwire/address.h
        src/address.cpp
        include/tickwire/packet.h
        include/tickwire/buffer_pool.h
        include/tickwire/packet_buffer.h
        src/buffer_pool.cpp
        include/tickwire/spsc_queue.h)

target_include_directories(tickwire
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_sources(tickwire
        PRIVATE
        src/dummy.cpp
)

target_sources(tickwire PRIVATE src/address.cpp)
if (WIN32 OR MINGW)
    message(STATUS "Building TickWire with Windows sockets backend")
    target_sources(tickwire PRIVATE src/platform/socket_win32.cpp)
else()
    message(STATUS "Building TickWire with POSIX sockets backend")
    target_sources(tickwire PRIVATE src/platform/socket_posix.cpp)
endif()

target_sources(tickwire PRIVATE src/socket.cpp)
if (WIN32 OR MINGW)
    target_link_libraries(tickwire PRIVATE ws2_32)
endif()

target_sources(tickwire PRIVATE src/buffer_pool.cpp)

