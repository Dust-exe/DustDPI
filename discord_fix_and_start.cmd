@ECHO OFF
TITLE DustDPI - Discord Onarici ve Hizli Baslatici

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] HATA: Lutfen bu dosyayi Sag Tiklayip "Yonetici Olarak Calistir" deyin!
    pause
    exit /B 1
)

echo ========================================================
echo   Dust Studio - Discord Baglanti ve Baslatma Onarici
echo ========================================================
echo.

echo [*] 1/5: Takili kalan Discord islemleri sonlandiriliyor...
taskkill /F /IM Discord.exe >nul 2>&1
taskkill /F /IM Update.exe >nul 2>&1

echo [*] 2/5: Guvenli DNS (Cloudflare / Google) yapilandiriliyor...
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { Get-NetAdapter | Where-Object Status -eq 'Up' | Set-DnsClientServerAddress -ServerAddresses ('1.1.1.1','8.8.8.8') -ErrorAction SilentlyContinue } catch {}" >nul 2>&1

echo [*] 3/5: Windows DNS onbellegi sifirlaniyor...
ipconfig /flushdns >nul 2>&1

echo [*] 4/5: DustDPI servisi baslatiliyor...
sc start "DustDPI" >nul 2>&1

echo [*] 5/5: Discord guncelleyici dongusu asilarak dogrudan aciliyor...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$found = $false; $apps = Get-ChildItem -Path \"$env:LOCALAPPDATA\Discord\app-*\Discord.exe\" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending; if ($apps) { $target = $apps[0].FullName; Start-Process $target; $found = $true; Write-Host '[OK] Discord dogrudan calistirildi:' $target } if (-not $found) { Start-Process \"$env:LOCALAPPDATA\Discord\Update.exe\" -ArgumentList '--processStart Discord.exe'; Write-Host '[OK] Discord baslatildi.' }"

echo.
echo ========================================================
echo [OK] Islem tamamlandi! Discord acildi.
echo      Pencere 3 saniye icinde kapanacaktir.
echo ========================================================
timeout /t 3 >nul
exit /B 0
