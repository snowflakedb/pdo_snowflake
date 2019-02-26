::
:: Setup PHP for Building
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

cd %php_sdk_dir%
call phpsdk_buildtree phpmaster
if not exist "%php_sdk_dir%\phpmaster\%vc_version%\%arch%\php-src" git clone https://github.com/php/php-src.git
cd php-src
git checkout tags/php-%php_full_version%
call phpsdk_deps --update --branch %php_major_minor_version%
buildconf && configure --disable-all --enable-cli && nmake

:success
:: Go back to where you came from
cd "%origdir%"
exit /b 0

:error
:: Go back to where you came from
cd "%origdir%"
exit /b 1
