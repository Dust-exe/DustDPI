@ECHO OFF
TITLE DustDPI - Servis Kaldirma

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] HATA: Bu dosyayi Sag Tiklayip "Yonetici Olarak Calistir" demelisiniz!
    pause
    exit /B 1
)

echo [*] Servis durduruluyor ve siliniyor...
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1

echo [*] WinDivert surucu kayitlari temizleniyor...
sc stop "WinDivert" >nul 2>&1
sc delete "WinDivert" >nul 2>&1
sc stop "WinDivert14" >nul 2>&1
sc delete "WinDivert14" >nul 2>&1

echo.
echo [OK] DustDPI servisi ve suruculeri basariyla kaldirildi!
pause
