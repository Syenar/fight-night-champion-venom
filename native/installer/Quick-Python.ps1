param(
    [Parameter(Mandatory)][string]$InstallerDirectory,
    [Parameter(Mandatory)][string]$GameFolder,
    [Parameter(Mandatory)][string]$Output,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
$package = [IO.Path]::GetFullPath($InstallerDirectory)
$game = [IO.Path]::GetFullPath($GameFolder)
$outputPath = [IO.Path]::GetFullPath($Output)
$cache = Join-Path (Split-Path -Parent $outputPath) '.venom-inputs'
$root = Join-Path $cache 'python-3.13.17-9e35319bffed'
$python = Join-Path $root 'python.exe'
$receipt = Join-Path $root 'venom-python-sha256.txt'
$asset = 'https://www.python.org/ftp/python/3.13.17/python-3.13.17-embed-amd64.zip'
$expected = '9E35319BFFEDAB8B36AD44D8F377CB4F1733BCEC257A0B6D04D94F01486DDB38'
function Assert-Local([string]$path) {
    for ($cursor = $path; $cursor; $cursor = Split-Path -Parent $cursor) {
        $node = Get-Item -LiteralPath $cursor -Force -ErrorAction SilentlyContinue
        if ($node -and ($node.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw "Setup cannot use a redirected cache path: $cursor"
        }
    }
}
foreach ($inputRoot in @($package,$game)) {
    $base = $inputRoot.TrimEnd('\') + '\'
    if ($cache -eq $inputRoot -or $cache.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)) {
        throw 'Setup cache must be outside the extracted installer and game folders.'
    }
}
Assert-Local $outputPath
Assert-Local $receipt
if (!(Test-Path -LiteralPath $root)) {
    Write-Output 'Preparing a portable Python runtime from python.org…'
    [IO.Directory]::CreateDirectory($cache) | Out-Null
    $stage = Join-Path $cache ('python-download-' + [guid]::NewGuid().ToString('N'))
    Assert-Local $stage
    [IO.Directory]::CreateDirectory($stage) | Out-Null
    $zip = Join-Path $stage 'python.zip'
    $files = Join-Path $stage 'files'
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -UseBasicParsing -Uri $asset -OutFile $zip
    if ((Get-FileHash -LiteralPath $zip).Hash -ne $expected) { throw 'Portable Python checksum mismatch.' }
    Expand-Archive -LiteralPath $zip -DestinationPath $files
    $signature = (Get-FileHash -LiteralPath (Join-Path $files 'python.exe')).Hash
    [IO.File]::WriteAllText((Join-Path $files 'venom-python-sha256.txt'), $signature, [Text.Encoding]::ASCII)
    if (!(Test-Path -LiteralPath $root)) {
        [IO.Directory]::Move($files,$root)
    }
}
if ((Get-Content -LiteralPath $receipt -Raw).Trim() -ne (Get-FileHash -LiteralPath $python).Hash) {
    throw 'Cached portable Python failed its identity check.'
}
$script = Join-Path $package 'Quick-Install-Venom.py'
$entry = 'import runpy,sys; from pathlib import Path; entry=sys.argv.pop(1); sys.argv[0]=entry; sys.path.insert(0,str(Path(entry).parent)); runpy.run_path(entry,run_name=''__main__'')'
$argsList = @('-u','-B','-c',$entry,$script,'--game-folder',$game,'--output',$outputPath)
if ($CheckOnly) { $argsList += '--check' }
& $python @argsList
exit $LASTEXITCODE
