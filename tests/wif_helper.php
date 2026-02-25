<?php
/** @noinspection PhpUnused - all functions are called from .phpt test files */

function getWifConfig(): array
{
    return [
        'provider' => getenv('SNOWFLAKE_TEST_WIF_PROVIDER') ?: '',
        'host' => getenv('SNOWFLAKE_TEST_WIF_HOST') ?: '',
        'account' => getenv('SNOWFLAKE_TEST_WIF_ACCOUNT') ?: '',
        'username' => getenv('SNOWFLAKE_TEST_WIF_USERNAME') ?: '',
        'impersonation_path' => getenv('SNOWFLAKE_TEST_WIF_IMPERSONATION_PATH') ?: '',
        'impersonation_username' => getenv('SNOWFLAKE_TEST_WIF_USERNAME_IMPERSONATION') ?: '',
    ];
}

function buildWifDsn($config, $options = []): string
{
    $dsn = "snowflake:host={$config['host']};account={$config['account']};authenticator=workload_identity";

    if (!empty($options['provider'])) {
        $dsn .= ";workload_identity_provider={$options['provider']}";
    }
    if (array_key_exists('impersonation_path', $options)) {
        $dsn .= ";workload_identity_impersonation_path={$options['impersonation_path']}";
    }
    if (!empty($options['token'])) {
        $dsn .= ";token={$options['token']}";
    }

    return $dsn;
}

function wifConnect($dsn): PDO
{
    $dbh = new PDO($dsn, "", "");
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    return $dbh;
}

function wifConnectAndVerifyUser($dsn, $expectedUser): PDO
{
    $dbh = wifConnect($dsn);
    $sth = $dbh->query("SELECT CURRENT_USER()");
    $row = $sth->fetch(PDO::FETCH_NUM);
    if ($row[0] !== $expectedUser) {
        throw new RuntimeException("Expected user '$expectedUser' but got '$row[0]'");
    }
    return $dbh;
}

function requireEnv($config, $keys): void
{
    foreach ($keys as $key) {
        if (empty($config[$key])) {
            $envName = 'SNOWFLAKE_TEST_WIF_' . strtoupper($key);
            echo "FAIL: $envName must be set\n";
            exit(1);
        }
    }
}
