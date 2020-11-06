--TEST--
pdo_snowflake - transaction
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    /*
     * NOTE: no autocommit=false option is supported by PDO Snowflake.
     * Based on PHP PDO doc, autocommit is on right after connection.
     * When beginTransaction() is called, autocommit is turned off and
     * all DMLs will be committed by commit() or rollbacked by rollback()
     * and autocommit is turned on.
     */
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    echo "inTransaction() before beginTransaction() return ";
    echo $dbh->inTransaction() ? "True\n" : "False\n";
    $dbh->beginTransaction();
    echo "inTransaction() after beginTransaction() return ";
    echo $dbh->inTransaction() ? "True\n" : "False\n";
    $dbh->exec("insert into t (c1, c2) values (23, 'Joe'),(25, 'Mary')");
    $dbh->commit();
    echo "inTransaction() after commit() return ";
    echo $dbh->inTransaction() ? "True\n" : "False\n";
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
        echo "Expected: " . $e->getMessage() . "\n";
        $dbh->rollback();
    }

    /* should be rollbacked and you should not see Ken */
    echo "Ken should not show up\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
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

    /* should be rollbacked and you should not see Ken */
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    $count = $dbh->exec("drop table if exists t");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;

    // rollback without beginTransaction
    $dbh = new PDO($dsn, $user, $password);
    echo "Connected to Snowflake\n";
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh->exec("insert into t (c1, c2) values (23, 'Joe'),(25, 'Mary')");
    try {
        $dbh->rollback();
        throw new Exception("must fail");
    } catch (PDOException $e) {
        echo "Expected: ".  $e->getMessage() . "\n";
    }
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inTransaction() before beginTransaction() return False
inTransaction() after beginTransaction() return True
inTransaction() after commit() return False
23 Joe
25 Mary
Ken is inserted in a transaction
Expected: SQLSTATE[42000]: Syntax error or access violation: 1003 SQL compilation error:
syntax error line 1 at position 14 unexpected '<EOF>'.
Ken should not show up
23 Joe
25 Mary
Ken is inserted in a transaction again but the connection is closed.
Connected to Snowflake
23 Joe
25 Mary
Connected to Snowflake
Expected: There is no active transaction
===DONE===

