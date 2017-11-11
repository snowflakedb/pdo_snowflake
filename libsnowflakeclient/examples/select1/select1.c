/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include "snowflake_client.h"


int main() {
    /* init */
    SNOWFLAKE_STATUS status;
    snowflake_global_init();
    SNOWFLAKE *sf = snowflake_init();

    /* connect*/
    snowflake_set_attr(sf, SF_CON_HOST, getenv("SNOWFLAKE_TEST_HOST"));
    snowflake_set_attr(sf, SF_CON_PORT, getenv("SNOWFLAKE_TEST_PORT"));
    snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
    snowflake_set_attr(sf, SF_CON_DATABASE, getenv("SNOWFLAKE_TEST_DATABASE"));
    snowflake_set_attr(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
    snowflake_set_attr(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
    snowflake_set_attr(sf, SF_CON_WAREHOUSE, getenv("SNOWFLAKE_TEST_WAREHOUSE"));
    snowflake_set_attr(sf, SF_CON_AUTOCOMMIT, &SF_BOOLEAN_TRUE);
    status = snowflake_connect(sf);

    /* query */
    SNOWFLAKE_STMT *sfstmt1 = snowflake_stmt(sf);
    SNOWFLAKE_STMT *sfstmt2 = snowflake_stmt(sf);
    SNOWFLAKE_STMT *sfstmt3 = snowflake_stmt(sf);
    SNOWFLAKE_BIND_OUTPUT c1;
    SNOWFLAKE_BIND_OUTPUT c2;
    SNOWFLAKE_BIND_OUTPUT c3;
    int out = 0;
    double dout = 0;
    char sout[10];
    c1.idx = 1;
    c1.type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    c2.idx = 2;
    c2.type = SF_C_TYPE_FLOAT64;
    c2.value = (void *) &dout;
    c3.idx = 3;
    c3.type = SF_C_TYPE_STRING;
    c3.value = (void *) sout;
    snowflake_bind_result(sfstmt1, &c1);
    snowflake_bind_result(sfstmt1, &c2);
    snowflake_bind_result(sfstmt1, &c3);
    snowflake_query(sfstmt2, "create or replace warehouse regress;");
    snowflake_query(sfstmt3, "use warehouse regress;");
    snowflake_query(sfstmt1, "select 1, 1.5, 'string';");
    printf("Number of rows: %d\n", (int) snowflake_num_rows(sfstmt1));

    while (snowflake_fetch(sfstmt1) != SF_STATUS_EOL) {
        printf("result: %d, %lf, %s\n", *((int *) c1.value), *((double *) c2.value), (char *) c3.value);
    }
    snowflake_stmt_close(sfstmt1);
    snowflake_stmt_close(sfstmt2);
    snowflake_stmt_close(sfstmt3);

    /* disconnect */
    snowflake_close(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();

    return status;
}
