--TEST--
pdo_snowflake - multiple statement query
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";

    // Test multi statement query with different multi statement count settings in the session setting.
    $count = $dbh->exec("alter session set MULTI_STATEMENT_COUNT=1");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    } 
    try{
        $sth = $dbh->query("select 1; select 2; select 3; select 4");
    }
    catch(PDOException $e) {
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    try{
        // Wrong multi statement count that is more than the actual statements in the query. It should throw an error.
        $dbh->setAttribute(PDO::SNOWFLAKE_STMT_MULTI_STMT_COUNT, 5);
        $sth = $dbh->query("select 1; select 2; select 3; select 4");
    }
    catch(PDOException $e) {
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    $dbh->setAttribute(PDO::SNOWFLAKE_STMT_MULTI_STMT_COUNT, 4);
    $sth = $dbh->query("select 1; select 2; select 3; select 4");
    while($row = $sth->fetch()) {
        echo "Result " . $row["1"] . "\n";
        echo "Count " . count($row) . "\n";
    }
    $sth->nextRowset();
    while($row = $sth->fetch(PDO::FETCH_ASSOC)) {
        echo "Result " . $row["2"] . "\n";
        echo "Count " . count($row) . "\n";
    }
    $sth->nextRowset();
    while($row = $sth->fetch(PDO::FETCH_NUM)) {
        echo "Result " . $row[0] . "\n";
        if (isset($row["3"])) {
            echo "FAIL. row[\"3\"] should not be set.\n";
        } else {
            echo "OK. row[\"3\"] is not set.\n";
        }
        echo "Count " . count($row) . "\n";
    }

    $sth->nextRowset();
    while($row = $sth->fetch(PDO::FETCH_BOTH)) {
        echo "Result " . $row[0] . "\n";
        echo "Result " . $row["4"] . "\n";
        echo "Count " . count($row) . "\n";
    }

    //Unset multi statement count and test the default behavior.
    $dbh->setAttribute(PDO::SNOWFLAKE_STMT_MULTI_STMT_COUNT, -1);
    try{
        $sth = $dbh->query("select 1; select 2; select 3; select 4");
    }
    catch(PDOException $e) {
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    $sth = $dbh->query("select 1");
    while($row = $sth->fetch()) {
        echo "Result " . $row["1"] . "\n";
        echo "Count " . count($row) . "\n";
    }

   //Control the multi statement count through session variable and test the behavior.     
    $count = $dbh->exec("alter session set MULTI_STATEMENT_COUNT=0");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    } 
    $sth = $dbh->query("create or replace temporary table test_multi_large(c1 number, c2 number); insert into test_multi_large select seq4(), TO_VARCHAR(seq4()) from table(generator(rowcount => 100000));select * from test_multi_large order by c1",);
    $row = $sth->fetch();
    echo "Result " . $row["status"] . "\n";

    $sth->nextRowset();
    $count = $sth->rowCount();
    echo "Inserted rows: " . $count . "\n";

    $sth->nextRowset();
    $row = $sth->fetch();
    $count = $sth->rowCount();
    echo "Selected rows: " . $count . "\n";

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
Expected error: SQLSTATE[0A000]: Feature not supported: 8 Actual statement count 4 did not match the desired statement count 1.
Expected error: SQLSTATE[0A000]: Feature not supported: 8 Actual statement count 4 did not match the desired statement count 5.
Result 1
Count 2
Result 2
Count 1
Result 3
OK. row["3"] is not set.
Count 1
Result 4
Result 4
Count 2
Expected error: SQLSTATE[0A000]: Feature not supported: 8 Actual statement count 4 did not match the desired statement count 1.
Result 1
Count 2
Result Table TEST_MULTI_LARGE successfully created.
Inserted rows: 100000
Selected rows: 100000
===DONE===
