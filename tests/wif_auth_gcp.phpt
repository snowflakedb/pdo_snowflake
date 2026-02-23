--TEST--
pdo_snowflake - WIF authentication (GCP)
--SKIPIF--
<?php
$wif_required = getenv('SNOWFLAKE_WIF_TEST_REQUIRED');

if (getenv('SNOWFLAKE_TEST_WIF_PROVIDER') !== 'GCP' ||
    !getenv('SNOWFLAKE_TEST_WIF_HOST') ||
    !getenv('SNOWFLAKE_TEST_WIF_ACCOUNT')) {

    if ($wif_required === 'true') {
        echo "FATAL: GCP WIF test required but environment variables not set\n";
        exit(1);
    }

    die('skip GCP WIF tests require SNOWFLAKE_TEST_WIF_PROVIDER=GCP, SNOWFLAKE_TEST_WIF_HOST, and SNOWFLAKE_TEST_WIF_ACCOUNT');
}
?>
--INI--
pdo_snowflake.cacert=cacert.pem
pdo_snowflake.loglevel=DEBUG
--FILE--
<?php
require_once __DIR__ . '/wif_helper.php';

$config = getWifConfig();
requireEnv($config, ['username', 'impersonation_path', 'impersonation_username']);

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
    $dsn = buildWifDsn($config, ['provider' => 'GCP']);
    $dbh = wifConnect($dsn);
    $dbh->query("SELECT CURRENT_USER()");
    echo "PASS: Test 2 - Connected with WIF provider GCP\n";
    $dbh = null;
} catch (PDOException $e) {
    echo "FAIL: Test 2 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 3: OIDC authentication with GCP metadata token
$ctx = stream_context_create(['http' => ['header' => "Metadata-Flavor: Google\r\n"]]);
$token = @file_get_contents(
    "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/identity?audience=snowflakecomputing.com",
    false, $ctx
);
$token = $token !== false ? trim($token) : '';

if (empty($token)) {
    echo "FAIL: Test 3 - Could not retrieve GCP identity token\n";
    exit(1);
}
try {
    $dsn = buildWifDsn($config, ['provider' => 'OIDC', 'token' => $token]);
    $dbh = wifConnect($dsn);
    $dbh->query("SELECT CURRENT_USER()");
    echo "PASS: Test 3 - OIDC authentication successful\n";
    $dbh = null;
} catch (PDOException $e) {
    echo "FAIL: Test 3 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 4: verify WIF user identity
try {
    $dsn = buildWifDsn($config, ['provider' => 'GCP']);
    $dbh = wifConnectAndVerifyUser($dsn, $config['username']);
    echo "PASS: Test 4 - WIF user verified\n";
    $dbh = null;
} catch (Exception $e) {
    echo "FAIL: Test 4 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 5: impersonation with valid path
try {
    $dsn = buildWifDsn($config, ['provider' => 'GCP', 'impersonation_path' => $config['impersonation_path']]);
    $dbh = wifConnectAndVerifyUser($dsn, $config['impersonation_username']);
    echo "PASS: Test 5 - WIF impersonation successful\n";
    $dbh = null;
} catch (Exception $e) {
    echo "FAIL: Test 5 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 6: empty impersonation path falls back to direct WIF
try {
    $dsn = buildWifDsn($config, ['provider' => 'GCP', 'impersonation_path' => '']);
    $dbh = wifConnectAndVerifyUser($dsn, $config['username']);
    echo "PASS: Test 6 - Empty impersonation path falls back to direct WIF\n";
    $dbh = null;
} catch (Exception $e) {
    echo "FAIL: Test 6 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 7: invalid impersonation path is rejected
try {
    $dsn = buildWifDsn($config, [
        'provider' => 'GCP',
        'impersonation_path' => 'invalid-sa@nonexistent-project.iam.gserviceaccount.com',
    ]);
    $dbh = new PDO($dsn, "", "");
    echo "FAIL: Test 7 - Should have thrown exception\n";
    exit(1);
} catch (PDOException $e) {
    if (strpos($e->getMessage(), "390100") !== false ||
        strpos($e->getMessage(), "Incorrect username or password") !== false ||
        strpos($e->getMessage(), "workload identity") !== false ||
        strpos($e->getMessage(), "impersonation") !== false ||
        strpos($e->getMessage(), "identity token") !== false) {
        echo "PASS: Test 7 - Invalid impersonation path correctly rejected\n";
    } else {
        echo "FAIL: Test 7 - Unexpected error: " . $e->getMessage() . "\n";
        exit(1);
    }
}

// Test 8: query execution after impersonation
try {
    $dsn = buildWifDsn($config, ['provider' => 'GCP', 'impersonation_path' => $config['impersonation_path']]);
    $dbh = wifConnectAndVerifyUser($dsn, $config['impersonation_username']);

    $row = $dbh->query("SELECT CURRENT_ROLE()")->fetch(PDO::FETCH_NUM);
    if (empty($row[0])) {
        echo "FAIL: Test 8 - CURRENT_ROLE() returned empty\n";
        exit(1);
    }

    $row = $dbh->query("SELECT 'lorem ipsum'")->fetch(PDO::FETCH_NUM);
    if ($row[0] !== 'lorem ipsum') {
        echo "FAIL: Test 8 - Unexpected query result\n";
        exit(1);
    }

    echo "PASS: Test 8 - Queries executed successfully after impersonation\n";
    $dbh = null;
} catch (Exception $e) {
    echo "FAIL: Test 8 - " . $e->getMessage() . "\n";
    exit(1);
}

// Test 9: multiple connections with same impersonation path
try {
    $dsn = buildWifDsn($config, ['provider' => 'GCP', 'impersonation_path' => $config['impersonation_path']]);

    $dbh1 = wifConnectAndVerifyUser($dsn, $config['impersonation_username']);
    $dbh2 = wifConnectAndVerifyUser($dsn, $config['impersonation_username']);

    echo "PASS: Test 9 - Multiple connections with impersonation successful\n";
    $dbh1 = null;
    $dbh2 = null;
} catch (Exception $e) {
    echo "FAIL: Test 9 - " . $e->getMessage() . "\n";
    exit(1);
}

echo "All 9 tests passed\n";
?>
--EXPECT--
PASS: Test 1 - Correctly rejected without provider
PASS: Test 2 - Connected with WIF provider GCP
PASS: Test 3 - OIDC authentication successful
PASS: Test 4 - WIF user verified
PASS: Test 5 - WIF impersonation successful
PASS: Test 6 - Empty impersonation path falls back to direct WIF
PASS: Test 7 - Invalid impersonation path correctly rejected
PASS: Test 8 - Queries executed successfully after impersonation
PASS: Test 9 - Multiple connections with impersonation successful
All 9 tests passed
