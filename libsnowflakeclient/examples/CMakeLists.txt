# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.

# includes catch.hpp, a test framework
include_directories(
        "../include"
        "utils"
)
set(SOURCE_UTILS
        example_setup.c)

add_executable(ex_connect ${SOURCE_UTILS} connect.c)
add_executable(ex_select1 ${SOURCE_UTILS} select1.c)
add_executable(ex_transaction ${SOURCE_UTILS} transaction.c)
add_executable(ex_ping_pong ${SOURCE_UTILS} ping_pong.c)
add_executable(ex_json_test ${SOURCE_UTILS} json_test.c)

target_link_libraries(
        ex_connect
        snowflakeclient
        -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
)

target_link_libraries(
        ex_select1
        snowflakeclient
        -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
)

target_link_libraries(
        ex_transaction
        snowflakeclient
        -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
)

target_link_libraries(
        ex_ping_pong
        snowflakeclient
        -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
)

target_link_libraries(
        ex_json_test
        snowflakeclient
        -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
)
