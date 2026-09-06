@ECHO OFF
TITLE DustDPI - Konsol Test Modu

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] HATA: Bu dosyayi Sag Tiklayip "Yonetici Olarak Calistir" demelisiniz!
    pause
    exit /B 1
)

PUSHD "%~dp0"
set _arch=x86
IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" (set _arch=x86_64)
IF DEFINED PROCESSOR_ARCHITEW6432 (set _arch=x86_64)

echo [*] DustDPI konsol modunda calistiriliyor...
"%CD%\%_arch%\dust_engine.exe" -5 --set-ttl 5 --blacklist "%CD%\blacklist.txt"

POPD
pause
