--TEST--
pdo_snowflake - WIF authentication
--INI--
pdo_snowflake.cacert=cacert.pem
pdo_snowflake.loglevel=DEBUG
--FILE--
<?php
$provider = getenv('SNOWFLAKE_TEST_WIF_PROVIDER');
$host = getenv('SNOWFLAKE_TEST_WIF_HOST');
$account = getenv('SNOWFLAKE_TEST_WIF_ACCOUNT');

echo "Testing WIF with provider: $provider\n";

// Test 1: shouldn't authenticate using wif without explicit provider
try {
    $dbh = new PDO(
        "snowflake:host=$host;account=$account;authenticator=workload_identity",
        "", ""
    );
    echo "FAIL: Should have thrown exception without provider\n";
    exit(1);
} catch (PDOException $e) {
    if (strpos($e->getMessage(), "workload_identity_provider") !== false || 
        strpos($e->getMessage(), "WORKLOAD_IDENTITY_PROVIDER") !== false || 
        strpos($e->getMessage(), "Required setting") !== false) {
        echo "PASS: Test 1 - Correctly rejected without provider\n";
    } else {
        echo "FAIL: Wrong error message: " . $e->getMessage() . "\n";
        exit(1);
    }
}

// Test 2: should authenticate using wif with explicit provider
try {
    $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider";
    $dbh = new PDO($dsn, "", "");
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    $sth = $dbh->query("SELECT CURRENT_USER()");
    $row = $sth->fetch(PDO::FETCH_NUM);
    echo "PASS: Test 2 - Connected with WIF provider $provider\n";
    $dbh = null;
} catch (PDOException $e) {
    echo "FAIL: Could not connect with WIF: " . $e->getMessage() . "\n";
    exit(1);
}

// Test 3: should authenticate using OIDC (GCP only)
if ($provider === "GCP") {
    // Fetch GCP access token from metadata service
    $token = "";
    $cmd = 'curl -H "Metadata-Flavor: Google" "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/identity?audience=snowflakecomputing.com" 2>/dev/null';
    $token = trim(shell_exec($cmd));
    
    if (empty($token)) {
        echo "SKIP: Test 3 - Could not retrieve GCP access token\n";
    } else {
        try {
            $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=OIDC;token=$token";
            $dbh = new PDO($dsn, "", "");
            $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            
            $sth = $dbh->query("SELECT CURRENT_USER()");
            $row = $sth->fetch(PDO::FETCH_NUM);
            echo "PASS: Test 3 - OIDC authentication successful\n";
            $dbh = null;
        } catch (PDOException $e) {
            echo "FAIL: OIDC authentication failed: " . $e->getMessage() . "\n";
            exit(1);
        }
    }
} else {
    echo "SKIP: Test 3 - OIDC test only runs on GCP\n";
}

echo "All tests completed\n";
?>
--EXPECTF--
Testing WIF with provider: %s
PASS: Test 1 - Correctly rejected without provider
PASS: Test 2 - Connected with WIF provider %s
%s: Test 3 - %s
All tests completed

