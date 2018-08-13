#!/bin/bash -e
#
# Build Squid proxy server image
#

set -o pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR
# Squid without auth
docker build --rm -t squid .
# Squid with auth
docker build -f Dockerfile.auth --rm -t squid-auth .
