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
    $count = $dbh->exec("create temporary table t (c1 int, c2 string, c3 boolean)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $sth = $dbh->prepare("insert into t(c1,c2,c3) values(?,?,?)");

    // data are taken as String if no data type is specified
    $data = array(11, "test111", 1);
    $ret = $sth->execute($data);
    $data = array(12, "test112", 0);
    $ret = $sth->execute($data);
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    // data are taken as String if no data type is specified
    $c1 = 13;
    $c2 = "test113";
    $c3 = TRUE;
    $sth->bindParam(1, $c1);
    $sth->bindParam(2, $c2);
    $sth->bindParam(3, $c3);
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

    // specify data types
    $c1 = 15;
    $c2 = "test115";
    $c3 = FALSE;
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $sth->bindParam(2, $c2, PDO::PARAM_STR);
    $sth->bindParam(3, $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

   // using bindValue()
    $c1 = 16;
    $c2 = "test116";
    $c3 = TRUE;
    $sth->bindValue(1, $c1, PDO::PARAM_INT);
    $sth->bindValue(2, $c2, PDO::PARAM_STR);
    $sth->bindValue(3, $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    // insert null
    $c1 = null;
    $c2 = null;
    $c3 = null;
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $sth->bindParam(2, $c2, PDO::PARAM_STR);
    $sth->bindParam(3, $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution failed.\n";
    }
    echo "inserted rows: " . $sth->rowCount() . "\n";

    echo "==> fetch by default\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . " " . $row["C3"] . "\n";
    }

    echo "==> bind Column\n";
    $sth = $dbh->query("select * from t order by 1");
    $sth->bindColumn(1, $id);
    $sth->bindColumn(2, $name);
    $sth->bindColumn(3, $flag);
    while($row = $sth->fetch(PDO::FETCH_BOUND)) {
        echo $id. " " . $name . " " . $flag . "\n";
    }

    echo "==> bind Column with type\n";
    $sth = $dbh->query("select * from t order by 1");
    $sth->bindColumn(1, $id, PDO::PARAM_INT);
    $sth->bindColumn(2, $name, PDO::PARAM_STR);
    $sth->bindColumn(3, $flag, PDO::PARAM_BOOL);
    while($row = $sth->fetch(PDO::FETCH_BOUND)) {
        echo $id. " " . $name . " " . $flag . "\n";
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
inserted rows: 1
inserted rows: 1
inserted rows: 1
==> fetch by default
11 test111 1
12 test112 
13 test113 1
14 test113 1
15 test115 
16 test116 1
  
==> bind Column
11 test111 1
12 test112 
13 test113 1
14 test113 1
15 test115 
16 test116 1
  
==> bind Column with type
11 test111 1
12 test112 
13 test113 1
14 test113 1
15 test115 
16 test116 1
0  
Expected ERR: 2049
===DONE===
