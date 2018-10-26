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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

if ! docker network ls | awk '{print $2}' | grep -q $NETWORK_NAME; then
    echo "==> Creating a network $NETWORK_NAME"
    docker network create --subnet 172.20.0.0/16 --ip-range 172.20.240.0/20 $NETWORK_NAME
else
    echo "==> $NETWORK_NAME network already up"
fi

if ! docker ps | awk '{print $2}' | grep -q $PROXY_NAME; then
    echo "==> Starting Squid Proxy server"
    docker run --net proxytest --ip $PROXY_IP -d squid
else
    echo "==> Squid Proxy server already up"
fi

echo "==> Proxy Ready for use"
