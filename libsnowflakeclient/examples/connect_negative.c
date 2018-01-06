/*
 * Copyright (c) 2018 Snowflake Computing, Inc. All rights reserved.
 */

#include <snowflake/client.h>
#include <example_setup.h>
#include <stdio.h>

int main() {
    /* init */
    SF_STATUS status;
    initialize_snowflake_example(SF_BOOLEAN_FALSE);

    SF_CONNECT *sf = NULL;
    SF_STATUS ret = SF_STATUS_SUCCESS;

    // account parameter is missing
    sf = snowflake_init();
    snowflake_set_attribute(sf, SF_CON_USER, "abc");
    snowflake_set_attribute(sf, SF_CON_PASSWORD, "abc");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to missing account\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);
    if (ret != SF_STATUS_SUCCESS) {
        goto cleanup;
    }

    // user parameter is missing
    sf = snowflake_init();
    snowflake_set_attribute(sf, SF_CON_ACCOUNT, "abc");
    snowflake_set_attribute(sf, SF_CON_PASSWORD, "abc");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to missing user\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);
    if (ret != SF_STATUS_SUCCESS) {
        goto cleanup;
    }

    // password parameter is missing
    sf = snowflake_init();
    snowflake_set_attribute(sf, SF_CON_ACCOUNT, "abc");
    snowflake_set_attribute(sf, SF_CON_USER, "abc");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to missing password\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);

    // invalid database
    sf = setup_snowflake_connection();
    snowflake_set_attribute(sf, SF_CON_DATABASE, "NEVER_EXISTS");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to invalid database\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);

    // invalid schema
    sf = setup_snowflake_connection();
    snowflake_set_attribute(sf, SF_CON_SCHEMA, "NEVER_EXISTS");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to invalid schema\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);

    // invalid warehouse
    sf = setup_snowflake_connection();
    snowflake_set_attribute(sf, SF_CON_WAREHOUSE, "NEVER_EXISTS");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to invalid warehouse\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);

    // invalid role
    sf = setup_snowflake_connection();
    snowflake_set_attribute(sf, SF_CON_ROLE, "NEVER_EXISTS");

    ret = snowflake_connect(sf);
    if (!ret) {
        fprintf(stderr, "Failed. Must fail to connect\n");
        ret = SF_STATUS_ERROR_GENERAL;
    } else {
        SF_ERROR *sferr = snowflake_error(sf);
        if (!sferr->error_code == SF_STATUS_ERROR_BAD_CONNECTION_PARAMS) {
            fprintf(stderr, "Failed. Wrong error code: %d\n",
                    sferr->error_code);
            ret = SF_STATUS_ERROR_GENERAL;
        } else {
            printf("OK. failed connection due to invalid role\n");
            ret = SF_STATUS_SUCCESS;
        }
    }
    snowflake_term(sf);

    snowflake_global_term();
cleanup:
    return ret;
}