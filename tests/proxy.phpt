--TEST--
pdo_snowflake - proxy
--INI--
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";
    if (getenv("all_proxy") || getenv("https_proxy") || getenv("http_proxy"))
    {
        // skip the test if the test evironment uses proxy already
        echo "OK\n";
    }
    // set invalid proxy in environment variables
	putenv("https_proxy=a.b.c");
    putenv("http_proxy=a.b.c");
    putenv("no_proxy");

    // use connection parameter to overwrite proxy setting in environment variables
    $dbh = new PDO("$dsn;proxy=;no_proxy=", $user, $password);
    $dbh = null;

    // test no proxy also works as well
    $dbh = new PDO("$dsn;proxy=a.b.c;no_proxy=*", $user, $password);
    $dbh = null;

    // unset environment variables
	putenv("https_proxy");
    putenv("http_proxy");
    putenv("no_proxy");

    echo "OK\n";

?>
===DONE===
<?php exit(0); ?>
--EXPECT--
OK
===DONE===

