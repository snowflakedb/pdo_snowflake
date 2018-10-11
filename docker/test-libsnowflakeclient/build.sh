#!/bin/bash -e
#
# Build libsnowflakeclient proxy test docker image
#

set -o pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PROXY_IP=${PROXY_IP:-172.20.128.10}
PROXY_PORT=${PROXY_PORT:-3128}

cd $DIR
docker build \
    --build-arg PROXY_IP=${PROXY_IP} \
    --build-arg PROXY_PORT=${PROXY_PORT} \
    -t test-libsnowflakeclient .
