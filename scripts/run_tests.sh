#!/bin/bash -e
#
# Run PHPT tests
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR/..

export REPORT_EXIT_STATUS=1
export NO_INTERACTION=true
echo "===> Test parameters"
cat $DIR/../testenv.ini | grep -v PASSWORD
echo "===> Running Tests"
if ! make test; then
    echo "Test Failed"
    ls -l tests
    for f in $(ls tests/*); do echo "===> $f"; cat $f; echo; done
    cat logs/*.txt || true
    exit 1
else
    gcov -o .libs snowflake_driver.c snowflake_stmt.c
fi
