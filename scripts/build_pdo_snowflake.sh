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
fi
phpize
./configure --enable-pdo_snowflake 
make

# workaround for libtool issue, which cannot keep the order of Link options
# and --whole-archive is ignored.
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
    -Wl,-soname -Wl,pdo_snowflake.so \
    -o .libs/pdo_snowflake.so
(cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules

