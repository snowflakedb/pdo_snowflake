#!/bin/bash -e
#
# Build and Test libsnowflakeclient on Ubuntu
#

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

NETWORK_NAME=proxytest
PROXY_NAME=squid
PROXY_IP=172.20.128.10
PROXY_PORT=3128

SNOWFLAKE_TEST_USER=$1
SNOWFLAKE_TEST_PASSWORD=$2
SNOWFLAKE_TEST_ACCOUNT=$3
SNOWFLAKE_TEST_WAREHOUSE=$4
SNOWFLAKE_TEST_DATABASE=$5
SNOWFLAKE_TEST_SCHEMA=$6
SNOWFLAKE_TEST_ROLE=$7

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

if ! docker network ls | awk '{print $2}' | grep -q $NETWORK_NAME; then
    echo "==> Creating a network $NETWORK_NAME"
    docker network create --subnet 172.20.0.0/16 --ip-range 172.20.240.0/20 $NETWORK_NAME
fi

if ! docker ps | awk '{print $2}' | grep -q $PROXY_NAME; then
    echo "==> Starting Squid Proxy server"
    docker run --net proxytest --ip $PROXY_IP -d squid
fi

echo "==> Starting libsnowflakeclient tests"
docker run --net proxytest \
    --privileged \
    -e "SNOWFLAKE_TEST_USER=$SNOWFLAKE_TEST_USER" \
    -e "SNOWFLAKE_TEST_PASSWORD=$SNOWFLAKE_TEST_PASSWORD" \
    -e "SNOWFLAKE_TEST_ACCOUNT=$SNOWFLAKE_TEST_ACCOUNT" \
    -e "SNOWFLAKE_TEST_WAREHOUSE=$SNOWFLAKE_TEST_WAREHOUSE" \
    -e "SNOWFLAKE_TEST_DATABASE=$SNOWFLAKE_TEST_DATABASE" \
    -e "SNOWFLAKE_TEST_SCHEMA=$SNOWFLAKE_TEST_SCHEMA" \
    -e "SNOWFLAKE_TEST_ROLE=$SNOWFLAKE_TEST_ROLE" \
    -it test-libsnowflakeclient /bin/bash -c "/root/build_run_libsnowflakeclient_proxy_test.sh"