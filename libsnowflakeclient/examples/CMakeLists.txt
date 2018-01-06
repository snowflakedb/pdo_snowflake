# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.

# includes catch.hpp, a test framework
include_directories(
        "../include"
        "utils"
)

SET(EXAMPLES
        connect
        connect_negative
        select1
        transaction
        ping_pong
        stmt_with_bad_connect
        crud
        error_handling
        large_result_set
        selectvar
        selectnum
        selectbool
        selectbin
        timezone
        selectnull
        selecttsntz
        selecttsltz
        selecttstz
        selecttime
        check_ctypes)

set(SOURCE_UTILS
        example_setup.c)

FOREACH(EX ${EXAMPLES})
    add_executable(ex_${EX} ${SOURCE_UTILS} ${EX}.c)
    target_link_libraries(
            ex_${EX}
            snowflakeclient
            -Wl,--whole-archive curl ssl crypto -Wl,--no-whole-archive
    )
ENDFOREACH()
