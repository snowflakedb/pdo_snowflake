#!/bin/bash -e
#
# Build Docker images
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

docker build -f $DIR/Dockerfile.ubuntu14 -t pdo-snowflake-ubuntu12:latest .
