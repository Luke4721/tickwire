cmake_minimum_required(VERSION 3.16)
project(TickWire)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

# Common sources
set(COMMON_SOURCES
        src/address.cpp
        src/buffer_pool.cpp
        src/network_receiver.cpp
        src/worker.cpp
)

# Platform-specific
if (WIN32)
    set(PLATFORM_SOURCES
            src/platform/socket_win32.cpp
    )
elseif(UNIX)
    set(PLATFORM_SOURCES
            src/platform/socket_posix.cpp
    )
endif()

# Main app
add_executable(tickwire_app
        src/main.cpp
        ${COMMON_SOURCES}
        ${PLATFORM_SOURCES}
)

# Benchmark
add_executable(tickwire_benchmark
        src/benchmark.cpp
        ${COMMON_SOURCES}
)

# 🔥 Required for Windows sockets
target_link_libraries(tickwire_app ws2_32)