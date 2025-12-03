# This script is to build the snowflake driver in github action
import os
import sys
import subprocess


def run_command(cmd):
    print(cmd)
    result = subprocess.Popen(cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    for line in result.stdout:
        print(line.strip().decode('utf-8'))
    for line in result.stderr:
        print(line.strip().decode('utf-8'))
    result.poll()
    if result.returncode != 0 and result.returncode is not None:
        exit(-1)


def run_command_no_exit(cmd):
    print(cmd)
    result = subprocess.Popen(cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    for line in result.stdout:
        print(line.strip().decode('utf-8'))
    for line in result.stderr:
        print(line.strip().decode('utf-8'))


def test_windows():
    print("====> Read build config from env")
    arch = os.environ.get('ARCH', 'x64')
    vs = os.environ.get('VS', 'undef')
    if vs == 'undef':
        print("==Please set VS in env variable==")
        exit(-1)
    print("arch = " + arch)
    print("vs = " + vs)
    ropo = os.environ.get('GITHUB_WORKSPACE')
    print("====> testing snowflake driver")
    print("====> working directory: " + ropo)
    os.chdir(ropo)
    print("====> remove unnecessary test for Windows")
    run_command("del .\\tests\\selectltz.phpt /q/f ")

    print("====> setup parameters and env")
    run_command("xcopy .\\.github\\workflows\\scripts\\parameters.json .\\ /I/Y/F")
    run_command("python .\\.github\\workflows\\scripts\\set_secrets.py .\\parameters.json")
    run_command(".\\scripts\\env.bat")


    print ("====> run test")
    print ("====> exclude WIF test (runs separately in wif_ssh_tests.yml)")
    run_command("del .\\tests\\wif_auth.phpt /q/f ")
    run_tests_file = os.path.join("D:\\php-sdk\\phpmaster", vs, arch, "php-src", "run-tests.php")
    run_command_no_exit("php.exe " + run_tests_file + " .\\tests -d extension=pdo_snowflake || ver>null")
    print ("====> parse test results")
    run_command("python .\\.github\\workflows\\scripts\\check_result.py .\\tests")



def test_posix():
    print("====> Read Test config from env")
    use_valgrind = os.environ.get('USE_VALGRIND', 'false')
    ropo = os.environ.get('GITHUB_WORKSPACE')
    print("====> testing snowflake driver")
    print("====> working directory: " + ropo)
    os.chdir(ropo)

    print("====> setup parameters and env")
    run_command("cp ./.github/workflows/scripts/parameters.json ./")
    run_command("python ./.github/workflows/scripts/set_secrets.py ./parameters.json")
    run_command("./scripts/env.sh && env | grep SNOWFLAKE_TEST > testenv.ini")

    print ("====> run test")
    print ("====> exclude WIF test (runs separately in wif_ssh_tests.yml)")
    run_command("rm -f ./tests/wif_auth.phpt")
    run_test_options = ""
    if use_valgrind == 'true' or use_valgrind == '1':
        run_test_options = run_test_options + ' -m'
    run_command_no_exit("php -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1' ./run-tests.php -d extension=modules/pdo_snowflake.so" + run_test_options)
    print ("====> parse test results")
    run_command("python ./.github/workflows/scripts/check_result.py ./tests")

    coverage = os.environ.get('REPORT_COVERAGE', 'undef')
    if coverage == 'true' or coverage == '1':
        print ("====> run code coverage")
        run_command("lcov -c -d .libs --output-file main_coverage.info")
        run_command("genhtml main_coverage.info --output-directory php_coverage_report")

def main():
    current_os = os.name
    if current_os == 'nt':
        test_windows()
    else:
        test_posix()


if __name__ == "__main__":
    main()
