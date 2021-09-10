--TEST--
pdo_snowflake - connect (negative)
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // no account
    try {
        $dbh = new PDO("snowflake:", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for missing account\n", $e->getCode());
    }

    // no user
    try {
        $dbh = new PDO("snowflake:account=$account", "", $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for missing user\n", $e->getCode());
    }

    // no password
    try {
        $dbh = new PDO("snowflake:account=$account", $user, "");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for missing password\n", $e->getCode());
    }

    // invalid applicaation name
    try {
        $dbh = new PDO("snowflake:account=$account;application=1234invalidapp", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid application name\n", $e->getCode());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Expected error code: 240005 for missing account
Expected error code: 240005 for missing user
Expected error code: 240005 for missing password
Expected error code: 240005 for invalid application name
===DONE===

