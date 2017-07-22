--TEST--
PDO_SNOWFLAKE: 
--SKIPIF--
<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_snowflake')) die('skip not loaded');
?>
--FILE--
<?php
echo "OK\n";

--EXPECT--
OK
