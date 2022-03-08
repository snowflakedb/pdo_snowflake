--TEST--
pdo_snowflake - insert and select BOOLEAN data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create temporary table t (c1 int, c2 boolean)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, True],
            "output" => [1, True]
        ],
        [
            "input" => [2, False],
            "output" => [2, False]
        ],
        [
            "input" => [3, ""],
            "output" => [3, True] /* TODO: why empty string is True? */
        ],
        [
            "input" => [4, null],
            "output" => [4, null]
        ],
        [
            "input" => [5, "tests"],
            "output" => [5, "tests"]
        ]
    ];

    foreach ($tests as $t) {
        $v1 = $t["input"][0];
        $sth->bindParam(1, $v1);
        $v2 = $t["input"][1];
        $sth->bindParam(2, $v2, PDO::PARAM_BOOL);
        $sth->execute();
    }

    $sth = $dbh->query("select * from t order by 1");

    $meta = $sth->getColumnMeta(0);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(1);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);

    echo "Results in String\n";

    while($row = $sth->fetch()) {
        $idx = $row[0];
        echo sprintf("C1: %s, ", $idx);
        if ($row[1]) {
            echo "C2: TRUE\n";
        } else {
            echo "C2: FALSE\n";
        }
        $expected = $tests[$idx-1]["output"];
        if ((bool)$expected[1] != (bool)$row[1]) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[1], $row[1]);
        }
    }
    $count = $dbh->exec("drop table if exists t");
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
name: C1, native_type: FIXED, scale: 0, precision: 38, len: 0
name: C2, native_type: BOOLEAN, scale: 0, precision: 0, len: 1
Results in String
C1: 1, C2: TRUE
C1: 2, C2: FALSE
C1: 3, C2: TRUE
C1: 4, C2: FALSE
C1: 5, C2: TRUE
===DONE===
