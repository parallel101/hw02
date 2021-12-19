cmake_minimum_required(VERSION 3.12)
project(hellocmake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB SRC
    "*.cpp"
)

file(GLOB HEADER
    "*.h"
    "*.hpp"
)

add_executable(main ${SRC} ${HEADER})

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:main> ${CMAKE_SOURCE_DIR}/build/bin/main.exe
)

source_group("Source Files"            FILES ${SRC})
source_group("Header Files"            FILES ${HEADER})
