#!/usr/bin/env python
#
# Create test schema
#
import os
import sys
import snowflake.connector

travis_job_id = os.getenv('TRAVIS_JOB_ID')
if not travis_job_id:
    print("[WARN] The environment variable TRAVIS_JOB_ID is not set. No test schema will be created.")
    sys.exit(0)

test_schema = 'TRAVIS_JOB_{0}'.format(travis_job_id)

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
con.cursor().execute("create or replace schema {0}".format(test_schema))
