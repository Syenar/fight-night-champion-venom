[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$UserDataRoot,
    [Parameter(Mandatory)][string]$RuntimeReceipt,
    [Parameter(Mandatory)][string]$ContentToolReceipt,
    [string]$PythonExecutable,
    [Parameter(Mandatory)][string]$RuntimeRoot,
    [Parameter(Mandatory)][string]$GameDataRoot,
    [Parameter(Mandatory)][string]$UpdateDataRoot,
    [Parameter(Mandatory)][string]$AptToolsRoot,
    [Parameter(Mandatory)][string]$ContentToolRoot,
    [Parameter(Mandatory)][string]$RunRoot,
    [ValidateSet('30', '60')][int]$FrameRate,
    [ValidateSet('none','full')][string]$ReadbackResolve,
    [ValidateSet('xinput','sdl')][string]$InputBackend = 'xinput',
    [string]$ControllerMappingFile,
    [switch]$CheckOnly,
    [switch]$PrepareOnly,
    [switch]$WaitForExit
)

$ErrorActionPreference = 'Stop'
$resolvedControllerMappingFile = $null
if ($PSBoundParameters.ContainsKey('ControllerMappingFile')) {
    if ([string]::IsNullOrWhiteSpace($ControllerMappingFile)) { throw 'ControllerMappingFile must name an existing absolute file path.' }
    if (![System.IO.Path]::IsPathFullyQualified($ControllerMappingFile)) { throw 'ControllerMappingFile must be an absolute file path.' }
    $mappingItem = Get-Item -LiteralPath $ControllerMappingFile -ErrorAction Stop
    if ($mappingItem -isnot [System.IO.FileInfo]) { throw 'ControllerMappingFile must name an existing file.' }
    if ($InputBackend -ne 'sdl') { throw 'ControllerMappingFile can only be used with InputBackend sdl.' }
    $resolvedControllerMappingFile = $mappingItem.FullName
}
$installedManifestPath = Join-Path $PSScriptRoot 'venom-install.json'
if (Test-Path -LiteralPath $installedManifestPath -PathType Leaf) {
    $installedManifest = Get-Content -LiteralPath $installedManifestPath -Raw | ConvertFrom-Json -AsHashtable
    if ($installedManifest.Format -ne 1) { throw 'Unsupported Venom installation manifest.' }
    $expectedLocalHash = $installedManifest.InstalledFiles['Start-Local-TU2.ps1']
    if ([string]::IsNullOrWhiteSpace($expectedLocalHash) -or (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash -ne $expectedLocalHash) {
        throw 'Installed component hash mismatch: Start-Local-TU2.ps1'
    }
}
$RuntimeRoot = (Resolve-Path -LiteralPath $RuntimeRoot).Path
$RunRoot = (Resolve-Path -LiteralPath $RunRoot).Path
$UserDataRoot = (Resolve-Path -LiteralPath $UserDataRoot).Path
$game = (Resolve-Path -LiteralPath $GameDataRoot).Path
$update = (Resolve-Path -LiteralPath $UpdateDataRoot).Path
$aptTools = (Resolve-Path -LiteralPath $AptToolsRoot).Path
if ($game -eq $update) {
    throw 'The game and title-update roots must be distinct.'
}

foreach ($writable in @($RunRoot, $UserDataRoot)) {
    foreach ($owned in @($game, $update)) {
        if ($writable -eq $owned -or $writable.StartsWith($owned.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
            throw 'Writable state must be outside the owned game and update trees.'
        }
    }
}
$sourceArchive = Join-Path $game 'data\xenon\fe\game\screens\createboxer\boxergrowth\boosts.big'
$builder = Join-Path $PSScriptRoot 'build-offline-xp-overlay.py'
$exe = Join-Path $RuntimeRoot 'venom_fnc_tu2_dlc.exe'
$overlayHash = 'D7EB8297F20C58E1DC9D5E5699499816E2028FA33CBA90CE39C65A544557DF9F'

$ContentToolRoot = (Resolve-Path -LiteralPath $ContentToolRoot).Path
$contentTool = Join-Path $ContentToolRoot 'fnc_content.exe'
$speechRoot = Join-Path $game 'datadlc\dlc_pack1\dlc\data\xenon\loc\eng\speech'
$speechFiles = @(
    @{ Name='speech_eng_dlc_samples.sbr'; Source='13EE8BF8835B9F6F11D6B9A4599D094BDB4452DA3002DF2471574D747F276D6E'; Decoded='08B7BB2C4AA798669AAA14F0976CEA4A0DC7F7871D886DFDCB5DB382E6DA4A2B' },
    @{ Name='speech_eng_dlc_samples.sbs'; Source='71C9602D5982F47F2095DA6DE1BFF276E30739A6430A1EAD78066D918B172DD2'; Decoded='68F0609AA77D7C20234FE601774C2AB2E660C05CC7BE0531947A37E4D1EAA654' }
)

$build = Get-Content -LiteralPath $RuntimeReceipt -Raw | ConvertFrom-Json -AsHashtable
$helper = Get-Content -LiteralPath $ContentToolReceipt -Raw | ConvertFrom-Json -AsHashtable
if ($build.compiledObjects.Count -ne 219 -or
    $build.inputs.retailXex -ne '90a698c71aa714e383ce092750ce767e07145f1ca38d8a9c9b3bd8005ea29ff6' -or
    $build.inputs.tu2Xexp -ne '8429d60620012112285809e6b6545af646056b9fccba3d1b88bb71c32ea9a8ae') {
    throw 'The title build receipt does not bind all219objects to the exact retail/TU2 inputs.'
}
if ((Resolve-Path -LiteralPath $helper.ToolRoot).ProviderPath -ne $ContentToolRoot) {
    throw 'Content helper receipt root differs from ContentToolRoot.'
}
$expected = [ordered]@{
    $contentTool = $helper.ToolFiles['fnc_content.exe']
    (Join-Path $ContentToolRoot 'rexruntimerd.dll') = $helper.ToolFiles['rexruntimerd.dll']
    (Join-Path $ContentToolRoot 'TracyClientrd.dll') = $helper.ToolFiles['TracyClientrd.dll']
    $exe = $build.titleSha256
    (Join-Path $RuntimeRoot 'rexruntimerd.dll') = $build.runtimeFiles['rexruntimerd.dll']
    (Join-Path $RuntimeRoot 'rexgpu-xenosrd.dll') = $build.runtimeFiles['rexgpu-xenosrd.dll']
    (Join-Path $RuntimeRoot 'TracyClientrd.dll') = $build.runtimeFiles['TracyClientrd.dll']
    (Join-Path $game 'data\xenon\shaders\adaptfixedshader.fxo') = '95975518EAAA000E6148A5A59141627A3A48631BB8B98BAE5617DE6CD5A41109'
    (Join-Path $game 'default.xex') = '90A698C71AA714E383CE092750CE767E07145F1CA38D8A9C9B3BD8005EA29FF6'
    (Join-Path $update 'default.xexp') = '8429D60620012112285809E6B6545AF646056B9FCCBA3D1B88BB71C32EA9A8AE'
    $sourceArchive = '2A464FADCF5E4A07DE5A0EBA2C16892B47656BACF96856B01323023057443330'
}

$siblingPatch = Join-Path $game 'default.xexp'
if (Test-Path -LiteralPath $siblingPatch) {
    if (!(Test-Path -LiteralPath $siblingPatch -PathType Leaf)) { throw 'The optional game-root title update is not a file.' }
    $expected[$siblingPatch] = '8429D60620012112285809E6B6545AF646056B9FCCBA3D1B88BB71C32EA9A8AE'
}

foreach ($asset in $speechFiles) {
    $expected[(Join-Path $speechRoot $asset.Name)] = $asset.Source
}
foreach ($item in $expected.GetEnumerator()) {
    if ($item.Value -notmatch '^[a-fA-F0-9]{64}$') { throw "Missing or invalid input hash: $($item.Key)" }
    if (!(Test-Path -LiteralPath $item.Key -PathType Leaf)) {
        throw "Missing exact TU2 input: $($item.Key)"
    }
    if ((Get-FileHash -LiteralPath $item.Key -Algorithm SHA256).Hash -ne $item.Value) {
        throw "Unexpected hash for exact TU2 input: $($item.Key)"
    }
}
foreach ($tool in @($builder, (Join-Path $aptTools 'aptpatch_display.py'), (Join-Path $aptTools 'refpack_enc.py'), (Join-Path $aptTools 'refpack.py'))) {
    if (!(Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw "Missing XP archive build input: $tool"
    }
}
if ($PythonExecutable) { $python = (Resolve-Path -LiteralPath $PythonExecutable).ProviderPath }
else { $python = (Get-Command python.exe -CommandType Application -ErrorAction Stop | Select-Object -First 1).Source }

function Copy-ControllerMappingFileToRun {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string]$SourcePath,
        [Parameter(Mandatory)][string]$RunRoot
    )
    $destination = Join-Path $RunRoot 'gamecontrollerdb.txt'
    Copy-Item -LiteralPath $SourcePath -Destination $destination
    if (!(Test-Path -LiteralPath $destination -PathType Leaf)) { throw 'The private SDL mapping copy was not created.' }
    return $destination
}
function New-TU2LaunchArgumentList {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string]$GameRoot,
        [Parameter(Mandatory)][string]$UpdateRoot,
        [Parameter(Mandatory)][string]$UserDataRoot,
        [Parameter(Mandatory)][string]$RunRoot,
        [Parameter(Mandatory)][string]$XpRoot,
        [Parameter(Mandatory)][string]$SpeechRoot,
        [Parameter(Mandatory)][ValidateSet('xinput','sdl')][string]$InputBackend,
        [string]$ControllerMappingFile,
        [int]$FrameRate,
        [string]$ReadbackResolve
    )
    $arguments = @(
        "--game_data_root=$GameRoot",
        "--update_data_root=$UpdateRoot",
        "--user_data_root=$UserDataRoot",
        "--storage_root=$RunRoot",
        "--log_file=$RunRoot\runtime.log",
        "--venom_offline_xp_ui_root=$XpRoot",
        "--venom_legends_speech_root=$SpeechRoot",
        '--venom_tu2_recover_823ddfe0=true',
        '--gpu_allow_invalid_fetch_constants=true',
        '--log_level=info',
        '--gpu_plugin=xenos',
        '--mount_cache=true',
        '--fullscreen=false',
        '--vsync=false',
        '--license_mask=4294967295',
        '--fnc_dlc_unlock=true',
        '--fnc_skip_intro=true',
        '--render_target_path_d3d12=rtv',
        '--host_present_from_non_ui_thread=false',
        "--input_backend=$InputBackend",
        '--mnk_mode=true',
        '--venom_writable_guest_cache=true',
        '--allow_game_relative_writes=false'
    )
    if ($ControllerMappingFile) {
        if (![System.IO.Path]::IsPathFullyQualified($ControllerMappingFile)) { throw 'The staged SDL mapping path must be absolute.' }
        $arguments += "--hid_mappings_file=$ControllerMappingFile"
    }
    if ($FrameRate) { $arguments += "--fnc_frame_rate=$FrameRate" }
    if ($ReadbackResolve) { $arguments += "--readback_resolve=$ReadbackResolve" }
    return $arguments
}
if ($CheckOnly) {
    Write-Output 'Exact TU2 launch inputs verified. The XP and Legends speech overlays are generated on launch.'
    return
}

