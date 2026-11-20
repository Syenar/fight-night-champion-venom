param(
    [Parameter(Mandatory)][string]$InstallerDirectory,
    [Parameter(Mandatory)][string]$GameFolder,
    [Parameter(Mandatory)][string]$Output,
    [ValidateNotNullOrEmpty()][string]$PreviousOutput,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
# A parent PowerShell 7 session can supply an incompatible module search path.
$env:PSModulePath = Join-Path $PSHOME 'Modules'
$package = [IO.Path]::GetFullPath($InstallerDirectory)
$game = [IO.Path]::GetFullPath($GameFolder)
$outputPath = [IO.Path]::GetFullPath($Output)
$cache = Join-Path (Split-Path -Parent $outputPath) '.venom-inputs'
$root = Join-Path $cache 'python-3.13.17-9e35319bffed'
$python = Join-Path $root 'python.exe'
$asset = 'https://www.python.org/ftp/python/3.13.17/python-3.13.17-embed-amd64.zip'
$expected = '9E35319BFFEDAB8B36AD44D8F377CB4F1733BCEC257A0B6D04D94F01486DDB38'
# SHA-256 of each member of the pinned embeddable Python archive.
$runtimeHashes = @{
    '_asyncio.pyd' = '0E53F227048492830BF0328BBC3BEBC336268821C88C1DA1BA1711855B51CD01'
    '_bz2.pyd' = '0ADF0680F54EEC173401230DF0AA561AA188C7F4D40E5F2BA790D333C6057D8D'
    '_ctypes.pyd' = '487D9041455132D74ED26C0EAE514675C31010A926DB0CD6C6F391FCCCC66C42'
    '_decimal.pyd' = '7E8E5D026E22BE2669B2AC6175FECE6B7FBE2FAF1E456FA4785130832D244955'
    '_elementtree.pyd' = '5436C3D722E4B7709D20CB9BBB8385253337651417CC6A5D447F7B9E8A9DA7A9'
    '_hashlib.pyd' = 'FC9075F3A836FDCBB826B541F4D1E9382D84C73F830F5F51F8CA3B9CCB9D42CB'
    '_lzma.pyd' = '09596EE41DADFABE006429F1CBB7505A3006DAA80206AE2DD2D0D8D5936B6229'
    '_multiprocessing.pyd' = '56BC0F253E74F965D5565377E40737093EE084A8CA4B4347F2F6BB9C15661C6D'
    '_overlapped.pyd' = 'B2DCAEFADF6194783F24184FD074B83983091D46C6B50E88DC8A652A8569D614'
    '_queue.pyd' = 'AA1BC394AC6ED619B2525E631665431E9FBD798CD7D8374755EFED9C3D325539'
    '_socket.pyd' = '03AC131EC3FA4C550D36FA7C5CAD3601CBE6C8F18E1A65B9BD8A65CE1131F9E1'
    '_sqlite3.pyd' = '721707B1A66B51E2C1A3BE964F4D5D0C6EB89DC6671B8EA121A10D46F5041338'
    '_ssl.pyd' = '74BFE7A396737CF5910CF4A789D9CF4263BA6BA93C994CE8D5E2F61FA972097D'
    '_uuid.pyd' = '0D7589AC0965D1AD7B19BCDFDDC726EEFA5896AAFA6702D68DC9CA6CD58820DC'
    '_wmi.pyd' = '927AAB8EEF564D2DEE1BBCF8E11F488789F664EAFB4A0D2AACD99E224BE2F10A'
    '_zoneinfo.pyd' = '12603A4FD550248D86C41D6FAEF73C77B27E20CCF6A942CEA1329AE393742E8B'
    'libcrypto-3.dll' = '75495FE0E079AB00D49BE740D3C710A0B94E3F74F9648CB7CC9F768FC7F52319'
    'libffi-8.dll' = 'EFF52743773EB550FCC6CE3EFC37C85724502233B6B002A35496D828BD7B280A'
    'libssl-3.dll' = '8957182A9C385BFD9F3F790921DE2DDC614CF91F99EDBED4E9E5446A46504E32'
    'LICENSE.txt' = '62BEC384DF47B0328307DB41455FF6EA2559E5546B394AC69148561B21703120'
    'pyexpat.pyd' = '4095F444C5D0FE9D24C1A46A8630459F8488C5392DC82BE9AF67F4045EA9C368'
    'python.cat' = '5680216E77E9B59BB555284DD4A754706203AD1E7C617DD6DC5F4B4E94EC6C9F'
    'python.exe' = '7C2FC2B303A4EE3CC14CC17B47BC6E8BCA98C78792EDD123B846A560DA44B203'
    'python3.dll' = 'A45D898BA8B796F1883FC2FCD55E4736163DFB7CEA825D52E3477F1957DFB45A'
    'python313._pth' = '35DDF94682FF9AA713A8D63557242AD00F3F28FDD39337F02C3BDA4C0F791577'
    'python313.dll' = '26026530507A7389E7892CE130E7C1753B32A2DF03EC3721D45AD8C1705D40EB'
    'python313.zip' = '58AF5BF33DCFDEDF72F60B0926FDB619CC2960C9D4200677531D35F626C8EE24'
    'pythonw.exe' = '9C1437A2E692E9EC9D8B9EB0FD8A6432F8376510AC81B174437AD6A216C2C84B'
    'select.pyd' = 'F2B8F6008D9E8697700B73FD645A76D83E096E8214A6E31E644DE32F56122F6A'
    'sqlite3.dll' = '9044E1DEA980CD0514DC2E266E039320E35B00C6326FD2C2D14C6B3301C8518D'
    'unicodedata.pyd' = '541F46CF1AB49239B008A563EEE6762086C7288F64631CB5CF59451CF22FC70C'
    'vcruntime140_1.dll' = 'A7146C08F89FE5B04541AB507CDB59FF7B44534D4BA3C668A426C6450A03434E'
    'vcruntime140.dll' = 'D1F4225DF2CD877DBF130D5668A021DCE3F94118455FF5EC952061C30AFC9CE7'
    'winsound.pyd' = 'BE5394496D9EF633DC6D81D95C7816F0B0FFA3E90763AF3AA41270FBA0C16FE6'
}
function Assert-Local([string]$path) {
    for ($cursor = $path; $cursor; $cursor = Split-Path -Parent $cursor) {
        $node = Get-Item -LiteralPath $cursor -Force -ErrorAction SilentlyContinue
        if ($node -and ($node.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw "Setup cannot use a redirected cache path: $cursor"
        }
    }
}
function Assert-PortablePython([string]$directory) {
    Assert-Local $directory
    $found = @{}
    foreach ($node in Get-ChildItem -LiteralPath $directory -Force) {
        if ($node.PSIsContainer -or ($node.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw "Unexpected portable Python cache entry: $($node.Name)"
        }
        # Accept the former receipt as an inert file for caches from earlier installers.
        if ($node.Name -eq 'venom-python-sha256.txt') { continue }
        if (!$runtimeHashes.ContainsKey($node.Name)) {
            throw "Unexpected portable Python cache file: $($node.Name)"
        }
        if ((Get-FileHash -LiteralPath $node.FullName).Hash -ne $runtimeHashes[$node.Name]) {
            throw "Portable Python checksum mismatch: $($node.Name)"
        }
        $found[$node.Name] = $true
    }
    foreach ($name in $runtimeHashes.Keys) {
        if (!$found.ContainsKey($name)) { throw "Missing portable Python cache file: $name" }
    }
}
if (!$PreviousOutput) { throw 'Choose an existing Venom build.' }
foreach ($inputRoot in @($package,$game,[IO.Path]::GetFullPath($PreviousOutput))) {
    $base = $inputRoot.TrimEnd('\') + '\'
    if ($cache -eq $inputRoot -or $cache.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)) {
        throw 'Setup cache must be outside the extracted installer and game folders.'
    }
}
Assert-Local $outputPath
Assert-Local $root
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
    Assert-PortablePython $files
    try {
        [IO.Directory]::Move($files,$root)
    } catch [IO.IOException] {
        # Another setup may have created the cache while this one downloaded.
        if (!(Test-Path -LiteralPath $root -PathType Container)) { throw }
    }
}
Assert-PortablePython $root
$script = Join-Path $package 'Local-Copy-Setup.py'
$entry = 'import runpy,sys; from pathlib import Path; entry=sys.argv.pop(1); sys.argv[0]=entry; sys.path.insert(0,str(Path(entry).parent)); runpy.run_path(entry,run_name=''__main__'')'
$argsList = @('-u','-B','-c',$entry,$script,'--game-folder',$game,'--output',$outputPath)
if ($PSBoundParameters.ContainsKey('PreviousOutput')) { $argsList += @('--previous-output', $PreviousOutput) }
if ($CheckOnly) { $argsList += '--check' }
& $python @argsList
exit $LASTEXITCODE
