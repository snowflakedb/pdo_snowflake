--TEST--
pdo_snowflake - connect with both CRL and OCSP enabled should fail
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    try {
        $dbh = new PDO("{$dsn};disableocspchecks=false;crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
        echo "FAIL: Connection should have failed\n";
    } catch(PDOException $e) {
        if (strpos($e->getMessage(), "Both host certificate revocation check methods") !== false) {
            echo "Expected error caught\n";
            echo "Error code: " . $e->getCode() . "\n";
        } else {
            echo "Unexpected error: " . $e->getMessage() . "\n";
        }
    }
    
    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Expected error caught
Error code: 1
OK
===DONE===

