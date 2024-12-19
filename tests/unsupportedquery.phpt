--TEST--
pdo_snowflake - unsupported query
--INI--
pdo_snowflake.logdir=sflog1
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
    echo 'Connected to Snowflake' . "\n";
    try {
        $sth = $dbh->query("put file://./1 @~");
    } catch (PDOException $e) {
        echo 'Something failed: ' . $e->getMessage();
    }

    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    $sth = $dbh->query("create or replace temporary table t (c1 varchar, c2 varchar)");
    $sth = $dbh->query("insert into t values ('str1', 'str2'), ('str3', 'str4')");
    $sth = $dbh->query("copy into @%t from t");

    try {
        $sth = $dbh->query("get @%t file://.");
        echo "Fail. Must fail to execute.\n";
    } catch (PDOException $e) {
        echo 'Expected error: ' . $e->getMessage() . "\n";
    }
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake

Warning: PDO::query(): SQLSTATE[HY000]: General error: 240000 Unsupported query type. in %s on line 8
Expected error: SQLSTATE[HY000]: General error: 240000 Unsupported query type.
===DONE===
