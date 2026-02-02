--TEST--
pdo_snowflake - OAuth Snowflake authorization code authentication (custom redirect URI)
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();
?>
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';
validateOAuthSnowflakeRedirectUriEnvVars();

deleteTemporaryCredentialFile();

$config = getOAuthSnowflakeRedirectUriDsnConfig();
$creds = getOAuthLoginCredentials();
$user = $creds['user'];

echo "Connection process started\n";
$dsn = buildOAuthSnowflakeRedirectUriDsn($config);
$proc = startConnectionProcess($dsn, $user);

echo "Providing credentials\n";
provideBrowserCredentials('internalOauthSnowflakeSuccess', $creds['user'], $creds['password']);

echo "Waiting for result\n";
$output = waitForConnection($proc);

if (strpos($output, 'SUCCESS') !== false) {
    echo "SUCCESS\n";
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
Connection process started
Providing credentials
Waiting for result
SUCCESS
===DONE===
