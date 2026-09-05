@ECHO OFF
TITLE DustDPI — Servis Kaldirma
:: ╔══════════════════════════════════════════════════════════╗
:: ║        🗑️ DustDPI — Servis Kaldirma Araci             ║
:: ╚══════════════════════════════════════════════════════════╝

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo.
    echo [!] HATA: Bu dosyayi SAG TIKLAYIP "Yonetici Olarak Calistir" secenegiyle baslatmalisiniz!
    echo.
    pause
    exit /B 1
)

echo.
echo ========================================================
echo   DustDPI Servisi ve Suruculeri Kaldiriliyor...
echo ========================================================
echo.

echo [*] DustDPI servisi durduruluyor ve siliniyor...
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1

echo [*] Eski GoodbyeDPI servisleri kontrol ediliyor...
sc stop "GoodbyeDPI" >nul 2>&1
sc delete "GoodbyeDPI" >nul 2>&1

echo [*] WinDivert surucu kayitlari temizleniyor...
sc stop "WinDivert" >nul 2>&1
sc delete "WinDivert" >nul 2>&1
sc stop "WinDivert14" >nul 2>&1
sc delete "WinDivert14" >nul 2>&1

echo.
echo ========================================================
echo   [OK] DustDPI servisi ve suruculeri basariyla kaldirildi!
echo ========================================================
echo.
pause
