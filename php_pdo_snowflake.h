/* Copyright (c) 2017-2019 Snowflake Computing Inc. All right reserved.  */

#ifndef PHP_PDO_SNOWFLAKE_H
#define PHP_PDO_SNOWFLAKE_H

extern zend_module_entry pdo_snowflake_module_entry;
#define phpext_pdo_snowflake_ptr &pdo_snowflake_module_entry

#include "php_version.h"

/**
 * PHP PDO Snowflake version for PHP info
 */
#define PDO_SNOWFLAKE_VERSION "1.2.3"

#ifdef PHP_WIN32
#define PHP_PDO_SNOWFLAKE_API __declspec(dllexport)
#else
#define PHP_PDO_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#endif /* PHP_PDO_SNOWFLAKE_H */
