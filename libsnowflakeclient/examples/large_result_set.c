/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>

int main() {
    /* init */
    SF_STATUS status;
    SF_CONNECT *sf = NULL;
    SF_STMT *sfstmt = NULL;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "Connecting to snowflake failed, exiting...\n");
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
        goto cleanup;
    } else {
        printf("Connected to Snowflake\n");
    }

    /* query */
    sfstmt = snowflake_stmt(sf);
    status = snowflake_query(sfstmt, "select seq4() from table(generator(rowcount=>200000));", 0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
    }
    SF_BIND_OUTPUT c1;
    int64 out = 0;
    uint64 counter = 0;
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    c1.len = sizeof(out);
    snowflake_bind_result(sfstmt, &c1);

    while ((status = snowflake_fetch(sfstmt)) != SF_STATUS_EOL) {
        if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
            SF_ERROR *error = snowflake_stmt_error(sfstmt);
            fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
            break;
        }
        counter++;

        if ((counter % 10000) == 0) {
            printf("Number of results fetched: %llu\n", counter);
        }
    }
    printf("Number of rows in result: %llu\n", snowflake_num_rows(sfstmt));
    printf("Number of rows fetched: %llu\n", counter);
    if (counter == snowflake_num_rows(sfstmt)) {
        printf("Number of rows fetched equals number of rows expected in result\n");
    } else {
        status = SF_STATUS_ERROR;
        fprintf(stderr, "Number of rows fetched is different from number of rows expected\n");
        goto cleanup;
    }

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_EOL) {
        status = SF_STATUS_SUCCESS;
    }

cleanup:
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
