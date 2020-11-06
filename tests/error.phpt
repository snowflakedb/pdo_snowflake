--TEST--
pdo_snowflake - error handlings
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("select 1 frooom dual");
    $earr = $dbh->errorInfo();
    $errcode = $dbh->errorCode();
    echo "sqlstate: " . $earr[0] . "\n";
    echo "errorCode: " . $errcode . "\n";
    echo "Snowflake Error: " . $earr[1] . "\n";

    $dbh = null; // Ensure closing db.

    try {
        // show warning
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING );
        echo 'Connected to Snowflake' . "\n";
        $sth = $dbh->query("select 1 frooom dual");
    } catch (PDOException $e) {
        echo 'Something failed: ' . $e->getMessage();
    }

    $dbh = null; // Ensure closing db.

    try {
        // exception should be raised on error
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
        echo 'Connected to Snowflake' . "\n";
        $sth = $dbh->query("select 1 frooom dual");
        echo "FAIL. The above query should fail.\n";
    } catch (PDOException $e) {
        echo "sqlstate: " . $e->errorInfo[0]. "\n";
        echo "Snowflake Error: " . $e->errorInfo[1] . "\n";
    }

    $dbh = null; // Ensure closing db.

    try {
        $dbh = new PDO($dsn, "HIHIHI", "HAHAHAH");
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
    }

    $dbh = null;

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
sqlstate: 42000
errorCode: 42000
Snowflake Error: 1003
Connected to Snowflake

Warning: PDO::query(): SQLSTATE[42000]: Syntax error or access violation: 1003 SQL compilation error:
syntax error line 1 at position 16 unexpected 'dual'. in %s on line 20
Connected to Snowflake
sqlstate: 42000
Snowflake Error: 1003
Connection failed: SQLSTATE[08001] [390100] Incorrect username or password was specified.
===DONE===

