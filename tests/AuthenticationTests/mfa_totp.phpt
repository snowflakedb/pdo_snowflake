--TEST--
pdo_snowflake - MFA authentication with TOTP codes from generator
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_MFA_USER',
    'SNOWFLAKE_AUTH_TEST_MFA_PASSWORD',
    'SNOWFLAKE_AUTH_MFA_SEED',
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
];
$user = getenv('SNOWFLAKE_AUTH_TEST_MFA_USER');
$password = getenv('SNOWFLAKE_AUTH_TEST_MFA_PASSWORD');
$mfaSeed = getenv('SNOWFLAKE_AUTH_MFA_SEED');

// DSN with authenticator for MFA token caching
$cachingDsn = buildMfaDsn($config, [
    'authenticator' => 'username_password_mfa',
    'client_request_mfa_token' => 'true',
]);

// Get TOTP codes from seed
$totpCodes = getTotpCodes($mfaSeed);

if (empty($totpCodes)) {
    echo "FAILED: No TOTP codes were generated\n";
    exit(1);
}

$connectionSuccess = false;
$lastError = '';

// TOTP codes are time-sensitive - try each quickly until one works
for ($i = 0; $i < count($totpCodes); $i++) {
    $totpCode = $totpCodes[$i];
    $dsn = buildMfaDsn($config, ['passcode' => $totpCode]);
    
    $output = tryMfaConnection($dsn, $user, $password);
    
    if (strpos($output, 'SUCCESS') !== false) {
        echo "MFA authentication: SUCCESS\n";
        $connectionSuccess = true;
        
        // Test MFA token caching - connect WITHOUT passcode
        $cacheOutput = tryMfaConnection($cachingDsn, $user, $password);
        if (strpos($cacheOutput, 'SUCCESS') !== false) {
            echo "MFA token cache: SUCCESS\n";
        } else {
            echo "MFA token cache: not available\n";
        }
        break;
    } else {
        $lastError = $output;
    }
}

if (!$connectionSuccess) {
    echo "FAILED: " . $lastError . "\n";
}

deleteTemporaryCredentialFile();
?>
===DONE===
<?php exit(0); ?>
--CLEAN--
<?php
require_once __DIR__ . '/auth_helper.php';
deleteTemporaryCredentialFile();
?>
--EXPECTF--
MFA authentication: SUCCESS
MFA token cache: %s
===DONE===
