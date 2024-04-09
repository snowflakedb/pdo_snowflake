--TEST--
pdo_snowflake - fetching
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
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
    $data = array(13, "test113", 1);
    $ret = $sth->execute($data);
    $data = array(14, "test114", 0);
    $ret = $sth->execute($data);
    $data = array(15, "test115", 1);
    $ret = $sth->execute($data);
    $data = array(16, "test116", 0);
    $ret = $sth->execute($data);
    $data = array(17, "test117", 1);
    $ret = $sth->execute($data);

    if (!$ret) {
        echo "Execution failed.\n";
    }

    $sth = $dbh->query("select * from t order by 1");
    echo "result columns: " . $sth->columnCount() . "\n";

    echo "==> fetch by default\n";
    if ($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . " " . $row["C3"] . "\n";
    }
    else {
        echo "fetching failed.\n";
    }
    
    echo "==> bind Column\n";
    $sth->bindColumn(1, $id);
    $sth->bindColumn(2, $name);
    $sth->bindColumn(3, $flag);
    if ($row = $sth->fetch(PDO::FETCH_BOUND)) {
        echo $id. " " . $name . " " . $flag . "\n";
    }
    else {
        echo "fetching failed.\n";
    }

    echo "==> fetchColumn\n";
    echo "column 2: " . $sth->fetchColumn(1) . "\n";

    echo "==> PDO::FETCH_OBJ\n";
    $sth->setFetchMode(PDO::FETCH_OBJ);
    if ($row = $sth->fetch()) {
        print_r($row);
    }
    else {
        echo "fetching failed.\n";
    }

    echo "==> fetchAll\n";
    $sth->setFetchMode(PDO::FETCH_ASSOC);
    if ($result = $sth->fetchAll()) {
        print_r($result);
    }
    else {
        echo "fetching failed.\n";
    }
    
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
result columns: 3
==> fetch by default
11 test111 1
==> bind Column
12 test112 
==> fetchColumn
column 2: test113
==> PDO::FETCH_OBJ
stdClass Object
(
    [C1] => 14
    [C2] => test114
    [C3] => 
)
==> fetchAll
Array
(
    [0] => Array
        (
            [C1] => 15
            [C2] => test115
            [C3] => 1
        )

    [1] => Array
        (
            [C1] => 16
            [C2] => test116
            [C3] => 
        )

    [2] => Array
        (
            [C1] => 17
            [C2] => test117
            [C3] => 1
        )

)
===DONE===
