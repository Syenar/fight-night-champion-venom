[CmdletBinding()]
param(
    [ValidateSet(30,60)][int]$FrameRate,
    [ValidateSet('none','full')][string]$ReadbackResolve,
    [ValidateSet('xinput','sdl')][string]$InputBackend,
    [string]$ControllerMappingFile,
    [switch]$CheckOnly,
    [switch]$WaitForExit
)
$ErrorActionPreference = 'Stop'
$config = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'venom-install.json') -Raw | ConvertFrom-Json -AsHashtable
if ($config.Format -ne 1) { throw 'Unsupported Venom installation manifest.' }
$installedLauncherHash = $config.InstalledFiles['Start-Installed-TU2.ps1']
if ([string]::IsNullOrWhiteSpace($installedLauncherHash) -or (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash -ne $installedLauncherHash) {
    throw 'Installed component hash mismatch: Start-Installed-TU2.ps1'
}
foreach ($name in @('Start-Local-TU2.ps1','build-offline-xp-overlay.py','Owned-Dlc.ps1','owned-dlc-manifest.json')) {
    if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $name)).Hash -ne $config.InstalledFiles[$name]) {
        throw "Installed component hash mismatch: $name"
    }
}
$paths = @{}
foreach ($name in @('RuntimeRoot','GameDataRoot','UpdateDataRoot','AptToolsRoot','ContentToolRoot','RunRoot','UserDataRoot','RuntimeReceipt','ContentToolReceipt','PythonExecutable')) {
    if (!$config.Paths[$name]) { throw "Missing installed path: $name" }
    $paths[$name] = $config.Paths[$name]
}
foreach ($name in @('RuntimeReceipt','ContentToolReceipt')) {
    $pin = $config.InstalledReceiptHashes[$name]
    if ($pin -notmatch '^[a-fA-F0-9]{64}$' -or (Get-FileHash -LiteralPath $paths[$name] -Algorithm SHA256).Hash -ne $pin) {
        throw "Installed receipt hash mismatch: $name"
    }
}
. (Join-Path $PSScriptRoot 'Owned-Dlc.ps1')
$dlcItems=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'owned-dlc-manifest.json') | ConvertFrom-Json
$localDlc=Join-Path $paths.UserDataRoot '0000000000000000\45410915'
Assert-VenomOwnedDlc -TitleRoot $localDlc -Files $dlcItems
$launchOptions = @{
    FrameRate = 60
    ReadbackResolve = 'full'
}
foreach ($name in @('FrameRate','ReadbackResolve','InputBackend','ControllerMappingFile','CheckOnly','WaitForExit')) {
    if ($PSBoundParameters.ContainsKey($name)) {
        $launchOptions[$name] = $PSBoundParameters[$name]
    }
}
& (Join-Path $PSScriptRoot 'Start-Local-TU2.ps1') @paths @launchOptions

