--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // full parameters - use JWT (keypair) auth via the global $dsn from common.php;
    // the extra ;application=... etc. just exercises connection-string parsing.
    $dbh = new PDO("$dsn;application=phptest;disablequerycontext=true;includeretryreason=false;logintimeout=250;maxhttpretries=8;retrytimeout=350;ocspfailopen=false;disableocspchecks=true", $user, $password);
    // create table for testing autocommit later
    $tablename = "autocommittest" . rand();
    $count = $dbh->exec("create or replace table " . $tablename . "(c1 int)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;

    echo "OK\n";

    // test auto commit in connect options
    // default to true
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (1)");
    $dbh = null;

    // set to true
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_AUTOCOMMIT => true]);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (2)");
    $dbh = null;

    // set to false
    $dbh = new PDO("$dsn;application=phptest", $user, $password, [PDO::ATTR_AUTOCOMMIT => false]);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // insert a row and the result will be tested later
    $dbh->exec("insert into " . $tablename . " values (3)");
    $dbh = null;

    $dbh = new PDO("$dsn;application=phptest", $user, $password);
    // check the result of previous test
    $sth = $dbh->query("select count(*) from " . $tablename);
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }
    // clean up
    $count = $dbh->exec("drop table if exists " . $tablename);
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;

    // MFA passcode using dummy passcode.
	// This feature is tested in libsfclient here we just confirm passcode can be used in
	// connection string and passed to libsfclient.
    $dbh = new PDO("$dsn;passcode=dummy", $user, $password);
    $dbh = null;

	// Client store temporary credential option.
	// This feature is tested in libsfclient here we just confirm it can be used in
	// connection string and passed to libsfclient.
    $dbh = new PDO("$dsn;client_store_temporary_credential=true", $user, $password);
    $dbh = null;

	// Client request MFA token option.
	// This feature is tested in libsfclient here we just confirm it can be used in
	// connection string and passed to libsfclient.
    $dbh = new PDO("$dsn;client_request_mfa_token=true", $user, $password);
    $dbh = null;

	// MFA passcode-in-password parameter passthrough.
	// This feature is tested for real in libsfclient; here we only confirm the
	// passcodeinpassword parameter can be parsed and passed through to the
	// driver. Under keypair (JWT) auth the password slot is not used at all,
	// so the connection succeeds - that's exactly the parameter-parsing
	// confirmation we want.
    $dbh = new PDO("$dsn;passcodeinpassword=true", $user, $password);
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
OK
1
2
2
===DONE===

