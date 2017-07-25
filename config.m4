dnl $Id$
dnl config.m4 for extension pdo_snowflake

PHP_ARG_ENABLE(pdo_snowflake, for Snowflake DB support for PDO,
    [  --enable-pdo_snowflake           Enable pdo_snowflake support])
PHP_ARG_ENABLE(coverage, whether to include code coverage symbols,
    [  --enable-coverage       Enable coverage support], no, no)

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

if test "$PHP_COVERAGE" = "yes"; then
  if test "$GCC" != "yes"; then
    AC_MSG_ERROR([GCC is required for --enable-coverage])
  fi
  dnl Check if ccache is being used
  case `$php_shtool path $CC` in
    *ccache*[)] gcc_ccache=yes;;
    *[)] gcc_ccache=no;;
  esac
  if test "$gcc_ccache" = "yes" && (test -z "$CCACHE_DISABLE" || test "$CCACHE_DISABLE" != "1"); then
    AC_MSG_ERROR([ccache must be disabled when --enable-coverage option is used. You can disable ccache by setting environment variable CCACHE_DISABLE=1.])
  fi
  lcov_version_list="1.5 1.6 1.7 1.9 1.10 1.11"
  AC_CHECK_PROG(LCOV, lcov, lcov)
  AC_CHECK_PROG(GENHTML, genhtml, genhtml)
  PHP_SUBST(LCOV)
  PHP_SUBST(GENHTML)

  if test "$LCOV"; then
    AC_CACHE_CHECK([for lcov version], php_cv_lcov_version, [
      php_cv_lcov_version=invalid
      lcov_version=`$LCOV -v 2>/dev/null | $SED -e 's/^.* //'` #'
      for lcov_check_version in $lcov_version_list; do
        if test "$lcov_version" = "$lcov_check_version"; then
          php_cv_lcov_version="$lcov_check_version (ok)"
        fi
      done
    ])
  else
    lcov_msg="To enable code coverage reporting you must have one of the following LCOV versions installed: $lcov_version_list"      
    AC_MSG_ERROR([$lcov_msg])
  fi
  case $php_cv_lcov_version in
    ""|invalid[)]
      lcov_msg="You must have one of the following versions of LCOV: $lcov_version_list (found: $lcov_version)."
      AC_MSG_ERROR([$lcov_msg])
      LCOV="exit 0;"
      ;;
  esac
  if test -z "$GENHTML"; then
    AC_MSG_ERROR([Could not find genhtml from the LCOV package])
  fi

  PHP_ADD_MAKEFILE_FRAGMENT

  dnl Remove all optimization flags from CFLAGS
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  changequote([,])

  dnl Add the special gcc flags
  CFLAGS="$CFLAGS -O0 -ggdb -fprofile-arcs -ftest-coverage"
  CXXFLAGS="$CXXFLAGS -ggdb -O0 -fprofile-arcs -ftest-coverage"
fi
