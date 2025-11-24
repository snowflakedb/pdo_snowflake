--TEST--
pdo_snowflake - WIF authentication
--SKIPIF--
<?php
// When SNOWFLAKE_WIF_TEST_REQUIRED is set, fail instead of skip when vars are missing
$wif_required = getenv('SNOWFLAKE_WIF_TEST_REQUIRED');

if (!getenv('SNOWFLAKE_TEST_WIF_PROVIDER') || 
    !getenv('SNOWFLAKE_TEST_WIF_HOST') || 
    !getenv('SNOWFLAKE_TEST_WIF_ACCOUNT')) {
    
    if ($wif_required === 'true') {
        echo "FATAL: WIF test required but environment variables not set\n";
        echo "Missing: SNOWFLAKE_TEST_WIF_PROVIDER, SNOWFLAKE_TEST_WIF_HOST, or SNOWFLAKE_TEST_WIF_ACCOUNT\n";
        exit(1);
    }
    
    die('skip WIF tests require SNOWFLAKE_TEST_WIF_PROVIDER, SNOWFLAKE_TEST_WIF_HOST, and SNOWFLAKE_TEST_WIF_ACCOUNT environment variables');
}
?>
--INI--
pdo_snowflake.cacert=cacert.pem
pdo_snowflake.loglevel=DEBUG
--FILE--
<?php
$provider = getenv('SNOWFLAKE_TEST_WIF_PROVIDER');
$host = getenv('SNOWFLAKE_TEST_WIF_HOST');
$account = getenv('SNOWFLAKE_TEST_WIF_ACCOUNT');
$expectedUsername = getenv('SNOWFLAKE_TEST_WIF_USERNAME') ?: '';
$impersonationPath = getenv('SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH') ?: '';
$impersonationUsername = getenv('SNOWFLAKE_TEST_WIF_USERNAME_IMPERSONATION') ?: '';

function supportsImpersonation($provider) {
    return ($provider === "GCP" || $provider === "AWS");
}

echo "Testing WIF with provider: $provider\n";

