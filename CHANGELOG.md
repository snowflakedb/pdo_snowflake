#### For the official PDO Snowflake Release Notes please refer to https://docs.snowflake.com/en/release-notes/clients-drivers/php-pdo

# Changelog
- v3.6.0
  - Added support for workload identity federation authentication.
  
- v3.5.0
  - Added support for `client_request_mfa_token` connection option to enable MFA token caching for username/password MFA authentication.
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
