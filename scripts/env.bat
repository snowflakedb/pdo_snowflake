:: Set the environment variables for tests

echo off
set TEST_PHP_EXECUTABLE=%PHP_HOME%\php.exe
del testenv.ini
del parameters.bat

echo @echo off>parameters.bat
jq -r ".testconnection | to_entries | map(\"set \(.key)=\(.value)\") | .[]" parameters.json >> parameters.bat

call .\parameters.bat

if defined TRAVIS_JOB_ID (
    echo "==> Set the test schema to TRAVIS_JOB_%TRAVIS_JOB_ID%"
    set SNOWFLAKE_TEST_SCHEMA=TRAVIS_JOB_%TRAVIS_JOB_ID%
)

set | findstr SNOWFLAKE_TEST_ >> .\testenv.ini
echo "==> Test Connection Parameters"
type testenv.ini