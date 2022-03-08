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
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(1);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(2);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
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
name: C1, native_type: OBJECT, scale: 0, precision: 0, len: 16777216
name: C2, native_type: ARRAY, scale: 0, precision: 0, len: 16777216
name: C3, native_type: VARIANT, scale: 0, precision: 0, len: 16777216
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

