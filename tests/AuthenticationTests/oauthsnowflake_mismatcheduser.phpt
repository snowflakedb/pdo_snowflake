--TEST--
pdo_snowflake - OAuth Snowflake authorization code authentication (mismatched user)
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();
?>
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';
validateOAuthSnowflakeEnvVars();

deleteTemporaryCredentialFile();

$config = getOAuthSnowflakeDsnConfig();
$creds = getOAuthLoginCredentials();
$wrongUser = 'differentUsername';

echo "Connection process started\n";
$dsn = buildOAuthSnowflakeDsn($config);
$proc = startConnectionProcess($dsn, $wrongUser);

echo "Providing credentials\n";
provideBrowserCredentials('internalOauthSnowflakeSuccess', $creds['user'], $creds['password']);

echo "Waiting for result\n";
$output = waitForConnection($proc);

if (stripos($output, 'The user you were trying to authenticate as differs from the user') !== false) {
    echo "PASS: User mismatch correctly rejected\n";
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
PASS: User mismatch correctly rejected
===DONE===
