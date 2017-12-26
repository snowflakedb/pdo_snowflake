--TEST--
pdo_snowflake - timezone
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dsn .=";timezone=America/New_York";
        $dbh = new PDO($dsn, $user, $password);
        echo 'Connected to Snowflake' . "\n";
        $sth = $dbh->query("show parameters like 'TIMEZONE'");
        while($row = $sth->fetch()) {
            echo "Result: " . $row["1"] . "\n";
        }
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage() . "\n";
        echo "dsn is: $dsn\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Result: America/New_York
===DONE===

