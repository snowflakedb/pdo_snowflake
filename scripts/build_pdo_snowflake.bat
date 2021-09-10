::
:: Build PDO Snowflake for Windows
::

@echo off

set platform=%1
set build_type=%2
set vs_version=%3
set php_full_version=%4
set php_sdk_dir=%5

set php_major_minor_version=%php_full_version:~0,3%

set origdir=%cd%
set scriptdir=%~dp0
call "%scriptdir%\_init.bat" %platform% %build_type% %vs_version%
if %ERRORLEVEL% NEQ 0 goto :error
set pdodir=%scriptdir%\..

set phpsrcdir="%php_sdk_dir%\phpmaster\%vc_version%\%arch%\php-src"
set depsdir="%php_sdk_dir%\phpmaster\%vc_version%\%arch%\deps"

:: Move libsnowflakeclient and dependency libraries to dependencies directory

:: Move Arrow
7z x "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\arrow\lib\arrow.zip" -o"%depsdir%\lib\" -y
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\arrow_deps\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error

:: Move Boost
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\boost\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error

:: Move AWS
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\aws\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%depsdir%\include\aws"
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\aws\include\aws" ^
    "%depsdir%\include\aws\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move Azure
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\azure\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
:: No delete since no directory for azure headers
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\azure\include\*" ^
    "%depsdir%\include\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move Curl
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\curl\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%depsdir%\include\curl"
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\curl\include\curl" ^
    "%depsdir%\include\curl\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move oob
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\oob\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
:: No delete since no directory for oob headers
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\oob\include\*" ^
    "%depsdir%\include\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move OpenSSL
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\openssl\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%depsdir%\include\openssl"
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\openssl\include\openssl" ^
    "%depsdir%\include\openssl\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move zlib
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\zlib\lib\*.lib" ^
    "%depsdir%\lib\" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
:: No delete since no directory for zlib headers
xcopy ^
    "%pdodir%\libsnowflakeclient\deps-build\%build_dir%\zlib\include\*" ^
    "%depsdir%\include\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move libsnowflakeclient
copy ^
    "%pdodir%\libsnowflakeclient\lib\%build_dir%\snowflakeclient.lib" ^
    "%depsdir%\lib\libsnowflakeclient_a.lib" ^
    /v /y
if %ERRORLEVEL% NEQ 0 goto :error
rmdir /S /Q "%depsdir%\include\libsnowflakeclient"
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%\libsnowflakeclient\include\snowflake" ^
    "%depsdir%\include\snowflake\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

:: Move PDO code to PHP sourc directory
rmdir /S /Q "%phpsrcdir%\ext\pdo_snowflake"
if %ERRORLEVEL% NEQ 0 goto :error
xcopy ^
    "%pdodir%" ^
    "%phpsrcdir%\ext\pdo_snowflake\" ^
    /v /y /e
if %ERRORLEVEL% NEQ 0 goto :error

cd %phpsrcdir%
buildconf --force && configure --disable-all --enable-cli --enable-pdo --with-pdo_snowflake=shared && nmake

:success
:: Go back to where you came from
cd "%origdir%"
exit /b 0

:error
:: Go back to where you came from
cd "%origdir%"
exit /b 1
