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
    } else {
        printf("Connected to Snowflake\n");
    }

    /* query */
    SNOWFLAKE_STMT *sfstmt = snowflake_stmt(sf);
    status = snowflake_query(sfstmt, "select seq4() from table(generator(timelimit=>60));");
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

cleanup:
    snowflake_stmt_close(sfstmt);
    /* disconnect */
    snowflake_close(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
