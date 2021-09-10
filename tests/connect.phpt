--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // full parameters
    $dbh = new PDO("$dsn;application=phptest", $user, $password);
    $dbh = null;

    if (!array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        // connect with the minimum requirement
        // This test runs only on Travis or the connect parameters
        // are for production.
        $dbh = new PDO("snowflake:account=$account", $user, $password);
        $dbh = null;
    }
    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
OK
===DONE===

