#define AppVersion "1.0.0"
[Setup]
AppId={{F87DB068-CBDC-4BDD-89E6-D6B71DB120AB}
AppName=Geometry Studio
AppVersion={#AppVersion}
AppPublisher=wcy
AppPublisherURL=https://github.com/wcy2432320756-a11y/wcy
DefaultDirName={localappdata}\Programs\GeometryStudio
DefaultGroupName=Geometry Studio
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=..\dist
OutputBaseFilename=GeometryStudio-{#AppVersion}-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\geodraw.exe
DisableProgramGroupPage=yes

[Files]
Source: "..\build\geodraw.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Icons]
Name: "{userprograms}\Geometry Studio"; Filename: "{app}\geodraw.exe"
Name: "{userdesktop}\Geometry Studio"; Filename: "{app}\geodraw.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\geodraw.exe"; Description: "Launch Geometry Studio"; Flags: nowait postinstall skipifsilent
