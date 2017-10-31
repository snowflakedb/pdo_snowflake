/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include "snowflake_client.h"


void main() {
    /* init */
    SNOWFLAKE *sf = snowflake_init();
    snowflake_global_init();

    /* connect*/
    snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
    snowflake_set_attr(sf, SF_CON_DATABASE, getenv("SNOWFLAKE_TEST_DATABASE"));
    snowflake_set_attr(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
    snowflake_set_attr(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
    snowflake_set_attr(sf, SF_CON_WAREHOUSE, getenv("SNOWFLAKE_TEST_WAREHOUSE"));
    snowflake_set_attr(sf, SF_CON_AUTOCOMMIT, &SF_BOOLEAN_TRUE);
    snowflake_connect(sf);

    /* query */
    SNOWFLAKE_STMT *sfstmt = snowflake_stmt(sf);
    SNOWFLAKE_BIND_OUTPUT c1;
    int out = 0;
    c1.idx = 1;
    c1.type = SF_C_TYPE_INT64;
    c1.value = (void *) &out;
    snowflake_bind_result(sfstmt, &c1);
    snowflake_query(sfstmt, "SELECT 1");
    printf("Number of rows: %d", (int) snowflake_num_rows(sfstmt));

    while (snowflake_fetch(sfstmt) != SF_STATUS_EOL) {
        printf("result: %d\n", *((int *) c1.value));
    }
    snowflake_stmt_close(sfstmt);

    /* disconnect */
    snowflake_close(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();
}
