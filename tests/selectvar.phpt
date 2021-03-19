--TEST--
pdo_snowflake - insert and select VARIANT, OBJECT and ARRAY
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";
    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec("create temporary table t (c1 object, c2 array, c3 variant)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t select parse_json(?),parse_json(?),parse_json(?)");
    $v1 = "{\"test1\":1}";
    $sth->bindParam(1, $v1);
    $v2 = "[1,2,3]";
    $sth->bindParam(2, $v2);
    $v3 = "[456,789]";
    $sth->bindParam(3, $v3);
    $sth->execute();
    $sth = $dbh->query("select * from t order by 1");
    $meta = $sth->getColumnMeta(0);
    print_r($meta);
    $meta = $sth->getColumnMeta(1);
    print_r($meta);
    $meta = $sth->getColumnMeta(2);
    print_r($meta);
    while($row = $sth->fetch()) {
        print_r(json_decode($row["C1"], true));
        print_r(json_decode($row["C2"], true));
        print_r(json_decode($row["C3"], true));
    }

    $count = $dbh->exec("drop table if exists t");

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Array
(
    [scale] => 0
    [native_type] => OBJECT
    [flags] => Array
        (
        )

    [name] => C1
    [len] => 16777216
    [precision] => 0
    [pdo_type] => 2
)
Array
(
    [scale] => 0
    [native_type] => ARRAY
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 16777216
    [precision] => 0
    [pdo_type] => 2
)
Array
(
    [scale] => 0
    [native_type] => VARIANT
    [flags] => Array
        (
        )

    [name] => C3
    [len] => 16777216
    [precision] => 0
    [pdo_type] => 2
)
Array
(
    [test1] => 1
)
Array
(
    [0] => 1
    [1] => 2
    [2] => 3
)
Array
(
    [0] => 456
    [1] => 789
)
===DONE===

