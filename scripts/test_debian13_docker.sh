#!/bin/bash -e
#
# Test PDO Snowflake in Debian 13 Docker
#

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( dirname "${THIS_DIR}")"
CONTAINER_PROJECT_DIR="/home/user/pdo_snowflake"

if [[ -z "${PHP_VERSION}" ]]; then
    echo "[ERROR] PHP_VERSION environment variable not set"
    exit 1
fi

CONTAINER_NAME="test_pdo_snowflake_debian13_php${PHP_VERSION}"

echo "[INFO] Building Debian 13 Docker image with PHP ${PHP_VERSION}"

USER_ID=$(id -u)
GROUP_ID=$(id -g)

pushd "$PROJECT_DIR"
docker build --pull -t "${CONTAINER_NAME}:1.0" \
    --build-arg USER_ID="$USER_ID" \
    --build-arg GROUP_ID="$GROUP_ID" \
    --build-arg PHP_VERSION="$PHP_VERSION" \
    . -f docker/test-debian13/Dockerfile.debian13
popd

echo "[INFO] Starting Debian 13 Docker container"
docker run --network=host \
    -e SNOWFLAKE_TEST_USER \
    -e SNOWFLAKE_TEST_PRIVATE_KEY \
    -e SNOWFLAKE_TEST_ACCOUNT \
    -e SNOWFLAKE_TEST_WAREHOUSE \
    -e SNOWFLAKE_TEST_DATABASE \
    -e SNOWFLAKE_TEST_SCHEMA \
    -e SNOWFLAKE_TEST_ROLE \
    -e PHP_HOME \
    -e TEST_PHP_EXECUTABLE \
    -e NO_INTERACTION \
    -e USE_VALGRIND \
    -e GITHUB_ACTIONS \
    -e GITHUB_WORKSPACE="$CONTAINER_PROJECT_DIR" \
    --mount type=bind,source="${PROJECT_DIR}",target="$CONTAINER_PROJECT_DIR" \
    "${CONTAINER_NAME}:1.0" \
    "${CONTAINER_PROJECT_DIR}/scripts/test_debian13.sh"
