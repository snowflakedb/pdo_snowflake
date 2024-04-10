--TEST--
pdo_snowflake - timezone
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dsn .=";timezone=America/New_York";
    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("show parameters like 'TIMEZONE'");
    while($row = $sth->fetch()) {
        echo "Result: " . $row["1"] . "\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Result: America/New_York
===DONE===

