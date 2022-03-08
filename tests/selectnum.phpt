--TEST--
pdo_snowflake - insert and select numeric data types
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    /* INSERT very large numeric values in string */
    $count = $dbh->exec(
        "create temporary table t (".
        "c1 int, ".
        "c2 number(38,4), ".
        "c3 number(38,0), ".
        "c4 number(18,0), ".
        "c5 number(18,2))");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?,?,?,?)");

    $tests = [
        [
            "input" => [1,
                        "1234567890123456789012345678901234.5678",
                        "98765432109876543210987654321098765432",
                        "123456789012345678",
                        "1234567890123456.78"],
            "output" => [1,
                        "1234567890123456789012345678901234.5678",
                        "98765432109876543210987654321098765432",
                        "123456789012345678",
                        "1234567890123456.78"],
        ],
        [
            "input" => [2, 123.456, 9.87654, 9.87654, 0],
            "output" => [2, "123.4560", "10", "10", "0.00"],
        ],
        [
            "input" => [3, null, null, null, null],
            "output" => [3, null, null, null, null],
        ],
        [
            "input" => [4, 123.456789, -456.0, -500.0000, -123.456789],
            "output" => [4, "123.4568", "-456", "-500", "-123.46"],
        ],
    ];

    foreach ($tests as $t) {
        $v1 = $t["input"][0];
        $sth->bindParam(1, $v1);
        $v2 = $t["input"][1];
        $sth->bindParam(2, $v2);
        $v3 = $t["input"][2];
        $sth->bindParam(3, $v3);
        $v4 = $t["input"][3];
        $sth->bindParam(4, $v4);
        $v5 = $t["input"][4];
        $sth->bindParam(5, $v5);
        try {
            $sth->execute();
        } catch(PDOException $e) {
            if ($e->errorInfo[1] != $t["error_code"]) {
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
    $meta = $sth->getColumnMeta(2);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(3);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);
    $meta = $sth->getColumnMeta(4);
    echo sprintf("name: %s, native_type: %s, scale: %s, precision: %s, len: %s\n", $meta["name"], $meta["native_type"], $meta["scale"], $meta["precision"], $meta["len"]);

    echo "Results in String\n";
    while($row = $sth->fetch()) {
        $idx = $row[0];
        echo sprintf("C1: %s, C2: %s, C3: %s, C4: %s, C5: %s\n",
            $idx, $row[1], $row[2], $row[3], $row[4]);
        $expected = $tests[$idx-1]["output"];
        if ($expected[1] != $row[1]) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[1], $row[1]);
        }
        if ($expected[2] != $row[2]) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[2], $row[2]);
        }
        if ($expected[3] != $row[3]) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[3], $row[3]);
        }
        if ($expected[4] != $row[4]) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[4], $row[4]);
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
name: C2, native_type: FIXED, scale: 4, precision: 38, len: 0
name: C3, native_type: FIXED, scale: 0, precision: 38, len: 0
name: C4, native_type: FIXED, scale: 0, precision: 18, len: 0
name: C5, native_type: FIXED, scale: 2, precision: 18, len: 0
Results in String
C1: 1, C2: 1234567890123456789012345678901234.5678, C3: 98765432109876543210987654321098765432, C4: 123456789012345678, C5: 1234567890123456.78
C1: 2, C2: 123.4560, C3: 10, C4: 10, C5: 0.00
C1: 3, C2: , C3: , C4: , C5: 
C1: 4, C2: 123.4568, C3: -456, C4: -500, C5: -123.46
===DONE===
