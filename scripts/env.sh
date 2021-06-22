#!/bin/bash -e
#
# Set the environment variables for tests
#

set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PARAMETER_FILE=$( cd "$DIR/.." && pwd)/parameters.json

[[ ! -e "$PARAMETER_FILE" ]] &&  echo "The parameter file doesn't exist: $PARAMETER_FILE" && exit 1

eval $(jq -r '.testconnection | to_entries | map("export \(.key)=\"\(.value|tostring)\"")|.[]' $PARAMETER_FILE)

echo "==> Test Connection Parameters"
env | grep SNOWFLAKE | grep -v PASSWORD
