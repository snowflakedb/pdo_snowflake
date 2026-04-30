<?php
    $p = parse_ini_file(__DIR__ . "/../testenv.ini");

    $account = $p['SNOWFLAKE_TEST_ACCOUNT'];
    $user = $p['SNOWFLAKE_TEST_USER'];
    $database = $p['SNOWFLAKE_TEST_DATABASE'];
    $schema = $p['SNOWFLAKE_TEST_SCHEMA'];
    $warehouse = $p['SNOWFLAKE_TEST_WAREHOUSE'];
    $role = $p['SNOWFLAKE_TEST_ROLE'];
    $priv_key_file = $p['SNOWFLAKE_TEST_PRIVATE_KEY_FILE'];

    // Keypair (JWT) auth only. $password is an empty placeholder so PDO's
    // 3-arg call signature still binds; the JWT params in $dsn drive auth.
    $password = '';

    if (array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        $host = $p['SNOWFLAKE_TEST_HOST'];
    } else {
        $host = $account . ".snowflakecomputing.com";
    }
    $port = "443";
    if (array_key_exists('SNOWFLAKE_TEST_PORT', $p)) {
        $port = $p['SNOWFLAKE_TEST_PORT'];
    }
    $protocol = "https";
    if (array_key_exists('SNOWFLAKE_TEST_PROTOCOL', $p)) {
        $protocol = $p['SNOWFLAKE_TEST_PROTOCOL'];
    }

    $dsn = "snowflake:host=$host;port=$port;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role;protocol=$protocol;authenticator=snowflake_jwt;priv_key_file=$priv_key_file";
?>
