--TEST--
pdo_snowflake - htap test
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
pdo_snowflake.logdir=logs
pdo_snowflake.loglevel=TRACE
--FILE--
<?php
    include __DIR__ . "/common.php";

    // test with default behavior
    $dbh = new PDO($dsn, $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("create or replace database db1");
    $sth = $dbh->query("create or replace hybrid table t1 (a int primary key, b int)");
    $sth = $dbh->query("insert into t1 values (1, 2), (2, 3), (3, 4)");
    $sth = $dbh->query("create or replace database db2");
    $sth = $dbh->query("create or replace hybrid table t2 (a int primary key, b int)");
    $sth = $dbh->query("insert into t2 values (1, 3), (2, 2), (3, 4)");
    $sth = $dbh->query("create or replace database db3");
    $sth = $dbh->query("create or replace hybrid table t3 (a int primary key, b int)");
    $sth = $dbh->query("insert into t3 values (1, 3), (2, 2), (3, 4)");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y, db3.public.t3 z where x.a = y.a and y.a = z.a;");
    $sth = $dbh->query("insert into db2.public.t2 (select y.a*100*z.a, y.b*15*z.b from db1.public.t1 y, db3.public.t3 z where y.a=z.a);");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y, db3.public.t3 z where x.a = y.a and y.a = z.a;");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y where x.a = y.a;");
    $sth = $dbh->query("select * from db2.public.t2 y, db3.public.t3 z where y.a = z.a;");
    $dbh = null;

    // test with non-default behavior
    $dbh = new PDO("$dsn;disablequerycontextcache=true;includeretryreason=false", $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo 'Connected to Snowflake' . "\n";
    $sth = $dbh->query("create or replace database db1");
    $sth = $dbh->query("create or replace hybrid table t1 (a int primary key, b int)");
    $sth = $dbh->query("insert into t1 values (1, 2), (2, 3), (3, 4)");
    $sth = $dbh->query("create or replace database db2");
    $sth = $dbh->query("create or replace hybrid table t2 (a int primary key, b int)");
    $sth = $dbh->query("insert into t2 values (1, 3), (2, 2), (3, 4)");
    $sth = $dbh->query("create or replace database db3");
    $sth = $dbh->query("create or replace hybrid table t3 (a int primary key, b int)");
    $sth = $dbh->query("insert into t3 values (1, 3), (2, 2), (3, 4)");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y, db3.public.t3 z where x.a = y.a and y.a = z.a;");
    $sth = $dbh->query("insert into db2.public.t2 (select y.a*100*z.a, y.b*15*z.b from db1.public.t1 y, db3.public.t3 z where y.a=z.a);");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y, db3.public.t3 z where x.a = y.a and y.a = z.a;");
    $sth = $dbh->query("select * from db1.public.t1 x, db2.public.t2 y where x.a = y.a;");
    $sth = $dbh->query("select * from db2.public.t2 y, db3.public.t3 z where y.a = z.a;");
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Connected to Snowflake
Connected to Snowflake
===DONE===

