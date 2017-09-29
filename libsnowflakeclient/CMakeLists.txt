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
        )

add_library(snowflakeclient STATIC ${SOURCE_FILES})

add_subdirectory(examples)
