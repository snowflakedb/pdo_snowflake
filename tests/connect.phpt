--TEST--
PDO_SNOWFLAKE:
--SKIPIF--
<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_snowflake')) die('skip not loaded');
?>
--FILE--
<?php
    $host = getenv('SNOWFLAKE_TEST_HOST');
    $port = getenv('SNOWFLAKE_TEST_PORT');
    $account = getenv('SNOWFLAKE_TEST_ACCOUNT');
    $user = getenv('SNOWFLAKE_TEST_USER');
    $password = getenv('SNOWFLAKE_TEST_PASSWORD');
    $database = getenv('SNOWFLAKE_TEST_DATABASE');
    $schema = getenv('SNOWFLAKE_TEST_SCHEMA');
    $warehouse = getenv('SNOWFLAKE_TEST_WAREHOUSE');
    $role = getenv('SNOWFLAKE_TEST_ROLE');
    $protocol = getenv('SNOWFLAKE_TEST_PROTOCOL');
    $insecure_mode = (bool) getenv('SNOWFLAKE_TEST_INSECURE_MODE');
    $dsn = "snowflake:host=$host;port=$port;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role";
    try {
        echo "dsn is: $dsn";
        $dbh = new PDO($dsn, $user, $password);
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
    }
?>
--EXPECT--
OK