#!/bin/bash -e
#
# Jenkins helper for WIF e2e tests. Builds the extension for linux/amd64,
# then runs the existing PHPT files on the remote test hosts.

set -o pipefail

export THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export REPO_ROOT="$(cd "${THIS_DIR}/.." && pwd )"
export ARTIFACT_DIR="$THIS_DIR/wif/artifacts"
GPG_PARAMETERS="${THIS_DIR}/wif/parameters/parameters_wif.json.gpg"

WIF_BUILD_IMAGE="${WIF_BUILD_IMAGE:-php:8.2-cli}"
WIF_RUNTIME_IMAGE="${WIF_RUNTIME_IMAGE:-php:8.2-cli}"

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BUILD_ID="${BUILD_NUMBER:-local}"

SSH_BASE_OPTS=(-o IdentitiesOnly=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null)

quote_env() {
  printf '%s=%q' "$1" "$2"
}

# Run a remote bash script with env NAME=quoted ... and a quoted heredoc.
ssh_env_bash() {
  local host="$1"
  local key="$2"
  shift 2
  local spec
  local -a env_args=()
  while [[ $# -gt 0 ]]; do
    spec="$1"
    shift
    env_args+=("$(quote_env "${spec%%=*}" "${spec#*=}")")
  done
  ssh -i "$key" "${SSH_BASE_OPTS[@]}" -p 443 "$host" env "${env_args[@]}" bash
}

scp_to_host() {
  local key="$1"
  local host="$2"
  local src="$3"
  local dest="$4"
  scp -P 443 -i "$key" "${SSH_BASE_OPTS[@]}" "$src" "$host:$dest"
}

get_branch() {
  local branch
  if [[ -n "${GIT_BRANCH:-}" ]]; then
    branch="${GIT_BRANCH}"
  else
    branch=$(git -C "$REPO_ROOT" rev-parse --abbrev-ref HEAD)
  fi
  branch="${branch#remotes/origin/}"
  branch="${branch#origin/}"
  echo "${branch}"
}

cleanup_wif() {
  rm -f "${PARAMETERS_FILE_PATH:-}"
  rm -f "${KEY_AWS_AZURE:-}" "${KEY_GCP:-}"
  if declare -F cleanup_gpg >/dev/null; then
    cleanup_gpg
  fi
}

setup_parameters() {
  source "$THIS_DIR/scripts/setup_gpg.sh"
  trap cleanup_wif EXIT
  if [[ -z "${PARAMETERS_SECRET:-}" ]]; then
    echo "ERROR: PARAMETERS_SECRET is not set" >&2
    exit 1
  fi
  PARAMETERS_FILE_PATH="$(mktemp)"
  gpg --quiet --batch --yes --decrypt \
    --passphrase="$PARAMETERS_SECRET" \
    --output "$PARAMETERS_FILE_PATH" \
    "$GPG_PARAMETERS"
  eval "$(jq -r '.wif | to_entries | map("export \(.key)=\(.value|tostring)")|.[]' "$PARAMETERS_FILE_PATH")"
}

require_ssh_keys() {
  : "${WIF_SSH_KEY_AWS_AZURE_FILE:?WIF_SSH_KEY_AWS_AZURE_FILE must be set}"
  : "${WIF_SSH_KEY_GCP_FILE:?WIF_SSH_KEY_GCP_FILE must be set}"
  if [[ ! -f "$WIF_SSH_KEY_AWS_AZURE_FILE" ]]; then
    echo "ERROR: AWS/Azure SSH key file not found: $WIF_SSH_KEY_AWS_AZURE_FILE" >&2
    exit 1
  fi
  if [[ ! -f "$WIF_SSH_KEY_GCP_FILE" ]]; then
    echo "ERROR: GCP SSH key file not found: $WIF_SSH_KEY_GCP_FILE" >&2
    exit 1
  fi
  KEY_AWS_AZURE="$(mktemp)"
  KEY_GCP="$(mktemp)"
  install -m 600 "$WIF_SSH_KEY_AWS_AZURE_FILE" "$KEY_AWS_AZURE"
  install -m 600 "$WIF_SSH_KEY_GCP_FILE" "$KEY_GCP"
}

# Build from an isolated copy of HEAD so parallel Jenkins stages do not
# race this workspace, targeting linux/amd64 to match the test hosts.
build_wif_artifacts() {
  mkdir -p "$ARTIFACT_DIR"
  rm -f "$ARTIFACT_DIR/pdo_snowflake.so" "$ARTIFACT_DIR/run-tests.php" "$ARTIFACT_DIR/cacert.pem"

  local src
  src="$(mktemp -d "${TMPDIR:-/tmp}/pdo-wif-src.XXXXXX")"
  git -C "$REPO_ROOT" archive HEAD | tar -x -C "$src"

  echo "==================================================================="
  echo "Building pdo_snowflake.so for linux/amd64 in ${WIF_BUILD_IMAGE}"
  echo "  host arch: $(uname -m)"
  echo "==================================================================="

  docker pull --platform linux/amd64 "$WIF_BUILD_IMAGE"
  docker run --rm --platform linux/amd64 \
    -v "$src":/src \
    -v "$ARTIFACT_DIR":/out \
    -e PHP_HOME=/usr/local \
    "$WIF_BUILD_IMAGE" \
    bash -c '
      set -euo pipefail
      apt-get update
      # PHPIZE_DEPS is defined by the official php image (autoconf, g++, make, ...).
      # shellcheck disable=SC2086
      apt-get install -y --no-install-recommends $PHPIZE_DEPS cmake file
      export PHP_HOME=/usr/local
      export PATH=/usr/local/bin:$PATH
      cd /src
      scripts/build_pdo_snowflake.sh
      test -f modules/pdo_snowflake.so
      test -f run-tests.php
      echo "=== file modules/pdo_snowflake.so ==="
      file modules/pdo_snowflake.so
      echo "=== ldd modules/pdo_snowflake.so ==="
      ldd modules/pdo_snowflake.so || true
      if ! file modules/pdo_snowflake.so | grep -Eq "x86-64|x86_64"; then
        echo "ERROR: extension is not linux/amd64" >&2
        file modules/pdo_snowflake.so
        exit 1
      fi
      cp modules/pdo_snowflake.so /out/pdo_snowflake.so
      cp run-tests.php /out/run-tests.php
      cp libsnowflakeclient/cacert.pem /out/cacert.pem
    '
  rm -rf "$src"

  echo "Artifacts:"
  ls -l "$ARTIFACT_DIR/pdo_snowflake.so" "$ARTIFACT_DIR/run-tests.php" "$ARTIFACT_DIR/cacert.pem"
}

run_wif_tests() {
  local cloud_provider="$1"
  local host="$2"
  local snowflake_host="$3"
  local rsa_key_path="$4"
  local snowflake_user="$5"
  local impersonation_path="$6"
  local snowflake_user_for_impersonation="$7"

  local provider_lower
  provider_lower=$(echo "$cloud_provider" | tr '[:upper:]' '[:lower:]')
  local test_file="wif_auth_${provider_lower}.phpt"
  local remote_dir="pdo_wif_${provider_lower}_${BUILD_ID}_${TIMESTAMP}"

  echo "==================================================================="
  echo "WIF tests: ${cloud_provider}  (host=${host}, remote_dir=${remote_dir})"
  echo "==================================================================="

  ssh_env_bash "$host" "$rsa_key_path" \
    WIF_TEST_DIR="$remote_dir" \
    <<'EOF'
set -e
set -o pipefail
mkdir -p "$WIF_TEST_DIR"
echo "Created test directory: $WIF_TEST_DIR"
EOF

  scp_to_host "$rsa_key_path" "$host" "$ARTIFACT_DIR/pdo_snowflake.so" "$remote_dir/pdo_snowflake.so"
  scp_to_host "$rsa_key_path" "$host" "$ARTIFACT_DIR/cacert.pem" "$remote_dir/cacert.pem"
  scp_to_host "$rsa_key_path" "$host" "$REPO_ROOT/tests/$test_file" "$remote_dir/$test_file"
  scp_to_host "$rsa_key_path" "$host" "$REPO_ROOT/tests/wif_helper.php" "$remote_dir/wif_helper.php"
  scp_to_host "$rsa_key_path" "$host" "$ARTIFACT_DIR/run-tests.php" "$remote_dir/run-tests.php"

  ssh_env_bash "$host" "$rsa_key_path" \
    BRANCH="$BRANCH" \
    SNOWFLAKE_TEST_WIF_HOST="$snowflake_host" \
    SNOWFLAKE_TEST_WIF_PROVIDER="$cloud_provider" \
    SNOWFLAKE_TEST_WIF_ACCOUNT="$SNOWFLAKE_TEST_WIF_ACCOUNT" \
    SNOWFLAKE_TEST_WIF_USERNAME="$snowflake_user" \
    SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH="$impersonation_path" \
    SNOWFLAKE_TEST_WIF_USERNAME_IMPERSONATION="$snowflake_user_for_impersonation" \
    SNOWFLAKE_WIF_TEST_REQUIRED="true" \
    WIF_TEST_DIR="$remote_dir" \
    TEST_FILE="$test_file" \
    RUNTIME_IMAGE="$WIF_RUNTIME_IMAGE" \
    <<'EOF'
set -e
set -o pipefail
docker run \
  --rm \
  --cpus=1 \
  -m 1g \
  -e BRANCH \
  -e SNOWFLAKE_TEST_WIF_PROVIDER \
  -e SNOWFLAKE_TEST_WIF_HOST \
  -e SNOWFLAKE_TEST_WIF_ACCOUNT \
  -e SNOWFLAKE_TEST_WIF_USERNAME \
  -e SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH \
  -e SNOWFLAKE_TEST_WIF_USERNAME_IMPERSONATION \
  -e SNOWFLAKE_WIF_TEST_REQUIRED \
  -e TEST_FILE \
  -v "$HOME/$WIF_TEST_DIR":/pdo_tests/tests_dir \
  "$RUNTIME_IMAGE" \
  bash -c '
    echo "Running $TEST_FILE on branch: $BRANCH"
    mkdir -p /etc/pki/tls/certs && ln -sf /pdo_tests/tests_dir/cacert.pem /etc/pki/tls/certs/ca-bundle.crt
    cd /pdo_tests/tests_dir
    if ! php run-tests.php -d extension=/pdo_tests/tests_dir/pdo_snowflake.so "$TEST_FILE"; then
      echo "===================Tests failed==================="
      echo "Displaying test failure details:"
      BASE=$(basename "$TEST_FILE" .phpt)
      for ext in out log diff; do
        if [ -f "$BASE.$ext" ]; then
          echo "===================$BASE.$ext==================="
          cat "$BASE.$ext"
        fi
      done
      exit 1
    fi
  '
EOF
}

run_tests_and_set_result() {
  local provider="$1"
  local host="$2"
  local snowflake_host="$3"
  local rsa_key_path="$4"
  local snowflake_user="$5"
  local impersonation_path="$6"
  local snowflake_user_for_impersonation="$7"
  local provider_lower
  provider_lower=$(echo "$provider" | tr '[:upper:]' '[:lower:]')
  local remote_dir="pdo_wif_${provider_lower}_${BUILD_ID}_${TIMESTAMP}"

  if run_wif_tests "$provider" "$host" "$snowflake_host" "$rsa_key_path" \
      "$snowflake_user" "$impersonation_path" "$snowflake_user_for_impersonation"; then
    echo "$provider tests passed"
  else
    echo "$provider tests failed"
    EXIT_STATUS=1
  fi

  ssh_env_bash "$host" "$rsa_key_path" \
    WIF_TEST_DIR="$remote_dir" \
    <<'EOF' || true
set -o pipefail
rm -rf "$WIF_TEST_DIR" || echo "Warning: Failed to cleanup test directory, continuing anyway"
EOF
}

BRANCH=$(get_branch)
export BRANCH
trap cleanup_wif EXIT
require_ssh_keys
setup_parameters
# setup_gpg.sh installs its own EXIT trap; replace it with the combined cleanup.
trap cleanup_wif EXIT
build_wif_artifacts

EXIT_STATUS=0
# Azure tests do not pass impersonation arguments.
run_tests_and_set_result "AZURE" "$HOST_AZURE" "$SNOWFLAKE_TEST_WIF_HOST_AZURE" \
  "$KEY_AWS_AZURE" "$SNOWFLAKE_TEST_WIF_USERNAME_AZURE" "" ""
run_tests_and_set_result "AWS" "$HOST_AWS" "$SNOWFLAKE_TEST_WIF_HOST_AWS" \
  "$KEY_AWS_AZURE" "$SNOWFLAKE_TEST_WIF_USERNAME_AWS" \
  "${SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH_AWS:-}" \
  "${SNOWFLAKE_TEST_WIF_USERNAME_AWS_IMPERSONATION:-}"
run_tests_and_set_result "GCP" "$HOST_GCP" "$SNOWFLAKE_TEST_WIF_HOST_GCP" \
  "$KEY_GCP" "$SNOWFLAKE_TEST_WIF_USERNAME_GCP" \
  "${SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH_GCP:-}" \
  "${SNOWFLAKE_TEST_WIF_USERNAME_GCP_IMPERSONATION:-}"

echo "Exit status: $EXIT_STATUS"
exit $EXIT_STATUS
