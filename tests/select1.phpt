--TEST--
pdo_snowflake - select 1
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("select 1");
    while($row = $sth->fetch()) {
        echo "Result " . $row["1"] . "\n";
        echo "Count " . count($row) . "\n";
    }
    $sth = $dbh->query("select 2");
    while($row = $sth->fetch(PDO::FETCH_ASSOC)) {
        echo "Result " . $row["2"] . "\n";
        echo "Count " . count($row) . "\n";
    }
    $sth = $dbh->query("select 3");
    while($row = $sth->fetch(PDO::FETCH_NUM)) {
        echo "Result " . $row[0] . "\n";
        if (isset($row["3"])) {
            echo "FAIL. row[\"3\"] should not be set.\n";
        } else {
            echo "OK. row[\"3\"] is not set.\n";
        }
        echo "Count " . count($row) . "\n";
    }
    $sth = $dbh->query("select 4");
    while($row = $sth->fetch(PDO::FETCH_BOTH)) {
        # interesting behavior of PHP fetch in 7.x
        # When a numeric value N is taken as a column label,
        # No value is stored in the index 0 but the N+1 has.
        # Fixed in 8.0 so comment out weird checks
        # if (isset($row[0])) {
        #     echo "FAIL. row[0] should not be set.\n";
        # } else {
        #     echo "OK. row[0] is not set.\n";
        # }
        if (version_compare(PHP_VERSION, '8.0.0') >= 0) {
            echo "Result " . $row[0] . "\n";
            echo "Result " . $row["4"] . "\n";
        } else {
            echo "Result " . $row["5"] . "\n";
            echo "Result " . $row["5"] . "\n";
        }
        echo "Count " . count($row) . "\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Result 1
Count 2
Result 2
Count 1
Result 3
OK. row["3"] is not set.
Count 1
Result 4
Result 4
Count 2
===DONE===

