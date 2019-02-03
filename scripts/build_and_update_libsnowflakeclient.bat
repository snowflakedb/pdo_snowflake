::
:: Build libsnowflakeclient and move the new libraries to PDO Snowflake
::

@echo off

set platform=%1
set build_type=%2
set vs_version=%3
set libsf_path=%4

set scriptdir=%~dp0
call "%scriptdir%\_init.bat" %platform% %build_type% %vs_version%
if %ERRORLEVEL% NEQ 0 goto :error
set origdir=%cd%

:: Lets go to libsnowflakeclient and build there first
cd "%libsf_path%"
set curdir=%cd%

:: Build libsnowflakeclient with dynamic runtime and without tests
echo "Building libsnowflakeclient with platform=%platform%, build type=%build_type%, visual studio version=%vs_version%, and located at: %libsf_path%"
call "%curdir%\scripts\build_libsnowflakeclient.bat" %platform% %build_type% %vs_version% ON OFF
:: Check that everything went okay
if %ERRORLEVEL% NEQ 0 goto :error

:: Remove the old files/directories
rmdir /S /Q "%origdir%\libsnowflakeclient\deps-build\%arcdir%"
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%origdir%\libsnowflakeclient\lib\%arcdir%"
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%origdir%\libsnowflakeclient\include\snowflake"
if %ERRORLEVEL% NEQ 0 goto :error

:: Make sure proper directories are created
mkdir "%origdir%\libsnowflakeclient\lib\%arcdir%"
if %ERRORLEVEL% NEQ 0 goto :error
mkdir "%origdir%\libsnowflakeclient\deps-build\%arcdir%"
if %ERRORLEVEL% NEQ 0 goto :error

:: Move the necessary files to PDO Snowflake
:: Move build libsnowflakeclient
xcopy ^
    "%curdir%\cmake-build-%arcdir%\%build_type%\*.lib" ^
    "%origdir%\libsnowflakeclient\lib\%arcdir%\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
:: Move headers
xcopy ^
    "%curdir%\include\snowflake" ^
    "%origdir%\libsnowflakeclient\include\snowflake\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error
:: Move dependency libraries
xcopy ^
    "%curdir%\deps-build\%arcdir%\*" ^
    "%origdir%\libsnowflakeclient\deps-build\%arcdir%\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:success
:: Go back to where you came from
cd "%origdir%"
exit /b 0

:error
:: Go back to where you came from
cd "%origdir%"
exit /b 1
