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
cc -shared \
    -g \
    .libs/snowflake_arraylist.o \
    .libs/pdo_snowflake.o \
    .libs/snowflake_driver.o \
    .libs/snowflake_stmt.o \
    -L libsnowflakeclient/lib \
    -L libsnowflakeclient/deps-build/linux/openssl/lib \
    -L libsnowflakeclient/deps-build/linux/curl/lib \
    -lsnowflakeclient \
    -Wl,--whole-archive \
    -lcrypto -lssl -lcurl \
    -Wl,--no-whole-archive \
    $LINK_OPTS \
    -Wl,-soname -Wl,pdo_snowflake.so \
    -o .libs/pdo_snowflake.so
(cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules

source $DIR/env.sh

env | grep SNOWFLAKE_TEST > $DIR/../testenv.ini
