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

    $sth = $dbh->prepare("insert into t(c1, c2) values(:v1,:v2)");
    $v1 = 11;
    $v2 = 'Hello';
    $sth->bindParam(':v1', $v1);
    $sth->bindParam(':v2', $v2);
    $sth->execute();

    echo "==> fetch by default\n";
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . " " . $row["C2"] . "\n";
    }

    echo "==> Mixing parameters\n";
    $v1 = 12;
    $v2 = 'HelloAgain';
    try {
        $sth->bindParam(1, $v1);
        $sth->bindParam(2, $v2);
        $sth->bindParam(':v1', $v1);
        $sth->bindParam(':v2', $v2);
        $sth->execute();
    } catch(Exception $e) {
        echo sprintf("Caught Exception: ". $e->getMessage(). "\n");
    }
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
==> fetch by default
11 Hello
==> Mixing parameters
Caught Exception: SQLSTATE[HY105]: Invalid parameter type: Mixing Named and Positional parameter is not allowed in Snowflake PDO Driver
===DONE===
