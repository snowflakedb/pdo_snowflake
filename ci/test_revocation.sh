#!/bin/bash
#
# Test certificate revocation validation using the revocation-validation framework.
#
# Runs inside a Docker container with PHP + Go installed.
#

set -o pipefail

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PDO_ROOT="$( dirname "${THIS_DIR}")"
WORKSPACE=${WORKSPACE:-${PDO_ROOT}}

echo "[Info] Starting revocation validation tests"

# Build a Docker image with PHP + Go + the built extension
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

# Copy the pdo_snowflake source and build the extension
COPY . /build/pdo_snowflake
WORKDIR /build/pdo_snowflake

# Build using the project's build script (handles C++ linking correctly)
# then copy the .so to PHP's extension directory
RUN export PHP_HOME=/usr && \
    scripts/build_pdo_snowflake.sh && \
    cp modules/pdo_snowflake.so $(php -r 'echo ini_get("extension_dir");')/ && \
    echo "extension=pdo_snowflake" > /etc/php.d/30-pdo_snowflake.ini

# Verify the extension loads
RUN php -m | grep -i pdo_snowflake && echo "[OK] pdo_snowflake extension loaded"

WORKDIR /work
DOCKERFILE

echo "[Info] Running revocation tests inside Docker..."

docker run --rm \
    -v "${WORKSPACE}:/mnt/workspace" \
    -e GITHUB_USER \
    -e GITHUB_TOKEN \
    "$DOCKER_TAG" \
    bash -c '
set -e

echo "[Info] PHP version: $(php -v | head -1)"
echo "[Info] pdo_snowflake loaded: $(php -m | grep pdo_snowflake)"

# Clone revocation-validation framework
REVOCATION_DIR="/tmp/revocation-validation"
rm -rf "$REVOCATION_DIR"
if [ -n "$GITHUB_USER" ] && [ -n "$GITHUB_TOKEN" ]; then
    git clone --depth 1 --branch dotnet-fix "https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
else
    git clone --depth 1 --branch dotnet-fix "https://github.com/snowflakedb/revocation-validation.git" "$REVOCATION_DIR"
fi

cd "$REVOCATION_DIR"

# Get the system extension directory so the framework uses the installed extension
PHP_EXT_DIR=$(php -r "echo ini_get(\"extension_dir\");")
echo "[Info] PHP extension dir: $PHP_EXT_DIR"

echo "[Info] Running tests with $(go version)..."

go run . \
    --client snowflake-php \
    --php-extension-dir "$PHP_EXT_DIR" \
    --output "/mnt/workspace/revocation-results.json" \
    --output-html "/mnt/workspace/revocation-report.html" \
    --log-level debug

# Fix ownership while still root inside container so Jenkins can archive the artifacts
chmod a+r /mnt/workspace/revocation-results.json /mnt/workspace/revocation-report.html 2>/dev/null || true
'

EXIT_CODE=$?

echo "[Info] Artifacts:"
ls -la "${WORKSPACE}/revocation-results.json" "${WORKSPACE}/revocation-report.html" 2>/dev/null || true

exit $EXIT_CODE
