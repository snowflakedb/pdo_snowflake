--TEST--
pdo_snowflake - put get query
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    echo 'Connected to Snowflake' . "\n";

    $sth = $dbh->query("create or replace temporary table t (c1 varchar, c2 varchar)");
    $sth = $dbh->query("insert into t values ('str1', 'str2'), ('str3', 'str4')");
    $sth = $dbh->query("copy into @%t from t");

    $sth = $dbh->query("get @%t file://.");
    echo 'get command succeeded' . "\n";

    $sth = $dbh->query("drop table t");
    $sth = $dbh->query("create or replace temporary table t (c1 varchar, c2 varchar)");
    $sth = $dbh->query("put file://./data_0_0_0.csv.gz @%t");
    echo 'put command succeeded' . "\n";

    echo 'verifying data' . "\n";
    $sth = $dbh->query("copy into t from @%t");
    $sth = $dbh->query("select * from t");
    while($row = $sth->fetch()) {
        $idx = $row[0];
        echo sprintf("C1: %s, C2: %s\n", $row[0], $row[1]);
    }
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
get command succeeded
put command succeeded
verifying data
C1: str1, C2: str2
C1: str3, C2: str4
===DONE===
