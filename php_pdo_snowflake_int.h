/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifndef PHP_PDO_SNOWFLAKE_INT_H
#define PHP_PDO_SNOWFLAKE_INT_H

ZEND_BEGIN_MODULE_GLOBALS(pdo_snowflake)
ZEND_END_MODULE_GLOBALS(pdo_snowflake)

ZEND_EXTERN_MODULE_GLOBALS(pdo_snowflake)
#define PDO_SNOWFLAKE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(pdo_snowflake, v)
#if defined(ZTS) && defined(COMPILE_DL_PDO_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

extern pdo_driver_t pdo_snowflake_driver;
extern struct pdo_stmt_methods snowflake_stmt_methods;
extern int _pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line);
#define pdo_snowflake_error(s) _pdo_snowflake_error(s, NULL, __FILE__, __LINE__)
#define pdo_snowflake_error_stmt(s) _pdo_snowflake_error(stmt->dbh, stmt, __FILE__, __LINE__)

enum {
    PDO_SNOWFLAKE_ATTR_SSL_CAPATH = PDO_ATTR_DRIVER_SPECIFIC,
	PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS
};

#endif /* PHP_PDO_SNOWFLAKE_INT_H */
