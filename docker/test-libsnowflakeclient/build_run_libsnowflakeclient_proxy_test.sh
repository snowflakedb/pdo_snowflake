#!/bin/bash -e
#
# Build and Test libsnowflakeclient on Ubuntu
#

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

#!/bin/bash -e
#
# Run a SnowSQL test
#
set -o pipefail

source ~/iptables.txt

echo "==> iptables"
iptables -L

# Test that proxy connection is really established
echo "==> checking noproxy connection that should not work"
if curl --noproxy "*" https://www.snowflake.net/ >& /dev/null; then
    echo "The outgoing network is not blocked. The connection should require a proxy at all times."
    exit 1
fi

# Check that the proper parameters were passed into the docker image
if [[ -z "$SNOWFLAKE_TEST_USER" || -z "$SNOWFLAKE_TEST_PASSWORD" || -z "$SNOWFLAKE_TEST_ACCOUNT" || -z "$SNOWFLAKE_TEST_WAREHOUSE" || \
	-z "$SNOWFLAKE_TEST_SCHEMA" || -z "$SNOWFLAKE_TEST_DATABASE" || -z "$SNOWFLAKE_TEST_ROLE" ]]; then
    echo "SNOWFLAKE_TEST_USER, SNOWFLAKE_TEST_PASSWORD, SNOWFLAKE_TEST_ACCOUNT, " \
    	 "SNOWFLAKE_TEST_WAREHOUSE, SNOWFLAKE_TEST_SCHEMA, SNOWFLAKE_TEST_DATABASE " \
    	 "and SNOWFLAKE_TEST_ROLE must all be set."
    exit 1
fi

# Download libsnowflakeclient
echo "==> downloading libsnowflakeclient"
git clone https://github.com/snowflakedb/libsnowflakeclient.git
cd libsnowflakeclient
SNOWFLAKE_TEST_CA_BUNDLE_FILE="${DIR}/libsnowflakeclient/cacert.pem"
export SNOWFLAKE_TEST_CA_BUNDLE_FILE

# Build libsnowflakeclient and run tests
echo "==> building libsnowflakeclient"
./scripts/build_libsnowflakeclient.sh
echo "==> build complete"
echo "==> env var proxies http_proxy=${http_proxy} and https_proxy=${https_proxy}"
echo "==> running libsnowflakeclient tests"
./scripts/run_tests.sh -s

