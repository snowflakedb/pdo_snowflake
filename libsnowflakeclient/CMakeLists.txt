# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
#
# CMakeList for Snowflake Client
#
cmake_minimum_required(VERSION 2.8)
project(snowflakeclient)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_FILES
        include/snowflake_client.h
        lib/snowflake_client_int.h
        lib/snowflake_client_version.h
        lib/snowflake_client.c
        lib/cJSON.h
        lib/cJSON.c
        lib/log.h
        lib/log.c)

include_directories(deps/curl-7.54.1/include)

link_directories(deps/curl-7.54.1/lib)

add_library(snowflakeclient STATIC ${SOURCE_FILES})

target_link_libraries(snowflakeclient curl)

add_subdirectory(examples)
