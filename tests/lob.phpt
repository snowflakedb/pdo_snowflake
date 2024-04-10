--TEST--
pdo_snowflake - max lob size
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    set_time_limit(720);
    ini_set('memory_limit', '1024M');
    include __DIR__ . "/common.php";

    $MAX_LOB_SIZE = 16 * 1024 * 1024;

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";
    $count = $dbh->exec("create temporary table t (c1 int, c2 varchar(" . $MAX_LOB_SIZE . "), c3 boolean)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $stringData = '';
    for ($i = 0; $i < $MAX_LOB_SIZE / 8; $i++) {
        $stringData .= sprintf("%'08d", $i);
    }

    $ret = $dbh->exec("insert into t(c1,c2,c3) values(1, '" . $stringData . "', true)");
    if (!$ret) {
        echo "Execution with literal failed.\n";
    }
    echo "inserted rows with literal: " . $ret . "\n";

    $sth = $dbh->prepare("insert into t(c1,c2,c3) values(?,?,?)");

    // data are taken as String if no data type is specified
    $data = array(2, $stringData, 0);
    $ret = $sth->execute($data);
    if (!$ret) {
        echo "Execution with array binding failed.\n";
    }
    echo "inserted rows with array binding: " . $sth->rowCount() . "\n";

    $c1 = 3;
    $c3 = TRUE;
    $sth->bindParam(1, $c1, PDO::PARAM_INT);
    $sth->bindParam(2, $stringData, PDO::PARAM_STR);
    $sth->bindParam(3, $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution with bindParam failed.\n";
    }
    echo "inserted rows with bindParam: " . $sth->rowCount() . "\n";

    // using bindValue()
    $c1 = 4;
    $c3 = FALSE;
    $sth->bindValue(1, $c1, PDO::PARAM_INT);
    $sth->bindValue(2, $stringData, PDO::PARAM_STR);
    $sth->bindValue(3, $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution with bindValue failed.\n";
    }
    echo "inserted rows with bindValue: " . $sth->rowCount() . "\n";

    // named binding
    $sth = $dbh->prepare("insert into t(c1,c2,c3) values(:v1,:v2, :v3)");

    $c1 = 5;
    $c3 = TRUE;
    $sth->bindParam(':v1', $c1, PDO::PARAM_INT);
    $sth->bindParam(':v2', $stringData, PDO::PARAM_STR);
    $sth->bindParam(':v3', $c3, PDO::PARAM_BOOL);
    $ret = $sth->execute();
    if (!$ret) {
        echo "Execution with named binding failed.\n";
    }
    echo "inserted rows with named binding: " . $sth->rowCount() . "\n";

    echo "==> verify insert result\n";
    $sth = $dbh->query("select * from t order by 1");
    $sth->bindColumn(1, $id, PDO::PARAM_INT);
    $sth->bindColumn(2, $name, PDO::PARAM_STR);
    $sth->bindColumn(3, $flag, PDO::PARAM_BOOL);
    $rownum = 0;
    while($row = $sth->fetch(PDO::FETCH_BOUND)) {
        $rownum++;
        if ($id != $rownum)
        {
            echo "incorrect id inserted. expected: " . $rownum . "actual: " . $id . "\n";
        }

        if (strlen($name) != $MAX_LOB_SIZE)
        {
            echo "incorrect string length at row: " . $rownum . "with length: " . strlen($name) . "\n";
        }

        if ($name != $stringData)
        {
            echo "incorrect string inserted at row: " . $rownum . "\n";
        }

        if ($flag!= ($rownum % 2 != 0))
        {
            echo "incorrect flag inserted at row: " . $rownum . "\n";
        }
    }
    echo "rows verified: " . $rownum . "\n";

    $count = $dbh->exec("drop table if exists t");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inserted rows with literal: 1
inserted rows with array binding: 1
inserted rows with bindParam: 1
inserted rows with bindValue: 1
inserted rows with named binding: 1
==> verify insert result
rows verified: 5
===DONE===
