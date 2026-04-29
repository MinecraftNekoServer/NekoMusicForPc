; Neko云音乐 Windows Installer Script
; NSIS 3.0+

!include "MUI2.nsh"

; Version (passed via -DVERSION= on command line)
!ifndef VERSION
  !define VERSION "0.0.0"
!endif

; General
Name "Neko云音乐"
OutFile "..\Neko云音乐-$VERSION-win.exe"
InstallDir "$PROGRAMFILES64\Neko云音乐"
SetCompressor lzma

; Manifest
ManifestSupportedOS all
RequestExecutionLevel admin

; UI Settings
!define MUI_ICON "app.ico"
!define MUI_ABORTWARNING
!define MUI_WELCOMEFINISHPAGE_BITMAP "/usr/share/nsis/Contrib/Graphics/Wizard/nsis3-branding.bmp"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; Installer
Section "Neko云音乐" SecMain
    SetOutPath "$INSTDIR"

    ; Install main executable
    File "../build/NekoMusic.exe"

    ; Install Qt dependencies
    File /nonfatal /r "../build/platforms"
    File /nonfatal /r "../build/multimedia"
    File /nonfatal /r "../build/iconengines"
    File /nonfatal /r "../build/imageformats"
    File /nonfatal /r "../build/styles"
    File /nonfatal /r "../build/translations"
    File /nonfatal /r "../build/sqldrivers"

    ; Qt DLLs
    File "../build/Qt6Core.dll"
    File "../build/Qt6Gui.dll"
    File "../build/Qt6Widgets.dll"
    File "../build/Qt6Multimedia.dll"
    File "../build/Qt6Network.dll"
    File "../build/Qt6Sql.dll"
    File "../build/Qt6Svg.dll"

    ; Optional: OpenSSL DLLs
    File /nonfatal "../build/libssl*.dll"
    File /nonfatal "../build/libcrypto*.dll"

    ; Create Start Menu Shortcut
    CreateDirectory "$SMPROGRAMS\Neko云音乐"
    CreateShortCut "$SMPROGRAMS\Neko云音乐\Neko云音乐.lnk" "$INSTDIR\NekoMusic.exe"
    CreateShortCut "$SMPROGRAMS\Neko云音乐\卸载Neko云音乐.lnk" "$INSTDIR\uninst.exe"

    ; Create Desktop Shortcut
    CreateShortCut "$DESKTOP\Neko云音乐.lnk" "$INSTDIR\NekoMusic.exe"

    ; Registry for uninstall
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "DisplayName" "Neko云音乐"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "QuietUninstallString" "$\"$INSTDIR\uninst.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "DisplayIcon" "$INSTDIR\NekoMusic.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "Publisher" "Neko"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐" \
                     "DisplayVersion" "${VERSION}"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

; Uninstaller
Section "Uninstall"
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Neko云音乐"

    ; Remove shortcuts
    Delete "$DESKTOP\Neko云音乐.lnk"
    RMDir /r "$SMPROGRAMS\Neko云音乐"

    ; Remove installed files
    RMDir /r "$INSTDIR"
SectionEnd
