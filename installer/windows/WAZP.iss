; Inno Setup script for the Windows installer.
; Build:  ISCC /DAppVersion=0.1.0 installer\windows\WAZP.iss
; (paths are relative to this file; expects a Release build in ..\..\build)

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif

#define Artefacts "..\..\build\WAZP_artefacts\Release"

[Setup]
AppId={{8E2C5B7A-3D41-4F96-A1C7-57A2B0E1F0D3}
AppName=WAZP
AppVersion={#AppVersion}
AppVerName=WAZP {#AppVersion}
AppPublisher=Ticco Ross
DefaultDirName={autopf}\Ticco Ross\WAZP
DefaultGroupName=Ticco Ross
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\dist
OutputBaseFilename=WAZP-{#AppVersion}-Windows-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=WAZP

[Types]
Name: "full";   Description: "VST3 plug-in and standalone app"
Name: "custom"; Description: "Choose components"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plug-in (Ableton Live, Bitwig, Reaper, Cubase...)"; Types: full custom
Name: "app";  Description: "Standalone app";                                         Types: full custom

[Files]
Source: "{#Artefacts}\VST3\WAZP.vst3\*"; DestDir: "{commoncf64}\VST3\WAZP.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\WAZP.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\WAZP"; Filename: "{app}\WAZP.exe"; Components: app

[Messages]
FinishedLabel=WAZP is installed.%n%nRescan plug-ins in your DAW (Ableton Live: Settings > Plug-Ins > Rescan).
