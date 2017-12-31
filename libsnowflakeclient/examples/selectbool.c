/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <snowflake/client.h>
#include <example_setup.h>


typedef struct test_case_to_string {
    const int64 c1in;
    const sf_bool *c2in;
    const char *c2out;
    const sf_bool c2_is_null;
} TEST_CASE_TO_STRING;

int main() {
    /* init */
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection();

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto error_conn;
    }

    /* Create a statement once and reused */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    /* NOTE: the numeric type here should fit into int64 otherwise
     * it is taken as a float */
    status = snowflake_query(
      sfstmt,
      "create or replace table t (c1 int, c2 boolean)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    /* insert data */
    status = snowflake_prepare(
      sfstmt,
      "insert into t(c1,c2) values(?,?)",
      0);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    const sf_bool large_value = (sf_bool) 64;
    const sf_bool zero_value = (sf_bool) 0;
    const sf_bool negative_value = (sf_bool) -12;

    TEST_CASE_TO_STRING test_cases[] = {
      {.c1in = 1, .c2in = &SF_BOOLEAN_TRUE, .c2out = "1", .c2_is_null=SF_BOOLEAN_FALSE},
      {.c1in = 2, .c2in = &SF_BOOLEAN_FALSE, .c2out = "", .c2_is_null=SF_BOOLEAN_FALSE},
      {.c1in = 3, .c2in = &large_value, .c2out = "1", .c2_is_null=SF_BOOLEAN_FALSE},
      {.c1in = 4, .c2in = &zero_value, .c2out = "", .c2_is_null=SF_BOOLEAN_FALSE},
      {.c1in = 5, .c2in = NULL, .c2out = "", .c2_is_null=SF_BOOLEAN_TRUE},
      {.c1in = 6, .c2in = &negative_value, .c2out = "1", .c2_is_null=SF_BOOLEAN_FALSE}
    };

    size_t i;
    size_t len;
    for (i = 0, len = sizeof(test_cases) / sizeof(TEST_CASE_TO_STRING);
         i < len; i++) {
        TEST_CASE_TO_STRING v = test_cases[i];

        SF_BIND_INPUT ic1;
        int64 ic1buf = v.c1in;
        ic1.idx = 1;
        ic1.c_type = SF_C_TYPE_INT64;
        ic1.value = (void *) &ic1buf;
        ic1.len = sizeof(ic1buf);
        status = snowflake_bind_param(sfstmt, &ic1);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        SF_BIND_INPUT ic2;
        ic2.idx = 2;
        ic2.c_type = SF_C_TYPE_BOOLEAN;
        ic2.value = (void *) v.c2in;
        ic2.len = sizeof(sf_bool);
        status = snowflake_bind_param(sfstmt, &ic2);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }
        status = snowflake_execute(sfstmt);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }
        printf("Inserted one row\n");
    }

    /* query */
    status = snowflake_query(sfstmt, "select * from t order by 1", 0);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    SF_BIND_OUTPUT c1 = {0};
    char c1buf[1024];
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_STRING;
    c1.value = (void *) c1buf;
    c1.len = sizeof(c1buf);
    c1.max_length = sizeof(c1buf);
    status = snowflake_bind_result(sfstmt, &c1);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    SF_BIND_OUTPUT c2 = {0};
    char c2buf[1024];
    c2.idx = 2;
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = (void *) c2buf;
    c2.len = sizeof(c2buf);
    c2.max_length = sizeof(c2buf);
    status = snowflake_bind_result(sfstmt, &c2);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) == SF_STATUS_SUCCESS) {
        TEST_CASE_TO_STRING v = test_cases[atoll(c1.value) - 1];
        printf("result: %s, '%s(%s)'\n",
               (char *) c1.value, (char *) c2.value,
               c2.is_null ? "NULL" : "NOT NULL");
        if (v.c2out != NULL && strcmp(v.c2out, c2.value) != 0 &&
            v.c2_is_null == c2.is_null) {
            fprintf(stderr, "ERROR: testcase: %s,"
                      " expected: %s(%s), got %s(%s)\n",
                    (char *) c1.value, v.c2out,
                    v.c2_is_null ? "NULL" : "NOT NULL",
                    (char *) c2.value,
                    c2.is_null ? "NULL" : "NOT NULL");
        }
    }

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
        goto error_stmt;
    }
    status = snowflake_query(sfstmt, "drop table if exists t", 0);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }
    status = SF_STATUS_SUCCESS;
    goto cleanup;
error_stmt:
    {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }
error_conn:
    {
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

cleanup:
    /* delete statement */
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
