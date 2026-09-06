@ECHO OFF
TITLE DustDPI - Stop Service
net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this script and select "Run as administrator".
    pause
    exit /B 1
)
sc stop "DustDPI"
timeout /t 2 >nul
