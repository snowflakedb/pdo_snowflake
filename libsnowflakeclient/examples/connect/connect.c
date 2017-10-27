/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include "snowflake_client.h"


int main()
{
    /* init */
    SNOWFLAKE_STATUS status;
    SNOWFLAKE *sf = snowflake_init();

    /* connect*/
    snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
    snowflake_set_attr(sf, SF_CON_DATABASE, getenv("SNOWFLAKE_TEST_DATABASE"));
    snowflake_set_attr(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
    snowflake_set_attr(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
    snowflake_set_attr(sf, SF_CON_WAREHOUSE, getenv("SNOWFLAKE_TEST_WAREHOUSE"));
    snowflake_set_attr(sf, SF_CON_AUTOCOMMIT, &SF_BOOLEAN_TRUE);
    status = snowflake_connect(sf);

    /* term */
    snowflake_term(sf); // purge snowflake context

    return status;
}
