cmake_minimum_required(VERSION 3.21)
project(tickwire LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tickwire STATIC
        src/dummy.cpp)

target_include_directories(tickwire
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_sources(tickwire
        PRIVATE
        src/dummy.cpp
)
