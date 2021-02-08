project(host-gtk)

set(SRC
	Application.cpp
	Application.hpp
	main.cpp
	pch.hpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_executable(${PROJECT_NAME} ${SRC})

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE shared librasterizer)

#GTK dep
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK30 REQUIRED gtk+-3.0) #sudo apt-get install libgtk-3-dev
target_include_directories(${PROJECT_NAME} PRIVATE ${GTK30_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GTK30_LINK_LIBRARIES})