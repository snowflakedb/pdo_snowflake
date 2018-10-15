--TEST--
pdo_snowflake - binding
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected to Snowflake\n";
    $count = $dbh->exec("create or replace table t (c1 int, c2 string)");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    }

    $sth = $dbh->prepare("insert into t(c1,c2) values(:number,:string)");

    // Binding a named parameter should throw an error
    try {
        $i = 11;
        $sth->bindParam(':number', $i);
    } catch (Exception $e) {
        echo 'Caught exception: ', $e->getMessage(), "\n";
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Caught exception: SQLSTATE[HYC00]: Optional feature not implemented: Named parameters are not supported yet in the Snowflake PDO Driver
===DONE===
