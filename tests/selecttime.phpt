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
    print_r($meta);
    $meta = $sth->getColumnMeta(1);
    print_r($meta);

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
    [native_type] => TIME
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 0
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 1, C2: 12:34:56.987000000
C1: 2, C2: 08:19:34.000123000
C1: 3, C2: 
===DONE===
