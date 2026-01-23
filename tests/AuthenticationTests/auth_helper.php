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

