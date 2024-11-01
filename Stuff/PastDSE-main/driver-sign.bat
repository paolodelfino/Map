@echo off

set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"
set DRIVER_DEBUG="E:\repos\Map\bin\x64\Debug\MouHidInputHook.sys"
set DRIVER_RELEASE="E:\repos\Map\bin\x64\Debug\MouHidInputHook.sys"
set CA="%~dp0\certs\VeriSign Class 3 Public Primary Certification Authority - G5.cer"
set CERT="%~dp0\certs\cert_0.pfx"

net session >nul 2>&1
if %ERRORLEVEL% EQU 0 (
	echo You are running this script as admin. Fine!
) else (
	echo ERROR: This script requires admin privileges!
	pause
	exit /b 1
)

date 01-01-14

echo ***************************
echo workdir.: %cd%
echo signtool: %SIGNTOOL%
echo driver..: %DRIVER%
echo ca......: %CA%
echo cert....: %CERT%
echo ---------------------------
%SIGNTOOL% sign /fd sha1 /a /ac %CA% /f %CERT% %DRIVER_DEBUG%
%SIGNTOOL% sign /fd sha1 /a /ac %CA% /f %CERT% %DRIVER_RELEASE%
echo ***************************
%SIGNTOOL% verify /kp /v %DRIVER_DEBUG%
%SIGNTOOL% verify /kp /v %DRIVER_RELEASE%
echo ***************************

net stop w32time
net start w32time
w32tm /resync /nowait

pause