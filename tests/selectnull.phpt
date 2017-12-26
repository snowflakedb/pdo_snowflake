--TEST--
pdo_snowflake - insert and select NULL data
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
        $count = $dbh->exec("create or replace table t (c1 int, c2 string, c3 number(18,0), c4 boolean)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t(c1,c2,c3,c4) values(?,?,?,?)");
        $sth->bindValue(1, 101); // bindValue
        $v2 = null;
        $sth->bindParam(2, $v2); // bindParam null
        $sth->bindValue(3, null, PDO::PARAM_NULL); // null with PARAM_NULL
        $v4 = null;
        $sth->bindParam(4, $v4, PDO::PARAM_BOOL); // cannot use PARAM_STR
        $sth->execute();

        /* SELECT null */
        echo "==> fetching null \n";
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
        echo "Results in String\n";
        while($row = $sth->fetch()) {
            echo sprintf(
                "C1: %s, C2:%s, C3:%s, C4:%s\n",
                 $row[0],
                 $row[1] == null ? "(NULL)" : "EMPTY",
                 $row[2] == null ? "(NULL)" : "EMPTY",
                 $row[3] == null ? "(NULL)" : "EMPTY");
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
==> fetching null 
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
    [native_type] => TEXT
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 16777216
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 101, C2:(NULL), C3:(NULL), C4:(NULL)
===DONE===
