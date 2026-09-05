@ECHO OFF
TITLE DustDPI - Servis Kurulumu

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

echo [*] Eski servis kayitlari temizleniyor...
sc stop "GoodbyeDPI" >nul 2>&1
sc delete "GoodbyeDPI" >nul 2>&1
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1

echo [*] DustDPI Secici Servisi olusturuluyor...
sc create "GoodbyeDPI" binPath= "\"%CD%\%_arch%\goodbyedpi.exe\" -5 --set-ttl 5 --blacklist \"%CD%\blacklist.txt\"" start= auto DisplayName= "DustDPI - Selective Service"
sc description "GoodbyeDPI" "Dust Studio Secici DPI Atlatma Servisi - Discord, Roblox ve hedef platformlar"

echo [*] Servis baslatiliyor...
sc start "GoodbyeDPI"

echo.
echo [OK] Servis basariyla kuruldu ve baslatildi!
pause
POPD
