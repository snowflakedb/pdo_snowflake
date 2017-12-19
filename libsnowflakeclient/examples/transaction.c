/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <string.h>
#include <snowflake_client.h>
#include "example_setup.h"


int main() {
    SF_ERROR *error;
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection_with_autocommit(
      SF_BOOLEAN_FALSE);
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto err_con;
    }

    error = NULL;
    SF_STMT *sfstmt = NULL;

    /* execute a DML */
    sfstmt = snowflake_stmt(sf);
    status = snowflake_query(
      sfstmt,
      "create or replace table t(c1 number(10,0), c2 string)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }

    /* begin transaction */
    status = snowflake_trans_begin(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto err_con;
    }

    status = snowflake_prepare(sfstmt, "INSERT INTO t(c1,c2) values(?,?)", 0);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    SF_BIND_INPUT p1, p2;

    /* insert one row */
    int64 v = 3;
    p1.idx = 1;
    p1.c_type = SF_C_TYPE_INT64;
    p1.value = (void *) &v;
    status = snowflake_bind_param(sfstmt, &p1);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }

    p2.idx = 2;
    p2.c_type = SF_C_TYPE_STRING;
    p2.value = (void *) "test2";
    p2.len = strlen(p2.value);
    status = snowflake_bind_param(sfstmt, &p2);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    if (snowflake_execute(sfstmt) != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    printf("Success. Query ID: %s, Affected Rows: %ld\n",
           snowflake_sfqid(sfstmt), (long) snowflake_affected_rows(sfstmt));
    status = snowflake_trans_commit(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto err_con;
    }

    /* insert additional row */
    v = 5;
    p1.idx = 1;
    p1.c_type = SF_C_TYPE_INT64;
    p1.value = (void *) &v;
    status = snowflake_bind_param(sfstmt, &p1);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }

    p2.idx = 2;
    p2.c_type = SF_C_TYPE_STRING;
    p2.value = (void *) "test4";
    p2.len = strlen(p2.value);
    status = snowflake_bind_param(sfstmt, &p2);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    if (snowflake_execute(sfstmt) != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    printf("Success. Query ID: %s, Affected Rows: %ld\n",
           snowflake_sfqid(sfstmt), (long) snowflake_affected_rows(sfstmt));

    /* fetch result */
    status = snowflake_query(sfstmt, "select c1, c2 from t order by 1 desc", 0);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    SF_BIND_OUTPUT v1;

    v1.idx = 1;
    v1.type = SF_C_TYPE_INT64;
    v1.value = &v;
    v1.max_length = sizeof(v1);
    status = snowflake_bind_result(sfstmt, &v1);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }

    status = snowflake_fetch(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    if (v != 5) {
        printf("Wrong result: %lld\n", v);
        goto end;
    }
    printf("Result: %lld\n", v);
    status = snowflake_trans_rollback(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto err_con;
    }

    /* fetch result (2nd) */
    status = snowflake_query(sfstmt, "select c1, c2 from t order by 1 desc", 0);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    status = snowflake_bind_result(sfstmt, &v1);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    status = snowflake_fetch(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    if (v != 3) {
        printf("Wrong result: %lld\n", v);
        goto end;
    }
    printf("Result: %lld\n", v);

    status = snowflake_query(
      sfstmt,
      "drop table if exists t",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        goto err_stmt;
    }
    status = SF_STATUS_SUCCESS;
    goto end;

err_stmt: /* error */
    /* SF_ERROR structure is included in a SF_STMT, so you don't
     * need to free the memory. */
    error = snowflake_stmt_error(sfstmt);
    printf("Error. Query ID: %s, Message: %s\n", error->sfqid, error->msg);
    status = snowflake_trans_rollback(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto err_con;
    }
    goto end;
err_con:
    error = snowflake_error(sf);
    printf("Error. Query ID: %s, Message: %s\n", error->sfqid, error->msg);
    goto end;
end: /* finally */
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    return status;
}
