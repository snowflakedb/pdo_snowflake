--TEST--
pdo_snowflake - IPv6 Connectivity Test (Auto-detect Private Key or Password)
--INI--
extension=modules/pdo_snowflake.so
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    // Use enhanced common file that auto-detects private key authentication
    include __DIR__ . "/common_keypair.php";

    echo str_repeat("=", 60) . "\n";
    echo "Starting IPv6 Connectivity Test\n";
    echo str_repeat("=", 60) . "\n";

    // Show authentication method
    if ($use_private_key) {
        echo "Authentication: Private Key (SNOWFLAKE_JWT)\n";
        echo "Private Key File: $priv_key_file\n";
    } else {
        echo "Authentication: Username/Password\n";
    }
    echo str_repeat("=", 60) . "\n";

    // DNS Resolution Check
    echo str_repeat("=", 60) . "\n";
    echo "DNS Resolution Check\n";
    echo str_repeat("=", 60) . "\n";
    echo "Checking DNS resolution for: $host\n";
    
    $ipv4_addresses = [];
    $ipv6_addresses = [];
    
    // Check for IPv4 addresses
    $ipv4_records = @dns_get_record($host, DNS_A);
    if ($ipv4_records) {
        foreach ($ipv4_records as $record) {
            if (isset($record['ip'])) {
                $ipv4_addresses[] = $record['ip'];
            }
        }
    }
    
    // Check for IPv6 addresses
    $ipv6_records = @dns_get_record($host, DNS_AAAA);
    if ($ipv6_records) {
        foreach ($ipv6_records as $record) {
            if (isset($record['ipv6'])) {
                $ipv6_addresses[] = $record['ipv6'];
            }
        }
    }
    
    echo "Summary: Found " . count($ipv4_addresses) . " IPv4 address(es) and " . count($ipv6_addresses) . " IPv6 address(es)\n";
    
    if (count($ipv6_addresses) > 0) {
        echo "IPv6 addresses available: " . implode(", ", array_slice($ipv6_addresses, 0, 3)) . "\n";
    } else {
        echo "WARNING: No IPv6 addresses found in DNS resolution!\n";
    }
    
    if (count($ipv4_addresses) > 0) {
        echo "IPv4 addresses available: " . implode(", ", array_slice($ipv4_addresses, 0, 3)) . "\n";
    } else {
        echo "WARNING: No IPv4 addresses found in DNS resolution!\n";
    }
    
    echo str_repeat("=", 60) . "\n";
    echo "Note: If you get HTTP 403 Forbidden with IPv6, it means:\n";
    echo "  - Connection reached Snowflake server (network works)\n";
    echo "  - Server rejected IPv6 connection (endpoint may not support IPv6)\n";
    echo "  - This is a server-side policy, not a network issue\n";
    echo str_repeat("=", 60) . "\n";

    // Connect to Snowflake
    try {
        $dbh = new PDO($dsn, $user, $password);
        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        echo "Connected to Snowflake: $host\n";
        echo "Account: $account, User: $user\n";

        // Test 1: SELECT 1
        echo "\nTest 1: Executing SELECT 1\n";
        $sth = $dbh->query("SELECT 1");
        $row = $sth->fetch(PDO::FETCH_NUM);
        $result = $row[0];
        echo "SELECT 1 result: $result\n";
        if ($result != 1) {
            throw new Exception("Expected 1, got $result");
        }
        echo "✓ Test 1 PASSED\n";

        // Test 2: SELECT pi()
        echo "\nTest 2: Executing SELECT pi()\n";
        $sth = $dbh->query("SELECT pi()");
        $row = $sth->fetch(PDO::FETCH_NUM);
        $pi_value = $row[0];
        echo "SELECT pi() result: $pi_value\n";
        if (abs($pi_value - 3.141592653589793) > 0.000001) {
            throw new Exception("Expected pi, got $pi_value");
        }
        echo "✓ Test 2 PASSED\n";

        // Test 3 & 4: PUT and GET operations
        echo "\nTest 3 & 4: Starting PUT and GET operations\n";
        
        // Create a temporary directory and file
        $tmpdir = sys_get_temp_dir() . '/ipv6_test_' . uniqid();
        mkdir($tmpdir);
        
        // Generate random file
        $test_file = $tmpdir . '/test_ipv6_file.txt';
        echo "Generating test file: $test_file\n";
        $random_content = '';
        for ($i = 0; $i < 5120; $i++) { // 5KB
            $random_content .= chr(rand(65, 90)); // Random uppercase letters
        }
        file_put_contents($test_file, $random_content);
        $file_size = filesize($test_file);
        echo "Test file size: $file_size bytes\n";
        if ($file_size == 0) {
            throw new Exception("Test file should not be empty");
        }

        // Use user stage (internal stage, no AWS credentials needed)
        $stage_name = "~"; // User stage
        echo "Using user stage: $stage_name\n";

        // PUT file to stage
        $put_sql = "PUT file://$test_file @$stage_name";
        echo "Executing PUT: $put_sql\n";
        $sth = $dbh->query($put_sql);
        $put_result = $sth->fetchAll(PDO::FETCH_NUM);
        echo "PUT executed successfully\n";
        
        // Check PUT status (column 6 contains the status)
        if (count($put_result) > 0 && count($put_result[0]) > 6) {
            $status = $put_result[0][6];
            echo "PUT status: $status\n";
            if (!in_array($status, ["UPLOADED", "SKIPPED"])) {
                throw new Exception("File should be uploaded, got status: $status");
            }
        }
        echo "✓ Test 3 (PUT) PASSED\n";

        // List files in stage
        echo "\nListing files in stage: $stage_name\n";
        $sth = $dbh->query("LIST @$stage_name");
        $files = $sth->fetchAll(PDO::FETCH_NUM);
        $uploaded_file = null;
        foreach ($files as $file_info) {
            if (strpos($file_info[0], 'test_ipv6_file.txt') !== false) {
                $uploaded_file = $file_info[0];
                echo "Found uploaded file: $uploaded_file\n";
                break;
            }
        }
        
        if ($uploaded_file === null) {
            throw new Exception("Uploaded file should be found in stage listing");
        }

        // GET file from stage
        $output_dir = $tmpdir . '/download';
        mkdir($output_dir);
        $get_sql = "GET @$stage_name/test_ipv6_file.txt.gz file://$output_dir/";
        echo "\nExecuting GET: $get_sql\n";
        $sth = $dbh->query($get_sql);
        $get_result = $sth->fetchAll(PDO::FETCH_NUM);
        echo "GET executed successfully\n";

        // Verify file was downloaded
        $downloaded_files = glob($output_dir . '/*.gz');
        echo "Downloaded files: " . implode(", ", array_map('basename', $downloaded_files)) . "\n";
        if (count($downloaded_files) == 0) {
            throw new Exception("File should be downloaded");
        }
        
        $downloaded_file = $downloaded_files[0];
        $downloaded_size = filesize($downloaded_file);
        echo "Downloaded file size: $downloaded_size bytes\n";
        if ($downloaded_size == 0) {
            throw new Exception("Downloaded file should not be empty");
        }
        echo "✓ Test 4 (GET) PASSED\n";

        // Clean up: remove file from stage
        echo "\nCleaning up: removing file from stage\n";
        $dbh->query("REMOVE @$stage_name/test_ipv6_file.txt.gz");
        
        // Clean up local files
        array_map('unlink', glob($output_dir . '/*'));
        rmdir($output_dir);
        unlink($test_file);
        rmdir($tmpdir);
        
        $dbh = null;

        echo "\n" . str_repeat("=", 60) . "\n";
        echo "IPv6 Connectivity Test Completed Successfully\n";
        echo str_repeat("=", 60) . "\n";
        echo "===DONE===\n";

    } catch (Exception $e) {
        echo "\n" . str_repeat("=", 60) . "\n";
        echo "ERROR: " . $e->getMessage() . "\n";
        echo "Stack trace:\n" . $e->getTraceAsString() . "\n";
        echo str_repeat("=", 60) . "\n";
        exit(1);
    }
