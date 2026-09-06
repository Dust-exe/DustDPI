@ECHO OFF
TITLE DustDPI - Network Diagnostic and Optimization Repair
COLOR 0B

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this script and select "Run as administrator".
    pause
    exit /B 1
)

echo ========================================================
echo   Dust Studio - Network Diagnostics & Traffic Repair
echo ========================================================
echo.

echo [*] 1/5: Terminating hung application instances...
taskkill /F /IM Discord.exe >nul 2>&1
taskkill /F /IM Update.exe >nul 2>&1
taskkill /F /IM RobloxPlayerBeta.exe >nul 2>&1

echo [*] 2/5: Configuring high-performance secure DNS fallback (1.1.1.1, 8.8.8.8)...
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { Get-NetAdapter | Where-Object Status -eq 'Up' | Set-DnsClientServerAddress -ServerAddresses ('1.1.1.1','8.8.8.8') -ErrorAction SilentlyContinue } catch {}" >nul 2>&1

echo [*] 3/5: Flushing Windows local DNS cache...
ipconfig /flushdns >nul 2>&1

echo [*] 4/5: Refreshing DustDPI autonomous routing service...
sc stop "DustDPI" >nul 2>&1
timeout /t 1 >nul
sc start "DustDPI" >nul 2>&1

echo [*] 5/5: Launching direct application routes...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$found = $false; $apps = Get-ChildItem -Path \"$env:LOCALAPPDATA\Discord\app-*\Discord.exe\" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending; if ($apps) { $target = $apps[0].FullName; Start-Process $target; $found = $true; Write-Host '[OK] Discord launched directly:' $target } if (-not $found) { Start-Process \"$env:LOCALAPPDATA\Discord\Update.exe\" -ArgumentList '--processStart Discord.exe' -ErrorAction SilentlyContinue }"

echo.
echo ========================================================
echo [OK] Network diagnostics and repair completed successfully!
echo      Closing window in 3 seconds...
echo ========================================================
timeout /t 3 >nul
exit /B 0
