/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <snowflake/client.h>
#include <example_setup.h>


int main() {
    /* init */
    SF_STATUS status;
    SF_BIND_OUTPUT results[2];
    int64 result1;
    char result2[1000];
    initialize_snowflake_example(SF_BOOLEAN_FALSE);

    /* Connect with all parameters set */
    SF_CONNECT *sf = setup_snowflake_connection();
    status = snowflake_connect(sf);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "connecting to snowflake failed\n");
        goto error_con;
    }

    /* Create a statement once and reused */
    SF_STMT *stmt = snowflake_stmt(sf);
    /* NOTE: the numeric type here should fit into int64 otherwise
     * it is taken as a float */
    status = snowflake_query(
      stmt,
      "select seq4(),randstr(999,random()) from table(generator(rowcount=>10))",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to run query generating increasing integers and random strings\n");
        goto error_stmt;
    }

    // Initialize bind outputs
    results[0].idx = 1;
    results[0].max_length = sizeof(result1);
    results[0].c_type = SF_C_TYPE_INT64;
    results[0].value = &result1;

    results[1].idx = 2;
    results[1].max_length = sizeof(result2);
    results[1].c_type = SF_C_TYPE_STRING;
    results[1].value = &result2;

    // Bind array of outputs
    status = snowflake_bind_result_array(stmt, results, 2);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind c2\n");
        goto error_stmt;
    }

    while ((status = snowflake_fetch(stmt)) == SF_STATUS_SUCCESS) {
        printf("column 1: %lld, column 2: %s\n", result1, result2);
    }

    if (status != SF_STATUS_EOF) {
        fprintf(stderr, "failed to fetch data\n");
        goto error_stmt;
    }

    printf("OK\n");
    /* success */
    status = SF_STATUS_SUCCESS;

error_stmt: /* error stmt */
    snowflake_stmt_term(stmt);

error_con: /* error connection */
    snowflake_term(sf);
    snowflake_global_term();
    return status;
}
