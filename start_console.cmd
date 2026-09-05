@ECHO OFF
TITLE DustDPI — Konsol Test Modu
:: ╔══════════════════════════════════════════════════════════╗
:: ║        💻 DustDPI — Konsol / Canlı Test Modu             ║
:: ╚══════════════════════════════════════════════════════════╝

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo.
    echo [!] HATA: Bu dosyayi SAG TIKLAYIP "Yonetici Olarak Calistir" secenegiyle baslatmalisiniz!
    echo.
    pause
    exit /B 1
)

PUSHD "%~dp0"
set _arch=x86
IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" (set _arch=x86_64)
IF DEFINED PROCESSOR_ARCHITEW6432 (set _arch=x86_64)

echo.
echo ========================================================
echo   DustDPI — Konsol Test Modu Baslatiliyor...
echo   Mimari: %_arch%
echo   Hedef:  %CD%\blacklist.txt
echo ========================================================
echo   Kapatmak icin bu pencereyi kapatin veya CTRL+C yapin.
echo ========================================================
echo.

"%CD%\%_arch%\goodbyedpi.exe" -5 --set-ttl 5 --blacklist "%CD%\blacklist.txt"

POPD
pause
