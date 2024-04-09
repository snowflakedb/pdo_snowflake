--TEST--
pdo_snowflake - twenty string columns
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dsn .=";timezone=America/New_York";
    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("create temporary table t(c1 string, c2 string, c3 string, c4 string, c5 string, c6 string, c7 string, c8 string, c9 string, c10 string, c11 string, c12 string, c13 string, c14 string, c15 string, c16 string, c17 string, c18 string, c19 string, c20 string)");
    // insert
    $count = $dbh->exec(
        "insert into t(c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15,c16,c17,c18,c19,c20) values('test1','test2','test3','test4','test5','test6','test7','test8','test9','test10','test11','test12','test13','test14','test15','test16','test17','test18','test19','test20')");
    if ($count == 0) {
        print_r($dbh->errorInfo());
    } else {
        echo "inserted rows: " . $count . "\n";
    }
    $sth = $dbh->query("select * from t order by 1");
    while($row = $sth->fetch()) {
        echo $row["C1"] . ",";
        echo $row["C2"] . ",";
        echo $row["C3"] . ",";
        echo $row["C4"] . ",";
        echo $row["C5"] . ",";
        echo $row["C6"] . ",";
        echo $row["C7"] . ",";
        echo $row["C8"] . ",";
        echo $row["C9"] . ",";
        echo $row["C10"] . ",";
        echo $row["C11"] . ",";
        echo $row["C12"] . ",";
        echo $row["C13"] . ",";
        echo $row["C14"] . ",";
        echo $row["C15"] . ",";
        echo $row["C16"] . ",";
        echo $row["C17"] . ",";
        echo $row["C18"] . ",";
        echo $row["C19"] . ",";
        echo $row["C20"] . "\n";
    }

    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
inserted rows: 1
test1,test2,test3,test4,test5,test6,test7,test8,test9,test10,test11,test12,test13,test14,test15,test16,test17,test18,test19,test20
===DONE===
