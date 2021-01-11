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

    echo "==> Bind parameters more than 8\n";
    $q = 'SELECT :id0, :id1, :id2, :id3, :id4, :id5, :id6, :id7 , :id8';
    $sth = $dbh->prepare($q);
    $sth->bindValue(":id0", 'a');
    $sth->bindValue(":id1", 'b');
    $sth->bindValue(":id2", 'c');
    $sth->bindValue(":id3", 'd');
    $sth->bindValue(":id4", 'e');
    $sth->bindValue(":id5", 'f');
    $sth->bindValue(":id6", 'g');
    $sth->bindValue(":id7", 'h');
    $sth->bindValue(":id8", 'i');

    $sth->execute();
    while($row = $sth->fetch()) {
        echo $row[0] . " " . $row[1] . " " . $row[2] . " " . $row[3] . " " . $row[4] . " " . $row[5] . " " . $row[6] . " " . $row[7] . " " . $row[8] . "\n";
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
==> Bind parameters more than 8
a b c d e f g h i
===DONE===
