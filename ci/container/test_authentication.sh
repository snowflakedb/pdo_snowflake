#!/bin/bash -e
#
# Run authentication tests inside Docker container
# Builds pdo_snowflake and runs external browser tests using PHPT runner
#

set -o pipefail

export WORKSPACE=${WORKSPACE:-/mnt/workspace}
export SOURCE_ROOT=${SOURCE_ROOT:-/mnt/host}

echo "=========================================="
echo "PHP External Browser Authentication Tests"
echo "=========================================="

cd "${SOURCE_ROOT}"

# Load authentication parameters from JSON file
AUTH_PARAMETER_FILE="./.github/workflows/parameters/private/parameters_aws_auth_tests.json"
if [ -f "$AUTH_PARAMETER_FILE" ]; then
    eval $(jq -r '.authtestparams | to_entries | map("export \(.key)=\(.value|tostring)")|.[]' "$AUTH_PARAMETER_FILE")
    echo "Parameters loaded"
else
    echo "ERROR: Parameters file not found: $AUTH_PARAMETER_FILE"
    exit 1
fi

# Build pdo_snowflake extension
echo ""
echo "Building pdo_snowflake extension..."
export PHP_HOME=${PHP_HOME:-/usr}
export PLATFORM=$(uname | tr '[:upper:]' '[:lower:]')
bash scripts/build_pdo_snowflake.sh

echo "Extension built"

# Run tests
echo ""
echo "Running authentication tests..."
echo ""

export REPORT_EXIT_STATUS=1
export NO_INTERACTION=true

# Temporarily disable 'exit on error' to capture test exit code
set +e

# Run external browser tests (subprocess loads extension to avoid log conflicts)
echo "Running external browser authentication tests..."
php run-tests.php \
  -p $(which php) \
  tests/AuthenticationTests/externalbrowser*.phpt

EXIT_CODE=$?

set -e

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo "All authentication tests PASSED"
else
    echo "Authentication tests FAILED"
    for f in tests/AuthenticationTests/externalbrowser*.out; do
        [ -f "$f" ] && echo "=== $f ===" && cat "$f"
    done
fi

exit $EXIT_CODE
