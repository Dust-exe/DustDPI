@ECHO OFF
TITLE DustDPI — Servisi Baslat
net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] HATA: Lutfen Sag Tiklayip "Yonetici Olarak Calistir" deyin!
    pause
    exit /B 1
)
sc start "DustDPI"
echo.
sc query "DustDPI"
pause
