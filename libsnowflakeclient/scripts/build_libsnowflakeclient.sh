#!/bin/bash -e
#
# Build Snowflake Client library
#
function usage() {
    echo "Usage: `basename $0` [-r] [-p]"
    echo "-p                 : Rebuild Snowflake Client with profile option. default: no profile"
    exit 2
}
set -o pipefail

export BUILD_WITH_PROFILE_OPTION=
while getopts "hp" opt; do
  case $opt in
    p) export BUILD_WITH_PROFILE_OPTION=true ;;
    h) usage;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    :) echo "Option -$OPTARG requires an argument." >&2; exit 1 ;;
  esac
done

# main
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..
rm -rf cmake-build
mkdir cmake-build
cd cmake-build
cmake ..  -G"Unix Makefiles"
make
