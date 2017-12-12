/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <snowflake_client.h>
#include <example_setup.h>


int main() {
    /* init */
    SF_STATUS status;
    SF_CONNECT *sf = NULL;
    SF_STMT *sfstmt = NULL;

    initialize_snowflake_example(SF_BOOLEAN_FALSE);

    /* Case 1: SQL syntax error */
    sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "FAIL. Failed to connect.\n");
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr,
                "Error code: %d, sqlstate: %s, message: %s\nIn File, %s, Line, %d\n",
                error->error_code, error->sqlstate, error->msg, error->file,
                error->line);
        goto cleanup1;
    }
    sfstmt = snowflake_stmt(sf);
    if (snowflake_query(sfstmt, "select 1 frooom dual", 0) !=
        SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        printf(
          "OK. Error code: %d, sqlstate: %s, message: %s\nIn File, %s, Line, %d\n",
          error->error_code, error->sqlstate, error->msg, error->file,
          error->line);
    } else {
        fprintf(stderr, "FAIL: Must fail with syntax error.\n");
    }
cleanup1:
    if (sfstmt) {
        snowflake_stmt_term(sfstmt);
    }
    sfstmt = NULL;
    snowflake_term(sf);

    /* Case 2: failed with incorrect password */
    sf = setup_snowflake_connection();
    snowflake_set_attr(sf, SF_CON_USER, "HIHIHI");
    snowflake_set_attr(sf, SF_CON_PASSWORD, "HAHAHA");
    status = snowflake_connect(sf);
    if (status == SF_STATUS_SUCCESS) {
        fprintf(stderr, "FAIL. Must fail with incorrect passowrd error.\n");
        goto cleanup2;
    }

    printf("OK. Connecting to snowflake failed...\n");
    SF_ERROR *error = snowflake_error(sf);
    printf(
      "OK. Error code: %d, sqlstate: %s, message: %s\nIn File, %s, Line, %d\n",
      error->error_code, error->sqlstate, error->msg, error->file,
      error->line);

cleanup2:
    /* delete statement */
    if (sfstmt) {
        snowflake_stmt_term(sfstmt);
    }

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
