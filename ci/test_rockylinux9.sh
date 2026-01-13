#!/bin/bash -e
#
# Test PDO Snowflake in Rocky Linux 9
#
# This script runs inside the Rocky Linux 9 Docker container.
# It builds the PDO Snowflake driver and runs tests.
#
# Required environment variables:
#   - SNOWFLAKE_TEST_USER
#   - SNOWFLAKE_TEST_PASSWORD
#   - SNOWFLAKE_TEST_ACCOUNT
#   - SNOWFLAKE_TEST_WAREHOUSE
#   - SNOWFLAKE_TEST_DATABASE
#   - SNOWFLAKE_TEST_SCHEMA
#   - SNOWFLAKE_TEST_ROLE
#

set -o pipefail

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( dirname "${THIS_DIR}")"

# Validate required environment variables
required_vars=(
    "SNOWFLAKE_TEST_USER"
    "SNOWFLAKE_TEST_PASSWORD"
    "SNOWFLAKE_TEST_ACCOUNT"
    "SNOWFLAKE_TEST_WAREHOUSE"
    "SNOWFLAKE_TEST_DATABASE"
    "SNOWFLAKE_TEST_SCHEMA"
    "SNOWFLAKE_TEST_ROLE"
)

for var in "${required_vars[@]}"; do
    if [[ -z "${!var}" ]]; then
        echo "[ERROR] ${var} environment variable not set."
        exit 1
    fi
done

echo "[INFO] Environment validated successfully"

# Display PHP version info
echo "[INFO] PHP version:"
php --version
echo "[INFO] PHP modules:"
php -m | grep -E "PDO|json" || true

# Set up environment
export PHP_HOME=/usr
export TEST_PHP_EXECUTABLE=/usr/bin/php
export NO_INTERACTION=true
export REPORT_EXIT_STATUS=1

# Build driver
pushd "${PROJECT_DIR}"
echo "[INFO] Building PDO Snowflake driver"
./scripts/build_pdo_snowflake.sh

# Verify driver is built
if [[ ! -f "modules/pdo_snowflake.so" ]]; then
    echo "[ERROR] pdo_snowflake.so not found after build"
    exit 1
fi

echo "[INFO] Verifying driver loads correctly"
php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake

# Set up test environment
echo "[INFO] Setting up test environment"
env | grep SNOWFLAKE_TEST > testenv.ini
cat testenv.ini | grep -v PASSWORD

# Run tests
echo "[INFO] Running tests"
php -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1' \
    ./run-tests.php -d extension=modules/pdo_snowflake.so ./tests || true

# Check results
echo "[INFO] Checking test results"
python3 ./.github/workflows/scripts/check_result.py ./tests

popd

echo "[INFO] Tests completed successfully"

