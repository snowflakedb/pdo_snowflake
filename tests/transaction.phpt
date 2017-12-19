--TEST--
pdo_snowflake - transaction
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    echo "Connected to Snowflake\n";

    try {
        $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $dbh->beginTransaction();
        $dbh->exec("insert into t (c1, c2) values (23, 'Joe'),(25, 'Mary')");
        $dbh->commit();
        $sth = $dbh->query("select * from t order by 1");
        while($row = $sth->fetch()) {
            echo $row["C1"] . " " . $row["C2"] . "\n";
        }
        try {
            $dbh->beginTransaction();
            $dbh->exec("insert into t (c1, c2) values (27, 'Ken')");
            echo "Ken is inserted in a transaction\n";
            $dbh->exec("insert into aa");
            $dbh->commit();
        } catch (PDOException $e) {
            echo $e->getMessage() . "\n";
            $dbh->rollback();
        }

        /* should be rollbacked and you should not see Ken */
        echo "Ken should not show up\n";
        $sth = $dbh->query("select * from t order by 1");
        while($row = $sth->fetch()) {
            echo $row["C1"] . " " . $row["C2"] . "\n";
        }
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
        echo "dsn is: $dsn";
    }


    // Trying to insert Ken again but will forget commit
    $dbh->beginTransaction();
    $dbh->exec("insert into t (c1, c2) values (27, 'Ken')");

    // The opened transaction should be closed automatically by PDO.
    $dbh = null;
    echo "Ken is inserted in a transaction again but the connection is closed.\n";

    // New connection
    $dbh = new PDO($dsn, $user, $password);
    echo "Connected to Snowflake\n";

    try {
        /* should be rollbacked and you should not see Ken */
        $sth = $dbh->query("select * from t order by 1");
        while($row = $sth->fetch()) {
            echo $row["C1"] . " " . $row["C2"] . "\n";
        }

        $count = $dbh->exec("drop table if exists t");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
        echo "dsn is: $dsn";
    }

    $dbh = null;

    /* TODO: add autocommit=false tests */
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
23 Joe
25 Mary
Ken is inserted in a transaction
SQLSTATE[42000]: Syntax error or access violation: 1003 SQL compilation error:
syntax error line 1 at position 14 unexpected '<EOF>'.
Ken should not show up
23 Joe
25 Mary
Ken is inserted in a transaction again but the connection is closed.
Connected to Snowflake
23 Joe
25 Mary
===DONE===

