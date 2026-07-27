#!/bin/bash -e
#
# Run authentication tests inside Docker container
# Builds pdo_snowflake and runs external browser tests using PHPT runner
#

set -o pipefail

export HOST_ROOT=${HOST_ROOT:-/mnt/host}
export BUILD_ROOT="/tmp/pdo_snowflake_build"

echo "=========================================="
echo "PHP External Browser Authentication Tests"
echo "=========================================="

# Copy source to writable location (avoids permission issues and stale build artifacts)
echo "Copying source to build directory..."
rm -rf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"
cp -r "$HOST_ROOT"/* "$BUILD_ROOT/" 2>/dev/null || true
cp -r "$HOST_ROOT"/.[!.]* "$BUILD_ROOT/" 2>/dev/null || true

# Clean stale build artifacts that may have host paths
cd "$BUILD_ROOT"
rm -rf .libs autom4te.cache 2>/dev/null || true
rm -f Makefile Makefile.fragments Makefile.objects config.h config.log config.status configure libtool 2>/dev/null || true
find . -name '*.o' -o -name '*.lo' -o -name '*.la' 2>/dev/null | xargs rm -f 2>/dev/null || true

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

# Run all authentication tests
php run-tests.php \
  -p $(which php) \
  tests/AuthenticationTests/

EXIT_CODE=$?

set -e

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo "All authentication tests PASSED"
else
    echo "Authentication tests FAILED"
    for f in tests/AuthenticationTests/*.out; do
        [ -f "$f" ] && echo "=== $f ===" && cat "$f"
    done
fi

# Copy test results back to host for CI
if [ -d "$HOST_ROOT/tests/AuthenticationTests" ]; then
    cp -f tests/AuthenticationTests/*.out "$HOST_ROOT/tests/AuthenticationTests/" 2>/dev/null || true
    cp -f tests/AuthenticationTests/junit-results.xml "$HOST_ROOT/tests/AuthenticationTests/" 2>/dev/null || true
fi

exit $EXIT_CODE
