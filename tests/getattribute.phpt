--TEST--
pdo_snowflake - getattribute
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
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
        echo "Successfully get driver version " . /*$driver_ver .*/ "\n";
    }
    else {
        echo "Failed get driver version.\n";
    }

    // query id with prepared query
    $qid = $dbh->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    $sth = $dbh->prepare("select 1");
    $sqid = $sth->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    if (!empty($qid) || !empty($sqid)) {
        echo "query id is not empty before any query executed: " . $qid . ", " . $sqid . "\n";
    }
    else {
        echo "query id is empty before any query executed.\n";
    }
    $sth->execute();
    $qid = $dbh->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    $sqid = $sth->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    if ($qid != $sqid)
    {
        echo "query id from connecton is different from statement: " . $qid . ", " . $sqid . "\n";
    }
    else if (empty($qid)) {
        echo "query id is empty after query executed.\n";
    }
    else {
        echo "query id is valid after query executed.\n";
    }

    // query id with directly executed query
    $sth = $dbh->query("create temporary table tb1(c1 varchar)");
    $qid1 = $dbh->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    $sqid1 = $sth->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);
    if ($qid == $qid1)
    {
        echo "query id is not updated after another query executed.\n";
    }
    if ($qid1 != $sqid1)
    {
        echo "query id from connecton is different from statement: " . $qid . ", " . $sqid . "\n";
    }
    else if (empty($qid1)) {
        echo "query id is empty after query executed.\n";
    }
    else {
        echo "query id is valid after query executed.\n";
    }

    // query id with failed query
    $sth = $dbh->query("select * from table_not_exists");
    if ($sth !== false)
    {
        echo "unexpected query success.\n";
    }

    $qid2 = $dbh->getAttribute(PDO::SNOWFLAKE_ATTR_QUERY_ID);

    if ($qid2 == $qid1)
    {
        echo "query id is not updated with failed query.\n";
    }
    if (empty($qid2)) {
        echo "query id is empty with failed query.\n";
    }
    else {
        echo "query id is valid with failed query.\n";
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
Successfully get driver version 
query id is empty before any query executed.
query id is valid after query executed.
query id is valid after query executed.
query id is valid with failed query.
===DONE===
