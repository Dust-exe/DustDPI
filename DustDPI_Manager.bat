@ECHO OFF
TITLE DustDPI Control Matrix
COLOR 0D

net session >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    COLOR 0C
    echo [!] HATA: DustDPI Yonetici olarak calistirilmalidir!
    echo     Lutfen bu dosyaya Sag Tiklayip "Yonetici Olarak Calistir" deyin.
    echo.
    pause
    exit /B 1
)

PUSHD "%~dp0"

:MENU
CLS
COLOR 0D
echo ====================================================================
echo          Dust Studio - DustDPI Selective Network Service
echo          Secici Ag Optimizasyonu  -  Sifir Yan Etki  -  Yuksek Hiz
echo ====================================================================
echo.
echo   [1] DustDPI Servisini Kur ve Baslat (Otomatik Baslangic)
echo   [2] Servisi Baslat
echo   [3] Servisi Durdur
echo   [4] Konsol / Canli Test Modunda Calistir (On Plan)
echo   [5] Servisi ve Suruculeri Tamamen Kaldir (Uninstall)
echo   [6] Hedef Listesini Duzenle (blacklist.txt)
echo   [7] Servis Calisma Durumunu Kontrol Et
echo.
echo   [0] Cikis
echo.
echo ====================================================================
set /p SECIM="Seciminiz [0-7]: "

if "%SECIM%"=="1" goto KUR
if "%SECIM%"=="2" goto BASLAT
if "%SECIM%"=="3" goto DURDUR
if "%SECIM%"=="4" goto KONSOL
if "%SECIM%"=="5" goto KALDIR
if "%SECIM%"=="6" goto DUZENLE
if "%SECIM%"=="7" goto DURUM
if "%SECIM%"=="0" goto CIKIS
goto MENU

:KUR
CLS
call "%~dp0service_install.cmd"
goto MENU

:BASLAT
CLS
call "%~dp0start_service.cmd"
goto MENU

:DURDUR
CLS
call "%~dp0stop_service.cmd"
goto MENU

:KONSOL
CLS
call "%~dp0start_console.cmd"
goto MENU

:KALDIR
CLS
call "%~dp0service_remove.cmd"
goto MENU

:DUZENLE
notepad "%~dp0blacklist.txt"
goto MENU

:DURUM
CLS
call "%~dp0status.cmd"
goto MENU

:CIKIS
POPD
exit /B 0
