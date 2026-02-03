--TEST--
pdo_snowflake - external browser authentication (successful)
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
$user = getenv('SNOWFLAKE_AUTH_TEST_BROWSER_USER');
$browserUser = getenv('SNOWFLAKE_AUTH_TEST_BROWSER_USER');
$browserPass = getenv('SNOWFLAKE_AUTH_TEST_OKTA_PASS');

echo "Connection process started\n";
$dsn = buildExternalBrowserDsn($config);
$proc = startConnectionProcess($dsn, $user);

echo "Providing credentials\n";
provideBrowserCredentials('success', $browserUser, $browserPass);

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
cleanupBrowserProcesses();
?>
--EXPECT--
Connection process started
Providing credentials
Waiting for result
SUCCESS
===DONE===
