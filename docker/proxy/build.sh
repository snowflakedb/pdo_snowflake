#!/bin/bash -e
#
# Build Squid proxy server image
#

set -o pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR
docker build --rm -t squid .
