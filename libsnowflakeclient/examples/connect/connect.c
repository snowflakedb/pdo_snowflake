/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>


int main()
{
    /* init */
    SNOWFLAKE_STATUS status;
    snowflake_global_init();
    SNOWFLAKE *sf = snowflake_init();

    snowflake_global_set_attribute(SF_GLOBAL_CA_BUNDLE_FILE, getenv("SNOWFLAKE_TEST_CA_BUNDLE_FILE"));
    //snowflake_global_set_attribute(SF_GLOBAL_DEBUG, &SF_BOOLEAN_TRUE);

    /* connect*/
    snowflake_set_attr(sf, SF_CON_PROTOCOL, getenv("SNOWFLAKE_TEST_PROTOCOL"));
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

    /* term */
    snowflake_term(sf); // purge snowflake context
    snowflake_global_term();
    if (status != SF_STATUS_SUCCESS) {
        printf("Failed\n");
    } else {
        printf("OK\n");
    }

    return status;
}
