@ECHO OFF
TITLE DustDPI - Quick Start & Network Repair
COLOR 0B

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this file and select "Run as administrator".
    pause
    exit /B 1
)

echo ========================================================
echo   Dust Studio - Application Route & Network Repair
echo ========================================================
echo.

echo [*] 1/4: Resetting hung socket states...
taskkill /F /IM Discord.exe >nul 2>&1
taskkill /F /IM Update.exe >nul 2>&1

echo [*] 2/4: Configuring reliable DNS servers (Cloudflare / Google)...
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { Get-NetAdapter | Where-Object Status -eq 'Up' | Set-DnsClientServerAddress -ServerAddresses ('1.1.1.1','8.8.8.8') -ErrorAction SilentlyContinue } catch {}" >nul 2>&1

echo [*] 3/4: Flushing Windows DNS resolver cache...
ipconfig /flushdns >nul 2>&1

echo [*] 4/4: Starting DustDPI service...
sc start "DustDPI" >nul 2>&1

echo [*] Launching optimized Discord instance...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$found = $false; $apps = Get-ChildItem -Path \"$env:LOCALAPPDATA\Discord\app-*\Discord.exe\" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending; if ($apps) { $target = $apps[0].FullName; Start-Process $target; $found = $true; Write-Host '[OK] Target launched directly:' $target } if (-not $found) { Start-Process \"$env:LOCALAPPDATA\Discord\Update.exe\" -ArgumentList '--processStart Discord.exe' -ErrorAction SilentlyContinue }"

echo.
echo ========================================================
echo [OK] Optimization finished. Enjoy unrestricted browsing!
echo ========================================================
timeout /t 3 >nul
exit /B 0
