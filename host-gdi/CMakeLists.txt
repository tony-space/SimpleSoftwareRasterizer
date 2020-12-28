project(host-gdi)

set(SRC
	main.cpp
	pch.hpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_executable(${PROJECT_NAME} WIN32 ${SRC})

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE shared librasterizer)