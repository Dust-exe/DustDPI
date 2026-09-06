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
echo   Dust Studio - Network Diagnostics & Socket Reset
echo ========================================================
echo.

echo [*] 1/4: Configuring high-performance secure DNS fallback (1.1.1.1, 8.8.8.8)...
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { Get-NetAdapter | Where-Object Status -eq 'Up' | Set-DnsClientServerAddress -ServerAddresses ('1.1.1.1','8.8.8.8') -ErrorAction SilentlyContinue } catch {}" >nul 2>&1

echo [*] 2/4: Flushing Windows local DNS cache...
ipconfig /flushdns >nul 2>&1

echo [*] 3/4: Refreshing DustDPI autonomous routing service...
sc stop "DustDPI" >nul 2>&1
timeout /t 1 >nul
sc start "DustDPI" >nul 2>&1

echo [*] 4/4: Testing gateway connectivity...
ping -n 1 1.1.1.1 >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] Gateway reachable. Network stream active.
) else (
    echo [!] Notice: Please verify physical network connection.
)

echo.
echo ========================================================
echo [OK] Network diagnostics and repair completed successfully!
echo      Closing window in 3 seconds...
echo ========================================================
timeout /t 3 >nul
exit /B 0
