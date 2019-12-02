#!/bin/bash -e
#
# Build Docker images for tests.
# 
# NOTE: the images don't include PDO driver for Snowflake.
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Ubuntu 18.04
docker build -f $DIR/Dockerfile.ubuntu18 -t pdo-snowflake:php7.2-ubuntu18.04 .
