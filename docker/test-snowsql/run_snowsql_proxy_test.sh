#!/bin/bash -e
#
# Run a SnowSQL test
#
set -o pipefail

source ~/iptables.txt

echo "==> iptables"
iptables -L

echo "==> checking noproxy connection that should not work"
if curl --noproxy "*" https://www.snowflake.net/ >& /dev/null; then
    echo "The outgoing network is not blocked. The connection should require a proxy at all times."
    exit 1
fi
if [[ -z "$SNOWSQL_USER" || -z "$SNOWSQL_ACCOUNT" ]]; then
    echo "SNOWSQL_ACCOUNT and SNOWSQL_USER must set."
    exit 1
fi

if [[ -z "$SNOWSQL_PWD" ]]; then
    echo "SNOWSQL_PWD must set the password for [$SNOWSQL_ACCOUNT.$SNOWSQL_USER]."
    exit 1
fi
echo "==> running a simple query"
/root/bin/snowsql -a $SNOWSQL_ACCOUNT -u $SNOWSQL_USER -q 'select 1'
