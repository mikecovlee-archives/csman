cmake_minimum_required(VERSION 3.2)
project(csman)

set(CMAKE_MODULE_PATH "${CMCMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

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

message(STATUS ":: Including dependency: mozart++")
add_subdirectory(third-party/mozart)
include_directories(third-party/mozart)

message(STATUS ":: Including dependency: curl")
add_subdirectory(third-party/curl)
include_directories(third-party/curl/include)

message(STATUS ":: Including dependency: jsoncpp")
add_subdirectory(third-party/jsoncpp)
include_directories(third-party/jsoncpp/include)

message(STATUS ":: Including dependency: pb-cpp")
add_subdirectory(third-party/pb-cpp)
include_directories(third-party/pb-cpp/include)

add_subdirectory(csman-os)
include_directories(csman-os/include)

add_subdirectory(csman-core)
include_directories(csman-core/include)

add_subdirectory(csman-cli)
include_directories(csman-cli/include)

