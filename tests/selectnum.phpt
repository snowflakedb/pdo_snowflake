--TEST--
pdo_snowflake - insert and select numeric data types
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    echo "==> very large numeric values in strings\n";
    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        /* INSERT very large numeric values in string */
        $count = $dbh->exec("create or replace table t (c1 number(38,2), c2 number(38,0), c3 number(18,0), c4 number(18,2))");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t values(?,?,?,?)");
        $v1 = "123456789012345678901234567890123456.78";
        $sth->bindParam(1, $v1);
        $v2 = "98765432109876543210987654321098765432";
        $sth->bindParam(2, $v2);
        $v3 = "123456789012345678";
        $sth->bindParam(3, $v3);
        $v4 = "1234567890123456.78";
        $sth->bindParam(4, $v4);
        $sth->execute();

        /* SELECT very large numeric values in string */
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
        $meta = $sth->getColumnMeta(2);
        print_r($meta);
        $meta = $sth->getColumnMeta(3);
        print_r($meta);
        while($row = $sth->fetch()) {
            echo "Results in String\n";
            echo sprintf("C1: %s\n", $row[0]);
            echo sprintf("C2: %s\n", $row[1]);
            echo sprintf("C3: %s\n", $row[2]);
            echo sprintf("C4: %s\n", $row[3]);

            echo "Results in Float\n";
            echo sprintf("C1: %f\n", (float)$row[0]);
            echo sprintf("C2: %f\n", (float)$row[1]);
            echo sprintf("C3: %f\n", (float)$row[2]);
            echo sprintf("C4: %f\n", (float)$row[3]);
        }
        $count = $dbh->exec("drop table if exists t");

    } catch(Exception $e) {
        print_r($e);
        echo "dsn is: $dsn\n";
        echo "user is: $user\n";
    }
    $dbh = null;

    echo "==> common numeric values\n";
    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        /* INSERT very large numeric values in string */
        $count = $dbh->exec(
            "create or replace table t (c1 number(38,4), c2 number(38,0), c3 float)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t(c1,c2,c3) values(?,?,?)");
        $v1 = 123.456;
        $sth->bindParam(1, $v1);
        $v2 = 9.87654;
        $sth->bindParam(2, $v2);
        $v3 = 9.87654;
        $sth->bindParam(3, $v3);
        $sth->execute();

        /* SELECT very large numeric values in string */
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
        $meta = $sth->getColumnMeta(2);
        print_r($meta);
        while($row = $sth->fetch()) {
            echo "Results in String\n";
            echo sprintf("C1: %s\n", $row[0]);
            echo sprintf("C2: %s\n", $row[1]);
            echo sprintf("C3: %s\n", $row[2]);
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
==> very large numeric values in strings
Connected to Snowflake
Array
(
    [scale] => 2
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
    [native_type] => FIXED
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 0
    [precision] => 38
    [pdo_type] => 2
)
Array
(
    [scale] => 0
    [native_type] => FIXED
    [flags] => Array
        (
        )

    [name] => C3
    [len] => 0
    [precision] => 18
    [pdo_type] => 2
)
Array
(
    [scale] => 2
    [native_type] => FIXED
    [flags] => Array
        (
        )

    [name] => C4
    [len] => 0
    [precision] => 18
    [pdo_type] => 2
)
Results in String
C1: 123456789012345678901234567890123456.78
C2: 98765432109876543210987654321098765432
C3: 123456789012345678
C4: 1234567890123456.78
Results in Float
C1: 123456789012345680420001586706644992.000000
C2: 98765432109876534857184263264815546368.000000
C3: 123456789012345680.000000
C4: 1234567890123456.750000
==> common numeric values
Connected to Snowflake
Array
(
    [scale] => 4
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
    [native_type] => FIXED
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 0
    [precision] => 38
    [pdo_type] => 2
)
Array
(
    [scale] => 0
    [native_type] => REAL
    [flags] => Array
        (
        )

    [name] => C3
    [len] => 0
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 123.4560
C2: 10
C3: 9.87654
===DONE===

