--TEST--
pdo_snowflake - CRUD
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
        echo "Connected to Snowflake\n";
        $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
        /* TODO: error check */
        $count = $dbh->exec("insert into t(c1,c2) values(1, 'test1'),(2,'test2'),(3,'test3')");
        /* TODO: error check and affected rows */
        $sth = $dbh->query("select * from t order by 1");
        while($row = $sth->fetch()) {
            echo $row["C1"] . " " . $row["C2"] . "\n";
        }
        $count = $dbh->exec("delete from t");
        /* TODO: error check and affected rows */
        $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");
        $c1 = 101;
        $c2 = "test101";
        $sth->bindParam(1, $c1, PDO::PARAM_INT);
        $sth->bindParam(2, $c2, PDO::PARAM_STR);
        $ret = $sth->execute();
        if (!$ret) {
            echo "Execution failed.";
        }
        $sth = $dbh->query("select * from t order by 1");
        while($row = $sth->fetch()) {
            echo $row["C1"] . " " . $row["C2"] . "\n";
        }
        $count = $dbh->exec("drop table if exists t");
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
?>
--EXPECT--
Connected to Snowflake
1 test1
2 test2
3 test3
101 test101
