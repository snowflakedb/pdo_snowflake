/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#include <example_setup.h>
#include <stdlib.h>

void initialize_snowflake_example(sf_bool debug) {
    snowflake_global_init(NULL);

    snowflake_global_set_attribute(SF_GLOBAL_CA_BUNDLE_FILE, getenv("SNOWFLAKE_TEST_CA_BUNDLE_FILE"));
    snowflake_global_set_attribute(SF_GLOBAL_DEBUG, &debug);
}

SF_CONNECT *setup_snowflake_connection() {
    return setup_snowflake_connection_with_autocommit(
      "UTC", SF_BOOLEAN_TRUE);
}

SF_CONNECT *setup_snowflake_connection_with_autocommit(
  const char* timezone, sf_bool autocommit) {
    SF_CONNECT *sf = snowflake_init();

    snowflake_set_attribute(sf, SF_CON_ACCOUNT,
                            getenv("SNOWFLAKE_TEST_ACCOUNT"));
    snowflake_set_attribute(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
    snowflake_set_attribute(sf, SF_CON_PASSWORD,
                            getenv("SNOWFLAKE_TEST_PASSWORD"));
    snowflake_set_attribute(sf, SF_CON_DATABASE,
                            getenv("SNOWFLAKE_TEST_DATABASE"));
    snowflake_set_attribute(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
    snowflake_set_attribute(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
    snowflake_set_attribute(sf, SF_CON_WAREHOUSE,
                            getenv("SNOWFLAKE_TEST_WAREHOUSE"));
    snowflake_set_attribute(sf, SF_CON_AUTOCOMMIT, &autocommit);
    snowflake_set_attribute(sf, SF_CON_TIMEZONE, timezone);
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
    return sf;
}
