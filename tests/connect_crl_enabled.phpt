--TEST--
pdo_snowflake - connect with CRL enabled and OCSP disabled
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $crlCacheDir = sys_get_temp_dir() . '/crl_cache_test_' . uniqid() . '_' . mt_rand();
    
    if (!mkdir($crlCacheDir, 0700, true)) {
        echo "Failed to create CRL cache directory\n";
        exit(1);
    }
    echo "CRL cache directory created: " . basename($crlCacheDir) . "\n";
    
    putenv("SF_CRL_RESPONSE_CACHE_DIR=" . $crlCacheDir);
    
    $filesBefore = glob($crlCacheDir . '/*');
    echo "Files in cache before connection: " . count($filesBefore) . "\n";
    
    $dbh = new PDO("{$dsn};disableocspchecks=true;crl_check=true;crl_advisory=false;crl_disk_caching=true", $user, $password);
    
    // Execute a simple query to verify the connection works
    $sth = $dbh->query("SELECT 1");
    $row = $sth->fetch(PDO::FETCH_NUM);
    echo "Query result: " . $row[0] . "\n";
    
    $dbh = null;
    
    $filesAfter = glob($crlCacheDir . '/*');
    $cacheFilesCreated = count($filesAfter);
    echo "Files in cache after connection: " . $cacheFilesCreated . "\n";
    
    if ($cacheFilesCreated > 0) {
        echo "CRL cache populated successfully\n";
    } else {
        echo "Warning: CRL cache not populated (may be expected for .local hosts)\n";
    }
    
    foreach ($filesAfter as $file) {
        if (is_file($file)) {
            unlink($file);
        }
    }
    
    if (rmdir($crlCacheDir)) {
        echo "CRL cache directory cleaned up\n";
    } else {
        echo "Warning: Failed to remove CRL cache directory\n";
    }
    
    putenv("SF_CRL_RESPONSE_CACHE_DIR");
    
    echo "OK\n";
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
CRL cache directory created: crl_cache_test_%s
Files in cache before connection: 0
Query result: 1
Files in cache after connection: %d
%s
CRL cache directory cleaned up
OK
===DONE===

