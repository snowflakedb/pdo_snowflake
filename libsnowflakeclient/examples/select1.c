/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>


int main() {
    /* init */
    SNOWFLAKE_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SNOWFLAKE *sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "Connecting to snowflake failed, exiting...\n");
        goto cleanup;
    }

    /* query */
    SNOWFLAKE_STMT *sfstmt = snowflake_stmt(sf);
    SNOWFLAKE_BIND_OUTPUT c1;
    int out = 0;
    c1.idx = 1;
    c1.type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    snowflake_bind_result(sfstmt, &c1);
    snowflake_query(sfstmt, "select 1;");
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) != SF_STATUS_EOL) {
        if (status == SF_STATUS_ERROR || status == SF_STATUS_WARNING) {
            printf("Ran into error during fetch");
            break;
        }
        printf("result: %d\n", *((int *) c1.value));
    }
    snowflake_stmt_close(sfstmt);

    // If we reached end of line, then we were successful
    if (status == SF_STATUS_EOL) {
        status = SF_STATUS_SUCCESS;
    }

cleanup:
    /* disconnect */
    snowflake_close(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
