/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
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

    /* query */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    snowflake_query(sfstmt, "select 1;", 0);
    SF_BIND_OUTPUT c1 = {0};
    int64 out = 0;
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    c1.len = sizeof(out);
    snowflake_bind_result(sfstmt, &c1);
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) != SF_STATUS_SUCCESS) {
        printf("result: %d\n", *((int *) c1.value));
    }

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_EOF) {
        status = SF_STATUS_SUCCESS;
    } else if (status > 0) {
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
