@ECHO OFF
TITLE DustDPI - Servisi Durdur
net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] HATA: Lutfen Sag Tiklayip "Yonetici Olarak Calistir" deyin!
    pause
    exit /B 1
)
sc stop "DustDPI"
echo.
sc query "DustDPI"
pause
