#!/bin/bash -e
#
# Build and Test PDO Snowflake on Ubuntu
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CONFIG_FILE=$1
CONFIG_FILE=${CONFIG_FILE:-/cfg/parameters.json}
[[ ! -f "$CONFIG_FILE" ]] && echo "The test connection parameter file is missing: $CONFIG_FILE" && exit 1

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
    python3 $DIR/drop_schema.py 
    travis_fold_end
}

# clean up
trap finish EXIT

cd
TRAVIS_BRANCH=${TRAVIS_BRANCH:-master}
git clone --depth=50 --branch=$TRAVIS_BRANCH https://github.com/snowflakedb/pdo_snowflake.git
cd pdo_snowflake

# set the test parameters
cp $CONFIG_FILE parameters.json  # replicate a parameter file
source ./scripts/env.sh

# Check Ubuntu version
# Ubuntu 16 has gcc5/gcov5 but doesn't work along with lcov12
UBUNTU_VERSION=$(lsb_release -r | awk '{print $2}')
if [[ "$UBUNTU_VERSION" != "16.04" ]]; then
    export REPORT_COVERAGE=1
fi

travis_fold_start create_schema "Create test schema"
python3 ./scripts/create_schema.py
if [[ -n "$TRAVIS_JOB_ID" ]]; then
    echo "==> Set the test schema to TRAVIS_JOB_${TRAVIS_JOB_ID}"
    export SNOWFLAKE_TEST_SCHEMA=TRAVIS_JOB_${TRAVIS_JOB_ID}
fi
travis_fold_end

travis_fold_start build_pdo_snowflake "Builds PHP PDO"
echo "PHP_HOME:   $PHP_HOME"
echo "phpize:     $(which phpize)"
echo "php-config: $(which php-config)"
./scripts/build_pdo_snowflake.sh
travis_fold_end

travis_fold_start phptests "Tests PHP PDO"
# Update tests with parameters
PHP_API_VER=$(php -i | grep "PHP API" | awk '{print $4}')
PHP_EXT=$(find /usr/lib/php -name "pdo.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done
PHP_EXT=$(find /usr/lib/php -name "json.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done

# Testing
./scripts/run_tests.sh
travis_fold_end
