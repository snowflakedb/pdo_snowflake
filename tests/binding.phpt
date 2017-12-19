--TEST--
pdo_snowflake - binding
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO($dsn, $user, $password);
        echo "Connected to Snowflake\n";
        $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }

        $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");

        // looks like every data are taken as STRING data type
        $data = array(11, "test111");
        $ret = $sth->execute($data);
        if (!$ret) {
            echo "Execution failed.\n";
        }
        echo "inserted rows: " . $sth->rowCount() . "\n";
        $data = array(12, "test112");
        $ret = $sth->execute($data);
        if (!$ret) {
            echo "Execution failed.\n";
        }
        echo "inserted rows: " . $sth->rowCount() . "\n";

        $sth = $dbh->prepare("insert into t(c1,c2) values(?,?)");

        // looks like every data are taken as STRING data type if type
        // is not specified.
        $c1 = 13;
        $c2 = "test113";
        $sth->bindParam(1, $c1);
        $sth->bindParam(2, $c2);
        $ret = $sth->execute();
        if (!$ret) {
            echo "Execution failed.\n";
        }

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
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inserted rows: 1
inserted rows: 1
11 test111
12 test112
13 test113
===DONE===
