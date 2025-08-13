--TEST--
pdo_snowflake - connect
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    // OKTA authenticaiton
    $dbh = new PDO("snowflake:account=$account;disablesamlcheck=true;application=phptest;authenticator=https://dev-90125362.okta.com/", $oktauser, $oktapwd, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
    $sth = $dbh->query("select 1234");
    while($row=$sth->fetch(PDO::FETCH_NUM)){
        echo "RESULT: " . $row[0]. "\n";
    }
    $dbh = null;
    echo "OK\n";

    // OKTA authenticaiton and SAML URL 
    $dbh = new PDO("snowflake:account=$account;disablesamlcheck=false;application=phptest;authenticator=https://dev-90125362.okta.com/", $oktauser, $oktapwd, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
        $sth = $dbh->query("select 1234");
    while($row=$sth->fetch(PDO::FETCH_NUM)){
        echo "RESULT: " . $row[0]. "\n";
    }
    $dbh = null;
    echo "OK\n";

    // full parameters
    $dbh = new PDO("$dsn;application=phptest;authenticator=snowflake;priv_key_file=tests/p8test.pem;priv_key_file_pwd=test;disablequerycontext=true;includeretryreason=false;logintimeout=250;maxhttpretries=8;retrytimeout=350;ocspfailopen=false;disableocspchecks=true", $user, $password);
    // create table for testing autocommit later
    $tablename = "autocommittest" . rand();
    $count = $dbh->exec("create or replace table " . $tablename . "(c1 int)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $dbh = null;

    if (!array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        // connect with the minimum requirement
        // This test runs only on Travis or the connect parameters
        // are for production.
        $dbh = new PDO("snowflake:account=$account", $user, $password);
        $dbh = null;
    }
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

	// MFA passcode in password, connection fails due to no passcode provided in password.
    try {
        $dbh = new PDO("$dsn;passcodeinpassword=true", $user, $password);
        echo "Fail. Must fail to connect.\n";
    } catch(PDOException $e) {
        echo sprintf("Expected error code: %d for passcodeinpassword\n", $e->getCode());
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
RESULT: 1234
OK
RESULT: 1234
OK
OK
1
2
2
Expected error code: 390100 for passcodeinpassword
===DONE===

