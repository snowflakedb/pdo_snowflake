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


def build_windows():
    print("====> Read build config from env")
    arch = os.environ.get('ARCH', 'x64')
    target = os.environ.get('TARGET', 'Release')
    vs = os.environ.get('VS', 'undef')
    php = os.environ.get('PHP', 'undef')
    if vs == 'undef':
        print("==Please set VS in env variable==")
        exit(-1)
    if php == 'undef':
        print("==Please set PHP in env variable==")
        exit(-1)
    print("arch = " + arch)
    print("target = " + target)
    print("vs = " + vs)
    print("php = " + php + " (make sure the source version you use here is same as the on in setup-php action)")

    ropo = os.environ.get('GITHUB_WORKSPACE')
    print("====> building snowflake driver")
    print("====> working directory: " + ropo)
    os.chdir(ropo)
    print("====> remove unnecessary files")
    run_command("rmdir .\\libsnowflakeclient\\lib\\linux /s/q")
    run_command("rmdir .\\libsnowflakeclient\\lib\\darwin /s/q")
    run_command("rmdir .\\libsnowflakeclient\\deps-build\\linux /s/q")
    run_command("rmdir .\\libsnowflakeclient\\deps-build\\darwin /s/q")
    if vs == 'VS16':
      run_command("rmdir .\\libsnowflakeclient\\deps-build\\win64\\vs17 /s/q")
    else:
      run_command("rmdir .\\libsnowflakeclient\\deps-build\\win64\\vs16 /s/q")

    print("====> setup php sdk and php source")
    run_command("scripts\\setup_php_sdk.bat " + arch + " " + target + " " + vs + " D:\\php-sdk")
    run_command("scripts\\run_setup_php.bat " + arch + " " + target + " " + vs + " " + php + " D:\\php-sdk")

    print ("====> build pdo driver")
    run_command("scripts\\run_build_pdo_snowflake.bat " + arch + " " + target + " " + vs + " " + php + " D:\\php-sdk")
    dll = os.path.join("D:\\php-sdk\\phpmaster", vs, arch, "php-src", arch, target + "_TS", "php_pdo_snowflake.dll")
    php_ext_dir = os.path.join("c:", "tools", "php", "ext")
    run_command("xcopy " + dll + " " + php_ext_dir +" /I/Y/F")
    run_command("php -dextension=pdo_snowflake -m")


def build_posix():
    ropo = os.environ.get('GITHUB_WORKSPACE')
    #scripts_dir = os.path.join(ropo, 'scripts')
    print("====> building snowflake driver" )
    print("====> working directory: " + ropo)
    print("====> prepare repository")
    os.chdir(ropo)
    print("====> remove unnecessary files")
    run_command("rm -rf ./libsnowflakeclient/deps-build/win64")
    print ("====> build pdo driver")
    run_command("./scripts/build_pdo_snowflake.sh")
    run_command("php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake")


def main():
    current_os = os.name
    if current_os == 'nt':
        build_windows()
    else:
        build_posix()



if __name__ == "__main__":
    main()
