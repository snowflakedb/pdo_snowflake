--TEST--
pdo_snowflake - error handlings
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
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

    try {
        $dbh = new PDO($dsn, $user, $password);
        echo 'Connected to Snowflake' . "\n";
        $sth = $dbh->query("select 1 frooom dual");
        $earr = $dbh->errorInfo();
        echo "sqlstate: " . $earr[0] . "\n";
        echo "Snowflake Error: " . $earr[1] . "\n";
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "dsn is: $dsn";
    }

    $dbh = null; // make sure closing db.

   try {
        $dbh = new PDO($dsn, "HIHIHI", "HAHAHAH");
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
sqlstate: 42000
Snowflake Error: 1003
Connection failed: SQLSTATE[08001] [390100] Incorrect username or password was specified.
===DONE===

