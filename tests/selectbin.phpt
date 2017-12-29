--TEST--
pdo_snowflake - insert and select BINARY data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        $count = $dbh->exec("create or replace table t (c1 int, c2 binary)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t values(?,?)");
        $v1 = 101;
        $sth->bindParam(1, $v1);
        $v2 = pack("nvc*", 0x1234, 0x5678, 65, 66);
        $sth->bindParam(2, $v2, PDO::PARAM_LOB);
        $sth->execute();

        $v1 = 102;
        $sth->bindParam(1, $v1);
        $v2 = pack("nvc*", 0xabcd, 0xdef0, 67, 68);
        $sth->bindParam(2, $v2, PDO::PARAM_LOB);
        $sth->execute();

        /* SELECT binary */
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
            echo "Results in String\n";
        while($row = $sth->fetch()) {
            echo sprintf("C1: %s, ", $row[0]);
            $out = unpack("nval1/vval2/cval3/cval4", hex2bin($row[1]));
            echo sprintf(
                "C2: %x,%x,%d,%d\n",
                $out["val1"],
                $out["val2"],
                $out["val3"],
                $out["val4"]);
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
    [native_type] => BINARY
    [flags] => Array
        (
        )

    [name] => C2
    [len] => 8388608
    [precision] => 0
    [pdo_type] => 2
)
Results in String
C1: 101, C2: 1234,5678,65,66
C1: 102, C2: abcd,def0,67,68
===DONE===
