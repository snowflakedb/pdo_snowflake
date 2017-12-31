/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <memory.h>
#include <snowflake/client.h>
#include <example_setup.h>


int main() {
    /* init */
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection();

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "Connecting to snowflake failed, exiting...\n");
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
      "create or replace table t (c1 object, c2 array, c3 variant)",
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
      "insert into t select parse_json(?),parse_json(?),parse_json(?)",
      0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    SF_BIND_INPUT ic1;
    char ic1buf[1024];
    strcpy(ic1buf, "{\"test1\":1}");
    ic1.idx = 1;
    ic1.c_type = SF_C_TYPE_STRING;
    ic1.value = (void *) ic1buf;
    ic1.len = strlen(ic1buf);
    snowflake_bind_param(sfstmt, &ic1);

    SF_BIND_INPUT ic2;
    char ic2buf[1024];
    strcpy(ic2buf, "'[1,2,3]'");
    ic2.idx = 2;
    ic2.c_type = SF_C_TYPE_STRING;
    ic2.value = (void *) ic2buf;
    ic2.len = strlen(ic2buf);
    snowflake_bind_param(sfstmt, &ic2);

    SF_BIND_INPUT ic3;
    char ic3buf[1024];
    strcpy(ic3buf, "'[456,789]'");
    ic3.idx = 3;
    ic3.c_type = SF_C_TYPE_STRING;
    ic3.value = (void *) ic3buf;
    ic3.len = strlen(ic3buf);
    snowflake_bind_param(sfstmt, &ic3);

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

    SF_BIND_OUTPUT c1 = {0};
    char c1buf[1024];
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_STRING;
    c1.value = (void *) c1buf;
    c1.len = sizeof(c1buf);
    c1.max_length = sizeof(c1buf);
    snowflake_bind_result(sfstmt, &c1);

    SF_BIND_OUTPUT c2 = {0};
    char c2buf[1024];
    c2.idx = 2;
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = (void *) c2buf;
    c2.len = sizeof(c2buf);
    c2.max_length = sizeof(c2buf);
    snowflake_bind_result(sfstmt, &c2);

    SF_BIND_OUTPUT c3 = {0};
    char c3buf[1024];
    c3.idx = 3;
    c3.c_type = SF_C_TYPE_STRING;
    c3.value = (void *) c3buf;
    c3.max_length = sizeof(c3buf);
    snowflake_bind_result(sfstmt, &c3);

    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) == SF_STATUS_SUCCESS) {
        printf("result: %s, %s, %s\n", (char *) c1.value, (char *) c2.value,
               (char *) c3.value);
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
