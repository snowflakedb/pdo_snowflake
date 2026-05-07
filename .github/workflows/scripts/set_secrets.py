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


def normalize_to_pem(content):
    """Accept either a full PEM (with -----BEGIN/-----END markers) or just the
    base64 body, and return a properly framed PKCS#8 PEM string.

    Snowflake JWT keypair auth requires PKCS#8 PEM. GitHub Secrets sometimes
    hold just the base64 body (e.g. when copy-pasted from `openssl rsa -outform DER | base64`
    or when the developer trimmed the BEGIN/END lines). Auto-wrap that case so
    libsnowflakeclient can parse the file.
    """
    content = content.replace('\r\n', '\n').replace('\r', '\n').strip()
    if content.startswith('-----BEGIN'):
        return content + '\n'
    body = ''.join(content.split())
    wrapped = '\n'.join(body[i:i + 64] for i in range(0, len(body), 64))
    return '-----BEGIN PRIVATE KEY-----\n' + wrapped + '\n-----END PRIVATE KEY-----\n'


def write_private_key_file(pem_content):
    """Write the PEM private key (passed via SNOWFLAKE_TEST_PRIVATE_KEY env var)
    to a file inside the repo root and return its absolute path. The file is
    gitignored (rsa_key.p8) and consumed by the PHP tests via
    SNOWFLAKE_TEST_PRIVATE_KEY_FILE in parameters.json / testenv.ini.

    Normalizes line endings, auto-wraps a bare base64 body in a PKCS#8 PEM
    header/footer, and emits diagnostic output so a malformed key fails loudly
    instead of silently producing an authenticator init error at PDO connect.
    """
    repo_root = os.environ.get('GITHUB_WORKSPACE') or os.getcwd()
    key_path = os.path.join(repo_root, 'rsa_key.p8')
    pem_content = normalize_to_pem(pem_content)
    with open(key_path, 'w', newline='\n') as f:
        f.write(pem_content)
    try:
        os.chmod(key_path, 0o600)
    except OSError:
        # chmod on Windows may not honor 0o600; safe to ignore in CI
        pass

    # Diagnostic output - only echo the PEM header / footer lines themselves
    # because those are public boilerplate (e.g. "-----BEGIN PRIVATE KEY-----"
    # is identical for every PEM in the world). The base64 body in between
    # is the actual secret and is NEVER printed - we only show its line count
    # and the on-disk file size.
    lines = pem_content.split('\n')
    first_line = lines[0] if lines else ''
    last_nonblank = next((line for line in reversed(lines) if line), '')
    starts_ok = first_line.startswith('-----BEGIN') and first_line.endswith('-----')
    ends_ok = last_nonblank.startswith('-----END') and last_nonblank.endswith('-----')
    print('Wrote private key to ' + key_path)
    print('  size:        ' + str(os.path.getsize(key_path)) + ' bytes')
    print('  total lines: ' + str(sum(1 for line in lines if line)))
    print('  header:      ' + (first_line if starts_ok else '<redacted - not a PEM header>'))
    print('  footer:      ' + (last_nonblank if ends_ok else '<redacted - not a PEM footer>'))
    if not starts_ok:
        print('[ERROR] private key does not start with a "-----BEGIN ...-----" header.')
        print('[ERROR] check that the SNOWFLAKE_TEST_PRIVATE_KEY GitHub secret holds')
        print('[ERROR] the full PEM contents (header + base64 body + footer).')
        sys.exit(1)
    if not ends_ok:
        print('[ERROR] private key does not end with a "-----END ...-----" footer.')
        sys.exit(1)
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
    set_secret_in_file(file, "[SNOWFLAKE_TEST_PRIVATE_KEY_FILE]", SNOWFLAKE_TEST_PRIVATE_KEY_FILE.replace("\\", "\\\\"))
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ACCOUNT]",          SNOWFLAKE_TEST_ACCOUNT)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_WAREHOUSE]",        SNOWFLAKE_TEST_WAREHOUSE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_DATABASE]",         SNOWFLAKE_TEST_DATABASE)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_SCHEMA]",           SNOWFLAKE_TEST_SCHEMA)
    set_secret_in_file(file, "[SNOWFLAKE_TEST_ROLE]",             SNOWFLAKE_TEST_ROLE)


if __name__ == "__main__":
    main(sys.argv[1:])
