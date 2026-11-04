#!/bin/bash -e
#
# Build and test PDO Snowflake inside the Debian 13 Docker container
#

cd "${GITHUB_WORKSPACE}"

echo "[INFO] PHP version:"
php --version

echo "[INFO] Building PDO Snowflake driver"
python3 ./.github/workflows/scripts/build_driver.py

echo "[INFO] Running tests"
python3 ./.github/workflows/scripts/test_driver.py

echo "[INFO] Tests completed successfully"
