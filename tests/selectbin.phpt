--TEST--
pdo_snowflake - insert and select BINARY data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create temporary table t (c1 int, c2 binary)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, pack("nvc*", 0x1234, 0x5678, 65, 66)],
            "output" => [1, "1234,5678,65,66"]
        ],
        [
            "input" => [2, pack("nvc*", 0xabcd, 0xdef0, 67, 68)],
            "output" => [2, "abcd,def0,67,68"]
        ],
        [
            "input" => [3, null],
            "output" => [3, null]
        ],
        [
            "input" => [4, pack("nvc*", 0xabcd, 0x0, 67, 68)],
            "output" => [4, "abcd,0,67,68"]
        ],
        [
            "input" => [5, pack("nvc*", 0xabcd, 0x0, 67, 0)],
            "output" => [5, "abcd,0,67,0"]
        ],
    ];

    foreach ($tests as $t) {
        $v1 = $t["input"][0];
        $sth->bindParam(1, $v1);
        $v2 = $t["input"][1];
        $sth->bindParam(2, $v2, PDO::PARAM_LOB);
        $sth->execute();
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
        echo sprintf("C1: %s, C2: ", $idx);
        if ($row[1] != null) {
            $out = unpack("nval1/vval2/cval3/cval4", hex2bin($row[1]));
            $row1 = sprintf(
                "%x,%x,%d,%d",
                $out["val1"],
                $out["val2"],
                $out["val3"],
                $out["val4"]);
            echo $row1 . "\n";
        } else {
            $row1 = null;
            echo "NULL\n";
        }
        $expected = $tests[$idx-1]["output"];
        if ($expected[1] != $row1) {
            echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                $idx, $expected[1], $row1);
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
name: C2, native_type: BINARY, scale: 0, precision: 0, len: 8388608
Results in String
C1: 1, C2: 1234,5678,65,66
C1: 2, C2: abcd,def0,67,68
C1: 3, C2: NULL
C1: 4, C2: abcd,0,67,68
C1: 5, C2: abcd,0,67,0
===DONE===
