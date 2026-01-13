#!/bin/bash -e
#
# Test PDO Snowflake in Rocky Linux 9 Docker
#
# This script builds a Rocky Linux 9 Docker image with PHP and build dependencies,
# then runs the test script inside the container. It is called by GitHub Actions.
#
# Required environment variables (set by GHA workflow):
#   - SNOWFLAKE_TEST_USER
#   - SNOWFLAKE_TEST_PASSWORD
#   - SNOWFLAKE_TEST_ACCOUNT
#   - SNOWFLAKE_TEST_WAREHOUSE
#   - SNOWFLAKE_TEST_DATABASE
#   - SNOWFLAKE_TEST_SCHEMA
#   - SNOWFLAKE_TEST_ROLE
#   - PHP_VERSION (optional, default: 8.1)
#

set -o pipefail

# Set constants
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( dirname "${THIS_DIR}")"
PHP_VERSION="${PHP_VERSION:-8.1}"
CONTAINER_NAME="test_pdo_snowflake_rockylinux9_php${PHP_VERSION}"

echo "[INFO] Building Rocky Linux 9 Docker image with PHP ${PHP_VERSION}"

# Get current user/group IDs to match host permissions
USER_ID=$(id -u)
GROUP_ID=$(id -g)

pushd "$PROJECT_DIR"
docker build --pull -t ${CONTAINER_NAME}:1.0 \
    --build-arg USER_ID=$USER_ID \
    --build-arg GROUP_ID=$GROUP_ID \
    --build-arg PHP_VERSION=$PHP_VERSION \
    . -f ci/image/Dockerfile.rockylinux9-test
popd

echo "[INFO] Starting Rocky Linux 9 Docker container"
docker run --network=host \
    -e SNOWFLAKE_TEST_USER \
    -e SNOWFLAKE_TEST_PASSWORD \
    -e SNOWFLAKE_TEST_ACCOUNT \
    -e SNOWFLAKE_TEST_WAREHOUSE \
    -e SNOWFLAKE_TEST_DATABASE \
    -e SNOWFLAKE_TEST_SCHEMA \
    -e SNOWFLAKE_TEST_ROLE \
    -e NO_INTERACTION=true \
    --mount type=bind,source="${PROJECT_DIR}",target=/home/user/pdo_snowflake \
    ${CONTAINER_NAME}:1.0 \
    /home/user/pdo_snowflake/ci/test_rockylinux9.sh

