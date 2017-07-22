--TEST--
PDO_SNOWFLAKE: 
--SKIPIF--
<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_snowflake')) die('skip not loaded');
print("Hello World");
?>
