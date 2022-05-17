#!/bin/bash -e
#
# Build PDO Snowflake
#
function usage() {
    echo "Usage: `basename $0`"
    exit 2
}
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
[[ -z "$PHP_HOME" ]] && echo "Set PHP_HOME to the top directory of PHP directory" && exit 1
source $DIR/_init.sh $@
cd $DIR/..

while getopts "h" opt; do
  case $opt in
    h) usage;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    :) echo "Option -$OPTARG requires an argument." >&2; exit 1 ;;
  esac
done

export PATH=$PHP_HOME/bin:$PATH
if [[ -e "Makefile" ]]; then
    tmp_dir=$(mktemp -d)
    mv libsnowflakeclient $tmp_dir/
    make clean
    mv $tmp_dir/libsnowflakeclient .
    rm -r "$tmp_dir"
fi
CONFIGURE_OPTS=("--enable-pdo_snowflake")
if [[ -n "$REPORT_COVERAGE" ]]; then
    CONFIGURE_OPTS+=("--enable-coverage")
fi
phpize
echo ./configure "${CONFIGURE_OPTS[@]}"
./configure "${CONFIGURE_OPTS[@]}"
make

# workaround for libtool issue, which cannot keep the order of Link options
# and --whole-archive is ignored.
LINK_OPTS=
if [[ -n "$REPORT_COVERAGE" ]]; then
    LINK_OPTS="-fprofile-arcs -ftest-coverage"
fi
if [[ "$PLATFORM" == "linux" ]]; then
    echo "Linking for Linux"
    g++ -shared \
        -fPIC \
        -DPIC \
        .libs/pdo_snowflake.o \
        .libs/snowflake_driver.o \
        .libs/snowflake_stmt.o \
        .libs/snowflake_paramstore.o \
        .libs/snowflake_arraylist.o \
        .libs/snowflake_treemap.o \
        .libs/snowflake_rbtree.o \
        libsnowflakeclient/lib/linux/libsnowflakeclient.a \
        libsnowflakeclient/deps-build/linux/openssl/lib/libcrypto.a \
        libsnowflakeclient/deps-build/linux/openssl/lib/libssl.a \
        libsnowflakeclient/deps-build/linux/curl/lib/libcurl.a \
        libsnowflakeclient/deps-build/linux/oob/lib/libtelemetry.a \
        libsnowflakeclient/deps-build/linux/aws/lib64/libaws-cpp-sdk-core.a \
        libsnowflakeclient/deps-build/linux/aws/lib64/libaws-cpp-sdk-s3.a \
        libsnowflakeclient/deps-build/linux/azure/lib/libazure-storage-lite.a \
        libsnowflakeclient/deps-build/linux/uuid/lib/libuuid.a \
        libsnowflakeclient/deps-build/linux/arrow/lib/libarrow.a \
        libsnowflakeclient/deps-build/linux/arrow_deps/lib/libjemalloc_pic.a \
        libsnowflakeclient/deps-build/linux/boost/lib/libboost_filesystem.a \
        libsnowflakeclient/deps-build/linux/boost/lib/libboost_regex.a \
        libsnowflakeclient/deps-build/linux/boost/lib/libboost_system.a \
        -O2 \
        -Wl,--whole-archive \
        -Wl,--no-whole-archive \
        -Wl,-soname \
        -Wl,pdo_snowflake.so \
        -o .libs/pdo_snowflake.so \
        libsnowflakeclient/deps-build/linux/openssl/lib/libssl.a \
        libsnowflakeclient/deps-build/linux/openssl/lib/libcrypto.a \
        libsnowflakeclient/deps-build/linux/cmocka/lib/libcmocka.a
elif [[ "$PLATFORM" == "darwin" ]]; then
    # Darwin uses -force_load instead
    echo "Linking for Darwin"
    cc -dynamiclib -undefined dynamic_lookup \
        -g \
        .libs/snowflake_paramstore.o \
        .libs/snowflake_arraylist.o \
        .libs/snowflake_treemap.o \
        .libs/snowflake_rbtree.o \
        .libs/pdo_snowflake.o \
        .libs/snowflake_driver.o \
        .libs/snowflake_stmt.o \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/arrow/lib/libarrow.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/arrow_deps/lib/libjemalloc_pic.a \
        libsnowflakeclient/deps-build/darwin/boost/lib/libboost_filesystem.a \
        libsnowflakeclient/deps-build/darwin/boost/lib/libboost_regex.a \
        libsnowflakeclient/deps-build/darwin/boost/lib/libboost_system.a \
        -Wl,-force_load,libsnowflakeclient/lib/darwin/libsnowflakeclient.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/openssl/lib/libcrypto.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/openssl/lib/libssl.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/curl/lib/libcurl.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/oob/lib/libtelemetry.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/aws/lib/libaws-cpp-sdk-core.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/aws/lib/libaws-cpp-sdk-s3.a \
        -Wl,-force_load,libsnowflakeclient/deps-build/darwin/azure/lib/libazure-storage-lite.a \
        $LINK_OPTS \
        -o .libs/pdo_snowflake.so
fi
(cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules

