#!/bin/bash -e
#
# Run Travis Build and Tests
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR/..
export PHP_HOME=$( cd "$(dirname $(which php))" && cd .. && pwd )
export LIB_SNOWFLAKE_CLIENT_DIR=$(cd ./libsnowflakeclient && pwd)
export SNOWFLAKE_TEST_CA_BUNDLE_FILE=$LIB_SNOWFLAKE_CLIENT_DIR/cacert.pem
eval $(jq -r '.testconnection | to_entries | map("export \(.key)=\(.value|tostring)")|.[]' ./parameters.json)
echo "PHP_HOME:   $PHP_HOME"
echo "phpize:     $(which phpize)"
echo "php-config: $(which php-config)"
REPORT_COVERAGE=1 $DIR/build_pdo_snowflake.sh
export REPORT_EXIT_STATUS=1
export NO_INTERACTION=true
echo "===> Test parameters"
env | grep SNOWFLAKE_TEST > $DIR/../testenv.ini
cat $DIR/../testenv.ini | grep -v PASSWORD
echo "===> Running Tests"
if ! make test; then
    echo "Test Failed"
    ls -l tests
    for f in $(ls tests/*); do echo "===> $f"; cat $f; echo; done
    exit 1
else
    gcov -o .libs snowflake_driver.c snowflake_stmt.c
fi
