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

    // invalid authenticator
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=invalid_authenticator;", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid authenticator\n", $e->getCode());
    }

    // invalid key file
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;priv_key_file=dummy;priv_key_file_pwd=dummy", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid key file\n", $e->getCode());
    }

    // invalid key file password
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;tests/p8test.pem;priv_key_file_pwd=dummy", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid key file password\n", $e->getCode());
    }

    // invalid jwt token
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;tests/p8test.pem;priv_key_file_pwd=test", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid jwt token\n", $e->getCode());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Expected error code: 240005 for missing account
Expected error code: 240005 for missing user
Expected error code: 240005 for missing password
Expected error code: 240005 for invalid application name
Expected error code: 240005 for invalid authenticator
Expected error code: 240000 for invalid key file
Expected error code: 240005 for invalid key file password
Expected error code: 240005 for invalid jwt token
===DONE===

