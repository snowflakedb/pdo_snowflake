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

# Builds
eval $(jq -r '.testconnection | to_entries | map("export \(.key)=\(.value|tostring)")|.[]' ./parameters.json)
echo "PHP_HOME:   $PHP_HOME"
echo "phpize:     $(which phpize)"
echo "php-config: $(which php-config)"
REPORT_COVERAGE=1 $DIR/build_pdo_snowflake.sh -r

# Tests
$LIB_SNOWFLAKE_CLIENT_DIR/scripts/run_tests.sh
$DIR/run_tests.sh
