#!/bin/bash
#
# Safe script to generate testenv.ini without killing your terminal
#

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$( cd "$DIR/.." && pwd )"
PARAMETER_FILE="$REPO_ROOT/parameters.json"
TESTENV_FILE="$REPO_ROOT/testenv.ini"

# Check if parameters.json exists
if [[ ! -e "$PARAMETER_FILE" ]]; then
    echo "ERROR: The parameter file doesn't exist: $PARAMETER_FILE"
    echo "Please create it first:"
    echo "  cp parameters.json.template parameters.json"
    echo "  nano parameters.json"
    exit 1
fi

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "ERROR: jq is not installed"
    echo "Please install it:"
    echo "  brew install jq"
    exit 1
fi

echo "==> Generating testenv.ini from parameters.json"

# Export all test connection parameters
eval $(jq -r '.testconnection | to_entries | map("export \(.key)=\"\(.value|tostring)\"")|.[]' "$PARAMETER_FILE")

# Save to testenv.ini (INI format for PHP parse_ini_file)
echo "# Auto-generated test environment file" > "$TESTENV_FILE"
echo "# Generated: $(date)" >> "$TESTENV_FILE"
echo "" >> "$TESTENV_FILE"

# Export each variable and save to file
jq -r '.testconnection | to_entries | map("\(.key)=\(.value|tostring)")|.[]' "$PARAMETER_FILE" >> "$TESTENV_FILE"

echo "==> Test Connection Parameters (without passwords):"
cat "$TESTENV_FILE" | grep -v PASSWORD | grep -v PRIVATE_KEY_PASSWORD

echo ""
echo "✓ Successfully created: $TESTENV_FILE"
echo ""

