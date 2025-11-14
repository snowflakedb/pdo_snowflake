# IPv6 Connectivity Test - Quick Guide

Test IPv6 connectivity with Snowflake using private key authentication.

## Quick Start

### 1. Configure credentials
Edit `parameters.json`:
```json
{
    "testconnection": {
        "SNOWFLAKE_TEST_HOST": "your_host.snowflakecomputing.com",
        "SNOWFLAKE_TEST_USER": "your_username",
        "SNOWFLAKE_TEST_ACCOUNT": "your_account",
        "SNOWFLAKE_TEST_WAREHOUSE": "your_warehouse",
        "SNOWFLAKE_TEST_DATABASE": "your_database",
        "SNOWFLAKE_TEST_SCHEMA": "your_schema",
        "SNOWFLAKE_TEST_ROLE": "your_role",
        "SNOWFLAKE_TEST_PROTOCOL": "https",
        "SNOWFLAKE_TEST_PRIVATE_KEY_FILE": "my_own_rsa_key.p8",
        "SNOWFLAKE_TEST_PRIVATE_KEY_PASSWORD": ""
    }
}
```

### 2. Place your private key
```bash
# Ensure your private key is in the repo root
chmod 600 my_own_rsa_key.p8
```

### 3. Run the test
```bash
./run_test_simple.sh
```

## What the Test Does

1. ✅ DNS resolution (IPv4/IPv6)
2. ✅ Connect with private key authentication
3. ✅ SELECT 1
4. ✅ SELECT pi()
5. ✅ PUT file to stage (upload)
6. ✅ GET file from stage (download)
7. ✅ Cleanup

## Files

```
pdo_snowflake/
├── parameters.json              # Your credentials
├── my_own_rsa_key.p8           # Your private key
├── run_test_simple.sh          # Run this!
├── tests/
│   ├── ipv6_connectivity_auto.php
│   └── common_keypair.php
└── scripts/
    └── generate_testenv.sh
```

## Troubleshooting

### Connection fails
```bash
# Check logs
tail -f sflog/*.log

# Verify config
cat parameters.json

# Test private key exists
ls -la my_own_rsa_key.p8
```

### Build driver (first time)
```bash
export PHP_HOME=/opt/homebrew
./scripts/build_pdo_snowflake.sh
```

### Regenerate config
```bash
./scripts/generate_testenv.sh
```

## Expected Output

```
============================================================
Starting IPv6 Connectivity Test
============================================================
Authentication: Private Key (SNOWFLAKE_JWT)
Private Key File: my_own_rsa_key.p8
...
Connected to Snowflake
✓ Test 1 PASSED
✓ Test 2 PASSED
✓ Test 3 (PUT) PASSED
✓ Test 4 (GET) PASSED
IPv6 Connectivity Test Completed Successfully
===DONE===
```

## Manual Run

```bash
export PHP_HOME=/opt/homebrew
$PHP_HOME/bin/php \
  -dextension=modules/pdo_snowflake.so \
  -dpdo_snowflake.logdir=sflog \
  -dpdo_snowflake.loglevel=DEBUG \
  -dpdo_snowflake.cacert=libsnowflakeclient/cacert.pem \
  tests/ipv6_connectivity_auto.php
```

---

**Quick run:** `./run_test_simple.sh`

