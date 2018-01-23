/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <snowflake/client.h>
#include <example_setup.h>

#define INPUT_ARRAY_SIZE 3

int main() {
    /* init */
    SF_STATUS status;
    int i;
    SF_BIND_INPUT input_array[INPUT_ARRAY_SIZE];
    int64 input1;
    char input2[1000];
    float64 input3;
    SF_BIND_INPUT string_input;
    char str[1000];
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
      "create or replace table t (c1 number(10,0) not null, c2 string, c3 double)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to create a table\n");
        goto error_stmt;
    }

    // Initialize bind inputs
    string_input.idx = 2;
    string_input.c_type = SF_C_TYPE_STRING;
    string_input.value = &str;
    string_input.len = sizeof(str);

    input_array[0].idx = 1;
    input_array[0].c_type = SF_C_TYPE_INT64;
    input_array[0].value = &input1;
    input_array[0].len = sizeof(input1);

    input_array[1].idx = 2;
    input_array[1].c_type = SF_C_TYPE_STRING;
    input_array[1].value = &input2;
    input_array[1].len = sizeof(input2);

    input_array[2].idx = 3;
    input_array[2].c_type = SF_C_TYPE_FLOAT64;
    input_array[2].value = &input3;
    input_array[2].len = sizeof(input3);

    status = snowflake_prepare(
      stmt,
      "insert into t values(?, ?, ?)",
      0
    );
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to prepare sql statement\n");
        goto error_stmt;
    }

    status = snowflake_bind_param_array(stmt, input_array, INPUT_ARRAY_SIZE);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind parameters\n");
        goto error_stmt;
    }

    // Set Data
    input1 = 1;
    strcpy(input2, "test1");
    input3 = 1.23;

    status = snowflake_execute(stmt);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to insert data\n");
        goto error_stmt;
    }

    // Bind new input value
    status = snowflake_bind_param(stmt, &string_input);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to bind new input\n");
        goto error_stmt;
    }

    // Set new data
    input1 = 2;
    strcpy(str, "test2");
    input3 = 2.34;

    status = snowflake_execute(stmt);
    if (status != SF_STATUS_SUCCESS) {
        fprintf(stderr, "failed to insert data\n");
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
