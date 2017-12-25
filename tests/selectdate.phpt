--TEST--
pdo_snowflake - insert and select DATE data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        /* INSERT bool */
        $count = $dbh->exec("create or replace table t (c1 int, c2 date)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t values(?,?)");
        $v1 = 101;
        $sth->bindParam(1, $v1);
        $v2 = new DateTime("now");
        $v2str = $v2->format("Y-m-d");
        $sth->bindParam(2, $v2str); // must convert to str
        $sth->execute();

        /* SELECT date */
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
            echo "Results in String\n";
        while($row = $sth->fetch()) {
            echo sprintf("C1: %s, C2: (TODAY)\n", $row[0]);
            if ($row[1] != $v2str) {
                echo sprintf("Incorrect Value. expected: %s, got: %s",
                $v2str, $row[1]);
            }
        }
        // $count = $dbh->exec("drop table if exists t");

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
C1: 101, C2: (TODAY)
===DONE===