function Initialize-TU2RunAssets {
    $run = Join-Path $RunRoot ('tu2-native-launch-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fff') + '-' + $PID)
    $overlay = Join-Path $run 'offline-xp'
    New-Item -ItemType Directory -Path $overlay, "$run\cache\replay", "$run\cache\replay_metadata", "$run\cache\replay\1\1" | Out-Null
    $patchedArchive = Join-Path $overlay 'boosts.big'
    & $python -B $builder $aptTools $sourceArchive $patchedArchive | Out-Host
    if ($LASTEXITCODE -ne 0 -or !(Test-Path -LiteralPath $patchedArchive -PathType Leaf)) {
        throw 'The offline XP archive could not be generated.'
    }
    if ((Get-FileHash -LiteralPath $patchedArchive -Algorithm SHA256).Hash -ne $overlayHash) {
        throw 'Generated offline XP archive differs from the verified native test.'
    }

    $speechOverlay = Join-Path $run 'legends-speech'
    New-Item -ItemType Directory -Path $speechOverlay | Out-Null
    foreach ($asset in $speechFiles) {
        $inputPath = Join-Path $speechRoot $asset.Name
        $outputPath = Join-Path $speechOverlay $asset.Name
        & $contentTool unlzx $inputPath $outputPath | Out-Host
        if ($LASTEXITCODE -ne 0 -or !(Test-Path -LiteralPath $outputPath -PathType Leaf)) {
            throw "The Legends commentary file could not be decoded: $($asset.Name)"
        }
        if ((Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash -ne $asset.Decoded) {
            throw "Decoded commentary differs from the verified runtime asset: $($asset.Name)"
        }
    }

    $mappingCopy = $null
    if ($resolvedControllerMappingFile) {
        $mappingCopy = Copy-ControllerMappingFileToRun -SourcePath $resolvedControllerMappingFile -RunRoot $run
    }
    [pscustomobject]@{
        Run = $run
        XpRoot = $overlay
        SpeechRoot = $speechOverlay
        ControllerMappingFile = $mappingCopy
    }
}

