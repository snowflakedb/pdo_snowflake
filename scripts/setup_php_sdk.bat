::
:: Create and setup PHP SDK
::

@echo off

set platform=%1
set build_type=%2
set vs_version=%3
set php_sdk_dir=%4

set origdir=%cd%
set scriptdir=%~dp0
call "%scriptdir%\_init.bat" %platform% %build_type% %vs_version%
if %ERRORLEVEL% NEQ 0 goto :error

set php_sdk_version=php-sdk-2.2.0
if "%vc_version%"=="vs18" set php_sdk_version=php-sdk-2.9.0

if not exist "%php_sdk_dir%" (
    git clone https://github.com/php/php-sdk-binary-tools.git "%php_sdk_dir%" || goto :error
)
cd /d "%php_sdk_dir%" || goto :error
git checkout %php_sdk_version% || goto :error

:success
:: Go back to where you came from
cd "%origdir%"
exit /b 0

:error
:: Go back to where you came from
cd "%origdir%"
exit /b 1
