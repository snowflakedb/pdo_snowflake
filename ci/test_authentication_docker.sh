#!/bin/bash -e
#
# Run authentication tests in Docker
# This script decrypts test parameters and launches the Docker container
#

set -o pipefail

export THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export REPO_ROOT="$(cd "${THIS_DIR}/.." && pwd)"
export WORKSPACE=${WORKSPACE:-/tmp}
export INTERNAL_REPO=${INTERNAL_REPO:-nexus.int.snowflakecomputing.com:8086}

echo "========================================"
echo "PHP Authentication Tests - Docker Runner"
echo "========================================"
echo ""

# Setup GPG if script exists
echo "Setting up gpg..."
source "$THIS_DIR/scripts/setup_gpg.sh"

# Check if running in Jenkins
if [[ -n "$JENKINS_HOME" ]]; then
  echo "Running in Jenkins environment"
  
  # Login to internal Docker registry
  source "$THIS_DIR/scripts/login_internal_docker.sh"
fi

# Decrypt parameters file
PARAM_FILE="${REPO_ROOT}/.github/workflows/parameters/private/parameters_aws_auth_tests.json"
PARAM_FILE_GPG="${PARAM_FILE}.gpg"

if [ -f "$PARAM_FILE_GPG" ]; then
    if [ -z "$PARAMETERS_SECRET" ]; then
        echo "ERROR: PARAMETERS_SECRET environment variable not set"
        exit 1
    fi

    echo "Decrypting test parameters..."
    gpg --quiet --batch --yes --decrypt \
      --passphrase="$PARAMETERS_SECRET" \
      --output "$PARAM_FILE" \
      "$PARAM_FILE_GPG"
    
    echo "Parameters decrypted successfully"
else
    echo "[ERROR]: Parameters file not found: $PARAM_FILE_GPG"
    exit 1
fi

echo ""
echo "Launching Docker container..."
echo "  Image: ${INTERNAL_REPO}/docker/snowdrivers-test-external-browser-php:1"
echo "  Workspace: ${WORKSPACE}"
echo ""

# Run tests in Docker
docker run \
  -v "${REPO_ROOT}:/mnt/host" \
  -v "${WORKSPACE}:/mnt/workspace" \
  -e GITHUB_ACTIONS \
  --rm \
  ${INTERNAL_REPO}/docker/snowdrivers-test-external-browser-php:1 \
  "/mnt/host/ci/container/test_authentication.sh"
