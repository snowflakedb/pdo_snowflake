--TEST--
pdo_snowflake - insert and select TIMESTAMP_NTZ data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create temporary table t (c1 int, c2 timestamp_ntz)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, "2017-09-27 05:17:30.987654"],
            "output" => [1, "2017-09-27 05:17:30.987654000"],
        ],
        [
            "input" => [2, "1969-11-21 08:19:34.123"],
            "output" => [2, "1969-11-21 08:19:34.123000000"],
        ],
        [
            "input" => [3, "1600-01-01 00:00:00.000"],
            "output" => [3, "1600-01-01 00:00:00.000000000"],
        ],
        [
            "input" => [4, "0001-01-01 00:00:00.000"],
            "output" => [4, "1-01-01 00:00:00.000000000"],
        ],
        [
            "input" => [5, "0000-01-01 00:00:00.000"],
            "output" => [5, "0-01-01 00:00:00.000000000"],
        ],
        [
            "input" => [6, null],
            "output" => [6, null],
        ],
        [
            "input" => [7, "9999-12-31 23:59:59.999999"],
            "output" => [7, "9999-12-31 23:59:59.999999000"],
        ],
    ];

    foreach ($tests as $t) {
        $v1 = $t["input"][0];
        $sth->bindParam(1, $v1);
        $v2 = $t["input"][1];
        $sth->bindParam(2, $v2);
        $sth->execute();
    }

    /* SELECT */
    $sth = $dbh->query("select * from t order by 1");
    $meta = $sth->getColumnMeta(0);
    print_r($meta);
    $meta = $sth->getColumnMeta(1);
    print_r($meta);

    echo "Results in String\n";
    while($row = $sth->fetch()) {
        $idx = $row[0];
        echo sprintf("C1: %s, C2: %s\n", $idx, $row[1]);
    }
    $count = $dbh->exec("drop table if exists t");

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
Array
(
    [scale] => 0
    [native_type] => FIXED
    [flags] => Array
        (
        )

    [name] => C1
    [len] => 0
    [precision] => 38
    [pdo_type] => 2
)
Array
(
    [scale] => 9
    [native_type] => TIMESTAMP_NTZ
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 0
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 1, C2: 2017-09-27 05:17:30.987654000
C1: 2, C2: 1969-11-21 08:19:34.123000000
C1: 3, C2: 1600-01-01 00:00:00.000000000
C1: 4, C2: %r0{0,3}%r1-01-01 00:00:00.000000000
C1: 5, C2: %r0{0,3}%r0-01-01 00:00:00.000000000
C1: 6, C2: 
C1: 7, C2: 9999-12-31 23:59:59.999999000
===DONE===
