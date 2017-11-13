# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
#
# CMakeList for Snowflake Client
#
cmake_minimum_required(VERSION 2.8)
project(snowflakeclient)

add_definitions(-DLOG_USE_COLOR)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_FILES
        include/snowflake_client.h
        lib/snowflake_client_int.h
        lib/snowflake_client_version.h
        lib/snowflake_client.c
        lib/cJSON.h
        lib/cJSON.c
        lib/log.h
        lib/log.c
        lib/arraylist.h
        lib/arraylist.c
        lib/snowflake_memory.h
        lib/snowflake_memory.c
        lib/connection.h
        lib/connection.c
        lib/basic_types.h
        lib/results.h
        lib/results.c
        lib/uuid4.h
        lib/uuid4.c)

find_library(CURL_LIB libcurl.a PATHS deps/curl/lib/ REQUIRED)
find_library(SSL_LIB libssl.a PATHS deps/openssl/lib/ REQUIRED)
find_library(CRYPTO_LIB libcrypto.a PATHS deps/openssl/lib/ REQUIRED)

include_directories(deps/curl/include deps/openssl/include)

message("libcurl is located at " ${CURL_LIB})
message("libssl is located at " ${SSL_LIB})
message("libcrypto is located at " ${CRYPTO_LIB})

add_library(snowflakeclient STATIC ${SOURCE_FILES})

set_property(TARGET snowflakeclient PROPERTY C_STANDARD 90)

target_link_libraries(snowflakeclient ${CURL_LIB})

add_subdirectory(examples)
