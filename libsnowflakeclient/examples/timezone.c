/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <snowflake_client.h>
#include <example_setup.h>
#include <memory.h>


int main() {
    /* init */
    const char *local_timezone = "America/Los_Angeles";
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);
    SF_CONNECT *sf = setup_snowflake_connection();
    snowflake_set_attr(sf, SF_CON_TIMEZONE, local_timezone);

    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
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
      "show parameters like 'TIMEZONE'",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n",
                error->msg, error->file, error->line);
        goto cleanup;
    }

    SF_BIND_OUTPUT c2 = {0};
    char c2buf[1024];
    c2.idx = 2;
    c2.c_type = SF_C_TYPE_STRING;
    c2.value = (void *) c2buf;
    c2.len = sizeof(c2buf);
    c2.max_length = sizeof(c2buf);
    snowflake_bind_result(sfstmt, &c2);

    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt));

    while ((status = snowflake_fetch(sfstmt)) == SF_STATUS_SUCCESS) {
        printf("result: '%s'\n", (char *) c2.value);
        if (strcmp(local_timezone, c2.value) != 0) {
            fprintf(
              stderr,
              "Error: expected: %s, got: %s\n",
              local_timezone, (char *) c2.value);
        }
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
