<?php
    $p = parse_ini_file(__DIR__ . "/../testenv.ini");

    $account = $p['SNOWFLAKE_TEST_ACCOUNT'];
    $user = $p['SNOWFLAKE_TEST_USER'];
    $database = $p['SNOWFLAKE_TEST_DATABASE'];
    $schema = $p['SNOWFLAKE_TEST_SCHEMA'];
    $warehouse = $p['SNOWFLAKE_TEST_WAREHOUSE'];
    $role = $p['SNOWFLAKE_TEST_ROLE'];

    // Auth selection:
    //   - If SNOWFLAKE_TEST_PRIVATE_KEY_FILE is set in testenv.ini (the GHA
    //     non-docker jobs that authenticate via keypair: SNOWFLAKE_TEST_PRIVATE_KEY
    //     + SNOWFLAKE_TEST_PASSWORD_KEYPAIR for the key passphrase), append
    //     JWT params to the DSN and leave $password empty.
    //   - Otherwise (rocky-linux docker job, password mode), $password is the
    //     real Snowflake login password from SNOWFLAKE_TEST_PASSWORD; DSN unchanged.
    $priv_key_file = array_key_exists('SNOWFLAKE_TEST_PRIVATE_KEY_FILE', $p)
        ? $p['SNOWFLAKE_TEST_PRIVATE_KEY_FILE']
        : '';
    $priv_key_file_pwd = array_key_exists('SNOWFLAKE_TEST_PRIVATE_KEY_PWD', $p)
        ? $p['SNOWFLAKE_TEST_PRIVATE_KEY_PWD']
        : '';
    if (!empty($priv_key_file)) {
        $password = '';
    } else {
        $password = array_key_exists('SNOWFLAKE_TEST_PASSWORD', $p)
            ? $p['SNOWFLAKE_TEST_PASSWORD']
            : '';
    }

    if (array_key_exists('SNOWFLAKE_TEST_HOST', $p)) {
        $host = $p['SNOWFLAKE_TEST_HOST'];
    } else {
        $host = $account . ".snowflakecomputing.com";
    }
    $port = "443";
    if (array_key_exists('SNOWFLAKE_TEST_PORT', $p)) {
        $port = $p['SNOWFLAKE_TEST_PORT'];
    }
    $protocol="https";
    if (array_key_exists('SNOWFLAKE_TEST_PROTOCOL', $p)) {
        $protocol = $p['SNOWFLAKE_TEST_PROTOCOL'];
    }

    $dsn = "snowflake:host=$host;port=$port;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role;protocol=$protocol";
    if (!empty($priv_key_file)) {
        $dsn .= ";authenticator=snowflake_jwt;priv_key_file=$priv_key_file";
        if (!empty($priv_key_file_pwd)) {
            $dsn .= ";priv_key_file_pwd=$priv_key_file_pwd";
        }
    }
?>
