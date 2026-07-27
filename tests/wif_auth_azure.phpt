--TEST--
pdo_snowflake - WIF authentication (Azure)
--SKIPIF--
<?php
$wif_required = getenv('SNOWFLAKE_WIF_TEST_REQUIRED');

if (getenv('SNOWFLAKE_TEST_WIF_PROVIDER') !== 'AZURE' ||
    !getenv('SNOWFLAKE_TEST_WIF_HOST') ||
    !getenv('SNOWFLAKE_TEST_WIF_ACCOUNT')) {

    if ($wif_required === 'true') {
        echo "FATAL: Azure WIF test required but environment variables not set\n";
        exit(1);
    }

    die('skip Azure WIF tests require SNOWFLAKE_TEST_WIF_PROVIDER=AZURE, SNOWFLAKE_TEST_WIF_HOST, and SNOWFLAKE_TEST_WIF_ACCOUNT');
}
?>
--INI--
pdo_snowflake.cacert=cacert.pem
pdo_snowflake.loglevel=DEBUG
--FILE--
<?php
require_once __DIR__ . '/wif_helper.php';

$config = getWifConfig();
requireEnv($config, ['username']);

// Test 1: missing provider is rejected
try {
    $dsn = buildWifDsn($config);
    $dbh = new PDO($dsn, "", "");
    echo "FAIL: Test 1 - Should have thrown exception without provider\n";
    exit(1);
} catch (PDOException $e) {
    if (strpos($e->getMessage(), "workload_identity_provider") !== false ||
        strpos($e->getMessage(), "WORKLOAD_IDENTITY_PROVIDER") !== false ||
        strpos($e->getMessage(), "Required setting") !== false) {
        echo "PASS: Test 1 - Correctly rejected without provider\n";
    } else {
        echo "FAIL: Test 1 - Wrong error: " . $e->getMessage() . "\n";
        exit(1);
    }
}

// Test 2: basic WIF authentication
try {
    $dsn = buildWifDsn($config, ['provider' => 'AZURE']);
    $dbh = wifConnect($dsn);
    $dbh->query("SELECT CURRENT_USER()");
    echo "PASS: Test 2 - Connected with WIF provider AZURE\n";
    $dbh = null;
} catch (PDOException $e) {
    echo "FAIL: Test 2 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 3: verify WIF user identity
try {
    $dsn = buildWifDsn($config, ['provider' => 'AZURE']);
    $dbh = wifConnectAndVerifyUser($dsn, $config['username']);
    echo "PASS: Test 3 - WIF user verified\n";
    $dbh = null;
} catch (Exception $e) {
    echo "FAIL: Test 3 - " . $e->getMessage() . "\n";
    exit(1);
}

echo "All 3 tests passed\n";
?>
--EXPECT--
PASS: Test 1 - Correctly rejected without provider
PASS: Test 2 - Connected with WIF provider AZURE
PASS: Test 3 - WIF user verified
All 3 tests passed
