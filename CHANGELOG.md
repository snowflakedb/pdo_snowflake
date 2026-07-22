#### For the official PDO Snowflake Release Notes please refer to https://docs.snowflake.com/en/release-notes/clients-drivers/php-pdo

# Changelog

- v4.1.0
  - Fixed a use-after-free in the named-parameter store on parameter rebind/teardown. (SNOW-3649747)
  - `PDO::quote()` now raises IM001 (driver does not support quoting).
  - Updated the `libsnowflakeclient` to v2.9.2 (snowflakedb/pdo_snowflake#521)
  - Added connection parameters: wif_host and wif_audience (snowflakedb/pdo_snowflake#523)

- v4.0.0
  - Added support for PHP 8.5 and drop support for PHP 8.1 (snowflakedb/pdo_snowflake#501)
  - Fixed misleading error message when building the driver on Windows. (snowflakedb/pdo_snowflake#507)
  - Updated the `libsnowflakeclient` to v2.9.1 (snowflakedb/pdo_snowflake#511)
  - Added support for the toml file connection. (snowflakedb/pdo_snowflake#510)

- v3.7.0
  - Updated the `libsnowflakeclient` to v2.8.0 (snowflakedb/pdo_snowflake#503)
  - Updated the `openssl` to v3.0.19 (snowflakedb/pdo_snowflake#503)
  - Updated the `libcurl` to v8.19.0 (snowflakedb/pdo_snowflake#503)
  - Add `crl_download_max_size` parameter that provides the control of the maximum size of the CRL (snowflakedb/pdo_snowflake#495)
  - Added the "log_query_text" and "log_query_parameters" and masking sql logging and parameter loggings. (snowflakedb/pdo_snowflake#494)

- v3.6.0
  - Updated the `libsnowflakeclient` to v2.7.1 (snowflakedb/pdo_snowflake#489)
  - Updated the `openssl` to v3.0.19 (snowflakedb/pdo_snowflake#489)
  - Added support for workload identity federation authentication (snowflakedb/pdo_snowflake#463)
  - Added support for WIF impersonation via `workload_identity_impersonation_path` connection parameter (snowflakedb/pdo_snowflake#456)
  - Added support for multistatement queries (snowflakedb/pdo_snowflake#484)
  
- v3.5.0
  - Added support for `client_request_mfa_token` connection option to enable MFA token caching for username/password MFA authentication. (snowflakedb/pdo_snowflake#479)
  - Added support for `multiple statement query`. (snowflakedb/pdo_snowflake#484)
  - Deprecate CentOS 7 builds. Rocky 8/RHEL8 is now the minimum system version. (snowflakedb/pdo_snowflake#482)
  - Updated `libsnowflakeclient` to v2.7.0 (snowflakedb/pdo_snowflake#482)
  - Updated `openssl` to v3.0.18 (snowflakedb/pdo_snowflake#482)
  - Updated `libcurl` to v8.16.0 (snowflakedb/pdo_snowflake#482)
  - Add an auto-detected application path to the CLIENT_ENVIRONMENT (snowflakedb/pdo_snowflake#479)

- v3.4.0
  - Added native okta authentication support (snowflakedb/pdo_snowflake#439).
  - Fixed aarch64 build (snowflakedb/pdo_snowflake#450).
  - Removed the ARM note from the readme (snowflakedb/pdo_snowflake#447).
  - Updated the `libsnowflakeclient` to v2.6.0 (snowflakedb/pdo_snowflake#472)
  - Removed PHP version prefix from the reported PDO version (snowflakedb/pdo_snowflake#454)
  - Added support for CRL (certificate revocation list) (snowflakedb/pdo_snowflake#457).
  - Added the changelog.yml GitHub workflow to ensure changelog is updated on release PRs (snowflakedb/pdo_snowflake#459).
  - Support Decfloat type (snowflakedb/pdo_snowflake#466).
  - Added OAuth 2.0 authentication support both authorization code and client credentials (snowflakedb/pdo_snowflake#471).
