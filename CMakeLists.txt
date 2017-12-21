#
# CMakeList for PHP PDO Driver for Snowflake on PHP 7.1.6
#
cmake_minimum_required(VERSION 2.8)
project(pdo_snowflake)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_FLAGS
        "-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -DPHP_ATOM_INC -D HAVE_CONFIG_H -fPIC -DPIC")

# coverage
set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -g -O0 -ggdb -fprofile-arcs -ftest-coverage -fPIC")

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()

# PHP home: dev env not used in TravisCI
set(PHP_HOME $ENV{PHP_HOME})
message("PHP is located at " $ENV{PHP_HOME})

set(SOURCE_FILES
        snowflake_arraylist.c
        snowflake_arraylist.h
        snowflake_driver.c
        snowflake_stmt.c
        php_pdo_snowflake_int.h
        php_pdo_snowflake.h
        pdo_snowflake.c)

# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")

include_directories(
        include
        libsnowflakeclient/include
        ${PHP_HOME}/include/php
        ${PHP_HOME}/include/php/main
        ${PHP_HOME}/include/php/TSRM
        ${PHP_HOME}/include/php/Zend
        ${PHP_HOME}/include/php/ext
        ${PHP_HOME}/include/php/ext/date/lib
)
add_subdirectory(libsnowflakeclient)

add_library(pdo_snowflake SHARED ${SOURCE_FILES})

SET_TARGET_PROPERTIES(pdo_snowflake PROPERTIES PREFIX "")
set_target_properties(pdo_snowflake PROPERTIES LINK_FLAGS "-Wl,-E")

target_link_libraries(pdo_snowflake
        snowflakeclient
        -fprofile-arcs
        -Wl,-soname -Wl,pdo_snowflake.so)
