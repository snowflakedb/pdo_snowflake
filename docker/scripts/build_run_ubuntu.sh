#!/bin/bash -e
#
# Build and Test PDO Snowflake on Ubuntu
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CONFIG_FILE=$1
CONFIG_FILE=${CONFIG_FILE:-/cfg/parameters.json}
[[ ! -f "$CONFIG_FILE" ]] && echo "The test connection parameter file is missing: $CONFIG_FILE" && exit 1

cd
git clone https://github.com/snowflakedb/pdo_snowflake.git
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

# Build and generate testenv.ini
./scripts/build_pdo_snowflake.sh

# Update tests with parameters
PHP_API_VER=$(php -i | grep "PHP API" | awk '{print $4}')
PHP_EXT=$(find /usr/lib/php -name "pdo.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done
PHP_EXT=$(find /usr/lib/php -name "json.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done

# Testing
./scripts/run_tests.sh
