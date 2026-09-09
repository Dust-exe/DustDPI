@ECHO OFF
TITLE DustDPI - Service Installation (Full Global Mode)

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Please right-click this script and select "Run as administrator".
    pause
    exit /B 1
)

PUSHD "%~dp0"
set _arch=x86
IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" (set _arch=x86_64)
IF DEFINED PROCESSOR_ARCHITEW6432 (set _arch=x86_64)

echo [*] Removing any legacy service registration...
sc stop "DustDPI" >nul 2>&1
sc delete "DustDPI" >nul 2>&1

echo [*] Registering DustDPI Full Optimization Service...
sc create "DustDPI" binPath= "\"%CD%\%_arch%\dust_engine.exe\" -5 -q --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253" start= auto DisplayName= "DustDPI Service"
sc description "DustDPI" "Dust Studio High-Performance Internet Freedom & Full Traffic Optimization Service"

echo [*] Starting DustDPI service...
net start "DustDPI"

echo.
echo [OK] DustDPI Full Mode service has been installed and started successfully!
pause
POPD
