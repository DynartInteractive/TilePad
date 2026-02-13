; TilePad Inno Setup Script
; Supports code signing via SignTool

#define MyAppName "TilePad"
#define MyAppVersion "0.6.0"
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
ChangesEnvironment=yes
#ifdef SignToolConfigured
SignTool=signtool
SignedUninstaller=yes
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "addtopath"; Description: "Add TilePad to PATH (for CLI usage)"; GroupDescription: "CLI Integration:"; Flags: unchecked

[Files]
; Main executable
Source: "_package\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Qt6 runtime DLLs
Source: "_package\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt6 plugins
Source: "_package\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "_package\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "_package\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "_package\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "_package\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs
Source: "_package\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "_package\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Tasks: addtopath; Check: NeedsAddPath(ExpandConstant('{app}'))

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
function NeedsAddPath(Param: string): Boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  Result := Pos(';' + Uppercase(Param) + ';', ';' + Uppercase(OrigPath) + ';') = 0;
end;
