--TEST--
pdo_snowflake - binding
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";
    $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");

    // data are taken as String if no data type is specified
    $data = array(11, "test111");
    $ret = $sth->execute($data);
    $data = array(12, "test112");
    $ret = $sth->execute($data);
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    // data are taken as String if no data type is specified
    $c1 = 13;
    $c2 = "test113";
    $sth->bindParam(1, $c1);
    $sth->bindParam(2, $c2);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    // reusing the previously bound parameter
    $c1 = 14;
    $sth->bindParam(1, $c1);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }

    echo "==> fetch by default\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    echo "==> bind Column\n";
    $sth = $dbh->query("select * from t order by 1");
    $sth->bindColumn(1, $id);
    $sth->bindColumn(2, $name);
    while($row = $sth->fetch(PDO::FETCH_BOUND)) {
        echo $id. " " . $name . "\n";
    }

    echo "==> bind Column with type\n";
    $sth = $dbh->query("select * from t order by 1");
    $sth->bindColumn(1, $id, PDO::PARAM_INT);
    $sth->bindColumn(2, $name, PDO::PARAM_STR);
    while($row = $sth->fetch(PDO::FETCH_BOUND)) {
        echo $id. " " . $name . "\n";
    }

    // not bound error
    $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");
    // data are taken as String if no data type is specified
    $c1 = 15;
    $sth->bindParam(1, $c1);
    try {
        $ret = $sth->execute();
    } catch(PDOException $e) {
        echo sprintf("Expected ERR: %d\n", $sth->errorInfo()[1]);
    }

    $count = $dbh->exec("drop table if exists t");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inserted rows: 1
inserted rows: 1
==> fetch by default
11 test111
12 test112
13 test113
14 test113
==> bind Column
11 test111
12 test112
13 test113
14 test113
==> bind Column with type
11 test111
12 test112
13 test113
14 test113
Expected ERR: 2049
===DONE===
