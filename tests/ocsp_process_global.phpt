--TEST--
pdo_snowflake - process-global OCSP latch is not overwritten by a later default DSN
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    function expect_xor($dsn, $user, $password, $label) {
        try {
            new PDO("{$dsn};crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
            echo "FAIL: {$label} should have failed\n";
        } catch (PDOException $e) {
            if (strpos($e->getMessage(), "Both host certificate revocation check methods") !== false) {
                echo "{$label}: xor\n";
            } else {
                echo "{$label}: unexpected " . $e->getMessage() . "\n";
            }
        }
    }

    function expect_ok($dsn, $user, $password, $label) {
        $dbh = new PDO($dsn, $user, $password);
        $dbh = null;
        echo "{$label}: ok\n";
    }

    // fail-open then default then CRL: opt-in must still be on
    expect_ok("{$dsn};ocspfailopen=true", $user, $password, "fail-open");
    expect_ok($dsn, $user, $password, "default after fail-open");
    expect_xor($dsn, $user, $password, "crl after fail-open+default");

    // disableocspchecks=true disables fail-open so CRL is allowed
    expect_ok("{$dsn};ocspfailopen=true;disableocspchecks=true", $user, $password, "disable fail-open");
    expect_ok("{$dsn};crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password, "crl after disable fail-open");

    // fail-closed then default then CRL: opt-in must still be on
    expect_ok("{$dsn};ocspfailopen=false", $user, $password, "fail-closed");
    expect_ok($dsn, $user, $password, "default after fail-closed");
    expect_xor($dsn, $user, $password, "crl after fail-closed+default");

    // disableocspchecks=true does not turn fail-closed off
    expect_ok("{$dsn};ocspfailopen=true;disableocspchecks=true", $user, $password, "disable after fail-closed");
    expect_xor($dsn, $user, $password, "crl after fail-closed+disable");

    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
fail-open: ok
default after fail-open: ok
crl after fail-open+default: xor
disable fail-open: ok
crl after disable fail-open: ok
fail-closed: ok
default after fail-closed: ok
crl after fail-closed+default: xor
disable after fail-closed: ok
crl after fail-closed+disable: xor
OK
===DONE===
