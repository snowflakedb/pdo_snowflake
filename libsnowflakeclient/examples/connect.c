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

    SNOWFLAKE *sf = NULL;

    // Try connecting with a NULL connection struct, should fail
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        printf("OK, connecting to snowflake failed\n");
    } else {
        fprintf(stderr, "Connecting to snowflake succeeded...exiting");
    }

    // Try to connect with empty connection struct, should fail
    sf = snowflake_init();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        SNOWFLAKE_ERROR *error = snowflake_error(sf);
        printf("OK, connecting to snowflake failed. Error message: %s. In File, %s, Line, %d\n", error->msg, error->file, error->line);
    } else {
        fprintf(stderr, "Connecting to snowflake succeeded...exiting");
    }

    // Connect to Snowflake with minimum parameters
    snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        SNOWFLAKE_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Failed connecting with minimum parameters set.\n");
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
    } else {
        printf("OK, connected with just account, user, password\n");
    }
    snowflake_term(sf); // purge snowflake context to setup next connection attempt

    // Connect with all parameters set
    sf = setup_snowflake_connection();

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        SNOWFLAKE_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Failed connecting with full parameters set\n");
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
    } else {
        printf("OK, connected with full parameters\n");
    }

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
