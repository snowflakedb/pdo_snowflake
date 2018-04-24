#!/bin/bash -e
#
# Run SnowSQL proxy test
#
set -o pipefail

NETWORK_NAME=proxytest
PROXY_NAME=squid
PROXY_IP=172.20.128.10

SNOWSQL_ACCOUNT=$1
SNOWSQL_USER=$2
SNOWSQL_PWD=$3

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

if ! docker network ls | awk '{print $2}' | grep -q $NETWORK_NAME; then
    echo "==> Creating a network $NETWORK_NAME"
    docker network create $NETWORK_NAME
fi

if ! docker ps | awk '{print $2}' | grep -q $PROXY_NAME; then
    echo "==> Starting Squid Proxy server"
    docker run --net proxytest --ip $PROXY_IP -d squid
fi

echo "==> Starting SnowSQL tests"
docker run --net proxytest \
    --privileged \
    -e "SNOWSQL_ACCOUNT=$SNOWSQL_ACCOUNT" \
    -e "SNOWSQL_USER=$SNOWSQL_USER" \
    -e "SNOWSQL_PWD=$SNOWSQL_PWD" \
    -it test-snowsql /bin/bash -c "/root/run_snowsql_proxy_test.sh"
