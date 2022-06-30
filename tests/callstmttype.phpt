--TEST--
pdo_snowflake - procedure call statement type
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";
    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";

    $count = $dbh->exec(
	    "ALTER SESSION SET USE_STATEMENT_TYPE_CALL_FOR_STORED_PROC_CALLS = true");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $count = $dbh->exec(
        "create or replace procedure " .
        "TEST_SP_CALL_STMT_ENABLED(in1 float, in2 variant) " .
        "returns string language javascript as $$ " .
        "let res = snowflake.execute({sqlText: 'select ? c1, ? c2', binds:[IN1, JSON.stringify(IN2)]}); " .
        "res.next(); " .
        "return res.getColumnValueAsString(1) + ' ' + res.getColumnValueAsString(2) + ' ' + IN2; " .
        "$$;");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }
    $sth = $dbh->query("call TEST_SP_CALL_STMT_ENABLED(1, to_variant('[2,3]'))");
    while($row = $sth->fetch()) {
        echo $row[0] . "\n";
    }

    $count = $dbh->exec("drop table if exists t");

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
1 "[2,3]" [2,3]
===DONE===

