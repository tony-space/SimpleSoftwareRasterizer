add_library(shared INTERFACE)

target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Qpar>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>)