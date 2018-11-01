#!/bin/bash -e
#
# Build Docker images for tests.
# 
# NOTE: the images don't include PDO driver for Snowflake.
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PROXY_IP=${PROXY_IP:-172.20.128.10}
PROXY_PORT=${PROXY_PORT:-3128}

# Ubuntu 14.04
docker build -f $DIR/Dockerfile.ubuntu14 \
       --build-arg PHP_VERSION=7.2 \
       --build-arg PROXY_IP=${PROXY_IP} \
       --build-arg PROXY_PORT=${PROXY_PORT} \
       -t pdo-snowflake-w-proxy:php7.2-ubuntu14.04 .

# Ubuntu 16.04
docker build -f $DIR/Dockerfile.ubuntu16 \
       --build-arg PROXY_IP=${PROXY_IP} \
       --build-arg PROXY_PORT=${PROXY_PORT} \
       -t pdo-snowflake-w-proxy:php7.2-ubuntu16.04 .

# Ubuntu 18.04
docker build -f $DIR/Dockerfile.ubuntu18 \
       --build-arg PROXY_IP=${PROXY_IP} \
       --build-arg PROXY_PORT=${PROXY_PORT} \
       -t pdo-snowflake-w-proxy:php7.2-ubuntu18.04 .
