--TEST--
pdo_snowflake - ocsp check
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // full parameters with ocsp
    $dbh = new PDO($dsn, $user, $password);
    $dbh = null;
    echo "Connecting with OCSP worked\n";

    // disable ocsp check and connect
    $dsn = "{$dsn};insecure_mode=true";
    $dbh = new PDO($dsn, $user, $password);
    $dbh = null;
    echo "Connecting without OCSP worked\n";

    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connecting with OCSP worked
Connecting without OCSP worked
OK
===DONE===

