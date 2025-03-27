--TEST--
pdo_snowflake - memory usage
--INI--
pdo_snowflake.logdir=sflog
pdo_snowflake.loglevel=INFO
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
    include __DIR__ . "/common.php";
    $column_num = 50;
    $row_num = 10000;
    $loop_num = 5;
    $table_name = "memory_usage_test" . rand();
    $dbh = new PDO($dsn, $user, $password);
    $create_query = "create or replace table " . $table_name . "(c0 int";
    $insert_query = "insert into " . $table_name . " select seq4()";
    for ($i = 1; $i < $column_num; $i++)
    {
        $create_query = $create_query . ", c" . $i . " int";
        $insert_query = $insert_query . ", seq4()";
    }
    $create_query = $create_query . ")";
    $insert_query = $insert_query . "from table(generator(rowcount => " . $row_num . "))";
    $dbh->query($create_query);
    $dbh->query($insert_query);
    $mem_usage_org = 0;
    for ($i = 0; $i < $loop_num; $i++) {
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
            $query = "select * from " . $table_name;

            echo 'Memory usage before query execution: ' . memory_get_usage() . "\n";
            $stmt = $pdo->prepare($query);
            $stmt->execute();
            echo 'Memory usage before fetching: ' . memory_get_usage() . "\n";
            $records = $stmt->fetchAll(PDO::FETCH_ASSOC);
            echo 'Memory usage after fetching: ' . memory_get_usage() . "\n";
        } catch (PDOException $e) {
            echo "Unexpected failure: " . $e->getMessage() . "\n";
            break;
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
    $dbh->query("drop table " . $table_name);
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
