#!/bin/bash
export PHP_HOME=/opt/homebrew
cd "$(dirname "$0")"
$PHP_HOME/bin/php \
  -dextension=modules/pdo_snowflake.so \
  -dpdo_snowflake.logdir=sflog \
  -dpdo_snowflake.loglevel=DEBUG \
  -dpdo_snowflake.cacert=libsnowflakeclient/cacert.pem \
  tests/ipv6_connectivity_auto.php
