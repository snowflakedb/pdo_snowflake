#!/bin/bash -e
#
# Run PHPT tests
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR/..

export REPORT_EXIT_STATUS=1
export NO_INTERACTION=true

source $DIR/env.sh

env | grep SNOWFLAKE_TEST > $DIR/../testenv.ini

echo "===> Test parameters"
if ! cat $DIR/../testenv.ini | grep -v PASSWORD; then
    echo "[ERROR] no connection parameter is set in the test environment file. set SNOWFLAKE_TEST_* in the environment variables, rebuild all code and run this script again."
    exit 1
fi
echo "===> Running Tests"
if ! make test; then
    echo "Test Failed"
    ls -l tests
    for f in $(ls tests/*); do echo "===> $f"; cat $f; echo; done
    cat logs/*.txt || true
    exit 1
elif [[ -n "$REPORT_COVERAGE" ]]; then
    gcov -o .libs snowflake_driver.c snowflake_stmt.c
fi
