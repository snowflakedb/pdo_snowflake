::
:: Run PHP Setup Task
::

set platform=%1
set build_type=%2
set vs_version=%3
set php_full_version=%4
set php_sdk_dir=%5

set task_args="%platform% %build_type% %vs_version% %php_full_version% %php_sdk_dir%"

set origdir=%cd%
set scriptdir=%~dp0
call "%scriptdir%\_init.bat" %platform% %build_type% %vs_version%
if %ERRORLEVEL% NEQ 0 goto :error

dir "%programfiles(x86)%\Microsoft Visual Studio"
%php_sdk_dir%\bin\vswhere
call %php_sdk_dir%\phpsdk-starter.bat -c %vc_version% -a %arch% -t "%scriptdir%\setup_php.bat" --task-args %task_args%
if %ERRORLEVEL% NEQ 0 goto :error

:success
:: Go back to where you came from
cd "%origdir%"
exit /b 0

:error
:: Go back to where you came from
cd "%origdir%"
exit /b 1
