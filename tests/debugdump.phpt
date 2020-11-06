--TEST--
pdo_snowflake - debugdump
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";
    $count = $dbh->exec("create or replace table t (c1 int, c2 string, c3 boolean)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $sth = $dbh->prepare("insert into t(c1,c2,c3) values(?,?,?)");

    // data are taken as String if no data type is specified
    $data = array(11, "test111", 1);
    $ret = $sth->execute($data);
    if (!$ret) {
        echo "Execution failed.\n";
    }

    $sth->debugDumpParams();

    $count = $dbh->exec("drop table if exists t");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
SQL: [37] insert into t(c1,c2,c3) values(?,?,?)
Params:  3
Key: Position #0:
paramno=0
name=[0] ""
is_param=1
param_type=2
Key: Position #1:
paramno=1
name=[0] ""
is_param=1
param_type=2
Key: Position #2:
paramno=2
name=[0] ""
is_param=1
param_type=2
===DONE===
