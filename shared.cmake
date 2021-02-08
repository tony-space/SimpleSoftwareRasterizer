add_library(shared INTERFACE)

target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Qpar>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>)

#Enable PSTL for GCC
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:GNU>:-D_GLIBCXX_PARALLEL>)
target_compile_options(shared INTERFACE $<$<CXX_COMPILER_ID:GNU>:-march=native>) #optional