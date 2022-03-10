--TEST--
pdo_snowflake - insert and select TIME data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec("create temporary table t (c1 int, c2 time)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, "12:34:56.987"],
            "output" => [1, "12:34:56.987000000"],
        ],
        [
            "input" => [2, "08:19:34.000123"],
            "output" => [2, "08:19:34.000123000"],
        ],
        [
            "input" => [3, null],
            "output" => [3, null],
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
        $expected = $tests[$idx-1]["output"];
        if ($expected[1] != $row[1]) {
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
name: C2, native_type: TIME, scale: 9, precision: 0, len: 0
Results in String
C1: 1, C2: 12:34:56.987000000
C1: 2, C2: 08:19:34.000123000
C1: 3, C2: 
===DONE===
