#!/bin/bash
#
# Test certificate revocation validation using the revocation-validation framework.
#
# Runs entirely inside a Docker container based on rockylinux9 with PHP + Go installed.
#

set -o pipefail

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PDO_ROOT="$( dirname "${THIS_DIR}")"
WORKSPACE=${WORKSPACE:-${PDO_ROOT}}

echo "[Info] Starting revocation validation tests"

# Build a Docker image with PHP + Go for running both the build and revocation tests
DOCKER_TAG="pdo-revocation-test:latest"
echo "[Info] Building test Docker image..."

docker build -t "$DOCKER_TAG" -f - "$PDO_ROOT" <<'DOCKERFILE'
FROM rockylinux:9

USER root

# Install base tools + Remi repo for PHP
RUN dnf -y update && \
    dnf install -y https://rpms.remirepo.net/enterprise/remi-release-9.rpm && \
    dnf install -y \
        autoconf automake cmake diffutils file \
        gcc gcc-c++ git jq libtool make python3 which && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    dnf clean all

# Install PHP 8.3
RUN dnf module reset php -y && \
    dnf module enable php:remi-8.3 -y && \
    dnf install -y php php-devel php-pdo php-json && \
    dnf clean all

# Install Go
RUN curl -fsSL https://go.dev/dl/go1.24.0.linux-amd64.tar.gz | tar -C /usr/local -xz
ENV PATH="/usr/local/go/bin:${PATH}"

WORKDIR /work
DOCKERFILE

echo "[Info] Running revocation tests inside Docker..."

docker run --rm \
    -v "${PDO_ROOT}:/mnt/host" \
    -v "${WORKSPACE}:/mnt/workspace" \
    -e GITHUB_USER \
    -e GITHUB_TOKEN \
    "$DOCKER_TAG" \
    bash -c '
set -e

echo "[Info] Building pdo_snowflake extension..."
cd /mnt/host
export PHP_HOME=/usr
scripts/build_pdo_snowflake.sh

echo "[Info] Extension built: $(ls -la modules/pdo_snowflake.so)"

# Clone revocation-validation framework
REVOCATION_DIR="/tmp/revocation-validation"
rm -rf "$REVOCATION_DIR"
if [ -n "$GITHUB_USER" ] && [ -n "$GITHUB_TOKEN" ]; then
    git clone --depth 1 --branch main "https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
else
    git clone --depth 1 --branch main "https://github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
fi

cd "$REVOCATION_DIR"

echo "[Info] Running tests with Go $(go version)..."

go run . \
    --client snowflake-php \
    --php-extension-dir "/mnt/host/modules" \
    --output "/mnt/workspace/revocation-results.json" \
    --output-html "/mnt/workspace/revocation-report.html" \
    --log-level debug
'

EXIT_CODE=$?

if [ -f "${WORKSPACE}/revocation-results.json" ]; then
    echo "[Info] Results: ${WORKSPACE}/revocation-results.json"
fi
if [ -f "${WORKSPACE}/revocation-report.html" ]; then
    echo "[Info] Report: ${WORKSPACE}/revocation-report.html"
fi

exit $EXIT_CODE