if ($PrepareOnly) {
    if ($WaitForExit) { throw 'PrepareOnly does not launch a game process.' }
    Initialize-TU2RunAssets
    return
}

$launchLock = [System.Threading.Mutex]::new($false, 'Local\FightNightChampionVenom-TU2-Launch')
$locked = $false
try {
    try {
        $locked = $launchLock.WaitOne(0)
    } catch [System.Threading.AbandonedMutexException] {
        $locked = $true
    }
    if (!$locked) { throw 'A native launch is already in progress.' }
    $old = @(Get-Process -Name 'venom*','xenia*','rexglue*' -ErrorAction SilentlyContinue |
        Where-Object { $_.ProcessName -ne 'VenomInstaller' })
    if ($old.Count) { throw "Close the previous game process before launching TU2 (PID $($old.Id -join ', '))." }

    $prepared = Initialize-TU2RunAssets
    $run = $prepared.Run
    $overlay = $prepared.XpRoot
    $speechOverlay = $prepared.SpeechRoot

    $psi = [System.Diagnostics.ProcessStartInfo]::new($exe)
    $psi.UseShellExecute = $false
    $psi.WorkingDirectory = $run
    $arguments = New-TU2LaunchArgumentList `
        -GameRoot $game `
        -UpdateRoot $update `
        -UserDataRoot $UserDataRoot `
        -RunRoot $run `
        -XpRoot $overlay `
        -SpeechRoot $speechOverlay `
        -InputBackend $InputBackend `
        -ControllerMappingFile $prepared.ControllerMappingFile `
        -FrameRate $FrameRate `
        -ReadbackResolve $ReadbackResolve
    foreach ($argument in $arguments) { $psi.ArgumentList.Add($argument) }

    $process = [System.Diagnostics.Process]::Start($psi)
    Write-Output "Started native TU2 PID $($process.Id)"
    Write-Output "Log: $run\runtime.log"
    if ($process.WaitForExit(5000)) {
        throw "Native TU2 stopped during startup with exit code $($process.ExitCode)."
    }
    if ($WaitForExit) {
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) { throw "Native TU2 exited with code $($process.ExitCode)." }
    }
} finally {
    if ($locked) { $launchLock.ReleaseMutex() }
    $launchLock.Dispose()
}








