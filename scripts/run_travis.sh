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

export PYVENV_HOME=$HOME/testsetup

function travis_fold_start() {
    local name=$1
    local message=$2
    echo "travis_fold:start:$name"
    tput setaf 3 || true
    echo $message
    tput sgr0 || true
    export travis_fold_name=$name
}

function travis_fold_end() {
    echo "travis_fold:end:$travis_fold_name"
    unset travis_fold_name
}

function finish {
    travis_fold_start drop_schema "Drop test schema"
    python $DIR/drop_schema.py 
    travis_fold_end
}

travis_fold_start pythonvenv "Set up Python Virtualenv (pyenv)"
pyenv local 3.6
pyenv versions
curl -O https://bootstrap.pypa.io/get-pip.py
python get-pip.py
pip install -U virtualenv
virtualenv $PYVENV_HOME
source $PYVENV_HOME/bin/activate
pip install -U snowflake-connector-python
travis_fold_end

trap finish EXIT

source $DIR/env.sh

travis_fold_start create_schema "Create test schema"
python $DIR/create_schema.py 
if [[ -n "$TRAVIS_JOB_ID" ]]; then
    echo "==> Set the test schema to TRAVIS_JOB_${TRAVIS_JOB_ID}"
    export SNOWFLAKE_TEST_SCHEMA=TRAVIS_JOB_${TRAVIS_JOB_ID}
fi
travis_fold_end

travis_fold_start build_pdo_snowflake "Builds PHP PDO"
echo "PHP_HOME:   $PHP_HOME"
echo "phpize:     $(which phpize)"
echo "php-config: $(which php-config)"
REPORT_COVERAGE=1 $DIR/build_pdo_snowflake.sh
./scripts/env.sh && env | grep SNOWFLAKE_TEST > testenv.ini
travis_fold_end

travis_fold_start phptests "Tests PHP PDO"
REPORT_COVERAGE=1 $DIR/run_tests.sh
travis_fold_end
