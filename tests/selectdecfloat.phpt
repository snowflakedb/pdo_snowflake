--TEST--
pdo_snowflake - insert and select Decfloat data type
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

    $count = $dbh->exec(
        "create temporary table t (c1 int, c2 decfloat)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, "123.456"],
            "output" => [1, "123.456"],
        ],
        [
            "input" => [2, "123456789012345678901234567890123456.781234"],
            "output" => [2, "123456789012345678901234567890123456.78"],
        ],
        [
            "input" => [3, "123456789012345678901234567890123456785"],
            "output" => [3, "1.2345678901234567890123456789012345679e38"],
        ],
        [
            "input" => [4, "123456789012345678901234567890123456780000"],
            "output" => [4, "1.2345678901234567890123456789012345678e41"],
        ],
        [
            "input" => [5, "0.0000000000000000000000000123456789012"],
            "output" => [5, "0.0000000000000000000000000123456789012"],
        ],
        [
            "input" => [6, "0.00000000000000000000000000000123456789012345678901"], 
            "output" => [6, "1.23456789012345678901e-30"],
        ],
        [
            "input" => [7, "1e37"], // not recognized date error
            "output" => [7, "10000000000000000000000000000000000000"],
        ],
        [
            "input" => [8, "-1e-38"],
            "output" => [8, "-1e-38"],
        ],
    ];

    foreach ($tests as $t) {
        $v1 = $t["input"][0];
        $sth->bindParam(1, $v1);
        $v2 = $t["input"][1];
        $sth->bindParam(2, $v2);
        try {
            $sth->execute();
        } catch(PDOException $e) {
            if ($e->errorInfo[1] != $t["input"][2]) {
                throw $e;
            }
            echo sprintf(
                "Expected ERR: testcase #%d -- %s\n",
                 $t["input"][0], $e->errorInfo[1]);
        }
    }

    /* SELECT */
    $sth = $dbh->query("select * from t order by 1");

    $meta = $sth->getColumnMeta(0);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(1);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);

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
name: C1, native_type: FIXED, scale: 0, precision: 38, len: 0
name: C2, native_type: DECFLOAT, scale: 0, precision: 38, len: 50
Results in String
C1: 1, C2: 123.456
C1: 2, C2: 123456789012345678901234567890123456.78
C1: 3, C2: 1.2345678901234567890123456789012345679e38
C1: 4, C2: 1.2345678901234567890123456789012345678e41
C1: 5, C2: 0.0000000000000000000000000123456789012
C1: 6, C2: 1.23456789012345678901e-30
C1: 7, C2: 10000000000000000000000000000000000000
C1: 8, C2: -1e-38
===DONE===
