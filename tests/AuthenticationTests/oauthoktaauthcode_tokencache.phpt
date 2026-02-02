--TEST--
pdo_snowflake - OAuth Okta authorization code authentication (token cache)
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

$config = getOAuthOktaAuthCodeConfig();
$config['oauth_redirect_uri'] = str_replace('portToReplace', '8005', $config['oauth_redirect_uri']);
$creds = getOktaCredentials();
$user = $creds['user'];

$dsn = buildOAuthOktaAuthCodeDsn($config);

// Clean up any existing cached tokens
deleteTemporaryCredentialFile();

// =============================================================================
// SECTION 1: Initial authentication using browser
// =============================================================================
echo "SECTION 1: Initial authentication using browser\n";

$proc = startConnectionProcess($dsn, $user);
provideBrowserCredentials('externalOauthOktaSuccess', $user, $creds['password']);
$output = waitForConnection($proc);

if (strpos($output, 'SUCCESS') !== false) {
    echo "Initial authentication: SUCCESS\n";
} else {
    echo "Initial authentication: FAILED - $output\n";
    exit(1);
}

// =============================================================================
// SECTION 2: Should authenticate using cached token without browser
// =============================================================================
echo "\nSECTION 2: Authenticate using cached token (no browser)\n";

$phpCode = buildConnectionPhpCode($dsn, $user);
$iniSettings = getPdoSnowflakeIniSettings();

$cmd = ['php'];
foreach ($iniSettings as $setting) {
    $cmd[] = '-d';
    $cmd[] = $setting;
}
$cmd[] = '-d';
$cmd[] = 'extension=modules/pdo_snowflake.so';
$cmd[] = '-r';
$cmd[] = $phpCode;

$proc = startProcess($cmd);
$output = waitForConnection($proc, 15);

if (strpos($output, 'SUCCESS') !== false) {
    echo "Cached token authentication: SUCCESS\n";
} else {
    echo "Cached token authentication: FAILED - $output\n";
}

// =============================================================================
// SECTION 3: Should open browser again when token is deleted
// =============================================================================
echo "\nSECTION 3: Re-authenticate after token deletion\n";

deleteTemporaryCredentialFile();

// Use different port for this connection
$config['oauth_redirect_uri'] = str_replace('portToReplace', '8006', getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_REDIRECT_URI'));
$dsn2 = buildOAuthOktaAuthCodeDsn($config);

$proc = startConnectionProcess($dsn2, $user);
provideBrowserCredentials('externalOauthOktaSuccess', $user, $creds['password']);
$output = waitForConnection($proc);

if (strpos($output, 'SUCCESS') !== false) {
    echo "Re-authentication after token deletion: SUCCESS\n";
} else {
    echo "Re-authentication after token deletion: FAILED - $output\n";
}

// Cleanup
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
SECTION 1: Initial authentication using browser
Initial authentication: SUCCESS

SECTION 2: Authenticate using cached token (no browser)
Cached token authentication: SUCCESS

SECTION 3: Re-authenticate after token deletion
Re-authentication after token deletion: SUCCESS
===DONE===
