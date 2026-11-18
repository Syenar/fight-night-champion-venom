[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$InstallRoot,
    [Parameter(Mandatory)][string]$RuntimeRoot,
    [Parameter(Mandatory)][string]$GameDataRoot,
    [Parameter(Mandatory)][string]$UpdateDataRoot,
    [Parameter(Mandatory)][string]$AptToolsRoot,
    [Parameter(Mandatory)][string]$ContentToolRoot,
    [Parameter(Mandatory)][string]$RunRoot,
    [Parameter(Mandatory)][string]$DlcTitleRoot,
    [Parameter(Mandatory)][string]$UserDataRoot,
    [Parameter(Mandatory)][string]$RuntimeReceipt,
    [Parameter(Mandatory)][string]$ContentToolReceipt,
    [Parameter(Mandatory)][string]$PythonExecutable
)
$ErrorActionPreference = 'Stop'
$resolved = @{}
foreach ($name in @('RuntimeRoot','GameDataRoot','UpdateDataRoot','AptToolsRoot','ContentToolRoot','RunRoot','UserDataRoot','RuntimeReceipt','ContentToolReceipt','PythonExecutable')) {
    $resolved[$name] = (Resolve-Path -LiteralPath $PSBoundParameters[$name]).ProviderPath
}
$destination = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($InstallRoot)
if (Test-Path -LiteralPath $destination) {
    throw "Choose a new installation folder. This path already exists: $destination"
}
$parent = Split-Path -Parent $destination
if (!(Test-Path -LiteralPath $parent -PathType Container)) {
    throw "The installation folder's parent must already exist: $parent"
}
$directory = Get-Item -LiteralPath $parent
for ($ancestor = $directory; $null -ne $ancestor; $ancestor = $ancestor.Parent) {
    if ($ancestor.Attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw 'Choose an installation parent without path redirection.'
    }
}
foreach ($ownedRoot in @($resolved.GameDataRoot, $resolved.UpdateDataRoot)) {
    $prefix = $ownedRoot.TrimEnd('\') + '\'
    if ($destination -eq $ownedRoot -or $destination.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'The installation folder must be outside the owned game and update trees.'
    }
}
. (Join-Path $PSScriptRoot 'Owned-Dlc.ps1')
$dlcItems=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'owned-dlc-manifest.json') | ConvertFrom-Json
$dlcSource=(Resolve-Path -LiteralPath $DlcTitleRoot).ProviderPath
Assert-VenomOwnedDlc -TitleRoot $dlcSource -Files $dlcItems
$launcher = Join-Path $PSScriptRoot 'Start-Local-TU2.ps1'
& $launcher @resolved -CheckOnly
$prepared = & $launcher @resolved -PrepareOnly
if (!(Test-Path -LiteralPath $prepared.Run -PathType Container)) {
    throw 'Local media preparation returned no usable result.'
}

Install-VenomOwnedDlc -Source $dlcSource -UserDataRoot $resolved.UserDataRoot -Files $dlcItems
$stage = Join-Path $parent ('.venom-setup-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage | Out-Null
$installedHashes = [ordered]@{}
foreach ($name in @('Start-Installed-TU2.ps1','Start-Local-TU2.ps1','build-offline-xp-overlay.py','Owned-Dlc.ps1','owned-dlc-manifest.json')) {
    $source = Join-Path $PSScriptRoot $name
    $copy = Join-Path $stage $name
    Copy-Item -LiteralPath $source -Destination $copy
    $sourceHash = (Get-FileHash -LiteralPath $source).Hash
    if ((Get-FileHash -LiteralPath $copy).Hash -ne $sourceHash) {
        throw "Installer file copy hash mismatch: $name"
    }
    $installedHashes[$name] = $sourceHash
}
$manifest = [ordered]@{
    Format = 1
    Paths = $resolved
    InstalledFiles = $installedHashes
    InstalledReceiptHashes = @{
        RuntimeReceipt = (Get-FileHash -LiteralPath $resolved.RuntimeReceipt -Algorithm SHA256).Hash
        ContentToolReceipt = (Get-FileHash -LiteralPath $resolved.ContentToolReceipt -Algorithm SHA256).Hash
    }
    PreparationRun = $prepared.Run
}
$path = Join-Path $stage 'venom-install.json'
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $path -Encoding utf8NoBOM
& (Join-Path $stage 'Start-Installed-TU2.ps1') -CheckOnly
[IO.Directory]::Move($stage,$destination)
Write-Output "Installed launcher: $(Join-Path $destination 'Start-Installed-TU2.ps1')"

