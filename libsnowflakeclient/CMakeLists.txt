#
# CMakeList for Snowflake Client
#
cmake_minimum_required(VERSION 2.8)
project(snowflakeclient)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_FILES
        lib/snowflake_client.c
        lib/snowflake_client.h
        lib/snowflake_client_int.h
        lib/snowflake_client_version.h)

add_library(snowflakeclient STATIC ${SOURCE_FILES})