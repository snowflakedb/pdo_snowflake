#!/bin/bash -e

set -o pipefail
export THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export RSA_KEY_PATH_AWS_AZURE="/tmp/rsa_wif_aws_azure"
export RSA_KEY_PATH_GCP="/tmp/rsa_wif_gcp"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
WIF_TEST_DIR="pdo_tests/${GIT_BRANCH}_${TIMESTAMP}"

# SSH options to bypass host key verification (required for CI/CD where runners change)
SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o IdentitiesOnly=yes"

run_wif_tests() {
  local cloud_provider="$1"
  local host="$2"
  local snowflake_host="$3"
  local rsa_key_path="$4"

  ssh -i "$rsa_key_path" $SSH_OPTS -p 443 "$host" env WIF_TEST_DIR="$WIF_TEST_DIR" bash << EOF
    set -e
    set -o pipefail

    mkdir -p pdo_tests
    mkdir -p "\$WIF_TEST_DIR"
    echo "Created test directory: \$WIF_TEST_DIR"
EOF

  scp -P 443 -i "$rsa_key_path" $SSH_OPTS "./modules/pdo_snowflake.so" "$host:$WIF_TEST_DIR/pdo_snowflake.so"
  scp -P 443 -i "$rsa_key_path" $SSH_OPTS "./libsnowflakeclient/cacert.pem" "$host:$WIF_TEST_DIR/cacert.pem"
  scp -P 443 -i "$rsa_key_path" $SSH_OPTS "./tests/wif_auth.phpt" "$host:$WIF_TEST_DIR/wif_auth.phpt"
  scp -P 443 -i "$rsa_key_path" $SSH_OPTS "./run-tests.php" "$host:$WIF_TEST_DIR/run-tests.php"


ssh -i "$rsa_key_path" $SSH_OPTS -p 443 "$host" env BRANCH="$GIT_BRANCH" SNOWFLAKE_TEST_WIF_HOST="$snowflake_host" SNOWFLAKE_TEST_WIF_PROVIDER="$cloud_provider" SNOWFLAKE_TEST_WIF_ACCOUNT="$SNOWFLAKE_TEST_WIF_ACCOUNT" WIF_TEST_DIR="$WIF_TEST_DIR" bash << EOF
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
      -v "./$WIF_TEST_DIR":/pdo_tests/tests_dir \
      php:8.2-cli \
        bash -c "
          echo \"Running tests on branch: \$BRANCH\"
          cd /pdo_tests/tests_dir
          if ! php run-tests.php -d extension=/pdo_tests/tests_dir/pdo_snowflake.so wif_auth.phpt; then
              echo \"===================Tests failed===================\"
              echo \"Displaying test failure details:\"
              if [ -f wif_auth.out ]; then
                echo \"===================wif_auth.out===================\"
                cat wif_auth.out
              fi
              if [ -f wif_auth.log ]; then
                echo \"===================wif_auth.log===================\"
                cat wif_auth.log
              fi
              if [ -f wif_auth.diff ]; then
                echo \"===================wif_auth.diff===================\"
                cat wif_auth.diff
              fi
              exit 1
          fi
        "
EOF
}

run_tests_and_set_result() {
  local provider="$1"
  local host="$2"
  local snowflake_host="$3"
  local rsa_key_path="$4"

  run_wif_tests "$provider" "$host" "$snowflake_host" "$rsa_key_path"
  local status=$?

  if [[ $status -ne 0 ]]; then
    echo "$provider tests failed with exit status: $status"
    EXIT_STATUS=1
  else
    echo "$provider tests passed"
  fi
  ssh -i "$rsa_key_path" $SSH_OPTS -p 443 "$host" env WIF_TEST_DIR="$WIF_TEST_DIR" bash << EOF
      set -o pipefail
      rm -rf "\$WIF_TEST_DIR" || echo "Warning: Failed to cleanup test directory, continuing anyway"
EOF
}

# Ensure trailing newline
echo -n "$WIF_SSH_KEY_AWS_AZURE" > "$RSA_KEY_PATH_AWS_AZURE"
echo "" >> "$RSA_KEY_PATH_AWS_AZURE"
echo -n "$WIF_SSH_KEY_GCP" > "$RSA_KEY_PATH_GCP"
echo "" >> "$RSA_KEY_PATH_GCP"
chmod 600 "$RSA_KEY_PATH_AWS_AZURE"
chmod 600 "$RSA_KEY_PATH_GCP"

# Run tests for all cloud providers
EXIT_STATUS=0
#set +e  # Don't exit on first failure
run_tests_and_set_result "AZURE" "$HOST_AZURE" "$SNOWFLAKE_TEST_WIF_HOST_AZURE" "$RSA_KEY_PATH_AWS_AZURE"
run_tests_and_set_result "AWS" "$HOST_AWS" "$SNOWFLAKE_TEST_WIF_HOST_AWS" "$RSA_KEY_PATH_AWS_AZURE"
run_tests_and_set_result "GCP" "$HOST_GCP" "$SNOWFLAKE_TEST_WIF_HOST_GCP" "$RSA_KEY_PATH_GCP"

set -e  # Re-enable exit on error
echo "Exit status: $EXIT_STATUS"
exit $EXIT_STATUS

