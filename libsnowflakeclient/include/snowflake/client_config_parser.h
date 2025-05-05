/**
 * Copyright (c) 2025 Snowflake Computing
 */

#ifndef SNOWFLAKE_CONFIGPARSER_H
#define SNOWFLAKE_CONFIGPARSER_H

#include "snowflake/client.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct client_config
  {
    // The log level
    char logLevel[64];

    // The log path
    char logPath[MAX_PATH];
  } client_config;

  /**
    * Construct client_config from client config file passed by user. This method searches the
    * config file in following order: 1. configFilePath param which is read from connection URL or
    * connection property. 2. Environment variable: SF_CLIENT_CONFIG_FILE containing full path to
    * sf_client_config file. 3. Searches for default config file name(sf_client_config.json under the
    * driver directory from where the driver gets loaded. 4. Searches for default config file
    * name(sf_client_config.json) under user home directory 5. Searches for default config file
    * name(sf_client_config.json) under tmp directory
    * 
    * @param configFilePath          The config file path passed in by the user.
    * @param clientConfig            The client_config object to be filled.
    * 
    * @return true if successful
    */
  sf_bool load_client_config(
    const char* configFilePath,
    client_config* clientConfig);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //SNOWFLAKE_CONFIGPARSER_H
