@ECHO OFF
TITLE DustDPI - Start Service
net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this script and select "Run as administrator".
    pause
    exit /B 1
)
sc start "DustDPI"
timeout /t 2 >nul
