/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <snowflake_client.h>
#include <example_setup.h>

/**
 * Example of simple CRUD
 */
int main() {
    int ret = -1;
    SNOWFLAKE_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);

    /* Connect with all parameters set */
    SNOWFLAKE *sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "connecting to snowflake failed\n");
        goto error_con;
    }

    /* Create a statement once and reused */
    SNOWFLAKE_STMT *stmt = snowflake_stmt(sf);
    /* NOTE: the numeric type here should fit into int64 otherwise
     * it is taken as a float */
    status = snowflake_query(
      stmt,
      "create or replace table t (c1 number(10,0), c2 string)");
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to create a table\n");
        goto error_stmt;
    }

    status = snowflake_query(
      stmt,
      "insert into t values(1, 'test1'),(2, 'test2')"
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to insert a table\n");
        goto error_stmt;
    }

    status = snowflake_query(
      stmt,
      "select * from t"
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to select a table\n");
        goto error_stmt;
    }
    printf("len = %lld\n", stmt->total_fieldcount);

    int64 c1v = 0;
    SNOWFLAKE_BIND_OUTPUT c1;
    c1.idx = 1;
    c1.max_length = sizeof(c1v);
    c1.type = SF_C_TYPE_INT64;
    c1.value = &c1v;
    status = snowflake_bind_result(stmt, &c1);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind c1\n");
        goto error_stmt;
    }

    char c2v[1000];
    SNOWFLAKE_BIND_OUTPUT c2;
    c2.idx = 2;
    c2.max_length = sizeof(c2v);
    c2.type = SF_C_TYPE_STRING;
    c2.value = &c2v;
    status = snowflake_bind_result(stmt, &c2);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind c2\n");
        goto error_stmt;
    }

    int64 total = 0;
    while ((status = snowflake_fetch(stmt)) == SF_STATUS_SUCCESS) {
        printf("c1: %d, c2: %s\n", c1v, c2v);
        total += c1v;
    }
    if (total != 3) {
        fprintf(
          stderr,
          "failed to get the all values. expected: 3, got :%lld", total);
        goto error_stmt;
    }

    if (status != SF_STATUS_EOL) {
        fprintf(stderr, "failed to fetch data\n");
        goto error_stmt;
    }
    ret = 0;

    error_stmt: /* error stmt */
    snowflake_stmt_close(stmt);

    error_con: /* error connection */
    snowflake_close(sf);
    snowflake_term(sf);
    snowflake_global_term();
    return ret;
}