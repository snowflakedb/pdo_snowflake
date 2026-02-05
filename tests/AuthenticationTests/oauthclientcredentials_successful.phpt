--TEST--
pdo_snowflake - OAuth Client Credentials authentication (successful)
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_DATABASE',
    'SNOWFLAKE_AUTH_TEST_SCHEMA',
    'SNOWFLAKE_AUTH_TEST_WAREHOUSE',
    'SNOWFLAKE_AUTH_TEST_ROLE',
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID',
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_SECRET',
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_TOKEN',
];
foreach ($required as $var) {
    if (!getenv($var)) {
        die("skip Missing env var: $var");
    }
}
?>
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';

deleteTemporaryCredentialFile();

$config = [
    'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
    'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
    'database' => getenv('SNOWFLAKE_AUTH_TEST_DATABASE'),
    'schema' => getenv('SNOWFLAKE_AUTH_TEST_SCHEMA'),
    'warehouse' => getenv('SNOWFLAKE_AUTH_TEST_WAREHOUSE'),
    'role' => getenv('SNOWFLAKE_AUTH_TEST_ROLE'),
    'oauth_client_id' => getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID'),
    'oauth_client_secret' => getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_SECRET'),
    'oauth_token_endpoint' => getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_TOKEN'),
];

$dsn = buildOAuthClientCredentialsDsn($config);

echo "Connection process started\n";
$proc = startConnectionProcess($dsn, $config['oauth_client_id']);

echo "Waiting for result\n";
$output = waitForConnection($proc);

if (strpos($output, 'SUCCESS') !== false) {
    echo "SUCCESS\n";
} else {
    echo "FAILED: $output\n";
}
?>
===DONE===
<?php exit(0); ?>
--CLEAN--
<?php
require_once __DIR__ . '/auth_helper.php';
deleteTemporaryCredentialFile();
?>
--EXPECT--
Connection process started
Waiting for result
SUCCESS
===DONE===
