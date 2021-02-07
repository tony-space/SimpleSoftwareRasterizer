cmake_minimum_required(VERSION 3.19)

project(SimpleSoftwareRasterizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING true)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/x64)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/x64)

add_subdirectory(third-party/glm)

include(shared.cmake)

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	add_subdirectory(host-gdi)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	add_subdirectory(host-gtk)
endif()

add_subdirectory(librasterizer)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "host-gdi")

set(Assets 
	assets/bunny.obj
	assets/Jupiter.bmp
	assets/Moon.bmp
	assets/UV_Grid_Sm.bmp
)

file(COPY ${Assets} DESTINATION assets)