--TEST--
pdo_snowflake - Okta authentication with wrong credentials should fail
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_OKTA_AUTH',
];
foreach ($required as $var) {
    if (!getenv($var)) {
        die("skip Missing env var: $var");
    }
}
?>
--INI--
extension=modules/pdo_snowflake.so
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
$host = getenv('SNOWFLAKE_AUTH_TEST_HOST');
$account = getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT');
$oktaUrl = getenv('SNOWFLAKE_AUTH_TEST_OKTA_AUTH');

$dsn = "snowflake:host=$host;account=$account;authenticator=$oktaUrl";

try {
    $pdo = new PDO($dsn, 'invalidUsername', 'fakepassword');
    echo "ERROR: Should have thrown exception\n";
} catch (PDOException $e) {
    $error = $e->getMessage();
    if (stripos($error, '401') !== false || stripos($error, 'okta') !== false) {
        echo "Okta wrong credentials: Expected error received\n";
    } else {
        echo "Okta wrong credentials: Unexpected error - $error\n";
    }
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Okta wrong credentials: Expected error received
===DONE===
