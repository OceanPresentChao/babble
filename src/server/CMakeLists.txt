aux_source_directory(. SRC_LIST)

include_directories(../common .)

add_executable(ChatServer ${SRC_LIST})