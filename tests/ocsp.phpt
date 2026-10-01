--TEST--
pdo_snowflake - ocsp check
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // default: OCSP off
    $dbh = new PDO($dsn, $user, $password);
    $dbh = null;
    echo "Connecting without OCSP worked\n";

    // disableocspchecks=false is the old default and is not an opt-in
    $dbh = new PDO("{$dsn};disableocspchecks=false;crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
    $dbh = null;

    // explicit fail-open / fail-closed opt-in
    $dbh = new PDO("{$dsn};ocspfailopen=true", $user, $password);
    $dbh = null;
    echo "Connecting with OCSP fail-open worked\n";

    $dbh = new PDO("{$dsn};ocspfailopen=false", $user, $password);
    $dbh = null;
    echo "Connecting with OCSP fail-closed worked\n";

    // disableocspchecks=true beats ocspfailopen=true
    $dbh = new PDO("{$dsn};ocspfailopen=true;disableocspchecks=true", $user, $password);
    $dbh = null;
    echo "Connecting with disableocspchecks over fail-open worked\n";

    // fail-closed stays on even with disableocspchecks=true
    $dbh = new PDO("{$dsn};ocspfailopen=false;disableocspchecks=true", $user, $password);
    $dbh = null;
    echo "Connecting with fail-closed over disableocspchecks worked\n";

    // insecure_mode still skips OCSP
    $dbh = new PDO("{$dsn};insecure_mode=true", $user, $password);
    $dbh = null;
    echo "Connecting with insecure_mode worked\n";

    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connecting without OCSP worked
Connecting with OCSP fail-open worked
Connecting with OCSP fail-closed worked
Connecting with disableocspchecks over fail-open worked
Connecting with fail-closed over disableocspchecks worked
Connecting with insecure_mode worked
OK
===DONE===
