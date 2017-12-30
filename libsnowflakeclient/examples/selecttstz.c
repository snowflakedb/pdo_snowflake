/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <snowflake_client.h>
#include <example_setup.h>
#include <string.h>
#include <stdlib.h>

typedef struct test_case_to_string {
    const int64 c1in;
    const char *c2in;
    const char *c2out;
    SF_ERROR_CODE error_code;
} TEST_CASE_TO_STRING;


#define USER_TZ "America/New_York"

int main() {
    /* init */
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection_with_autocommit(
      USER_TZ, SF_BOOLEAN_TRUE); // set the session timezone

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        goto error_conn;
    }

    /* Create a statement once and reused */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    status = snowflake_query(
      sfstmt,
      "create or replace table t (c1 int, c2 timestamp_tz(5))",
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
      {.c1in = 1, .c2in = "2014-05-03 13:56:46.123 +09:00", .c2out = "2014-05-03 13:56:46.12300 +09:00"},
      {.c1in = 2, .c2in = "1969-11-21 05:17:23.0123 -01:00", .c2out = "1969-11-21 05:17:23.01230 -01:00"},
      {.c1in = 3, .c2in = "1960-01-01 00:00:00.0000", .c2out = "1960-01-01 00:00:00.00000 -05:00"},
      // timestamp before 1600 doesn't work properly.
      {.c1in = 4, .c2in = "1500-01-01 00:00:00.0000", .c2out = "1500-01-01 00:03:02.00000 -04:56"},

      {.c1in = 5, .c2in = "0001-01-01 00:00:00.0000", .c2out = "1-01-01 00:03:02.00000 -04:56"},
      {.c1in = 6, .c2in = "9999-01-01 00:00:00.0000", .c2out = "9999-01-01 00:00:00.00000 -05:00"},
      {.c1in = 7, .c2in = "99999-12-31 23:59:59.9999", .c2out = "", .error_code=100035},
      {.c1in = 8, .c2in = NULL, .c2out = ""},
      /*
      {.c1in = 9, .c2in = "9999-12-31 23:59:59.9999", .c2out = "9999-12-31 23:59:59.99990 -05:00"},
       */
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
        ic2.len = v.c2in != NULL ? strlen(v.c2in) : 0;
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
    status = snowflake_query(sfstmt, "select * from t", 0);
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
        if (v.error_code == SF_ERROR_NONE) {
            printf("result: %s, '%s(%s)'\n",
                   (char *) c1.value, (char *) c2.value,
                   c2.is_null ? "NULL" : "NOT NULL");
            if (v.c2out != NULL && strcmp(v.c2out, c2.value) != 0) {
                fprintf(stderr, "ERROR: testcase: %s, expected: %s, got %s\n",
                        (char *) c1.value, v.c2out, (char *) c2.value);
            }
        }
    }

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
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
