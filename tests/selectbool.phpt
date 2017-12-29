--TEST--
pdo_snowflake - insert and select BOOLEAN data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        $count = $dbh->exec(
            "create or replace table t (c1 int, c2 boolean)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t values(?,?)");

        $tests = [
            [
                "input" => [101, True],
                "output" => null
            ],
            [
                "input" => [102, False],
                "output" => null
            ],
            [
                "input" => [103, ""],
                "output" => null
            ],
            [
                "input" => [104, null],
                "output" => null
            ],
            [
                "input" => [105, "tests"],
                "output" => null
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
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
        echo "Results in String\n";

        $cnt = 0;
        while($row = $sth->fetch()) {
            echo sprintf("C1: %s, ", $row[0]);
            if ($row[1]) {
                echo "C2: TRUE\n";
            } else {
                echo "C2: FALSE\n";
            }
            if ($tests[$cnt]["output"] == null) {
                $output = $tests[$cnt]["input"];
            } else {
                $output = $tests[$cnt]["output"];
            }
            if ($output[0] != $row[0]) {
                echo sprintf("ERR: testcase #%d -- expected: %s, got: %s\n",
                    $cnt, $output[0], $row[0]);
            }
            ++$cnt;
        }
        $count = $dbh->exec("drop table if exists t");

    } catch(Exception $e) {
        print_r($e);
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
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
    [scale] => 0
    [native_type] => BOOLEAN
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 1
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 101, C2: TRUE
C1: 102, C2: FALSE
C1: 103, C2: TRUE
C1: 104, C2: FALSE
C1: 105, C2: TRUE
===DONE===
