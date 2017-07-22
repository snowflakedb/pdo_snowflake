dnl $Id$
dnl config.m4 for extension pdo_snowflake

PHP_ARG_ENABLE(pdo_snowflake, for Snowflake DB support for PDO,
    [  --enable-pdo_snowflake           Enable pdo_snowflake support])

if test "$PHP_PDO_SNOWFLAKE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-pdo_snowflake -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pdo_snowflake.h"  # you most likely want to change this
  dnl if test -r $PHP_PDO_SNOWFLAKE/$SEARCH_FOR; then # path given as parameter
  dnl   PDO_SNOWFLAKE_DIR=$PHP_PDO_SNOWFLAKE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pdo_snowflake files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PDO_SNOWFLAKE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PDO_SNOWFLAKE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pdo_snowflake distribution])
  dnl fi

  dnl # --with-pdo_snowflake -> add include path
  dnl PHP_ADD_INCLUDE($PDO_SNOWFLAKE_DIR/include)

  dnl # --with-pdo_snowflake -> check for lib and symbol presence
  dnl LIBNAME=pdo_snowflake # you may want to change this
  dnl LIBSYMBOL=pdo_snowflake # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PDO_SNOWFLAKE_DIR/$PHP_LIBDIR, PDO_SNOWFLAKE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PDO_SNOWFLAKELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong pdo_snowflake lib version or lib not found])
  dnl ],[
  dnl   -L$PDO_SNOWFLAKE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PDO_SNOWFLAKE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pdo_snowflake, pdo_snowflake.c snowflake_driver.c snowflake_stmt.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
