#!/bin/bash -e
#
# Build SnowSQL proxy test docker image
#

set -o pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SNOWSQL_VERSION=${SNOWSQL_VERSION:-1.1.53}
PROXY_IP=${PROXY_IP:-172.20.128.10}
PROXY_PORT=${PROXY_PORT:-3128}

cd $DIR
if [[ ! -e "snowsql-${SNOWSQL_VERSION}-linux_x86_64.bash" ]]; then
    curl -O https://s3-us-west-2.amazonaws.com/sfc-snowsql-updates/bootstrap/1.1/linux_x86_64/snowsql-${SNOWSQL_VERSION}-linux_x86_64.bash
fi
docker build \
    --build-arg SNOWSQL_VERSION=${SNOWSQL_VERSION} \
    --build-arg PROXY_IP=${PROXY_IP} \
    --build-arg PROXY_PORT=${PROXY_PORT} \
    -t test-snowsql .
