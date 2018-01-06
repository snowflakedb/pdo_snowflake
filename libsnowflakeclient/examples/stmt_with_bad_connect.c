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
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = snowflake_init();

    /* query, try running a query with a connection struct that has not connected */
    SF_STMT *sfstmt = snowflake_stmt(sf);
    snowflake_prepare(sfstmt, "select 1;", 0);
    SF_BIND_OUTPUT c1 = {0};
    int out = 0;
    c1.idx = 1;
    c1.c_type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    snowflake_bind_result(sfstmt, &c1);
    status = snowflake_execute(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        printf(
          "OK, running query to snowflake failed. Error message: %s. In File, %s, Line, %d\n",
          error->msg, error->file, error->line);
    } else {
        fprintf(stderr, "Running query succeeded...exiting");
        goto cleanup;
    }

    // Connect with minimum parameters and retry running query
    snowflake_set_attribute(sf, SF_CON_ACCOUNT,
                            getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attribute(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attribute(sf, SF_CON_PASSWORD,
                            getenv("SNOWFLAKE_TEST_PASSWORD"));
    char *host, *port, *protocol;
    host = getenv("SNOWFLAKE_TEST_HOST");
    if (host) {
        snowflake_set_attribute(sf, SF_CON_HOST, host);
    }
    port = getenv("SNOWFLAKE_TEST_PORT");
    if (port) {
        snowflake_set_attribute(sf, SF_CON_PORT, port);
    }
    protocol = getenv("SNOWFLAKE_TEST_PROTOCOL");
    if (protocol) {
        snowflake_set_attribute(sf, SF_CON_PROTOCOL, protocol);
    }
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_error(sf);
        fprintf(stderr, "Failed connecting with minimum parameters set.\n");
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);

    } else {
        printf("OK, connected with just account, user, password\n");
    }

    // Retry query now that connection works
    status = snowflake_execute(sfstmt);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr,
                "Failed running query with connect snowflake object.\n");
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
    } else {
        printf("OK, query executed successfully\n");
    }
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) != SF_STATUS_EOL) {
        if (status > 0) {
            SF_ERROR *error = snowflake_stmt_error(sfstmt);
            fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                    error->msg, error->file, error->line);
            break;
        }
        printf("result: %d\n", *((int *) c1.value));
    }
    // If we reached end of line, then we were successful
    if (status == SF_STATUS_EOL) {
        status = SF_STATUS_SUCCESS;
    }

cleanup:
    /* Clean up stmt struct*/
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
