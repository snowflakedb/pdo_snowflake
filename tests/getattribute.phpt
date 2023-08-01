--TEST--
pdo_snowflake - getattribute
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    if (version_compare(PHP_VERSION, '8.0.0') >= 0) {
        # php 8.0 changed the default value of ATTR_ERRMODE so we set it
        # to get consistant expected output
        $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT );
    }
    echo "Connected to Snowflake\n";
    $attributes = array("ERRMODE", "CASE", "ORACLE_NULLS", "PERSISTENT", "AUTOCOMMIT");

    foreach ($attributes as $val) {
        echo "PDO::ATTR_$val: ";
        echo $dbh->getAttribute(constant("PDO::ATTR_$val")) . "\n";
    }

    $dbh->setAttribute( PDO::ATTR_AUTOCOMMIT, false );
        echo "PDO::ATTR_AUTOCOMMIT: ";
        echo $dbh->getAttribute(PDO::ATTR_AUTOCOMMIT) . "\n";

    // test driver information returned from phpinfo()
    ob_start();
    phpinfo(INFO_MODULES) ;
    $pinfo = ob_get_contents();
    ob_end_clean();
    $f1 = (strpos($pinfo, "PDO Driver for Snowflake => enabled") === false);
    $f2 = (strpos($pinfo, "pdo_snowflake.cacert") === false);
    $f3 = (strpos($pinfo, "pdo_snowflake.debug") === false);
    $f4 = (strpos($pinfo, "pdo_snowflake.logdir") === false);
    $f5 = (strpos($pinfo, "pdo_snowflake.loglevel") === false);
    echo (int)$f1 . " " . (int)$f2 . " " . (int)$f3 . " " . (int)$f4 . " " . (int)$f5 . "\n";

    $driver_ver = $dbh->getAttribute(PDO::ATTR_CLIENT_VERSION);
    if (strcmp($driver_ver, '1.2.7') >= 0) {
        // comment out driver version to avoid update test case for each release
        echo "Successfully get dirver version " . /*$driver_ver .*/ "\n";
    }
    else {
        echo "Failed get dirver version.\n";
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
PDO::ATTR_ERRMODE: 0
PDO::ATTR_CASE: 0
PDO::ATTR_ORACLE_NULLS: 0
PDO::ATTR_PERSISTENT: 
PDO::ATTR_AUTOCOMMIT: 1
PDO::ATTR_AUTOCOMMIT: 0
0 0 0 0 0
Successfully get dirver version 
===DONE===
