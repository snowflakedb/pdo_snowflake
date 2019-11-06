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
    cc -shared \
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
        libsnowflakeclient/deps-build/linux/aws/lib64/libaws-cpp-sdk-core.a \
        libsnowflakeclient/deps-build/linux/aws/lib64/libaws-cpp-sdk-s3.a \
        libsnowflakeclient/deps-build/linux/azure/lib/libazure-storage-lite.a \
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
    cc -shared \
        -g \
        .libs/snowflake_paramstore.o \
        .libs/snowflake_arraylist.o \
        .libs/snowflake_treemap.o \
        .libs/snowflake_rbtree.o \
        .libs/pdo_snowflake.o \
        .libs/snowflake_driver.o \
        .libs/snowflake_stmt.o \
        -L libsnowflakeclient/lib/darwin \
        -L libsnowflakeclient/deps-build/darwin/openssl/lib \
        -L libsnowflakeclient/deps-build/darwin/curl/lib \
        -L libsnowflakeclient/deps-build/darwin/aws/lib \
        -L libsnowflakeclient/deps-build/darwin/azure/lib \
        -lsnowflakeclient \
        -Wl,-force-load,crypto \
        -Wl,-force-load,ssl \
        -Wl,-force-load,curl \
        -Wl,-force-load,pthread \
        -Wl,-force-load,aws-cpp-sdk-core \
        -Wl,-force-load,aws-cpp-sdk-s3 \
        -Wl,-force-load,azure-storage-lite \
        $LINK_OPTS \
        -Wl,-soname -Wl,pdo_snowflake.so \
        -o .libs/pdo_snowflake.so
fi
(cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules

