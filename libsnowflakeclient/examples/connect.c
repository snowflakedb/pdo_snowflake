/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>


int main()
{
    /* init */
    SNOWFLAKE_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SNOWFLAKE *sf = setup_snowflake_connection();

    status = snowflake_connect(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();
    if (status != SF_STATUS_SUCCESS) {
        SNOWFLAKE_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
        printf("Failed\n");
    } else {
        printf("OK\n");
    }

    return status;
}
