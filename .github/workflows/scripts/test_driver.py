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
    run_tests_file = os.path.join("D:\\php-sdk\\phpmaster", vs.replace("VS", "vc"), arch, "php-src", "run-tests.php")
    run_command("php.exe " + run_tests_file + " .\\tests -d extension=pdo_snowflake || ver>null")
    print ("====> parse test results")
    run_command("python .\\.github\\workflows\\scripts\\check_result.py .\\tests")



def test_posix():
    print("====> Read Test config from env")
    use_valgrind = os.environ.get('USE_VALGRIND', 'false')
    use_fpm = os.environ.get('USE_FPM', 'false')
    ropo = os.environ.get('GITHUB_WORKSPACE')
    print("====> testing snowflake driver")
    print("====> working directory: " + ropo)
    os.chdir(ropo)

    print("====> setup parameters and env")
    run_command("cp ./.github/workflows/scripts/parameters.json ./")
    run_command("python ./.github/workflows/scripts/set_secrets.py ./parameters.json")
    run_command("./scripts/env.sh && env | grep SNOWFLAKE_TEST > testenv.ini")

    print ("====> run test")
    run_test_options = ""
    if use_valgrind == 'true' or use_valgrind == '1':
        run_test_options = run_test_options + ' -m'
    if use_fpm == 'true' or use_fpm == '1':
        run_command("php-fpm -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1' ./run-tests.php -d extension=modules/pdo_snowflake.so" + run_test_options)
    else :
        run_command("php -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1' ./run-tests.php -d extension=modules/pdo_snowflake.so" + run_test_options)
    print ("====> parse test results")
    run_command("python ./.github/workflows/scripts/check_result.py ./tests")


def main():
    current_os = os.name
    if current_os == 'nt':
        test_windows()
    else:
        test_posix()


if __name__ == "__main__":
    main()
