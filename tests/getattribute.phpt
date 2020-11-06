--TEST--
pdo_snowflake - getattribute
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    echo "Connected to Snowflake\n";
    $attributes = array("ERRMODE", "CASE", "ORACLE_NULLS", "PERSISTENT");

    foreach ($attributes as $val) {
        echo "PDO::ATTR_$val: ";
        echo $dbh->getAttribute(constant("PDO::ATTR_$val")) . "\n";
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
===DONE===
