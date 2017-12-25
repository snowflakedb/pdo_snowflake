/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>

const char *c_type_to_string(SF_C_TYPE type) {
    switch (type) {
        case SF_C_TYPE_STRING:
            return "SF_C_TYPE_STRING";
        case SF_C_TYPE_UINT8:
            return "SF_C_TYPE_UINT8";
        case SF_C_TYPE_INT8:
            return "SF_C_TYPE_INT8";
        case SF_C_TYPE_UINT64:
            return "SF_C_TYPE_UINT64";
        case SF_C_TYPE_INT64:
            return "SF_C_TYPE_INT64";
        case SF_C_TYPE_FLOAT64:
            return "SF_C_TYPE_FLOAT64";
        case SF_C_TYPE_BOOLEAN:
            return "SF_C_TYPE_BOOLEAN";
        case SF_C_TYPE_TIMESTAMP:
            return "SF_C_TYPE_TIMESTAMP";
        default:
            return "unknown";
    }
}

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

    sfstmt = snowflake_stmt(sf);
    status = snowflake_query(sfstmt, "select seq4(), randstr(1000,random()), as_double(10.01) from table(generator(rowcount=>1));", 0);
    if (status != SF_STATUS_SUCCESS) {
        SF_ERROR *error = snowflake_stmt_error(sfstmt);
        fprintf(stderr, "Error message: %s\nIn File, %s, Line, %d\n", error->msg, error->file, error->line);
        goto cleanup;
    } else {
        printf("Query executed successfully\n");
    }
    SF_COLUMN_DESC *desc = snowflake_desc(sfstmt);

    // Check seq4 type
    if (desc[0].c_type != SF_C_TYPE_INT64) {
        fprintf(stderr, "Error, seq4 column is not type SF_C_TYPE_INT64. Actual type is %s\n", c_type_to_string(desc[0].c_type));
        status = SF_STATUS_ERROR;
    } else {
        printf("INT64 type check succeeded\n");
    }

    // Check randstr type
    if (desc[1].c_type != SF_C_TYPE_STRING) {
        fprintf(stderr, "Error, randstr column is not type SF_C_TYPE_STRING. Actual type is %s\n", c_type_to_string(desc[1].c_type));
        status = SF_STATUS_ERROR;
    } else {
        printf("STRING type check succeeded\n");
    }

    // Check as_double type
    if (desc[2].c_type != SF_C_TYPE_FLOAT64) {
        fprintf(stderr, "Error, as_double('10.01') column is not type SF_C_TYPE_FLOAT64. Actual type is %s\n", c_type_to_string(desc[2].c_type));
        status = SF_STATUS_ERROR;
    } else {
        printf("FLOAT64 type check succeeded\n");
    }

cleanup:
    snowflake_stmt_term(sfstmt);

    /* close and term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
