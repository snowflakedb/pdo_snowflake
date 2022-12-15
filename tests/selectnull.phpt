--TEST--
pdo_snowflake - insert and select null data
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create temporary table t (c1 varchar, c2 binary, c3 boolean, c4 number, c5 date, c6 time, c7 timestamp_tz)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?,?,?,?,?,?)");

    $v1 = null;
    $sth->bindParam(1, $v1, PDO::PARAM_STR);
    $v2 = null;
    $sth->bindParam(2, $v2, PDO::PARAM_LOB);
    $v3 = null;
    $sth->bindParam(3, $v3, PDO::PARAM_BOOL);
    $v4 = null;
    $sth->bindParam(4, $v4, PDO::PARAM_INT);
    $v5 = null;
    $sth->bindParam(5, $v5, PDO::PARAM_NULL);
    $v6 = null;
    $sth->bindParam(6, $v6);
    $v7 = null;
    $sth->bindParam(7, $v7);
    $sth->execute();

    $sth = $dbh->query("select * from t");

   while($row = $sth->fetch()) {
        for ($i = 0; $i <= 6; $i++) {
            if (is_null($row[$i])) {
                $value = "NULL";
            } else {
                $value = "NOT NULL";
            }
            echo sprintf("C%d: %s", $i + 1, $value);
            if ($i == 6) {
                echo "\n";
            } else {
                echo ", ";
            }
        }
    }

    $count = $dbh->exec("drop table if exists t");
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
C1: NULL, C2: NULL, C3: NULL, C4: NULL, C5: NULL, C6: NULL, C7: NULL
===DONE===
