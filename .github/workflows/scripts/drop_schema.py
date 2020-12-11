#!/usr/bin/env python
#
# Create test schema
#
import os
import sys
import snowflake.connector

github_run_id = os.getenv('GITHUB_RUN_ID')
if not github_run_id:
    print("[WARN] The environment variable GITHUB_RUN_ID is not set. No test schema will be dropped.")
    sys.exit(0)

test_schema = 'GITHUB_RUN_{0}'.format(github_run_id)

params = {
    'account': os.getenv("SNOWFLAKE_TEST_ACCOUNT"),
    'user': os.getenv("SNOWFLAKE_TEST_USER"),
    'password': os.getenv("SNOWFLAKE_TEST_PASSWORD"),
    'database': os.getenv("SNOWFLAKE_TEST_DATABASE"),
    'role': os.getenv("SNOWFLAKE_TEST_ROLE"),
}
host=os.getenv("SNOWFLAKE_TEST_HOST")
if host:
    params['host'] = host
port=os.getenv("SNOWFLAKE_TEST_PORT")
if port:
    params['port'] = port
protocol=os.getenv("SNOWFLAKE_TEST_PROTOCOL")
if protocol:
    params['protocol'] = protocol

con = snowflake.connector.connect(**params)
con.cursor().execute("drop schema if exists {0}".format(test_schema))
