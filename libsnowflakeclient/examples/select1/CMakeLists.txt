# Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
#
cmake_minimum_required(VERSION 2.8)
project(ex_select1)

add_executable(ex_select1 select1.c)

target_link_libraries(ex_select1 snowflakeclient)