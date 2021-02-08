project(librasterizer)

set(SRC
	include/rasterizer/finally.hpp
	include/rasterizer/gamma_bgra_t.hpp
	include/rasterizer/linear_rgba_t.hpp
	include/rasterizer/Mesh.hpp
	include/rasterizer/obj-loader.hpp
	include/rasterizer/Rasterizer.hpp
	include/rasterizer/Texture.hpp

	detail/basic-matrices.cpp
	detail/basic-matrices.hpp
	detail/BoundingBox2D.hpp
	detail/clipping.cpp
	detail/clipping.hpp
	detail/gamma_bgra_t.cpp
	detail/glm-include.hpp
	detail/linear_rgba_t.cpp
	detail/LookUpTable.hpp
	detail/Mesh.cpp
	detail/MeshCube.cpp
	detail/MeshSphere.cpp
	detail/obj-loader.cpp
	detail/Rasterizer.cpp
	detail/Texture.cpp
	detail/Tile.cpp
	detail/Tile.hpp
	detail/Vertex.hpp

	pch.hpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_link_libraries(${PROJECT_NAME} PRIVATE shared)
target_link_libraries(${PROJECT_NAME} PUBLIC glm)

#OpenMP dependency for Parallel STL (GCC)
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	find_package(OpenMP)
	if(OpenMP_CXX_FOUND)
		target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
	endif()
	target_compile_definitions(${PROJECT_NAME} PUBLIC TILE_SIZE=4)
	target_compile_definitions(${PROJECT_NAME} PUBLIC TRY_PARALLELIZE_PAR_UNSEQ=std::execution::par,)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC TILE_SIZE=4)
	target_compile_definitions(${PROJECT_NAME} PUBLIC TRY_PARALLELIZE_PAR_UNSEQ=std::execution::par_unseq,)
endif()