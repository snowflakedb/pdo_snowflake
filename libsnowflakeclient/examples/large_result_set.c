/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake/client.h>
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
    status = snowflake_query(sfstmt, "select seq4(),randstr(1000,random()) from table(generator(rowcount=>100000));", 0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
    }
    SF_BIND_OUTPUT c1 = {0};
    int64 out = 0;
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    c1.max_length = sizeof(out);
    snowflake_bind_result(sfstmt, &c1);

    SF_BIND_OUTPUT c2 = {0};
    char c2buf[1001];
    c2.idx = 2;
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = (void *) c2buf;
    c2.max_length = sizeof(c2buf);
    snowflake_bind_result(sfstmt, &c2);

    uint64 counter = 0;
    while ((status = snowflake_fetch(sfstmt)) != SF_STATUS_EOL) {
        if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
            SF_ERROR *error = snowflake_stmt_error(sfstmt);
            fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
            break;
        }
        counter++;

        if ((out % 10000) == 0) {
            printf("%lld\t%s\n", out, c2buf);
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
