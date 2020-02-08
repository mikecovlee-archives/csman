cmake_minimum_required(VERSION 3.2)
project(pb-cpp)

include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

enable_testing()

#### Check C++14
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
check_cxx_compiler_flag("/std:c++14" COMPILER_SUPPORTS_CXX14)
if (COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
else ()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif ()

#### Check C99
check_c_compiler_flag("-std=c99" COMPILER_SUPPORTS_C99)
check_c_compiler_flag("/std:c99" COMPILER_SUPPORTS_C99)
if (COMPILER_SUPPORTS_C99)
    set(CMAKE_C_STANDARD 99)
else ()
    message(FATAL "The compiler ${CMAKE_C_COMPILER} has no C99 support. Please use a different C compiler.")
endif ()

include_directories(include)

aux_source_directory(./include SRCS)
aux_source_directory(./src SRCS_IMPL)

add_library(pb-cpp STATIC
        ${SRCS}
        ${SRCS_IMPL}
        )
set_target_properties(pb-cpp PROPERTIES LINKER_LANGUAGE CXX)