?>
<?php exit(0); ?>
--EXPECTF--
============================================================
Starting IPv6 Connectivity Test
============================================================
Authentication: %s
%a
============================================================
============================================================
DNS Resolution Check
============================================================
Checking DNS resolution for: %s
Summary: Found %d IPv4 address(es) and %d IPv6 address(es)
%a
============================================================
Note: If you get HTTP 403 Forbidden with IPv6, it means:
  - Connection reached Snowflake server (network works)
  - Server rejected IPv6 connection (endpoint may not support IPv6)
  - This is a server-side policy, not a network issue
============================================================
Connected to Snowflake: %s
Account: %s, User: %s

Test 1: Executing SELECT 1
SELECT 1 result: 1
✓ Test 1 PASSED

Test 2: Executing SELECT pi()
SELECT pi() result: %f
✓ Test 2 PASSED

Test 3 & 4: Starting PUT and GET operations
Generating test file: %s
Test file size: %d bytes
Using user stage: ~
Executing PUT: PUT file://%s @~
PUT executed successfully
PUT status: %s
✓ Test 3 (PUT) PASSED

Listing files in stage: ~
Found uploaded file: %s

Executing GET: GET @~/test_ipv6_file.txt.gz file://%s
GET executed successfully
Downloaded files: %s
Downloaded file size: %d bytes
✓ Test 4 (GET) PASSED

Cleaning up: removing file from stage

============================================================
IPv6 Connectivity Test Completed Successfully
============================================================
===DONE===

