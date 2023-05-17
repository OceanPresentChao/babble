cmake_minimum_required(VERSION 3.5)
project(babble)

set(CMAKE_CXX_STANDARD 14)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/crawler)