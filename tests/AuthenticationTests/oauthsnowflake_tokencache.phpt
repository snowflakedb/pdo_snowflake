--TEST--
pdo_snowflake - OAuth Snowflake authorization code authentication (token cache)
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();
?>
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';
validateOAuthSnowflakeEnvVars();

$config = getOAuthSnowflakeDsnConfig();
$creds = getOAuthLoginCredentials();
$user = $creds['user'];
$dsn = buildOAuthSnowflakeDsn($config);

// Clean up any existing cached tokens
deleteTemporaryCredentialFile();

// =============================================================================
// SECTION 1: Initial authentication using browser
// =============================================================================
echo "SECTION 1: Initial authentication using browser\n";

$proc = startConnectionProcess($dsn, $user);
provideBrowserCredentials('internalOauthSnowflakeSuccess', $creds['user'], $creds['password']);
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

// Build PHP code that connects directly without browser interaction
$phpCode = sprintf(
    'try {
        $pdo = new PDO(%s, %s, "");
        $stmt = $pdo->query("SELECT 1");
        $result = $stmt->fetch(PDO::FETCH_NUM);
        echo ($result[0] == "1") ? "SUCCESS" : "ERROR: unexpected result";
    } catch (PDOException $e) {
        echo "ERROR: " . $e->getMessage();
    }',
    var_export($dsn, true),
    var_export($user, true)
);

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

$proc = startConnectionProcess($dsn, $user);
provideBrowserCredentials('internalOauthSnowflakeSuccess', $creds['user'], $creds['password']);
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
