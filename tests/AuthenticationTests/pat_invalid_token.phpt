--TEST--
pdo_snowflake - PAT authentication with invalid token should fail
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_OKTA_USER',
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
$user = getenv('SNOWFLAKE_AUTH_TEST_OKTA_USER');

$dsn = buildPatDsn($config, ['token' => 'invalidToken']);

try {
    $pdo = new PDO($dsn, $user, '');
    echo "ERROR: Should have thrown exception\n";
} catch (PDOException $e) {
    $error = $e->getMessage();
    if (stripos($error, 'invalid') !== false || stripos($error, 'token') !== false) {
        echo "PAT invalid token: Expected error received\n";
    } else {
        echo "PAT invalid token: Unexpected error - $error\n";
    }
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
PAT invalid token: Expected error received
===DONE===
