@ECHO OFF
TITLE DustDPI - Service & Driver Removal

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this script and select "Run as administrator".
    pause
    exit /B 1
)

echo [*] Stopping DustDPI and network drivers...
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1
sc stop "WinDivert" >nul 2>&1
sc delete "WinDivert" >nul 2>&1
sc stop "WinDivert14" >nul 2>&1
sc delete "WinDivert14" >nul 2>&1

echo.
echo [OK] DustDPI service and kernel filters have been removed cleanly.
pause
