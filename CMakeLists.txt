cmake_minimum_required(VERSION 3.16)
project(TickWire)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

# --- Common sources (platform-independent) ---
# Do NOT list headers here. Headers are found via include_directories.
# Listing them as sources does nothing and creates noise in the build system.
set(COMMON_SOURCES
        src/address.cpp
        src/buffer_pool.cpp
        src/network_receiver.cpp
        src/worker.cpp
        src/socket.cpp
)

# --- Platform-specific socket implementation ---
if(WIN32)
    set(PLATFORM_SOURCES
            src/platform/socket_win32.cpp
    )
elseif(UNIX)
    set(PLATFORM_SOURCES
            src/platform/socket_posix.cpp
    )
endif()

# --- Main application ---
add_executable(tickwire_app
        src/main.cpp
        ${COMMON_SOURCES}
        ${PLATFORM_SOURCES}
)

# --- Benchmark ---
add_executable(tickwire_benchmark
        src/benchmark.cpp
        ${COMMON_SOURCES}
        ${PLATFORM_SOURCES}
)

# --- Windows socket library ---
# Both targets need ws2_32 because COMMON_SOURCES includes address.cpp
# which pulls in winsock2.h transitively. Linking only one target causes
# a linker failure the moment the other calls any Winsock function.
if(WIN32)
    target_link_libraries(tickwire_app       ws2_32)
    target_link_libraries(tickwire_benchmark ws2_32)
endif()