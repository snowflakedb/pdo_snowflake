#!/bin/bash -e
#
# Build Docker images
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Ubuntu 14.04
docker build -f $DIR/Dockerfile.ubuntu14 -t pdo-snowflake-ubuntu14 .

# Ubuntu 16.04
docker build -f $DIR/Dockerfile.ubuntu16 -t pdo-snowflake-ubuntu16 .
