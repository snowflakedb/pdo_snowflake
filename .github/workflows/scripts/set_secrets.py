import os
import re
import sys
import fileinput


def read_env_variable(var):
    if os.environ.get(var, 'undef') != 'undef':
        print (var + ' is set')
    else:
        print (var + ' not set')
    return os.environ.get(var, 'undef')


def set_secret_in_file(file, variable, value):
    f = fileinput.FileInput(file, inplace=True, backup=variable + '.bak')
    try:
        for line in f:
            sys.stdout.write(line.replace(variable, value))
    finally:
        f.close()


def main(argv):
    file = argv[0]
    SNOWFLAKE_TEST_USER = read_env_variable("SNOWFLAKE_TEST_USER")
    SNOWFLAKE_TEST_PASSWORD = read_env_variable("SNOWFLAKE_TEST_PASSWORD")
    SNOWFLAKE_TEST_ACCOUNT = read_env_variable("SNOWFLAKE_TEST_ACCOUNT")
    SNOWFLAKE_TEST_WAREHOUSE = read_env_variable("SNOWFLAKE_TEST_WAREHOUSE")
    SNOWFLAKE_TEST_DATABASE = read_env_variable("SNOWFLAKE_TEST_DATABASE")
    SNOWFLAKE_TEST_SCHEMA = read_env_variable("SNOWFLAKE_TEST_SCHEMA")
    SNOWFLAKE_TEST_ROLE = read_env_variable("SNOWFLAKE_TEST_ROLE")

    set_secret_in_file(file, "[SNOWFLAKE_TEST_USER]", SNOWFLAKE_TEST_USER)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_PASSWORD]", SNOWFLAKE_TEST_PASSWORD)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ACCOUNT]", SNOWFLAKE_TEST_ACCOUNT)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_WAREHOUSE]", SNOWFLAKE_TEST_WAREHOUSE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_DATABASE]", SNOWFLAKE_TEST_DATABASE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_SCHEMA]", SNOWFLAKE_TEST_SCHEMA)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ROLE]", SNOWFLAKE_TEST_ROLE)


if __name__ == "__main__":
    main(sys.argv[1:])
