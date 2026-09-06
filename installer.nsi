!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

Name "DustDPI"
OutFile "DustDPI_Setup.exe"
InstallDir "$PROGRAMFILES64\DustDPI"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "DustDPI"
VIAddVersionKey "CompanyName" "Dust Studio"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2026 Dust Studio. All rights reserved."
VIAddVersionKey "FileDescription" "DustDPI High-Performance Internet Freedom & Traffic Optimization Setup"
VIAddVersionKey "FileVersion" "1.0.0"
VIAddVersionKey "OriginalFilename" "DustDPI_Setup.exe"

!define MUI_ABORTWARNING
!define PRODUCT_NAME "DustDPI"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Dust Studio"
!define PRODUCT_WEB_SITE "https://dust-studio.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\DustDPI.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define MUI_ICON "app.ico"
!define MUI_UNICON "app.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Finish Page options
!define MUI_FINISHPAGE_RUN "$INSTDIR\DustDPI.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch DustDPI Dashboard"
!insertmacro MUI_PAGE_FINISH

; Uninstaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Turkish"

Section "MainSection" SEC01
  ; Terminate running processes and clean locks
  DetailPrint "Terminating active services and processes..."
  nsExec::ExecToLog 'taskkill.exe /F /IM "DustDPI.exe" /T'
  nsExec::ExecToLog 'taskkill.exe /F /IM "dust_engine.exe" /T'
  nsExec::ExecToLog 'taskkill.exe /F /IM "goodbyedpi.exe" /T'
  nsExec::ExecToLog 'sc.exe stop "DustDPI"'
  nsExec::ExecToLog 'sc.exe delete "DustDPI"'
  nsExec::ExecToLog 'sc.exe stop "GoodbyeDPI"'
  nsExec::ExecToLog 'sc.exe delete "GoodbyeDPI"'
  nsExec::ExecToLog 'sc.exe stop "WinDivert"'
  nsExec::ExecToLog 'sc.exe stop "WinDivert14"'
  Sleep 1500

  ; Clean legacy binaries if any
  Delete "$INSTDIR\x86_64\goodbyedpi.exe"
  Delete "$INSTDIR\x86\goodbyedpi.exe"

  SetOutPath "$INSTDIR"
  SetOverwrite on

  ; Program files and GUI
  File "DustDPI.exe"
  File "DustDPI_Manager.bat"
  File "blacklist.txt"
  File "network_diagnostics_and_repair.cmd"
  File "discord_fix_and_start.cmd"
  File "service_install.cmd"
  File "service_install_full_mode.cmd"
  File "service_remove.cmd"
  File "start_console.cmd"
  File "start_service.cmd"
  File "stop_service.cmd"
  File "status.cmd"
  File "README.md"
  File "LICENSE"
  File "DISCLAIMER.md"
  File "THIRD_PARTY_LICENSES.md"
  File "app.ico"

  ; Binaries
  SetOutPath "$INSTDIR\x86_64"
  File "x86_64\dust_engine.exe"
  File "x86_64\WinDivert.dll"
  File "x86_64\WinDivert64.sys"
  File "x86_64\LICENSE_WINDIVERT.txt"

  SetOutPath "$INSTDIR\x86"
  File "x86\dust_engine.exe"
  File "x86\WinDivert.dll"
  File "x86\WinDivert32.sys"
  File "x86\WinDivert64.sys"
  File "x86\LICENSE_WINDIVERT.txt"

  SetOutPath "$INSTDIR"

  ; Register DustDPI service
  ${If} ${RunningX64}
    nsExec::ExecToLog 'sc.exe create "DustDPI" binPath= "\"$INSTDIR\x86_64\dust_engine.exe\" -5 --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \"$INSTDIR\blacklist.txt\"" start= auto DisplayName= "DustDPI Service"'
  ${Else}
    nsExec::ExecToLog 'sc.exe create "DustDPI" binPath= "\"$INSTDIR\x86\dust_engine.exe\" -5 --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \"$INSTDIR\blacklist.txt\"" start= auto DisplayName= "DustDPI Service"'
  ${EndIf}

  nsExec::ExecToLog 'sc.exe description "DustDPI" "Dust Studio High-Performance Internet Freedom & Selective Traffic Optimization Service"'
  nsExec::ExecToLog 'sc.exe start "DustDPI"'

  ; Start Menu Shortcuts
  CreateDirectory "$SMPROGRAMS\DustDPI"
  CreateShortcut "$SMPROGRAMS\DustDPI\DustDPI.lnk" "$INSTDIR\DustDPI.exe" "" "$INSTDIR\app.ico" 0
  CreateShortcut "$SMPROGRAMS\DustDPI\Target Domain Filter.lnk" "notepad.exe" "$INSTDIR\blacklist.txt"
  CreateShortcut "$SMPROGRAMS\DustDPI\Network Diagnostics.lnk" "$INSTDIR\network_diagnostics_and_repair.cmd" "" "$INSTDIR\app.ico" 0
  CreateShortcut "$SMPROGRAMS\DustDPI\Start Service.lnk" "$INSTDIR\start_service.cmd" "" "$INSTDIR\app.ico" 0
  CreateShortcut "$SMPROGRAMS\DustDPI\Stop Service.lnk" "$INSTDIR\stop_service.cmd" "" "$INSTDIR\app.ico" 0
  CreateShortcut "$SMPROGRAMS\DustDPI\Uninstall DustDPI.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0

  ; Desktop Shortcut
  CreateShortcut "$DESKTOP\DustDPI.lnk" "$INSTDIR\DustDPI.exe" "" "$INSTDIR\app.ico" 0
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\DustDPI.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely uninstall $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Stop and delete service and drivers
  nsExec::ExecToLog 'sc.exe stop "DustDPI"'
  nsExec::ExecToLog 'sc.exe delete "DustDPI"'
  nsExec::ExecToLog 'sc.exe stop "WinDivert"'
  nsExec::ExecToLog 'sc.exe delete "WinDivert"'
  nsExec::ExecToLog 'sc.exe stop "WinDivert14"'
  nsExec::ExecToLog 'sc.exe delete "WinDivert14"'

  ; Delete shortcuts
  Delete "$DESKTOP\DustDPI.lnk"
  Delete "$SMPROGRAMS\DustDPI\*.*"
  RMDir "$SMPROGRAMS\DustDPI"

  ; Delete files
  Delete "$INSTDIR\x86_64\*.*"
  RMDir "$INSTDIR\x86_64"
  Delete "$INSTDIR\x86\*.*"
  RMDir "$INSTDIR\x86"

  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd
