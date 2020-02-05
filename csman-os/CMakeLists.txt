set(CSMAN_OS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/csman-os)
message(STATUS "Configuring csman-os: module root path: ${CSMAN_OS_SOURCE_DIR}")

include_directories(include)

#### sources
set(CSMAN_OS_SRC
        include/csman/os/os.hpp
        src/os.cpp
        src/unix.hpp
        src/unix.cpp
        src/win32.hpp
        src/win32.cpp)

add_library(csman-os STATIC ${CSMAN_OS_SRC})
target_link_libraries(csman-os mozart++)

