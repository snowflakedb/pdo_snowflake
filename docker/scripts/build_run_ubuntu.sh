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

# Build and generate testenv.ini
REPORT_COVERAGE=1 ./scripts/build_pdo_snowflake.sh

# Update tests with parameters
PHP_API_VER=$(php -i | grep "PHP API" | awk '{print $4}')
PHP_EXT=$(find /usr/lib/php -name "pdo.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done
PHP_EXT=$(find /usr/lib/php -name "json.so" | grep $PHP_API_VER) && for f in $(ls tests/*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done

# Testing
./scripts/run_tests.sh
