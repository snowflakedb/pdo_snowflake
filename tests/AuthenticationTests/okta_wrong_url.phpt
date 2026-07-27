--TEST--
pdo_snowflake - Okta authentication with wrong Okta URL should fail
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
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
require_once __DIR__ . '/auth_helper.php';

$config = [
    'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
    'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
];

$dsn = buildOktaDsn($config, ['authenticator' => 'https://invalid.okta.com']);

try {
    $pdo = new PDO($dsn, 'invalidUsername', 'fakepassword');
    echo "ERROR: Should have thrown exception\n";
} catch (PDOException $e) {
    $error = $e->getMessage();
    if (stripos($error, 'authenticator') !== false || 
        stripos($error, 'not accepted') !== false ||
        stripos($error, 'configuration') !== false ||
        stripos($error, 'authentication failed') !== false) {
        echo "Okta wrong URL: Expected error received\n";
    } else {
        echo "Okta wrong URL: Unexpected error - $error\n";
    }
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Okta wrong URL: Expected error received
===DONE===
