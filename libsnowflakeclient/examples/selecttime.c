/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <snowflake/client.h>
#include <example_setup.h>

typedef struct test_case_to_string {
    const int64 c1in;
    const char *c2in;
    const char *c2out;
    SF_STATUS error_code;
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
      "create or replace table t (c1 int, c2 time)",
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

    TEST_CASE_TO_STRING test_cases[] = {
      {.c1in = 1, .c2in = "13:56:46.123", .c2out = "13:56:46.123000000"},
      {.c1in = 2, .c2in = "05:08:12.0001234", .c2out = "05:08:12.000123400"},
      {.c1in = 3, .c2in = "GARBAGE", .c2out = "", .error_code=100108},
      {.c1in = 4, .c2in = "98:98:12", .c2out = "", .error_code=100108},
      {.c1in = 5, .c2in = "08:34:23.9876543210", .c2out = "", .error_code=100108},
      {.c1in = 6, .c2in = "08:34:23.987654321", .c2out = "08:34:23.987654321"},
    };

    size_t i;
    size_t len;
    for (i = 0, len = sizeof(test_cases) / sizeof(TEST_CASE_TO_STRING);
         i < len; i++) {
        TEST_CASE_TO_STRING v = test_cases[i];

        SF_BIND_INPUT ic1 = {0};
        ic1.idx = 1;
        ic1.c_type = SF_C_TYPE_INT64;
        ic1.value = (void *) &v.c1in;
        ic1.len = sizeof(v.c1in);
        status = snowflake_bind_param(sfstmt, &ic1);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        SF_BIND_INPUT ic2 = {0};
        ic2.idx = 2;
        ic2.c_type = SF_C_TYPE_STRING;
        ic2.value = (void *) v.c2in;
        ic2.len = strlen(v.c2in);
        status = snowflake_bind_param(sfstmt, &ic2);
        if (status != SF_STATUS_SUCCESS) {
            goto error_stmt;
        }

        status = snowflake_execute(sfstmt);
        if (status != SF_STATUS_SUCCESS) {
            SF_ERROR *error = snowflake_stmt_error(sfstmt);
            if (v.error_code != error->error_code) {
                goto error_stmt;
            }
            printf("Expected ERROR: %d\n", error->error_code);
        } else {
            printf("Inserted one row\n");
        }
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
        if (v.error_code == SF_STATUS_SUCCESS) {
            printf("result: %s, '%s'\n", (char *) c1.value, (char *) c2.value);
            if (strcmp(v.c2out, c2.value) != 0) {
                fprintf(stderr, "ERROR: testcase: %s, expected: %s, got %s\n",
                        (char *) c1.value, v.c2out, (char *) c2.value);
            }
        }
    }

    // If we reached end of line, then we were successful
    if (status > 0) {
        goto error_stmt;
    }
    status = SF_STATUS_SUCCESS;
    goto cleanup;

error_stmt:
    {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error: %d: %s\nIn File, %s, Line, %d\n",
                error->error_code,
                error->msg, error->file, error->line);
        goto cleanup;
    }
error_conn:
    {
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Error: %d: %s\nIn File, %s, Line, %d\n",
                error->error_code,
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