// Test 1: shouldn't authenticate using wif without explicit provider
try {
    $dbh = new PDO(
        "snowflake:host=$host;account=$account;authenticator=workload_identity",
        "", ""
    );
    echo "FAIL: Test 1 - Should have thrown exception without provider\n";
    exit(1);
} catch (PDOException $e) {
    if (strpos($e->getMessage(), "workload_identity_provider") !== false || 
        strpos($e->getMessage(), "WORKLOAD_IDENTITY_PROVIDER") !== false || 
        strpos($e->getMessage(), "Required setting") !== false) {
        echo "PASS: Test 1 - Correctly rejected without provider\n";
    } else {
        echo "FAIL: Test 1 - Wrong error message: " . $e->getMessage() . "\n";
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
    echo "FAIL: Test 2 - Could not connect with WIF: " . $e->getMessage() . "\n";
    exit(1);
}

// Test 3: should authenticate using OIDC (GCP only)
if ($provider === "GCP") {
    // Fetch GCP identity token from metadata service
    $cmd = 'curl -H "Metadata-Flavor: Google" "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/identity?audience=snowflakecomputing.com" 2>/dev/null';
    $token = trim(shell_exec($cmd));
    
    if (empty($token)) {
        echo "FAIL: Test 3 - Could not retrieve GCP identity token on GCP VM\n";
        exit(1);
    }
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=OIDC;token=$token";
        $dbh = new PDO($dsn, "", "");
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        
        $sth = $dbh->query("SELECT CURRENT_USER()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        echo "PASS: Test 3 - OIDC authentication successful\n";
        $dbh = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 3 - OIDC authentication failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "SKIP: Test 3 - OIDC test only runs on GCP\n";
}

// Test 4: should authenticate using wif and verify user
if (!empty($expectedUsername)) {
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider";
        $dbh = new PDO($dsn, "", "");
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->query("SELECT CURRENT_USER()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if ($row[0] === $expectedUsername) {
            echo "PASS: Test 4 - WIF user verified: $expectedUsername\n";
        } else {
            echo "FAIL: Test 4 - Expected user '$expectedUsername' but got '" . $row[0] . "'\n";
            exit(1);
        }
        $dbh = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 4 - WIF user verification failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else if (supportsImpersonation($provider)) {
    echo "FAIL: Test 4 - SNOWFLAKE_TEST_WIF_USERNAME must be set on $provider\n";
    exit(1);
} else {
    echo "SKIP: Test 4 - SNOWFLAKE_TEST_WIF_USERNAME not set\n";
}

// Test 5: should authenticate using wif with impersonation
if (supportsImpersonation($provider)) {
    if (empty($impersonationPath)) {
        echo "FAIL: Test 5 - SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH must be set on $provider\n";
        exit(1);
    }
    if (empty($impersonationUsername)) {
        echo "FAIL: Test 5 - SNOWFLAKE_TEST_WIF_USERNAME_IMPERSONATION must be set on $provider\n";
        exit(1);
    }
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider;workload_identity_impersonation_path=$impersonationPath";
        $dbh = new PDO($dsn, "", "");
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->query("SELECT CURRENT_USER()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if ($row[0] === $impersonationUsername) {
            echo "PASS: Test 5 - WIF impersonation successful, user: $impersonationUsername\n";
        } else {
            echo "FAIL: Test 5 - Expected impersonated user '$impersonationUsername' but got '" . $row[0] . "'\n";
            exit(1);
        }
        $dbh = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 5 - WIF impersonation failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "SKIP: Test 5 - Impersonation only supported on GCP and AWS\n";
}

// Test 6: should authenticate using wif without impersonation when path is empty
if (supportsImpersonation($provider)) {
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider;workload_identity_impersonation_path=";
        $dbh = new PDO($dsn, "", "");
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->query("SELECT CURRENT_USER()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if ($row[0] === $expectedUsername) {
            echo "PASS: Test 6 - Empty impersonation path falls back to direct WIF, user: $expectedUsername\n";
        } else {
            echo "FAIL: Test 6 - Expected user '$expectedUsername' but got '" . $row[0] . "'\n";
            exit(1);
        }
        $dbh = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 6 - Empty impersonation path test failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "SKIP: Test 6 - Impersonation only supported on GCP and AWS\n";
}

// Test 7: should fail authentication with invalid impersonation path
if (supportsImpersonation($provider)) {
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider;workload_identity_impersonation_path=invalid-service-account@nonexistent-project.iam.gserviceaccount.com";
        $dbh = new PDO($dsn, "", "");
        echo "FAIL: Test 7 - Should have thrown exception with invalid impersonation path\n";
        exit(1);
    } catch (PDOException $e) {
        if (strpos($e->getMessage(), "390100") !== false ||
            strpos($e->getMessage(), "Incorrect username or password") !== false ||
            strpos($e->getMessage(), "workload identity") !== false ||
            strpos($e->getMessage(), "impersonation") !== false ||
            strpos($e->getMessage(), "token") !== false) {
            echo "PASS: Test 7 - Invalid impersonation path correctly rejected\n";
        } else {
            echo "FAIL: Test 7 - Unexpected error message: " . $e->getMessage() . "\n";
            exit(1);
        }
    }
} else {
    echo "SKIP: Test 7 - Impersonation only supported on GCP and AWS\n";
}

// Test 8: should execute queries after impersonation connection
if (supportsImpersonation($provider)) {
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider;workload_identity_impersonation_path=$impersonationPath";
        $dbh = new PDO($dsn, "", "");
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->query("SELECT CURRENT_USER()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if ($row[0] !== $impersonationUsername) {
            echo "FAIL: Test 8 - Expected impersonated user '$impersonationUsername' but got '" . $row[0] . "'\n";
            exit(1);
        }

        $sth = $dbh->query("SELECT CURRENT_ROLE()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if (empty($row[0])) {
            echo "FAIL: Test 8 - CURRENT_ROLE() returned empty\n";
            exit(1);
        }

        $sth = $dbh->query("SELECT 'lorem ipsum'");
        $row = $sth->fetch(PDO::FETCH_NUM);
        if ($row[0] !== 'lorem ipsum') {
            echo "FAIL: Test 8 - Expected 'lorem ipsum' but got '" . $row[0] . "'\n";
            exit(1);
        }

        echo "PASS: Test 8 - Queries executed successfully after impersonation\n";
        $dbh = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 8 - Query execution after impersonation failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "SKIP: Test 8 - Impersonation only supported on GCP and AWS\n";
}

// Test 9: should handle multiple connections with same impersonation path
if (supportsImpersonation($provider)) {
    try {
        $dsn = "snowflake:host=$host;account=$account;authenticator=workload_identity;workload_identity_provider=$provider;workload_identity_impersonation_path=$impersonationPath";

        $dbh1 = new PDO($dsn, "", "");
        $dbh1->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $sth1 = $dbh1->query("SELECT CURRENT_USER()");
        $row1 = $sth1->fetch(PDO::FETCH_NUM);
        if ($row1[0] !== $impersonationUsername) {
            echo "FAIL: Test 9 - First connection: expected '$impersonationUsername' but got '" . $row1[0] . "'\n";
            exit(1);
        }

        $dbh2 = new PDO($dsn, "", "");
        $dbh2->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        $sth2 = $dbh2->query("SELECT CURRENT_USER()");
        $row2 = $sth2->fetch(PDO::FETCH_NUM);
        if ($row2[0] !== $impersonationUsername) {
            echo "FAIL: Test 9 - Second connection: expected '$impersonationUsername' but got '" . $row2[0] . "'\n";
            exit(1);
        }

        echo "PASS: Test 9 - Multiple connections with impersonation successful\n";
        $dbh1 = null;
        $dbh2 = null;
    } catch (PDOException $e) {
        echo "FAIL: Test 9 - Multiple connections failed: " . $e->getMessage() . "\n";
        exit(1);
    }
} else {
    echo "SKIP: Test 9 - Impersonation only supported on GCP and AWS\n";
}

echo "All tests completed\n";
?>
--EXPECTF--
Testing WIF with provider: %s
PASS: Test 1 - Correctly rejected without provider
PASS: Test 2 - Connected with WIF provider %s
%s: Test 3 - %s
%s: Test 4 - %s
%s: Test 5 - %s
%s: Test 6 - %s
%s: Test 7 - %s
%s: Test 8 - %s
%s: Test 9 - %s
All tests completed
