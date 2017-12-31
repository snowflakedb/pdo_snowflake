/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <snowflake/client.h>
#include <example_setup.h>
#include <memory.h>

int fetch_data(SF_STMT *stmt, int64 expected_sum) {
    int ret = -1;
    SF_STATUS status;

    status = snowflake_query(
      stmt,
      "select * from t",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to select a table\n");
        goto exit;
    }

    int64 c1v = 0;
    SF_BIND_OUTPUT c1 = {0};
    c1.idx = 1;
    c1.max_length = sizeof(c1v);
    c1.c_type = SF_C_TYPE_INT64;
    c1.value = &c1v;
    status = snowflake_bind_result(stmt, &c1);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind c1\n");
        goto exit;
    }

    char c2v[1000];
    SF_BIND_OUTPUT c2 = {0};
    c2.idx = 2;
    c2.max_length = sizeof(c2v);
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = &c2v;
    status = snowflake_bind_result(stmt, &c2);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind c2\n");
        goto exit;
    }

    uint64 num_fields = snowflake_num_fields(stmt);
    SF_COLUMN_DESC *descs = snowflake_desc(stmt);

    if (num_fields != 2) {
        fprintf(stderr,
                "failed to get the number of fields. expected: 2, got: %llu",
                num_fields);
        goto exit;
    }

    int i;
    for (i = 0; i < num_fields; ++i) {
        printf(
          "name: %s, type: %d, C type: %d, byte_size: %lld, "
            "internal_size: %lld, precision: %lld, scale: %lld, null ok: %d\n",
          descs[i].name,
          descs[i].type,
          descs[i].c_type,
          descs[i].byte_size,
          descs[i].internal_size,
          descs[i].precision,
          descs[i].scale,
          descs[i].null_ok);
    }
    int64 total = 0;
    while ((status = snowflake_fetch(stmt)) == SF_STATUS_SUCCESS) {
        printf("c1: %lld, c2: %s\n", c1v, c2v);
        total += c1v;
    }
    if (total != expected_sum) {
        fprintf(
          stderr,
          "failed to get the all values. expected: %lld, got :%lld\n",
          expected_sum, total);
        goto exit;
    }

    if (status != SF_STATUS_EOL) {
        fprintf(stderr, "failed to fetch data\n");
        goto exit;
    }
    ret = 0;
exit:
    return ret;
}

/**
 * Example of simple CRUD
 */
int main() {
    int ret = -1;
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);

    /* Connect with all parameters set */
    SF_CONNECT *sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "connecting to snowflake failed\n");
        goto error_con;
    }

    /* Create a statement once and reused */
    SF_STMT *stmt = snowflake_stmt(sf);
    /* NOTE: the numeric type here should fit into int64 otherwise
     * it is taken as a float */
    status = snowflake_query(
      stmt,
      "create or replace table t (c1 number(10,0) not null, c2 string)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to create a table\n");
        goto error_stmt;
    }

    status = snowflake_query(
      stmt,
      "insert into t values(1, 'test1'),(2, 'test2')",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to insert a table\n");
        goto error_stmt;
    }

    printf("affected rows: %lld\n", snowflake_affected_rows(stmt));

    ret = fetch_data(stmt, 3);
    if (ret != 0) {
        goto error_stmt;
    }

    status = snowflake_prepare(stmt, "update t set c1=? where c2=?", 0);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to prepare updating\n");
        goto error_stmt;
    }

    int64 p1v = 102;
    SF_BIND_INPUT p1;
    p1.idx = 1;
    p1.c_type = SF_C_TYPE_INT64;
    p1.value = &p1v;
    p1.len = sizeof(p1v);
    status = snowflake_bind_param(stmt, &p1);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind p1\n");
        goto error_stmt;
    }

    char p2v[1000];
    strcpy(p2v, "test2");
    SF_BIND_INPUT p2;
    p2.idx = 2;
    p2.c_type = SF_C_TYPE_STRING;
    p2.value = &p2v;
    p2.len = sizeof(p2v);

    status = snowflake_bind_param(stmt, &p2);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind p1\n");
        goto error_stmt;
    }

    status = snowflake_execute(stmt);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to exec\n");
        goto error_stmt;
    }

    printf("affected rows: %lld\n", snowflake_affected_rows(stmt));

    /* update the second row */
    p1v = 101;
    p1.idx = 1;
    p1.c_type = SF_C_TYPE_INT64;
    p1.value = &p1v;
    p1.len = sizeof(p1v);
    status = snowflake_bind_param(stmt, &p1);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind p1\n");
        goto error_stmt;
    }

    strcpy(p2v, "test1");
    p2.idx = 2;
    p2.c_type = SF_C_TYPE_STRING;
    p2.value = &p2v;
    p2.len = sizeof(p2v);

    status = snowflake_bind_param(stmt, &p2);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind p1\n");
        goto error_stmt;
    }

    status = snowflake_execute(stmt);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to exec\n");
        goto error_stmt;
    }
    printf("affected rows: %lld\n", snowflake_affected_rows(stmt));

    ret = fetch_data(stmt, 203);
    if (ret != 0) {
        goto error_stmt;
    }
    status = snowflake_query(stmt, "drop table if exists t", 0);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to drop table t\n");
        goto error_stmt;
    }
    printf("OK\n");
    /* success */
    ret = 0;

error_stmt: /* error stmt */
    snowflake_stmt_term(stmt);

error_con: /* error connection */
    snowflake_term(sf);
    snowflake_global_term();
    return ret;
}
