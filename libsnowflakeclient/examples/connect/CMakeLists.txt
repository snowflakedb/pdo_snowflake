# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
#
cmake_minimum_required(VERSION 2.8)
project(ex_connect)

add_executable(ex_connect connect.c)

target_link_libraries(ex_connect snowflakeclient)