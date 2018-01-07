#!/bin/bash -e
#
# Run C API tests
#
function usage() {
    echo "Usage: `basename $0` [-m]"
    echo "-m                 : Use Valgrind. default: no valgrind"
    exit 2
}

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

VALGRIND_CMD=(
    "valgrind"
    "--tool=memcheck"
    "--leak-check=full"
    "--error-exitcode=1"
    "--run-libc-freeres=no"
)

use_valgrind=()
while getopts "hm" opt; do
  case $opt in
    m) use_valgrind=("${VALGRIND_CMD[@]}") ;;
    h) usage;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    :) echo "Option -$OPTARG requires an argument." >&2; exit 1 ;;
  esac
done

for f in $(ls $DIR/../cmake-build/examples/ex_*); do
    echo -e "\n==> $f"
    echo "${use_valgrind[@]}" $f
    "${use_valgrind[@]}" $f
done
