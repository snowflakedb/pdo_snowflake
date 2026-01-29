--TEST--
pdo_snowflake - Okta authentication successful
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_OKTA_AUTH',
    'SNOWFLAKE_AUTH_TEST_OKTA_USER',
    'SNOWFLAKE_AUTH_TEST_OKTA_PASS',
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
$oktaUser = getenv('SNOWFLAKE_AUTH_TEST_OKTA_USER');
$oktaPassword = getenv('SNOWFLAKE_AUTH_TEST_OKTA_PASS');

$dsn = "snowflake:host=$host;account=$account;authenticator=$oktaUrl";

try {
    $pdo = new PDO($dsn, $oktaUser, $oktaPassword);
    $stmt = $pdo->query("SELECT 1");
    $result = $stmt->fetch(PDO::FETCH_NUM);
    if ($result[0] == "1") {
        echo "Okta authentication: SUCCESS\n";
    } else {
        echo "Okta authentication: ERROR - unexpected result\n";
    }
} catch (PDOException $e) {
    echo "Okta authentication: FAILED - " . $e->getMessage() . "\n";
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
Okta authentication: SUCCESS
===DONE===
