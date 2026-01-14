#!/bin/bash -e
#
# Test PDO Snowflake in Rocky Linux 9
#
# This script runs inside the Rocky Linux 9 Docker container.
# It builds the PDO Snowflake driver and runs tests using the same
# Python scripts as the Ubuntu Linux tests.
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

# Environment variables are passed by test_rockylinux9_docker.sh:
#   PHP_HOME, TEST_PHP_EXECUTABLE, NO_INTERACTION, USE_VALGRIND, GITHUB_WORKSPACE

cd "${GITHUB_WORKSPACE}"

# Display PHP version info
echo "[INFO] PHP version:"
php --version

# Build driver using same script as Ubuntu Linux
echo "[INFO] Building PDO Snowflake driver"
python3 ./.github/workflows/scripts/build_driver.py

# Test driver using same script as Ubuntu Linux
echo "[INFO] Running tests"
python3 ./.github/workflows/scripts/test_driver.py

echo "[INFO] Tests completed successfully"

