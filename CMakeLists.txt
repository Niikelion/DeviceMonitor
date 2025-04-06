cmake_minimum_required(VERSION 3.12)
project(DeviceMonitor VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_TESTS "Build tests" true)
option(BUILD_EXAMPLE "Build example project" true)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(lib)

if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif()