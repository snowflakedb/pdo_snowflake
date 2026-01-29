<?php
/**
 * Authentication Test Helper
 * Common functions for external browser authentication tests
 */

/**
 * Skip test if running in GitHub Actions
 * External browser tests require interactive browser sessions not available in GitHub Actions
 */
function skipIfGitHubActions(): void {
    if (getenv('GITHUB_ACTIONS') === 'true') {
        die('skip External browser tests not supported in GitHub Actions');
    }
}

/**
 * Validate required environment variables for external browser tests
 * Throws exception if any are missing (configuration error, not skip condition)
 */
function validateExternalBrowserEnvVars(): void {
    $required = [
        'SNOWFLAKE_AUTH_TEST_HOST',
        'SNOWFLAKE_AUTH_TEST_ACCOUNT',
        'SNOWFLAKE_AUTH_TEST_BROWSER_USER',
        'SNOWFLAKE_AUTH_TEST_OKTA_PASS',
        'SNOWFLAKE_AUTH_TEST_DATABASE',
        'SNOWFLAKE_AUTH_TEST_SCHEMA',
        'SNOWFLAKE_AUTH_TEST_WAREHOUSE',
        'SNOWFLAKE_AUTH_TEST_ROLE',
    ];

    $missing = array_filter($required, function($var) {
        return !getenv($var);
    });

    if (!empty($missing)) {
        throw new RuntimeException(
            'Missing required environment variables: ' . implode(', ', $missing)
        );
    }
}

/**
 * Get connection configuration from environment variables
 */
function getExternalBrowserDsnConfig(): array {
    return [
        'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
        'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
        'database' => getenv('SNOWFLAKE_AUTH_TEST_DATABASE'),
        'schema' => getenv('SNOWFLAKE_AUTH_TEST_SCHEMA'),
        'warehouse' => getenv('SNOWFLAKE_AUTH_TEST_WAREHOUSE'),
        'role' => getenv('SNOWFLAKE_AUTH_TEST_ROLE'),
    ];
}

/**
 * Build DSN string for external browser authentication
 */
function buildExternalBrowserDsn(array $config): string {
    return sprintf(
        "snowflake:host=%s;port=443;protocol=https;account=%s;database=%s;schema=%s;warehouse=%s;role=%s;authenticator=externalbrowser",
        $config['host'],
        $config['account'],
        $config['database'],
        $config['schema'],
        $config['warehouse'],
        $config['role']
    );
}

/**
 * Get pdo_snowflake INI settings for subprocess
 * Returns array of "key=value" strings for -d flags
 * 
 * Note: Hardcoded to match --INI-- section in test files
 * Parent process doesn't load extension to avoid log file conflicts
 */
function getPdoSnowflakeIniSettings(): array {
    return [
        'pdo_snowflake.logdir=sflog',
        'pdo_snowflake.loglevel=DEBUG',
        'pdo_snowflake.cacert=libsnowflakeclient/cacert.pem',
    ];
}

/**
 * Build PHP code for connection test
 * Returns PHP code string that connects and runs SELECT 1
 */
function buildConnectionPhpCode(string $dsn, string $user): string {
    return sprintf(
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
}

/**
 * Build connection command for subprocess
 * Returns array of command arguments
 */
function buildConnectionCommand(string $dsn, string $user): array {
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
    
    return $cmd;
}

/**
 * Start subprocess
 * Returns process handle and pipes
 */
function startProcess(array $cmd): array {
    $process = proc_open(
        $cmd,
        [['pipe', 'r'], ['pipe', 'w'], ['pipe', 'w']],
        $pipes
    );
    
    fclose($pipes[0]);
    stream_set_blocking($pipes[1], false);
    stream_set_blocking($pipes[2], false);
    
    return ['process' => $process, 'stdout' => $pipes[1], 'stderr' => $pipes[2]];
}

/**
 * Start connection process in background
 * Convenience wrapper combining buildConnectionCommand + startProcess
 */
function startConnectionProcess(string $dsn, string $user): array {
    $cmd = buildConnectionCommand($dsn, $user);
    return startProcess($cmd);
}

/**
 * Provide browser credentials via Playwright
 */
function provideBrowserCredentials(string $scenario, string $user, string $password): void {
    $cmd = sprintf(
        'node /externalbrowser/provideBrowserCredentials.js %s %s %s 2>&1',
        escapeshellarg($scenario),
        escapeshellarg($user),
        escapeshellarg($password)
    );
    shell_exec($cmd);
}

/**
 * Clean up browser processes after test
 * Ensures no orphaned chromium processes remain
 */
function cleanupBrowserProcesses(): void {
    shell_exec('node /externalbrowser/cleanBrowserProcesses.js 2>/dev/null');
}

/**
 * Wait for connection process to complete
 * Returns output string (stdout + stderr)
 */
function waitForConnection(array $procInfo, int $timeout = 30): string {
    $output = '';
    $error = '';
    $start = time();
    
    while ((time() - $start) < $timeout) {
        $status = proc_get_status($procInfo['process']);
        $output .= stream_get_contents($procInfo['stdout']);
        $error .= stream_get_contents($procInfo['stderr']);
        if (!$status['running']) break;
        usleep(100000); // 100ms
    }
    
    // Final read
    $output .= stream_get_contents($procInfo['stdout']);
    $error .= stream_get_contents($procInfo['stderr']);
    
    // If still running after timeout, kill it
    $status = proc_get_status($procInfo['process']);
    if ($status['running']) {
        proc_terminate($procInfo['process'], 9); // SIGKILL
        $error .= "\nTIMEOUT: Process exceeded {$timeout} seconds\n";
    }
    
    fclose($procInfo['stdout']);
    fclose($procInfo['stderr']);
    proc_close($procInfo['process']);
    
    return $output . $error;
}

// =============================================================================
// OAuth Snowflake Authorization Code Authentication Helpers
// =============================================================================

/**
 * Validate required environment variables for OAuth Snowflake authorization code tests
 */
function validateOAuthSnowflakeEnvVars(): void {
    $required = [
        'SNOWFLAKE_AUTH_TEST_HOST',
        'SNOWFLAKE_AUTH_TEST_ACCOUNT',
        'SNOWFLAKE_AUTH_TEST_DATABASE',
        'SNOWFLAKE_AUTH_TEST_SCHEMA',
        'SNOWFLAKE_AUTH_TEST_WAREHOUSE',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_WILDCARDS_CLIENT_ID',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_WILDCARDS_CLIENT_SECRET',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_ROLE',
        'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID',
        'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_USER_PASSWORD',
    ];

    $missing = array_filter($required, function($var) {
        return !getenv($var);
    });

    if (!empty($missing)) {
        throw new RuntimeException(
            'Missing required environment variables: ' . implode(', ', $missing)
        );
    }
}

/**
 * Validate required environment variables for OAuth Snowflake with custom redirect URI
 */
function validateOAuthSnowflakeRedirectUriEnvVars(): void {
    $required = [
        'SNOWFLAKE_AUTH_TEST_HOST',
        'SNOWFLAKE_AUTH_TEST_ACCOUNT',
        'SNOWFLAKE_AUTH_TEST_DATABASE',
        'SNOWFLAKE_AUTH_TEST_SCHEMA',
        'SNOWFLAKE_AUTH_TEST_WAREHOUSE',
        'SNOWFLAKE_AUTH_TEST_ROLE',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_CLIENT_ID',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_CLIENT_SECRET',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_REDIRECT_URI',
        'SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_ROLE',
        'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID',
        'SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_USER_PASSWORD',
    ];

    $missing = array_filter($required, function($var) {
        return !getenv($var);
    });

    if (!empty($missing)) {
        throw new RuntimeException(
            'Missing required environment variables: ' . implode(', ', $missing)
        );
    }
}

/**
 * Get OAuth Snowflake authorization code connection configuration
 */
function getOAuthSnowflakeDsnConfig(): array {
    return [
        'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
        'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
        'database' => getenv('SNOWFLAKE_AUTH_TEST_DATABASE'),
        'schema' => getenv('SNOWFLAKE_AUTH_TEST_SCHEMA'),
        'warehouse' => getenv('SNOWFLAKE_AUTH_TEST_WAREHOUSE'),
        'role' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_ROLE'),
        'oauth_client_id' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_WILDCARDS_CLIENT_ID'),
        'oauth_client_secret' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_WILDCARDS_CLIENT_SECRET'),
    ];
}

/**
 * Get OAuth Snowflake authorization code connection configuration with custom redirect URI
 */
function getOAuthSnowflakeRedirectUriDsnConfig(): array {
    return [
        'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
        'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
        'database' => getenv('SNOWFLAKE_AUTH_TEST_DATABASE'),
        'schema' => getenv('SNOWFLAKE_AUTH_TEST_SCHEMA'),
        'warehouse' => getenv('SNOWFLAKE_AUTH_TEST_WAREHOUSE'),
        'role' => getenv('SNOWFLAKE_AUTH_TEST_ROLE'),
        'oauth_client_id' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_CLIENT_ID'),
        'oauth_client_secret' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_CLIENT_SECRET'),
        'oauth_redirect_uri' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_REDIRECT_URI'),
        'oauth_scope_role' => getenv('SNOWFLAKE_AUTH_TEST_INTERNAL_OAUTH_SNOWFLAKE_ROLE'),
    ];
}

/**
 * Get OAuth login credentials
 */
function getOAuthLoginCredentials(): array {
    return [
        'user' => getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_CLIENT_ID'),
        'password' => getenv('SNOWFLAKE_AUTH_TEST_EXTERNAL_OAUTH_OKTA_USER_PASSWORD'),
    ];
}

/**
 * Build DSN string for OAuth Snowflake authorization code authentication
 */
function buildOAuthSnowflakeDsn(array $config, array $options = []): string {
    $dsn = sprintf(
        "snowflake:host=%s;port=443;protocol=https;account=%s;database=%s;schema=%s;warehouse=%s;role=%s;authenticator=oauth_authorization_code;oauth_client_id=%s;oauth_client_secret=%s;oauth_scope=session:role:%s",
        $config['host'],
        $config['account'],
        $config['database'],
        $config['schema'],
        $config['warehouse'],
        $config['role'],
        $config['oauth_client_id'],
        $config['oauth_client_secret'],
        $config['role']
    );

    // Add optional parameters
    if (!empty($options['oauth_redirect_uri'])) {
        $dsn .= ';oauth_redirect_uri=' . $options['oauth_redirect_uri'];
    }
    if (!empty($options['browser_response_timeout'])) {
        $dsn .= ';browser_response_timeout=' . $options['browser_response_timeout'];
    }

    return $dsn;
}

/**
 * Build DSN string for OAuth Snowflake with custom redirect URI
 */
function buildOAuthSnowflakeRedirectUriDsn(array $config): string {
    return sprintf(
        "snowflake:host=%s;port=443;protocol=https;account=%s;database=%s;schema=%s;warehouse=%s;role=%s;authenticator=oauth_authorization_code;oauth_client_id=%s;oauth_client_secret=%s;oauth_redirect_uri=%s;oauth_scope=session:role:%s",
        $config['host'],
        $config['account'],
        $config['database'],
        $config['schema'],
        $config['warehouse'],
        $config['role'],
        $config['oauth_client_id'],
        $config['oauth_client_secret'],
        $config['oauth_redirect_uri'],
        $config['oauth_scope_role']
    );
}

/**
 * Delete temporary credential cache file
 */
function deleteTemporaryCredentialFile(): bool {
    $homeDir = getenv('HOME') ?: getenv('USERPROFILE');
    $cacheDir = $homeDir . '/.cache/snowflake';
    $pattern = $cacheDir . '/temporary_credential*';
    
    $files = glob($pattern);
    $deleted = true;
    
    foreach ($files as $file) {
        if (!unlink($file)) {
            $deleted = false;
        }
    }
    
    return $deleted;
}

// =============================================================================
// MFA Authentication Helpers
// =============================================================================

/**
 * Get TOTP codes from the generator script
 * Called INDEPENDENTLY at test level before any driver interaction (matches ODBC)
 * Returns array of TOTP codes to try
 */
function getTotpCodes(string $seed = ''): array {
    $totpGeneratorPath = '/externalbrowser/totpGenerator.js';
    
    $cmd = 'node ' . $totpGeneratorPath;
    if (!empty($seed)) {
        $cmd .= ' ' . escapeshellarg($seed);
    }
    $cmd .= ' 2>&1';
    
    $output = shell_exec($cmd);
    
    if ($output === null) {
        return [];
    }
    
    // Parse output - codes are space or newline separated
    $output = trim($output);
    $codes = preg_split('/\s+/', $output, -1, PREG_SPLIT_NO_EMPTY);
    
    // Ensure all codes are zero-padded to 6 digits (TOTP standard)
    $paddedCodes = [];
    foreach ($codes as $code) {
        // If it's numeric, ensure it's zero-padded to 6 digits
        if (is_numeric($code)) {
            $paddedCodes[] = str_pad((string)$code, 6, '0', STR_PAD_LEFT);
        } else {
            $paddedCodes[] = $code;
        }
    }
    
    return $paddedCodes ?: [];
}

/**
 * Try MFA connection with given DSN, user, and password
 * Runs in subprocess and returns output (SUCCESS or error message)
 * PHP driver takes user/password from PDO constructor args (NOT from DSN like ODBC)
 */
function tryMfaConnection(string $dsn, string $user, string $password, int $timeout = 30): string {
    $phpCode = sprintf(
        'try {
            $pdo = new PDO(%s, %s, %s);
            $stmt = $pdo->query("SELECT 1");
            $result = $stmt->fetch(PDO::FETCH_NUM);
            echo ($result[0] == "1") ? "SUCCESS" : "ERROR: unexpected result";
        } catch (PDOException $e) {
            echo "ERROR: " . $e->getMessage();
        }',
        var_export($dsn, true),
        var_export($user, true),
        var_export($password, true)
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
    return waitForConnection($proc, $timeout);
}

/**
 * Generate a unique PAT token name with timestamp suffix
 */
function generatePatTokenName(string $prefix = 'PAT_PHP_'): string {
    return $prefix . round(microtime(true) * 1000);
}

/**
 * Get a PAT token using the patHelper.js script
 */
function getPatToken(string $tokenName): string {
    $cmd = "node /externalbrowser/patHelper.js getPAT " . escapeshellarg($tokenName) . " 2>&1";
    $output = shell_exec($cmd);
    $output = trim($output ?? '');
    
    // Check if output is an error (Node.js errors start with "node:" or contain "Error")
    if (strpos($output, 'node:') === 0 || stripos($output, 'Error') !== false) {
        echo "PAT Helper Error: $output\n";
        return '';
    }
    
    return $output;
}

/**
 * Delete a PAT token using the patHelper.js script
 */
function deletePatToken(string $tokenName): void {
    $cmd = "node /externalbrowser/patHelper.js deletePAT " . escapeshellarg($tokenName) . " 2>&1";
    shell_exec($cmd);
}
