--TEST--
PDO_SNOWFLAKE:
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
    $insecure_mode = (bool) getenv('SNOWFLAKE_TEST_INSECURE_MODE');
    $dsn = "snowflake:host=$host;port=$port;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role";
    $ca_bundle_file = getenv('SNOWFLAKE_TEST_CA_BUNDLE_FILE');
    $options = array(PDO::SNOWFLAKE_ATTR_SSL_CAPATH => $ca_bundle_file);
    try {
        $dbh = new PDO($dsn, $user, $password, $options);
        echo 'Connected to Snowflake';
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "\n";
        echo "dsn is: $dsn\n";
    }

    $dbh = null;
?>
--EXPECT--
Connected to Snowflake
