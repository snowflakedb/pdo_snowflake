/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifndef PHP_PDO_SNOWFLAKE_H
#define PHP_PDO_SNOWFLAKE_H

extern zend_module_entry pdo_snowflake_module_entry;
#define phpext_pdo_snowflake_ptr &pdo_snowflake_module_entry

#include "php_version.h"

#define PHP_PDO_SNOWFLAKE_VERSION PHP_VERSION

#ifdef PHP_WIN32
#define PHP_PDO_SNOWFLAKE_API __declspec(dllexport)
#else
#define PHP_PDO_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#endif /* PHP_PDO_SNOWFLAKE_H */
