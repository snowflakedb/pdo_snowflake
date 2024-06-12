#!/bin/bash -e
#
# Initialize varizbles

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PLATFORM=$(echo $(uname) | tr '[:upper:]' '[:lower:]')

# Find cmake, gcc and g++ on target machine. Need cmake 3.0+, gcc/g++ 4.9+
if [[ -z "$CMAKE" ]]; then
    if [[ "$(which cmake3)" ]]; then
        CMAKE="$(which cmake3)"
    elif [[ "$(which cmake)" ]]; then
        CMAKE="$(which cmake)"
    else
        echo "[ERROR] cmake not found! Please install cmake."
        exit 1
    fi
fi

if [[ -z "$GCC" || -z "$GXX" ]]; then
    GCC="$(which gcc)"
    GXX="$(which g++)"
fi

if [[ "$PLATFORM" == "darwin" ]]; then
    export CC=clang
    export CXX=clang++
    export GCC=$CC
    export GXX=$CXX
fi

export BUILD_WITH_PROFILE_OPTION=
export BUILD_SOURCE_ONLY=
target=Release
while getopts ":hpt:s" opt; do
  case $opt in
    t) target=$OPTARG ;;
    p) export BUILD_WITH_PROFILE_OPTION=true ;;
    h) usage;;
    s) export BUILD_SOURCE_ONLY=true ;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    :) echo "Option -$OPTARG requires an argument."; >&2 exit 1 ;;
  esac
done

[[ "$target" != "Debug" && "$target" != "Release" ]] && \
    echo "target must be either Debug/Release." && usage

echo "Options:"
echo "  target       = $target"
echo "PATH="$PATH
