--TEST--
pdo_snowflake - insert and select TIMESTAMP_LTZ data type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $tz="America/New_York";

        // NOTE: you could set the default timezone but will impact other
        // thread.
        // date_default_timezone_set($tz);
        // Instead, set timezone using setTimeZone method for DateTime object.

        $dsn .=sprintf(";timezone=%s", $tz);
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo "Connected to Snowflake\n";

        $count = $dbh->exec("create or replace table t (c1 int, c2 timestamp_ltz)");
        if ($count == 0) {
            print_r($dbh->errorInfo());
        }
        $sth = $dbh->prepare("insert into t values(?,?)");
        $v1 = 101;
        $sth->bindParam(1, $v1);
        $v2 = new DateTime("now");
        $v2->setTimeZone(new DateTimeZone($tz));
        $v2str = $v2->format("Y-m-d H:i:s.u000 P");
        $sth->bindParam(2, $v2str); // must convert to str
        $sth->execute();

        $sth->bindValue(1, 102);
        $v2str2 = "1969-11-21 08:19:34.123000000 -05:00";
        $sth->bindValue(2, $v2str2);
        $sth->execute();

        /* SELECT date */
        $sth = $dbh->query("select * from t order by 1");
        $meta = $sth->getColumnMeta(0);
        print_r($meta);
        $meta = $sth->getColumnMeta(1);
        print_r($meta);
            echo "Results in String\n";
        $cnt = 0;
        while($row = $sth->fetch()) {
            echo sprintf("C1: %s, C2: ", $row[0]);
            switch($cnt) {
            case 0:
                if (substr($v2str, 0, strlen($row[1])) != $row[1]) {
                    echo sprintf("Incorrect Value. expected: %s, got: %s\n",
                    $v2str, $row[1]);
                } else {
                    echo "(TODAY)\n";
                }
                break;
            case 1:
                if (substr($v2str2, 0, strlen($row[1])) !=  $row[1]) {
                    echo sprintf("Incorrect Value. expected: %s, got: %s\n",
                    $v2str2, $row[1]);
                } else {
                    echo "(OLD DATE)\n";
                }
                break;
            default:
                break;
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
    [scale] => 9
    [native_type] => TIMESTAMP_LTZ
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
C1: 102, C2: (OLD DATE)
===DONE===
