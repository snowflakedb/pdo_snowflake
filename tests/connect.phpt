--TEST--
PDO_SNOWFLAKE:
--FILE--
<?php
    $p = parse_ini_file(getenv('PWD') . "/testenv.ini");

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

    $ca_bundle_file = $p['SNOWFLAKE_TEST_CA_BUNDLE_FILE'];
    $options = array(PDO::SNOWFLAKE_ATTR_SSL_CAPATH => $ca_bundle_file);
    try {
        $dbh = new PDO($dsn, $user, $password, $options);
        echo 'Connected to Snowflake';
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
?>
--EXPECT--
Connected to Snowflake
