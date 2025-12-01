#### For the official PDO Snowflake Release Notes please refer to https://docs.snowflake.com/en/release-notes/clients-drivers/php-pdo

# Changelog
- v3.4.0
  - Added native okta authentication support (#439).
  - Fixed aarch64 build (#450).
  - Removed the ARM note from the readme (#447).
  - Updated the `libsnowflakeclient` to v2.4.0 (#453)
  - Removed PHP version prefix from the reported PDO version (#454)
  - Added support for CRL (certificate revocation list) (#457).
  - Added the changelog.yml GitHub workflow to ensure changelog is updated on release PRs.
  - 