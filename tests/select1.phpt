--TEST--
pdo_snowflake - select1
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
        $sth = $dbh->query("select 1");
        while($row = $sth->fetch()) {
            echo "Result " . $row["1"] . "\n";
            echo "Count " . count($row) . "\n";
        }
        $sth = $dbh->query("select 2");
        while($row = $sth->fetch(PDO::FETCH_ASSOC)) {
            echo "Result " . $row["2"] . "\n";
            echo "Count " . count($row) . "\n";
        }
        $sth = $dbh->query("select 3");
        while($row = $sth->fetch(PDO::FETCH_NUM)) {
            echo "Result " . $row[0] . "\n";
            if (isset($row["3"])) {
                echo "FAIL. row[\"3\"] should not be set.\n";
            } else {
                echo "OK. row[\"3\"] is not set.\n";
            }
            echo "Count " . count($row) . "\n";
        }
        $sth = $dbh->query("select 4");
        while($row = $sth->fetch(PDO::FETCH_BOTH)) {
            # interesting behavior of PHP fetch
            # When a numeric value N is taken as a column label,
            # No value is stored in the index 0 but the N+1 has.
            if (isset($row[0])) {
                echo "FAIL. row[0] should not be set.\n";
            } else {
                echo "OK. row[0] is not set.\n";
            }
            echo "Result " . $row["5"] . "\n";
            echo "Count " . count($row) . "\n";
        }
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "dsn is: $dsn";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Result 1
Count 2
Result 2
Count 1
Result 3
OK. row["3"] is not set.
Count 1
OK. row[0] is not set.
Result 4
Count 2
===DONE===

