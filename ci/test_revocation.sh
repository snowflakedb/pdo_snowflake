#!/bin/bash
#
# Test certificate revocation validation using the revocation-validation framework.
#

set -o pipefail

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PDO_ROOT="$( dirname "${THIS_DIR}")"
WORKSPACE=${WORKSPACE:-${PDO_ROOT}}

echo "[Info] Starting revocation validation tests"

# Build the PHP extension if not already built
PDO_EXT_DIR="$PDO_ROOT/modules"
if [ ! -f "$PDO_EXT_DIR/pdo_snowflake.so" ]; then
    echo "[Info] Building pdo_snowflake extension..."
    if [ -z "$PHP_HOME" ]; then
        # Try common locations
        for p in /usr /usr/local /opt/homebrew; do
            if [ -x "$p/bin/phpize" ]; then
                export PHP_HOME="$p"
                break
            fi
        done
    fi
    if [ -z "$PHP_HOME" ]; then
        echo "[Error] PHP_HOME not set and phpize not found. Cannot build pdo_snowflake."
        exit 1
    fi
    echo "[Info] Using PHP_HOME=$PHP_HOME"
    (cd "$PDO_ROOT" && export PATH=$PHP_HOME/bin:$PATH && scripts/build_pdo_snowflake.sh)
fi

if [ ! -f "$PDO_EXT_DIR/pdo_snowflake.so" ]; then
    echo "[Error] pdo_snowflake.so not found in $PDO_EXT_DIR after build"
    exit 1
fi

echo "[Info] Using extension: $PDO_EXT_DIR/pdo_snowflake.so"

set -e

# Clone revocation-validation framework
REVOCATION_DIR="/tmp/revocation-validation"
REVOCATION_BRANCH="${REVOCATION_BRANCH:-main}"

rm -rf "$REVOCATION_DIR"
if [ -n "$GITHUB_USER" ] && [ -n "$GITHUB_TOKEN" ]; then
    git clone --depth 1 --branch "$REVOCATION_BRANCH" "https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
else
    git clone --depth 1 --branch "$REVOCATION_BRANCH" "https://github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
fi

cd "$REVOCATION_DIR"

echo "[Info] Running tests with Go $(go version | grep -oE 'go[0-9]+\.[0-9]+')..."

go run . \
    --client snowflake-php \
    --php-extension-dir "${PDO_EXT_DIR}" \
    --output "${WORKSPACE}/revocation-results.json" \
    --output-html "${WORKSPACE}/revocation-report.html" \
    --log-level debug

EXIT_CODE=$?

if [ -f "${WORKSPACE}/revocation-results.json" ]; then
    echo "[Info] Results: ${WORKSPACE}/revocation-results.json"
fi
if [ -f "${WORKSPACE}/revocation-report.html" ]; then
    echo "[Info] Report: ${WORKSPACE}/revocation-report.html"
fi

exit $EXIT_CODE
