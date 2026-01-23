--TEST--
pdo_snowflake - external browser authentication (mismatched user)
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();
?>
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';
validateExternalBrowserEnvVars();

$config = getExternalBrowserDsnConfig();
$wrongUser = 'WrongUser';
$browserUser = getenv('SNOWFLAKE_AUTH_TEST_BROWSER_USER');
$browserPass = getenv('SNOWFLAKE_AUTH_TEST_OKTA_PASS');

echo "Connection process started\n";
$dsn = buildExternalBrowserDsn($config);
$proc = startConnectionProcess($dsn, $wrongUser);

echo "Providing credentials\n";
provideBrowserCredentials('success', $browserUser, $browserPass);

echo "Waiting for result\n";
$output = waitForConnection($proc);

if (stripos($output, 'The user you were trying to authenticate as differs from the user') !== false) {
    echo "PASS: User mismatch correctly rejected\n";
} else {
    echo "FAILED: $output\n";
}
?>
===DONE===
<?php exit(0); ?>
--CLEAN--
<?php
require_once __DIR__ . '/auth_helper.php';
cleanupBrowserProcesses();
?>
--EXPECT--
Connection process started
Providing credentials
Waiting for result
PASS: User mismatch correctly rejected
===DONE===
