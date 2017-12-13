--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    $p = parse_ini_file(__DIR__ . "/../testenv.ini");

    $account = $p['SNOWFLAKE_TEST_ACCOUNT'];
    $user = $p['SNOWFLAKE_TEST_USER'];
    $password = $p['SNOWFLAKE_TEST_PASSWORD'];
    $database = $p['SNOWFLAKE_TEST_DATABASE'];
    $schema = $p['SNOWFLAKE_TEST_SCHEMA'];
    $warehouse = $p['SNOWFLAKE_TEST_WAREHOUSE'];
    $role = $p['SNOWFLAKE_TEST_ROLE'];

    if (array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        $host = $p['SNOWFLAKE_TEST_HOST'];
    } else {
        $host = $account . ".snowflakecomputing.com";
    }
    $port = "443";
    if (array_key_exists('SNOWFLAKE_TEST_PORT', $p)) {
        $port = $p['SNOWFLAKE_TEST_PORT'];
    }
    $protocol="https";
    if (array_key_exists('SNOWFLAKE_TEST_PROTOCOL', $p)) {
        $protocol = $p['SNOWFLAKE_TEST_PROTOCOL'];
    }

    $dsn = "snowflake:host=$host;port=$port;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role;protocol=$protocol";

    try {
        $dbh = new PDO($dsn, $user, $password);
        echo 'Connected to Snowflake' . "\n";
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
===DONE===

