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

add_library(pb-cpp STATIC
        include/pb-cpp/defaults.hpp
        include/pb-cpp/multibar.hpp
        include/pb-cpp/progressbar.hpp
        include/pb-cpp/util.hpp
        include/pb-cpp/unit.hpp
        include/pb-cpp/impl/mpsc.inc
        include/pb-cpp/impl/progressbar.inc
        src/util/generic.cpp
        src/util/non_windows.cpp
        src/util/windows.cpp
        src/defaults.cpp
        src/multibar.cpp
        src/multibar_notifying_pipe.cpp
        src/progressbar.cpp
        )
set_target_properties(pb-cpp PROPERTIES LINKER_LANGUAGE CXX)
