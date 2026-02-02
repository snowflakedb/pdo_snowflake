--TEST--
pdo_snowflake - OAuth Okta authorization code authentication (successful)
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
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_REDIRECT_URI',
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_AUTH_URL',
    'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_TOKEN',
    'SNOWFLAKE_AUTH_TEST_OKTA_USER',
    'SNOWFLAKE_AUTH_TEST_OKTA_PASS',
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

$config = getOAuthOktaAuthCodeConfig();
$config['oauth_redirect_uri'] = str_replace('portToReplace', '8007', $config['oauth_redirect_uri']);
$creds = getOktaCredentials();

$dsn = buildOAuthOktaAuthCodeDsn($config);

// Start the connection process
$proc = startConnectionProcess($dsn, $creds['user']);

// Provide credentials using the external browser script
provideBrowserCredentials('externalOauthOktaSuccess', $creds['user'], $creds['password']);

// Wait for the connection to complete
$output = waitForConnection($proc);

if (strpos($output, 'SUCCESS') !== false) {
    echo "OAuth Okta authorization code authentication successful\n";
} else {
    echo "FAILED: $output\n";
}

deleteTemporaryCredentialFile();
?>
===DONE===
<?php exit(0); ?>
--CLEAN--
<?php
require_once __DIR__ . '/auth_helper.php';
cleanupBrowserProcesses();
deleteTemporaryCredentialFile();
?>
--EXPECT--
OAuth Okta authorization code authentication successful
===DONE===
