#!/bin/bash -e
#
# Run C API tests
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for f in $(ls $DIR/../cmake-build/examples/ex_*); do
    echo -e "\n==> $f"
    $f
done
