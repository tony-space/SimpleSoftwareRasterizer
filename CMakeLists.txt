cmake_minimum_required(VERSION 3.19)

project(SimpleSoftwareRasterizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(shared.cmake)
add_subdirectory(host-gdi)
add_subdirectory(librasterizer)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "host-gdi")