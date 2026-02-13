; TilePad Inno Setup Script
; Supports code signing via SignTool

#define MyAppName "TilePad"
#define MyAppVersion "0.5.0"
#define MyAppPublisher "Dynart"
#define MyAppURL "https://github.com/goph-R/TilePad"
#define MyAppExeName "TilePad.exe"

; To enable code signing, configure a SignTool in Inno Setup:
;   Tools > Configure Sign Tools > Add:
;     Name: signtool
;     Command: signtool.exe sign /f "$path_to_cert.pfx" /p $password /tr http://timestamp.digicert.com /td sha256 /fd sha256 $f
;
; Then uncomment the SignTool directive below:
; #define SignToolConfigured

[Setup]
AppId={{B3A7F8E2-5C1D-4A9B-8E6F-2D3C4B5A6E7F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE
OutputDir=Output
OutputBaseFilename=tilepad-setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallMode=x64compatible
#ifdef SignToolConfigured
SignTool=signtool
SignedUninstaller=yes
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main executable
Source: "_package\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Qt6 runtime DLLs
Source: "_package\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "_package\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "_package\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt6 platform plugins
Source: "_package\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs

; Qt6 style plugins
Source: "_package\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs; Check: DirExists(ExpandConstant('{src}\_package\styles'))

; Qt6 image format plugins
Source: "_package\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs; Check: DirExists(ExpandConstant('{src}\_package\imageformats'))

; Visual C++ runtime (if not using static linking)
Source: "_package\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Install VC++ redistributable silently if present
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Visual C++ Runtime..."; Flags: waituntilterminated skipifdoesntexist
; Launch after install
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
