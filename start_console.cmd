@ECHO OFF
TITLE DustDPI - Foreground Live Diagnostic Console
COLOR 0B

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

echo [*] Launching DustDPI in live foreground mode...
echo [*] Press Ctrl+C at any time to terminate the engine.
icacls "blacklist.txt" /grant *S-1-5-32-545:(M) >nul 2>&1
"%_arch%\dust_engine.exe" -5 -q --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist "blacklist.txt"
POPD
pause
