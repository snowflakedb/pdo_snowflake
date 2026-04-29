import os
import sys
import fileinput


def read_env_variable(var):
    value = os.environ.get(var)
    if value:
        print(var + ' is set')
    else:
        print('[ERROR] required env var ' + var + ' is missing')
        sys.exit(1)
    return value


def set_secret_in_file(file, variable, value):
    f = fileinput.FileInput(file, inplace=True, backup=variable + '.bak')
    try:
        for line in f:
            sys.stdout.write(line.replace(variable, value))
    finally:
        f.close()


def write_private_key_file(pem_content):
    """Write the PEM private key (passed via SNOWFLAKE_TEST_PRIVATE_KEY env var)
    to a file inside the repo root and return its absolute path. The file is
    gitignored (rsa_key.p8) and consumed by the PHP tests via
    SNOWFLAKE_TEST_PRIVATE_KEY_FILE in parameters.json / testenv.ini.
    """
    repo_root = os.environ.get('GITHUB_WORKSPACE') or os.getcwd()
    key_path = os.path.join(repo_root, 'rsa_key.p8')
    with open(key_path, 'w', newline='\n') as f:
        f.write(pem_content)
        if not pem_content.endswith('\n'):
            f.write('\n')
    try:
        os.chmod(key_path, 0o600)
    except OSError:
        # chmod on Windows may not honor 0o600; safe to ignore in CI
        pass
    print('Wrote private key to ' + key_path)
    return key_path


def main(argv):
    file = argv[0]
    SNOWFLAKE_TEST_USER             = read_env_variable("SNOWFLAKE_TEST_USER")
    SNOWFLAKE_TEST_PRIVATE_KEY      = read_env_variable("SNOWFLAKE_TEST_PRIVATE_KEY")
    SNOWFLAKE_TEST_ACCOUNT          = read_env_variable("SNOWFLAKE_TEST_ACCOUNT")
    SNOWFLAKE_TEST_WAREHOUSE        = read_env_variable("SNOWFLAKE_TEST_WAREHOUSE")
    SNOWFLAKE_TEST_DATABASE         = read_env_variable("SNOWFLAKE_TEST_DATABASE")
    SNOWFLAKE_TEST_SCHEMA           = read_env_variable("SNOWFLAKE_TEST_SCHEMA")
    SNOWFLAKE_TEST_ROLE             = read_env_variable("SNOWFLAKE_TEST_ROLE")

    SNOWFLAKE_TEST_PRIVATE_KEY_FILE = write_private_key_file(SNOWFLAKE_TEST_PRIVATE_KEY)

    set_secret_in_file(file, "[SNOWFLAKE_TEST_USER]",             SNOWFLAKE_TEST_USER)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_PRIVATE_KEY_FILE]", SNOWFLAKE_TEST_PRIVATE_KEY_FILE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ACCOUNT]",          SNOWFLAKE_TEST_ACCOUNT)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_WAREHOUSE]",        SNOWFLAKE_TEST_WAREHOUSE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_DATABASE]",         SNOWFLAKE_TEST_DATABASE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_SCHEMA]",           SNOWFLAKE_TEST_SCHEMA)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ROLE]",             SNOWFLAKE_TEST_ROLE)


if __name__ == "__main__":
    main(sys.argv[1:])
