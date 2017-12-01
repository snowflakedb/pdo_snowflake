#!/bin/bash -e
#
# Build PDO Snowflake
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
[[ -z "$PHP_HOME" ]] && echo "Set PHP_HOME to the top directory of PHP directory" && exit 1

cd $DIR/..
./libsnowflakeclient/scripts/build_libsnowflakeclient.sh
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
cc -shared \
    .libs/pdo_snowflake.o \
    .libs/snowflake_driver.o \
    .libs/snowflake_stmt.o \
    -L libsnowflakeclient/cmake-build \
    -L libsnowflakeclient/deps-build/linux/openssl/lib \
    -L libsnowflakeclient/deps-build/linux/curl/lib \
    -Wl,--whole-archive \
    -lsnowflakeclient -lcrypto -lssl -lcurl \
    -Wl,--no-whole-archive \
    $LINK_OPTS \
    -Wl,-soname -Wl,pdo_snowflake.so \
    -o .libs/pdo_snowflake.so
(cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules

