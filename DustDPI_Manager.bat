@ECHO OFF
TITLE DustDPI Control Matrix
COLOR 0B

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    COLOR 0C
    echo [!] ERROR: DustDPI requires Administrator privileges.
    echo     Please right-click this file and select "Run as administrator".
    echo.
    pause
    exit /B 1
)

PUSHD "%~dp0"

:MENU
CLS
COLOR 0B
echo ====================================================================
echo           Dust Studio - DustDPI Traffic Optimization Engine
echo           Selective Network Freedom  -  Zero Lag  -  High Speed
echo ====================================================================
echo.
echo   [1] Install and Start DustDPI Service (Auto-Start with Windows)
echo   [2] Start DustDPI Service
echo   [3] Stop DustDPI Service
echo   [4] Run in Live Foreground Console Mode (Testing)
echo   [5] Remove Service and Drivers Completely (Uninstall)
echo   [6] Open Target Filter List in Notepad (blacklist.txt)
echo   [7] Check Service Operational Status
echo   [8] Run Network Diagnostic & Quick Repair
echo.
echo   [0] Exit
echo.
echo ====================================================================
set /p CHOICE="Enter Selection [0-8]: "

if "%CHOICE%"=="1" goto INSTALL
if "%CHOICE%"=="2" goto START
if "%CHOICE%"=="3" goto STOP
if "%CHOICE%"=="4" goto CONSOLE
if "%CHOICE%"=="5" goto REMOVE
if "%CHOICE%"=="6" goto EDIT
if "%CHOICE%"=="7" goto STATUS
if "%CHOICE%"=="8" goto DIAG
if "%CHOICE%"=="0" goto EXIT
goto MENU

:INSTALL
CLS
call "%~dp0service_install.cmd"
goto MENU

:START
CLS
call "%~dp0start_service.cmd"
goto MENU

:STOP
CLS
call "%~dp0stop_service.cmd"
goto MENU

:CONSOLE
CLS
call "%~dp0start_console.cmd"
goto MENU

:REMOVE
CLS
call "%~dp0service_remove.cmd"
goto MENU

:EDIT
icacls "%~dp0blacklist.txt" /grant *S-1-5-32-545:(M) >nul 2>&1
notepad "%~dp0blacklist.txt"
sc query "DustDPI" | find "RUNNING" >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    net stop "DustDPI" >nul 2>&1
    net start "DustDPI" >nul 2>&1
)
goto MENU

:STATUS
CLS
call "%~dp0status.cmd"
goto MENU

:DIAG
CLS
call "%~dp0network_diagnostics_and_repair.cmd"
goto MENU

:EXIT
POPD
exit /B 0
