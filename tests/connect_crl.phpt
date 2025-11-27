--TEST--
pdo_snowflake - connect with CRL
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    echo "Test 1: Connect with CRL enabled and OCSP disabled\n";
    try {
        $dbh = new PDO("{$dsn};disableocspchecks=true;crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
        echo "Connection with CRL enabled succeeded\n";
    } catch(PDOException $e) {
        echo "Expected error: " . $e->getMessage() . "\n";
    }

    echo "Test 2: Connect with both CRL and OCSP enabled (should fail)\n";
    try {
        $dbh = new PDO("{$dsn};disableocspchecks=false;crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
        echo "Fail. Must fail to connect with both CRL and OCSP enabled.\n";
    } catch(PDOException $e) {
        echo "Expected error: " . $e->getMessage() . "\n";
    }

    echo "Test 3: Connect with CRL enabled and OCSP defaults (should fail)\n";
    try {
        $dbh = new PDO("{$dsn};crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
        echo "Fail. Must fail to connect with both CRL and OCSP enabled.\n";
    } catch(PDOException $e) {
        echo "Expected error: " . $e->getMessage() . "\n";
    }

    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Test 1: Connect with CRL enabled and OCSP disabled
Connection with CRL enabled succeeded
Test 2: Connect with both CRL and OCSP enabled (should fail)
Expected error: SQLSTATE[HY000] [1] Both host certificate revocation check methods (OCSP and CRL) are enabled. Please turn off crl_check or toggle OCSP with disableocspchecks.
Test 3: Connect with CRL enabled and OCSP defaults (should fail)
Expected error: SQLSTATE[HY000] [1] Both host certificate revocation check methods (OCSP and CRL) are enabled. Please turn off crl_check or toggle OCSP with disableocspchecks.
OK
===DONE===
