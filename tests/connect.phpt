--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
===DONE===

