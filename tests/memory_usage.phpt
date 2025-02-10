--TEST--
pdo_snowflake - memory usage
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=DEBUG
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";

    $dbh = new PDO($dsn, $user, $password);
    $mem_usage_org = 0;
    for ($i = 0; $i < 5; $i++) {
        #collect original memory usage with second one since there is memory usage for one time initialize
        # in the first one.
        if ($i == 1)
        {
            $mem_usage_org = memory_get_usage();
        }
        try {
            echo 'Memory usage before connection: ' . memory_get_usage() . "\n";
            $pdo = new PDO($dsn, $user, $password);
            $pdo->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
            $query = 'SELECT * FROM SNOWFLAKE_SAMPLE_DATA.TPCH_SF1000.ORDERS LIMIT 10000';

            echo 'Memory usage before query execution: ' . memory_get_usage() . "\n";
            $stmt = $pdo->prepare($query);
            $stmt->execute();
            echo 'Memory usage before fetching: ' . memory_get_usage() . "\n";
            $records = $stmt->fetchAll(PDO::FETCH_ASSOC);
            echo 'Memory usage after fetching: ' . memory_get_usage() . "\n";
        } catch (PDOException $e) {
            echo "Unexpected failure: " . $e->getMessage() . "\n";
        } finally {
            if ($records) {
                $records = null;
                echo 'Memory usage after fetching data cleaning: ' . memory_get_usage() . "\n";
            }
            if ($stmt) {
                $stmt = null;
                echo 'Memory usage after stmt cleaning: ' . memory_get_usage() . "\n";
            }
            if ($pdo) {
                $pdo = null;
                echo 'Memory usage after pdo cleaning: ' . memory_get_usage() . "\n";
            }
        }
        if (($mem_usage_org > 0) && (memory_get_usage() > $mem_usage_org))
        {
            echo 'Possible memory leak: original: ' . $mem_usage_org. 'current: ' . memory_get_usage() . "\n";
        }
    }
    $dbh = null;
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
Memory usage before connection: %s
Memory usage before query execution: %s
Memory usage before fetching: %s
Memory usage after fetching: %s
Memory usage after fetching data cleaning: %s
Memory usage after stmt cleaning: %s
Memory usage after pdo cleaning: %s
Memory usage before connection: %s
Memory usage before query execution: %s
Memory usage before fetching: %s
Memory usage after fetching: %s
Memory usage after fetching data cleaning: %s
Memory usage after stmt cleaning: %s
Memory usage after pdo cleaning: %s
Memory usage before connection: %s
Memory usage before query execution: %s
Memory usage before fetching: %s
Memory usage after fetching: %s
Memory usage after fetching data cleaning: %s
Memory usage after stmt cleaning: %s
Memory usage after pdo cleaning: %s
Memory usage before connection: %s
Memory usage before query execution: %s
Memory usage before fetching: %s
Memory usage after fetching: %s
Memory usage after fetching data cleaning: %s
Memory usage after stmt cleaning: %s
Memory usage after pdo cleaning: %s
Memory usage before connection: %s
Memory usage before query execution: %s
Memory usage before fetching: %s
Memory usage after fetching: %s
Memory usage after fetching data cleaning: %s
Memory usage after stmt cleaning: %s
Memory usage after pdo cleaning: %s
===DONE===

