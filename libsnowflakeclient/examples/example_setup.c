/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <example_setup.h>

void initialize_snowflake_example(sf_bool debug) {
    snowflake_global_init();

    snowflake_global_set_attribute(SF_GLOBAL_CA_BUNDLE_FILE, getenv("SNOWFLAKE_TEST_CA_BUNDLE_FILE"));
    snowflake_global_set_attribute(SF_GLOBAL_DEBUG, &debug);
}

SNOWFLAKE *setup_snowflake_connection() {
    SNOWFLAKE *sf = snowflake_init();

    snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
    snowflake_set_attr(sf, SF_CON_DATABASE, getenv("SNOWFLAKE_TEST_DATABASE"));
    snowflake_set_attr(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
    snowflake_set_attr(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
    snowflake_set_attr(sf, SF_CON_WAREHOUSE, getenv("SNOWFLAKE_TEST_WAREHOUSE"));
    snowflake_set_attr(sf, SF_CON_AUTOCOMMIT, &SF_BOOLEAN_TRUE);

    return sf;
}