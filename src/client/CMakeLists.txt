aux_source_directory(. SRC_LIST)
aux_source_directory(../common COMMON_LIST)

include_directories(../common .)

add_executable(ChatClient ${SRC_LIST} ${COMMON_LIST})