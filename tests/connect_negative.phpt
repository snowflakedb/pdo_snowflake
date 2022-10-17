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
        // check the error message to be more specific for the expected error
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    // invalid key file
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;priv_key_file=dummy;priv_key_file_pwd=dummy", $user, "");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        // check the error message to be more specific for the expected error
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    // invalid key file password
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;priv_key_file=tests/p8test.pem;priv_key_file_pwd=dummy", $user, "");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        // check the error message to be more specific for the expected error
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }

    // invalid jwt token
    // We don't test valid jwt authentication in PHP as such case is covered in
    // libsnowflakeclient.
    // use invalid jwt token and check the error message to ensure keypair auth
    // is used and the invalid token is sent to server as expected
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=snowflake_jwt;priv_key_file=tests/p8test.pem;priv_key_file_pwd=test", $user, "");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error: %s\n", $e->getMessage());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Expected error code: 240005 for missing account
Expected error code: 240005 for missing user
Expected error code: 240005 for missing password
Expected error code: 240005 for invalid application name
Expected error: SQLSTATE[08001] [240005] unsupported authenticator
Expected error: SQLSTATE[HY000] [240000] authenticator initialization failed
Expected error: SQLSTATE[HY000] [240000] authenticator initialization failed
Expected error: SQLSTATE[08001] [390144] JWT token is invalid.
===DONE===

