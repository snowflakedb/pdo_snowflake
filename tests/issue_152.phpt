--TEST--
pdo_snowflake - issue 152
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";

    $dbh->query('USE '.$database);
    $dbh->query('USE WAREHOUSE '.$warehouse);

    // $query = $dbh->prepare("CREATE OR REPLACE TABLE obfuscated_table_name (daily_budget number(38,6))");
    // $query->execute();
    $dbh->exec("CREATE OR REPLACE TABLE obfuscated_table_name (daily number(38,6))");
    echo 'Created table with single column of type number(38,6)' . "\n";

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Created table with single column of type number(38,6)
===DONE===

