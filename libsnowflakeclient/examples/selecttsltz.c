/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>
#include <string.h>


#define USER_TZ "America/New_York"

int main() {
    /* init */
    SF_STATUS status;
    const char *tz = USER_TZ;

    /* The client application must set the same timezone to both:
     * 1) the environment variable TZ
     * 2) the session parameter of Snowflake
     * */
    // setenv("TZ", tz, 1);
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection_with_autocommit(
      tz, SF_BOOLEAN_TRUE);

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    /* Create a statement once and reused */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    /* NOTE: the numeric type here should fit into int64 otherwise
     * it is taken as a float */
    status = snowflake_query(
      sfstmt,
      "create or replace table t (c1 int, c2 timestamp_ltz(5))",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    /* insert data */
    status = snowflake_prepare(
      sfstmt,
      "insert into t(c1,c2) values(?,?)",
      0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    SF_BIND_INPUT ic1;
    int64 ic1buf = 101;
    ic1.idx = 1;
    ic1.c_type = SF_C_TYPE_INT64;
    ic1.value = (void *) &ic1buf;
    ic1.len = sizeof(ic1buf);
    snowflake_bind_param(sfstmt, &ic1);

    SF_BIND_INPUT ic2;
    char ic2buf[1024];

    const char *r1 = "2014-05-03 13:56:46.00123 -04:00";
    strcpy(ic2buf, r1);
    ic2.idx = 2;
    ic2.c_type = SF_C_TYPE_STRING;
    ic2.value = (void *) ic2buf;
    ic2.len = strlen(ic2buf);
    snowflake_bind_param(sfstmt, &ic2);

    status = snowflake_execute(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }
    printf("Inserted one row\n");

    ic1buf = 102;
    ic1.idx = 1;
    ic1.c_type = SF_C_TYPE_INT64;
    ic1.value = (void *) &ic1buf;
    ic1.len = sizeof(ic1buf);
    snowflake_bind_param(sfstmt, &ic1);

    const char *r2 = "1969-11-21 05:17:23.12300 -05:00";
    strcpy(ic2buf, r2);
    ic2.idx = 2;
    ic2.c_type = SF_C_TYPE_STRING;
    ic2.value = (void *) ic2buf;
    ic2.len = strlen(ic2buf);
    snowflake_bind_param(sfstmt, &ic2);

    status = snowflake_execute(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }
    printf("Inserted one row\n");

    /* query */
    status = snowflake_query(sfstmt, "select * from t", 0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    SF_BIND_OUTPUT c1;
    char c1buf[1024];
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_STRING;
    c1.value = (void *) c1buf;
    c1.len = sizeof(c1buf);
    c1.max_length = sizeof(c1buf);
    snowflake_bind_result(sfstmt, &c1);

    SF_BIND_OUTPUT c2;
    char c2buf[1024];
    c2.idx = 2;
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = (void *) c2buf;
    c2.len = sizeof(c2buf);
    c2.max_length = sizeof(c2buf);
    snowflake_bind_result(sfstmt, &c2);

    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    int counter = 0;
    while ((status = snowflake_fetch(sfstmt)) == SF_STATUS_SUCCESS) {
        printf("result: %s, '%s'\n", (char *) c1.value, (char *) c2.value);
        if (counter == 0) {
            if (strcmp(r1, c2.value) != 0) {
                fprintf(stderr, "expected: %s, got: %s\n", r1,
                        (char *) c2.value);
            }
        } else {
            if (strcmp(r2, c2.value) != 0) {
                fprintf(stderr, "expected: %s, got: %s\n", r2,
                        (char *) c2.value);
            }
        }
        ++counter;
    }

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_EOL) {
        status = SF_STATUS_SUCCESS;
    } else if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
    }

cleanup:
    /* delete statement */
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
