@echo off
set SCNAME=MapD
set DRIVER="E:\repos\Map\bin\x64\Debug\MouHidInputHook.sys"
if not exist %DRIVER% set DRIVER="E:\repos\Map\bin\x64\Debug\MouHidInputHook.sys"

net session >nul 2>&1
if %ERRORLEVEL% EQU 0 (
	echo You are running this script as admin. Fine!
) else (
	echo ERROR: This script requires admin privileges!
	pause
	exit /b 1
)

echo ***************************
echo Service Name: %SCNAME%
echo Driver......: %DRIVER%
echo ***************************

sc create %SCNAME% binPath= %DRIVER% type= kernel
echo ***************************
sc start %SCNAME%
echo ***************************
sc query %SCNAME%

REM pause
timeout /t 3