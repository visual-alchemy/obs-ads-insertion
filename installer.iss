; Inno Setup Script for OBS Auto SF Fit Plugin
; See https://jrsoftware.org/ishelp/ for details.

#define AppName "OBS Auto SF Fit"
#define AppVersion "1.0.0"
#define AppPublisher "visual-alchemy"
#define AppURL "https://github.com/visual-alchemy/obs-ads-insertion"

#ifndef Config
  #define Config "Release"
#endif

[Setup]
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={code:GetOBSPath}
DirExistsWarning=no
EnableDirDoesntExistWarning=yes
AppendDefaultDirName=no
OutputBaseFilename=obs-auto-sf-fit-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
DisableProgramGroupPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "build_x64\{#Config}\obs-auto-sf-fit.dll"; DestDir: "{app}\obs-plugins\64bit"; Flags: ignoreversion
Source: "data\ghost.png"; DestDir: "{app}\data\obs-plugins\obs-auto-sf-fit"; Flags: ignoreversion
Source: "data\locale\en-US.ini"; DestDir: "{app}\data\obs-plugins\obs-auto-sf-fit\locale"; Flags: ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{app}\data\obs-plugins\obs-auto-sf-fit"
Type: files; Name: "{app}\obs-plugins\64bit\obs-auto-sf-fit.dll"

[Code]
function GetOBSPath(Param: String): String;
var
  OBSPath: String;
begin
  if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\OBS Studio', '', OBSPath) then
  begin
    Result := OBSPath;
  end
  else if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\WOW6432Node\OBS Studio', '', OBSPath) then
  begin
    Result := OBSPath;
  end
  else
  begin
    Result := ExpandConstant('{commonpf}\obs-studio');
  end;
end;

// Helper to check if a process is running
function IsModuleLoaded(const ModuleName: String): Boolean;
var
  WMI: Variant;
  WQL: String;
  Cols: Variant;
begin
  Result := False;
  try
    WMI := CreateOleObject('WbemScripting.SWbemLocator');
    WMI := WMI.ConnectServer('.', 'root\CIMV2');
    WQL := Format('SELECT * FROM Win32_Process WHERE Name = ''%s''', [ModuleName]);
    Cols := WMI.ExecQuery(WQL);
    Result := (Cols.Count > 0);
  except
  end;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
  // Check if OBS is running
  while IsModuleLoaded('obs64.exe') do
  begin
    if MsgBox('OBS Studio is currently running. Please close OBS before continuing with the installation.', mbConfirmation, MB_RETRYCANCEL) = IDCANCEL then
    begin
      Result := False;
      Exit;
    end;
  end;
end;

