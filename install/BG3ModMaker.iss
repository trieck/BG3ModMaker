
#define AppName "BG3ModMaker"
#define AppPublisher "Thomas A. Rieck"
#define AppExeName "BG3ModStudio.exe"
#define ReleaseDir "..\x64\Release\"
#define AppVersion GetStringFileInfo(ReleaseDir + AppExeName, "ProductVersion")

[Setup]
AppId={{C4F25E0E-73CA-41C3-BC5A-C8B415EAE13F}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\{#AppName}
UninstallDisplayIcon={app}\{#AppExeName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
SolidCompression=yes
WizardStyle=modern
OutputDir=.\
OutputBaseFilename=BG3ModMaker_{#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#ReleaseDir}{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}Catalog.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}Iconizer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}Index.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}DirectXTex.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}Lexilla.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}lz4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}pugixml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}Scintilla.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}xapian-30.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#ReleaseDir}zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\redist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall
Source: "INSTALL.pdf"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Microsoft Visual C++ Redistributable..."
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\INSTALL.pdf"; Description: "View installation guide"; Flags: shellexec postinstall skipifsilent
