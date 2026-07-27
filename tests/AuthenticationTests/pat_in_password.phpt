--TEST--
pdo_snowflake - PAT authentication using token as password
--SKIPIF--
<?php
require_once __DIR__ . '/auth_helper.php';
skipIfGitHubActions();

$required = [
    'SNOWFLAKE_AUTH_TEST_HOST',
    'SNOWFLAKE_AUTH_TEST_ACCOUNT',
    'SNOWFLAKE_AUTH_TEST_OKTA_USER',
];
foreach ($required as $var) {
    if (!getenv($var)) {
        die("skip Missing env var: $var");
    }
}

// Test if PAT helper works
$testToken = getPatToken('PAT_TEST_SKIP');
deletePatToken('PAT_TEST_SKIP');
if (empty($testToken)) {
    die("skip PAT helper script not working");
}
?>
--INI--
extension=modules/pdo_snowflake.so
pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
--FILE--
<?php
require_once __DIR__ . '/auth_helper.php';

$config = [
    'host' => getenv('SNOWFLAKE_AUTH_TEST_HOST'),
    'account' => getenv('SNOWFLAKE_AUTH_TEST_ACCOUNT'),
];
$user = getenv('SNOWFLAKE_AUTH_TEST_OKTA_USER');

$tokenName = generatePatTokenName();
$token = getPatToken($tokenName);

if (empty($token)) {
    echo "PAT in password: FAILED - Could not get PAT token\n";
    deletePatToken($tokenName);
    exit(1);
}

$dsn = buildBaseDsn($config);

try {
    $pdo = new PDO($dsn, $user, $token);
    $stmt = $pdo->query("SELECT 1");
    $result = $stmt->fetch(PDO::FETCH_NUM);
    if ($result[0] == "1") {
        echo "PAT in password: SUCCESS\n";
    } else {
        echo "PAT in password: ERROR - unexpected result\n";
    }
} catch (PDOException $e) {
    echo "PAT in password: FAILED - " . $e->getMessage() . "\n";
} finally {
    deletePatToken($tokenName);
}
?>
===DONE===
<?php exit(0); ?>
--EXPECT--
PAT in password: SUCCESS
===DONE===
