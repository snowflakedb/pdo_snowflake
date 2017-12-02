#!/bin/bash -e
#
# Build Snowflake Client library
#
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..
rm -rf cmake-build
mkdir cmake-build
cd cmake-build
cmake ..  -G"Unix Makefiles"
make
