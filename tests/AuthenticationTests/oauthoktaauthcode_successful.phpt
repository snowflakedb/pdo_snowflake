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

$host = getenv('SNOWFLAKE_AUTH_TEST_HOST');
$account = getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT');
$database = getenv('SNOWFLAKE_AUTH_TEST_DATABASE');
$schema = getenv('SNOWFLAKE_AUTH_TEST_SCHEMA');
$warehouse = getenv('SNOWFLAKE_AUTH_TEST_WAREHOUSE');
$role = getenv('SNOWFLAKE_AUTH_TEST_ROLE');
$clientId = getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID');
$clientSecret = getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_SECRET');
$redirectUri = str_replace('portToReplace', '8007', getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_REDIRECT_URI'));
$authUrl = getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_AUTH_URL');
$tokenUrl = getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_TOKEN');
$user = getenv('SNOWFLAKE_AUTH_TEST_OKTA_USER');
$password = getenv('SNOWFLAKE_AUTH_TEST_OKTA_PASS');

$dsn = "snowflake:host=$host;port=443;protocol=https;account=$account;database=$database;schema=$schema;warehouse=$warehouse;role=$role;authenticator=oauth_authorization_code;oauth_client_id=$clientId;oauth_client_secret=$clientSecret;oauth_redirect_uri=$redirectUri;oauth_authorization_endpoint=$authUrl;oauth_token_endpoint=$tokenUrl;oauth_scope=session:role:$role";

// Start the connection process
$proc = startConnectionProcess($dsn, $user);

// Provide credentials using the external browser script
// Scenario: externalOauthOktaSuccess
provideBrowserCredentials('externalOauthOktaSuccess', $user, $password);

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
