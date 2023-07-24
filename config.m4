dnl $Id$
dnl config.m4 for extension pdo_snowflake

# AC_CANONICAL_HOST is needed to access the 'host_os' variable    
AC_CANONICAL_HOST

build_linux=no
build_mac=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    darwin*)
        build_mac=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

PHP_ARG_ENABLE(pdo_snowflake, for Snowflake DB support for PDO,
    [  --enable-pdo_snowflake           Enable pdo_snowflake support])
PHP_ARG_ENABLE(coverage, whether to include code coverage symbols,
    [  --enable-coverage       Enable coverage support], no, no)

if test "$PHP_PDO_SNOWFLAKE" != "no"; then
  PHP_SNOWFLAKE_LIBNAME=libsnowflake

  dnl # --with-pdo_snowflake -> check with-path
  SEARCH_PATH="libsnowflakeclient"
  SEARCH_FOR="include/snowflake/client.h"
  if test -r $PHP_PDO_SNOWFLAKE/$SEARCH_FOR; then # path given as parameter
    SNOWFLAKE_CLIENT_DIR=$PHP_PDO_SNOWFLAKE
  else # search default path list
    AC_MSG_CHECKING([for libsnowflakeclient files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        SNOWFLAKE_CLIENT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  if test -z "$SNOWFLAKE_CLIENT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the pdo_snowflake distribution])
  fi

  dnl # --with-pdo_snowflake -> add include path
  PHP_ADD_INCLUDE($SNOWFLAKE_CLIENT_DIR/include)

  dnl # link Snowflake client library, not working as
  dnl # the position of -Wl,--whole-archive 
  dnl # (or -Wl,-force_load in OS X) is ignored by libtool
  LDFLAGS="$LDFLAGS -fPIC"
  if test "$build_mac" == "yes"; then
    LDFLAGS="$LDFLAGS -dynamiclib -undefined dynamic_lookup"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/lib/darwin/libsnowflakeclient.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/openssl/lib/libcrypto.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/openssl/lib/libssl.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/curl/lib/libcurl.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/oob/lib/libtelemetry.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/aws/lib/libaws-cpp-sdk-core.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/aws/lib/libaws-cpp-sdk-s3.a"
    LDFLAGS="$LDFLAGS -Wl,-force_load,$SNOWFLAKE_CLIENT_DIR/deps-build/darwin/azure/lib/libazure-storage-lite.a"
  fi
  if test "$build_linux" == "yes"; then
    LDFLAGS="$LDFLAGS -Wl,--whole-archive"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/lib/linux/libsnowflakeclient.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/openssl/lib/libcrypto.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/openssl/lib/libssl.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/curl/lib/libcurl.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/oob/lib/libtelemetry.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/aws/lib64/libaws-cpp-sdk-core.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/aws/lib64/libaws-cpp-sdk-s3.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/azure/lib/libazure-storage-lite.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/arrow/lib/libarrow.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/arrow_deps/lib/libjemalloc_pic.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/boost/lib/libboost_filesystem.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/boost/lib/libboost_regex.a"
    LDFLAGS="$LDFLAGS $SNOWFLAKE_CLIENT_DIR/deps-build/linux/boost/lib/libboost_system.a"
    LDFLAGS="$LDFLAGS -Wl,--no-whole-archive"
  fi
  dnl # IMPORTANT NOTE: Change scripts/build_pdo_snowflake.sh to update
  dnl # the actual link options for pdo_snowflake.so

  CFLAGS="-std=c99 -Werror $CFLAGS"

  PHP_NEW_EXTENSION(
    pdo_snowflake,
    pdo_snowflake.c snowflake_driver.c snowflake_stmt.c snowflake_paramstore.c snowflake_arraylist.c snowflake_treemap.c snowflake_rbtree.c,
    $ext_shared,
    ,
    "-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1")
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
  lcov_version_list="1.5 1.6 1.7 1.9 1.10 1.11 1.12 1.13 1.14 1.15 1.16"
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
