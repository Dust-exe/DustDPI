@ECHO OFF
TITLE DustDPI — Windows Servis Kurulumu
:: ╔══════════════════════════════════════════════════════════╗
:: ║        🚀 DustDPI — Windows Servisi Kurulum Aracı        ║
:: ╚══════════════════════════════════════════════════════════╝

:: Yonetici haklari kontrolu
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
echo   DustDPI — Secici DPI Baypass Servisi Kuruluyor...
echo   Mimari: %_arch%
echo ========================================================
echo.

echo [*] 1/3 Eski servis kayitlari temizleniyor...
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1
sc stop "GoodbyeDPI" >nul 2>&1
sc delete "GoodbyeDPI" >nul 2>&1

echo [*] 2/3 Yeni DustDPI servisi olusturuluyor (Sadece blacklist.txt hedeflenir)...
sc create "DustDPI" binPath= "\"%CD%\%_arch%\goodbyedpi.exe\" -5 --set-ttl 5 --blacklist \"%CD%\blacklist.txt\"" start= auto
sc description "DustDPI" "Dust Studio Secici DPI Atlatma Servisi — Sadece Discord, Roblox ve hedef platformlari filtreler; Antigravity, IDE ve genel trafige dokunmaz."

echo [*] 3/3 DustDPI servisi baslatiliyor...
sc start "DustDPI"

echo.
echo ========================================================
echo   [OK] DustDPI servisi basariyla kuruldu ve baslatildi!
echo.
echo   - Windows her acildiginda otomatik olarak arkada calisacak.
echo   - Sadece blacklist.txt icindeki siteler filtrelenecek.
echo   - Antigravity IDE, Steam, bankacilik vb. ASLA etkilenmeyecek!
echo ========================================================
echo.
pause
POPD
