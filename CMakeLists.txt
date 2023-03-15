cmake_minimum_required(VERSION 3.5)
project(babble)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/client)
add_subdirectory(src/server)