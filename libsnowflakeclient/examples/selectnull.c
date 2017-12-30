/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <string.h>
#include <snowflake_client.h>
#include <example_setup.h>
#include <stdlib.h>

typedef struct test_case_to_string {
    const int64 c1in;
    const char *c2in;
    const int64 *c3in;
    const sf_bool *c4in;

    const char *c2out;
    const sf_bool c2_is_null;
    const char *c3out;
    const sf_bool c3_is_null;
    const char *c4out;
    const sf_bool c4_is_null;
} TEST_CASE_TO_STRING;


int main() {
    /* init */
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE); // no autocommit
    SF_CONNECT *sf = setup_snowflake_connection();

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto error_conn;
    }

    /* Create a statement once and reused */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    status = snowflake_query(
      sfstmt,
      "create or replace table t (c1 int, c2 string, c3 number(18,0), c4 boolean)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    /* insert data */
    status = snowflake_prepare(
      sfstmt,
      "insert into t(c1,c2,c3,c4) values(?,?,?,?)",
      0);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    TEST_CASE_TO_STRING test_cases[] = {
      {.c1in = 1, .c2in = NULL, .c3in = NULL, .c4in = NULL, .c2out = "", .c2_is_null = SF_BOOLEAN_TRUE, .c3out="", .c3_is_null=SF_BOOLEAN_TRUE, .c4out= "", .c4_is_null = SF_BOOLEAN_TRUE}
    };

    size_t i;
    size_t len;
    for (i = 0, len = sizeof(test_cases) / sizeof(TEST_CASE_TO_STRING);
         i < len; i++) {
        TEST_CASE_TO_STRING v = test_cases[i];
        SF_BIND_INPUT ic1;
        ic1.idx = 1;
        ic1.c_type = SF_C_TYPE_INT64;
        ic1.value = (void *) &v.c1in;
        ic1.len = sizeof(v.c1in);
        status = snowflake_bind_param(sfstmt, &ic1);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        SF_BIND_INPUT ic2;
        ic2.idx = 2;
        ic2.c_type = SF_C_TYPE_STRING;
        ic2.value = NULL;
        ic2.len = (size_t) 0;
        status = snowflake_bind_param(sfstmt, &ic2);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        SF_BIND_INPUT ic3;
        ic3.idx = 3;
        ic3.c_type = SF_C_TYPE_INT64;
        ic3.value = NULL;
        ic3.len = (size_t) 0;
        status = snowflake_bind_param(sfstmt, &ic3);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        SF_BIND_INPUT ic4;
        ic4.idx = 4;
        ic4.c_type = SF_C_TYPE_BOOLEAN;
        ic4.value = NULL;
        ic4.len = (size_t) 0;
        status = snowflake_bind_param(sfstmt, &ic4);
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
    status = snowflake_query(sfstmt, "select * from t", 0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
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

    SF_BIND_OUTPUT c3 = {0};
    char c3buf[1024];
    c3.idx = 3;
    c3.c_type = SF_C_TYPE_INT64;
    c3.value = (void *) c3buf;
    c3.len = sizeof(c3buf);
    c3.max_length = sizeof(c3buf);
    status = snowflake_bind_result(sfstmt, &c3);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    SF_BIND_OUTPUT c4 = {0};
    char c4buf[1024];
    c4.idx = 4;
    c4.c_type = SF_C_TYPE_BOOLEAN;
    c4.value = (void *) c4buf;
    c4.len = sizeof(c4buf);
    c4.max_length = sizeof(c4buf);
    status = snowflake_bind_result(sfstmt, &c4);
    if (status != SF_STATUS_SUCCESS) {
        goto error_stmt;
    }

    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) == SF_STATUS_SUCCESS) {
        TEST_CASE_TO_STRING v = test_cases[atoll(c1.value) - 1];
        printf("result: %s, '%s(%s)', '%s(%s)', '%s(%s)'\n",
               (char *) c1.value,
               (char *) c2.value,
               c2.is_null ? "NULL" : "NOT NULL",
               (char *) c3.value,
               c3.is_null ? "NULL" : "NOT NULL",
               (char *) c4.value,
               c4.is_null ? "NULL" : "NOT NULL");
        if (strcmp(v.c2out, c2.value) != 0 && v.c2_is_null == c2.is_null) {
            fprintf(stderr, "ERROR: testcase: %s,"
                      " expected: %s(%s), got %s(%s)\n",
                    (char *) c1.value, v.c2out,
                    v.c2_is_null ? "TRUE" : "FALSE",
                    (char *) c2.value,
                    c2.is_null ? "TRUE" : "FALSE");
        }
        if (strcmp(v.c3out, c3.value) != 0 && v.c3_is_null == c3.is_null) {
            fprintf(stderr, "ERROR: testcase: %s,"
                      " expected: %s(%s), got %s(%s)\n",
                    (char *) c1.value, v.c3out,
                    v.c3_is_null ? "TRUE" : "FALSE",
                    (char *) c3.value,
                    c3.is_null ? "TRUE" : "FALSE");
        }
        if (strcmp(v.c4out, c4.value) != 0 && v.c4_is_null == c4.is_null) {
            fprintf(stderr, "ERROR: testcase: %s,"
                      " expected: %s(%s), got %s(%s)\n",
                    (char *) c1.value, v.c4out,
                    v.c4_is_null ? "TRUE" : "FALSE",
                    (char *) c4.value,
                    c4.is_null ? "TRUE" : "FALSE");
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
