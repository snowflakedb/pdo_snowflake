::
:: Initialize variables
::

echo === setting up global environment variables

set platform=%1
set build_type=%2
set vs_version=%3

set curdir=%cd%

if defined arch (
    if not "%platform%"=="" (
        if not "%arch%"=="%platform%" (
	        echo The specified platform parameter doesn't match the existing arch. arch: %arch%, platform: %platform%
		    echo If you want to switch, restart Widnows terminal and run the command again.
	        goto :error
        )
    )
)

set arch=
set arcdir=
if "%platform%"=="x64" (
    set arch=x64
    set arcdir=win64
)
if "%platform%"=="x86" (
    set arch=x86
    set arcdir=win32
)
if "%arch%"=="" (
    echo Specify the architecture. [x86, x64]
	goto :error
)

set target_name=
if "%build_type%"=="Debug" (
    set target_name=debug
)
if "%build_type%"=="Release" (
    set target_name=release
)

if "%target_name%"=="" (
    echo Specify the build type. [Debug, Release]
	goto :error
)

set cmake_generator=
set vsdir=
set vc_version=
if "%vs_version%"=="VS16" (
    set cmake_generator=Visual Studio 16 2019
    set vsdir=vs16
    set vc_version=vc16
)
if "%vs_version%"=="VS15" (
    set cmake_generator=Visual Studio 15 2017
    set vsdir=vs15
    set vc_version=vc15
)
if "%vs_version%"=="VS14" (
    set cmake_generator=Visual Studio 14 2015
    set vsdir=vs14
    set vc_version=vc14
)
if "%cmake_generator%"=="" (
    echo Specify the visual studio version used. [VS16, VS15, VS14]
    goto :error
)

set build_dir=%arcdir%\%vsdir%

echo "Building with platform: %platform%, build type: %build_type%, visual studio version: %vs_version%, cmake generator: %cmake_generator%"

cd "%curdir%"
exit /b 0

:error
cd "%curdir%"
exit /b 1