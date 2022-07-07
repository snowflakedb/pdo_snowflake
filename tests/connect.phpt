--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // full parameters
    $dbh = new PDO("$dsn;application=phptest", $user, $password);
    // create table for testing autocommit later
    $tablename = "autocommittest" . rand();
    $count = $dbh->exec("create or replace table " . $tablename . "(c1 int)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;

    if (!array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        // connect with the minimum requirement
        // This test runs only on Travis or the connect parameters
        // are for production.
        $dbh = new PDO("snowflake:account=$account", $user, $password);
        $dbh = null;
    }
    echo "OK\n";

    // test auto commit in connect options
    // default to true
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (1)");
    $dbh = null;

    // set to true
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_AUTOCOMMIT => true]);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (2)");
    $dbh = null;

    // set to false
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_AUTOCOMMIT => false]);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (3)");
    $dbh = null;

    $dbh = new PDO("$dsn;application=phptest", $user, $password);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // clean up
    $count = $dbh->exec("drop table if exists " . $tablename);
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
OK
1
2
2
===DONE===

