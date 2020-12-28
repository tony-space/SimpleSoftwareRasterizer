project(shared)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_options(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
target_compile_options(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/W3>)
target_compile_options(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Qpar>)
target_compile_options(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>)