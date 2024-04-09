--TEST--
pdo_snowflake - select many
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $counter = 0;
    $prev_col1 = -1;

    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("select seq8(), randstr(1000, random()) from table(generator(rowcount=>20000))");
    while($row = $sth->fetch()) {
        if ($row[0] % 10000 == 0) {
            echo "Result :" . $row["0"] . " " . $row["1"] . "\n";
        }
        $counter += $row[0];
        if ($prev_col1 + 1 != $row[0]) {
            echo sprintf("ERROR: the id is not sequential. expected: %d, got: %d\n", $prev_col1 + 1, $row[0]);
            break;
        }
        $prev_col1++;
    }
    echo sprintf("Sum: %d\n", $counter);
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Connected to Snowflake
Result :0 %s
Result :10000 %s
Sum: 199990000
===DONE===
