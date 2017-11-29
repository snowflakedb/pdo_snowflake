# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
#
# CMakeList for Snowflake Client
#
cmake_minimum_required(VERSION 2.8)
project(snowflakeclient)

add_definitions(-DLOG_USE_COLOR)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES
        lib/snowflake_client.c
        lib/cJSON.c
        lib/log.c
        lib/arraylist.c
        lib/snowflake_memory.c
        lib/connection.c
        lib/results.c
        lib/uuid4.c
        lib/basic_types.c)

find_library(CURL_LIB libcurl.a PATHS build/linux/curl/lib/ REQUIRED)
find_library(SSL_LIB libssl.a PATHS build/linux/openssl/lib/ REQUIRED)
find_library(CRYPTO_LIB libcrypto.a PATHS build/linux/openssl/lib/ REQUIRED)

include_directories(
        build/linux/curl/include
        build/linux/openssl/include
        include)

message("libcurl is located at " ${CURL_LIB})
message("libssl is located at " ${SSL_LIB})
message("libcrypto is located at " ${CRYPTO_LIB})

add_library(snowflakeclient STATIC ${SOURCE_FILES})

set_property(TARGET snowflakeclient PROPERTY C_STANDARD 90)

target_link_libraries(snowflakeclient rt dl z m
        "-Wl,--whole-archive ${CURL_LIB} ${SSL_LIB} ${CRYPTO_LIB}" "-Wl,--no-whole-archive"
        "-Wl,--as-needed -static-libgcc")

add_subdirectory(examples)
