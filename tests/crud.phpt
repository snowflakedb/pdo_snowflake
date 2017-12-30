--TEST--
pdo_snowflake - CRUD
--INI--
pdo_snowflake.log=/tmp/sflog
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create or replace table t (c1 int, c2 string)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    // insert
    $count = $dbh->exec(
        "insert into t(c1,c2) values(1, 'test1'),(2,'test2'),(3,'test3')");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    } else {
        echo "inserted rows: " . $count . "\n";
    }
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    // update
    $sth = $dbh->prepare("update t set c2=? where c1=?");
    $c1 = 3;
    $c2 = "test101";
    $sth->bindParam(1, $c2, PDO::PARAM_STR);
    $sth->bindParam(2, $c1, PDO::PARAM_INT);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "updated rows: " . $sth->rowCount() . "\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    // delete
    $sth = $dbh->prepare("delete from t where c1=?");
    $c1 = 2;
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "deleted rows: " . $sth->rowCount() . "\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    // insert with binding
    $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");
    $c1 = 11;
    $c2 = "test111";
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $sth->bindParam(2, $c2, PDO::PARAM_STR);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    $c1 = 12;
    $c2 = "test112";
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $sth->bindParam(2, $c2, PDO::PARAM_STR);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    $sth = $dbh->query("select * from t order by 1");
    $meta = $sth->getColumnMeta(0);
    echo sprintf(
        "name: %s, native_type: %s, len: %d, precision: %d, scale: %d\n",
         $meta["name"], $meta["native_type"], $meta["len"],
          $meta["precision"], $meta["scale"]);
    $meta = $sth->getColumnMeta(1);
    echo sprintf(
        "name: %s, native_type: %s, len: %d, precision: %d, scale: %d\n",
         $meta["name"], $meta["native_type"], $meta["len"],
          $meta["precision"], $meta["scale"]);
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }
    $count = $dbh->exec("drop table if exists t");
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inserted rows: 3
1 test1
2 test2
3 test3
updated rows: 1
1 test1
2 test2
3 test101
deleted rows: 1
1 test1
3 test101
inserted rows: 1
inserted rows: 1
name: C1, native_type: FIXED, len: 0, precision: 38, scale: 0
name: C2, native_type: TEXT, len: 16777216, precision: 0, scale: 0
1 test1
3 test101
11 test111
12 test112
===DONE===

