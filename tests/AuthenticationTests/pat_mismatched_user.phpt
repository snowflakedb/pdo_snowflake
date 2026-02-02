--TEST--
pdo_snowflake - PAT authentication with mismatched username should fail
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

// Test if PAT helper works
$testToken = getPatToken('PAT_TEST_SKIP');
deletePatToken('PAT_TEST_SKIP');
if (empty($testToken)) {
    die("skip PAT helper script not working");
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

$tokenName = generatePatTokenName();
$token = getPatToken($tokenName);

if (empty($token)) {
    echo "PAT mismatched user: FAILED - Could not get PAT token\n";
    exit(1);
}

$dsn = buildPatDsn($config, ['token' => $token]);

try {
    $pdo = new PDO($dsn, 'differentUsername', '');
    echo "ERROR: Should have thrown exception\n";
} catch (PDOException $e) {
    $error = $e->getMessage();
    if (stripos($error, 'invalid') !== false || stripos($error, 'token') !== false) {
        echo "PAT mismatched user: Expected error received\n";
    } else {
        echo "PAT mismatched user: Unexpected error - $error\n";
    }
} finally {
    deletePatToken($tokenName);
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
PAT mismatched user: Expected error received
===DONE===
