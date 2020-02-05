set(CSMAN_CORE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/csman-core)
message(STATUS "Configuring csman-core: module root path: ${CSMAN_CORE_SOURCE_DIR}")

include_directories(include)

#### sources
set(CSMAN_CORE_SRC
        src/source.cpp
        src/network.cpp
        src/core.cpp
        src/fwd.cpp
        src/parser.cpp
        src/op_query.cpp
        src/ops.cpp
        include/csman/core/ops.hpp
        include/csman/core/parser.hpp
        include/csman/core/source.hpp
        include/csman/core/network.hpp
        include/csman/core/fwd.hpp
        include/csman/core/core.hpp)

add_library(csman-core STATIC ${CSMAN_CORE_SRC})
target_link_libraries(csman-core csman-os)
target_link_libraries(csman-core mozart++)
target_link_libraries(csman-core libcurl)
target_link_libraries(csman-core jsoncpp_lib)

## test and benchmark targets here
file(GLOB_RECURSE CPP_SRC_LIST tests/test-*.cpp)
foreach(v ${CPP_SRC_LIST})
    string(REGEX MATCH "tests/.*" relative_path ${v})
    string(REGEX REPLACE "tests/" "" target_name ${relative_path})
    string(REGEX REPLACE ".cpp" "" target_name ${target_name})

    add_executable(csman-core-${target_name} ${v})
    target_link_libraries(csman-core-${target_name} csman-core)
    add_test(csman-core-${target_name} csman-core-${target_name})
endforeach()
