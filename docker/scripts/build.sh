#!/bin/bash -e
#
# Build Docker images
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Ubuntu 14.04
docker build -f $DIR/../Dockerfile.ubuntu14 --build-arg PHP_VERSION=7.0 -t pdo-snowflake:php7.0-ubuntu14.04 .
docker build -f $DIR/../Dockerfile.ubuntu14 --build-arg PHP_VERSION=7.1 -t pdo-snowflake:php7.1-ubuntu14.04 .

# Ubuntu 16.04
docker build -f $DIR/../Dockerfile.ubuntu16 -t pdo-snowflake:php7.1-ubuntu16.04 .

# Ubuntu 18.04
docker build -f $DIR/../Dockerfile.ubuntu18 -t pdo-snowflake:php7.2-ubuntu18.04 .
