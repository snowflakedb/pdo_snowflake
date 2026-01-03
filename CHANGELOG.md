#### For the official PDO Snowflake Release Notes please refer to https://docs.snowflake.com/en/release-notes/clients-drivers/php-pdo

# Changelog
- v3.4.0
  - Added native okta authentication support (snowflakedb/pdo_snowflake#439).
  - Fixed aarch64 build (snowflakedb/pdo_snowflake#450).
  - Removed the ARM note from the readme (snowflakedb/pdo_snowflake#447).
  - Updated the `libsnowflakeclient` to v2.4.0 (snowflakedb/pdo_snowflake#453)
  - Removed PHP version prefix from the reported PDO version (snowflakedb/pdo_snowflake#454)
  - Added support for CRL (certificate revocation list) (snowflakedb/pdo_snowflake#457).
  - Added the changelog.yml GitHub workflow to ensure changelog is updated on release PRs (snowflakedb/pdo_snowflake#459).
  - Update the `libsnowflakeclient` to v2.6.0 (snowflakedb/pdo_snowflake#466)
  - Support Decfloat type.