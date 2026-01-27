#!/bin/bash -e
#
# Run authentication tests inside Docker container
# Builds pdo_snowflake and runs external browser tests using PHPT runner
#

set -o pipefail

export WORKSPACE=${WORKSPACE:-/mnt/workspace}
export SOURCE_ROOT=${SOURCE_ROOT:-/mnt/host}
export BUILD_DIR=${BUILD_DIR:-/tmp/pdo_snowflake_build}

echo "=========================================="
echo "PHP External Browser Authentication Tests"
echo "=========================================="

echo "Copying source to build directory..."
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Copy source files, excluding generated build artifacts that contain host-specific paths
cp -r "${SOURCE_ROOT}/." "${BUILD_DIR}/"
cd "${BUILD_DIR}"

rm -f Makefile Makefile.* config.h config.log config.nice config.status configure configure~ libtool 2>/dev/null || true
rm -rf .libs autom4te.cache modules 2>/dev/null || true
find . \( -name "*.lo" -o -name "*.o" -o -name "*.la" \) -delete 2>/dev/null || true

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
