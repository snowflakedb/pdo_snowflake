--TEST--
pdo_snowflake - connect (negative)
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // The negative tests below intentionally feed malformed DSNs to
    // exercise the driver's parameter-validation paths. Some of those
    // paths require a non-empty password so that the *intended* check
    // (missing account / invalid app name / invalid authenticator)
    // is what fires first - otherwise the now-empty $password from
    // common.php would short-circuit to "240005 missing password"
    // before the actual scenario is reached. The "no password" test
    // below explicitly passes "" to exercise that branch.

    // no account
    try {
        $dbh = new PDO("snowflake:", $user, "dummy");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for missing account\n", $e->getCode());
    }

    // no user
    try {
        $dbh = new PDO("snowflake:account=$account", "", "dummy");
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
        $dbh = new PDO("snowflake:account=$account;application=1234invalidapp", $user, "dummy");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for invalid application name\n", $e->getCode());
    }

    // invalid authenticator
    try {
        $dbh = new PDO("snowflake:account=$account;authenticator=invalid_authenticator;", $user, "dummy");
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
        // Ignore the error detail that server changed serveral times
        echo sprintf("Expected error: %s\n", substr($e->getMessage(), 0, 15));
    }

    // login timeout
    // use an invalid(unreachable) proxy to trigger login timeout
    $starttime = time();
    try {
        $dbh = new PDO("$dsn;proxy=172.123.111.222;logintimeout=4", $user, "dummy");
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        $spendtime = time() - $starttime;
        if (($spendtime > 6) || ($spendtime < 2))
        {
            echo "Fail. connection failed after $spendtime seconds\n";
        }
        else
        {
            echo "Test for logintimeout passed\n";
        }
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Expected error code: 240005 for missing account
Expected error code: 240005 for missing user
Expected error code: 240005 for missing password
Expected error code: 240005 for invalid application name
Expected error: SQLSTATE[HY000] [240000] authentication failed
Expected error: SQLSTATE[HY000] [240000] authenticator initialization failed
Expected error: SQLSTATE[HY000] [240000] authenticator initialization failed
Expected error: SQLSTATE[08001]
Test for logintimeout passed
===DONE===
