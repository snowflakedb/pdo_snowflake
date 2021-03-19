--TEST--
pdo_snowflake - insert and select DATE data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
        "create temporary table t (c1 int, c2 date)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->prepare("insert into t values(?,?)");

    $tests = [
        [
            "input" => [1, "1989-12-30"],
            "output" => [1, "1989-12-30"],
        ],
        [
            "input" => [2, "1701-05-21"],
            "output" => [2, "1701-05-21"],
        ],
        [
            "input" => [3, "0001-01-01"],
            "output" => [3, "1-01-01"],
        ],
        [
            "input" => [4, "0000-01-01"],
            "output" => [4, "0-01-01"],
        ],
        [
            "input" => [5, null],
            "output" => [5, null],
        ],
        [
            "input" => [6, "", 100040], // not recognized date error
            "output" => [6, ""],
        ],
        [
            "input" => [7, "GGGGGG", 100040], // not recognized date error
            "output" => [7, ""],
        ],
        [
            "input" => [8, "9999-12-31"],
            "output" => [8, "9999-12-31"],
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
    }
    $count = $dbh->exec("drop table if exists t");

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
Expected ERR: testcase #6 -- 100040
Expected ERR: testcase #7 -- 100040
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
    [scale] => 0
    [native_type] => DATE
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 0
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 1, C2: 1989-12-30
C1: 2, C2: 1701-05-21
C1: 3, C2: %r0{0,3}%r1-01-01
C1: 4, C2: %r0{0,3}%r0-01-01
C1: 5, C2: 
C1: 8, C2: 9999-12-31
===DONE===
