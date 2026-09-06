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
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1

echo [*] DustDPI Secici Servisi olusturuluyor...
sc create "DustDPI" binPath= "\"%CD%\%_arch%\dust_engine.exe\" -9 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \"%CD%\blacklist.txt\"" start= auto DisplayName= "DustDPI Service"
sc description "DustDPI" "Dust Studio Secici Ag Optimizasyon ve Paket Yonetim Servisi"

echo [*] DustDPI servisi baslatiliyor...
sc start "DustDPI"

echo.
echo [OK] DustDPI servisi basariyla kuruldu ve baslatildi!
pause
POPD
