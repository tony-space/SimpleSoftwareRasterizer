project(librasterizer)

set(SRC
	include/rasterizer/gamma_bgra_t.hpp
	include/rasterizer/linear_rgba_t.hpp
	include/rasterizer/Rasterizer.hpp

	detail/Rasterizer.cpp

	pch.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_include_directories(${PROJECT_NAME} INTERFACE "include")
target_link_libraries(${PROJECT_NAME} PRIVATE shared)
target_link_libraries(${PROJECT_NAME} PUBLIC glm)