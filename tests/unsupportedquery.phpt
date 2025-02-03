--TEST--
pdo_snowflake - unsupported query
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $sth = $dbh->query("alter session set MULTI_STATEMENT_COUNT=0");
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
    echo 'Connected to Snowflake' . "\n";
    try {
        $sth = $dbh->query("select 1; select 2; select 3");
    } catch (PDOException $e) {
        echo 'Something failed: ' . $e->getMessage();
    }

    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    try {
        $sth = $dbh->query("select 1; select 2; select 3");
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

Warning: PDO::query(): SQLSTATE[0A000]: Feature not supported: 6 Multiple SQL statements in a single API call are not supported; use one API call per statement instead. in %s on line 9
Expected error: SQLSTATE[0A000]: Feature not supported: 6 Multiple SQL statements in a single API call are not supported; use one API call per statement instead.
===DONE===
