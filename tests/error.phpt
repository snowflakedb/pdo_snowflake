--TEST--
pdo_snowflake - error handlings
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO($dsn, $user, $password);
        echo 'Connected to Snowflake' . "\n";
        $sth = $dbh->query("select 1 frooom dual");
        $earr = $dbh->errorInfo();
        echo "sqlstate: " . $earr[0] . "\n";
        echo "Snowflake Error: " . $earr[1] . "\n";
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        echo "dsn is: $dsn";
    }

    $dbh = null; // make sure closing db.

   try {
        $dbh = new PDO($dsn, "HIHIHI", "HAHAHAH");
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
sqlstate: 42000
Snowflake Error: 1003
Connection failed: SQLSTATE[08001] [390100] Incorrect username or password was specified.
===DONE===

