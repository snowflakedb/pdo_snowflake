/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifdef HAVE_CONFIG_H

#include "config.h"

#endif

#include "php.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_snowflake.h"
#include "php_pdo_snowflake_int.h"

ZEND_DECLARE_MODULE_GLOBALS(pdo_snowflake)

#ifdef COMPILE_DL_PDO_SNOWFLAKE
#   if (PHP_VERSION_ID >= 70000)
#       ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#       endif /* ZTS */
#   endif /* PHP_VERSION_ID */

ZEND_GET_MODULE(pdo_snowflake)
#endif /* COMPILE_DL_PDO_SNOWFLAKE */

/* {{{ PHP_INI_BEGIN
*/
PHP_INI_BEGIN()
#if PDO_DBG_ENABLED
    STD_PHP_INI_ENTRY
    ("pdo_snowflake.debug", NULL, PHP_INI_SYSTEM, OnUpdateString, debug,
     zend_pdo_snowflake_globals, pdo_snowflake_globals)
#endif
PHP_INI_END()
/* }}} */

/* TODO: adhoc logger until Snowflake client provides it. */
void pdo_snowflake_log(int line, const char *filename, const char *severity,
                       char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    FILE *fp = fopen("/tmp/php.log", "a");
    fprintf(fp, "[%-6s] %-70s:%5d - ", severity, filename, line);
    vfprintf(fp, fmt, args);
    fprintf(fp, "\n");
    fclose(fp);

    va_end (args);
}

/* {{{ PHP_MINIT_FUNCTION
 */
static PHP_MINIT_FUNCTION(pdo_snowflake) {
    REGISTER_INI_ENTRIES();

    REGISTER_PDO_CLASS_CONST_LONG("SNOWFLAKE_ATTR_SSL_CAPATH",
                                  (zend_long) PDO_SNOWFLAKE_ATTR_SSL_CAPATH);
    REGISTER_PDO_CLASS_CONST_LONG(
      "SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS",
      (zend_long) PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS);

    return php_pdo_register_driver(&pdo_snowflake_driver);
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
static PHP_MSHUTDOWN_FUNCTION(pdo_snowflake) {
    php_pdo_unregister_driver(&pdo_snowflake_driver);
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(pdo_snowflake) {
    php_info_print_table_start();

    php_info_print_table_header(2, "PDO Driver for Snowflake", "enabled");
    /* TODO: get Snowflake Driver version, etc*/
    php_info_print_table_row(2, "Client API version", "0.1");

    php_info_print_table_end();

#ifndef PHP_WIN32
    DISPLAY_INI_ENTRIES();
#endif
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(pdo_snowflake) {
#if (PHP_VERSION_ID >= 70000)
#if defined(COMPILE_DL_PDO_SNOWFLAKE) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
#endif
#if PDO_DBG_ENABLED
    pdo_snowflake_globals->debug = NULL;  /* The actual string */
#endif
}
/* }}} */

/* {{{ pdo_snowflake_functions[] */
const zend_function_entry pdo_snowflake_functions[] = {
  PHP_FE_END
};
/* }}} */

/* {{{ pdo_snowflake_deps[] */
static const zend_module_dep pdo_snowflake_deps[] = {
  ZEND_MOD_REQUIRED("pdo")
  ZEND_MOD_END
};
/* }}} */

/* {{{ pdo_snowflake_module_entry */
zend_module_entry pdo_snowflake_module_entry = {
  STANDARD_MODULE_HEADER_EX, NULL,
  pdo_snowflake_deps,
  "pdo_snowflake",
  pdo_snowflake_functions,
  PHP_MINIT(pdo_snowflake),
  PHP_MSHUTDOWN(pdo_snowflake),
  NULL,
  NULL,
  PHP_MINFO(pdo_snowflake),
  PHP_PDO_SNOWFLAKE_VERSION,
  PHP_MODULE_GLOBALS(pdo_snowflake),
  PHP_GINIT(pdo_snowflake),
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */
