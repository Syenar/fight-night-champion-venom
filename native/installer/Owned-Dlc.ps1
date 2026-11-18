function Get-VenomOwnedDlcPath {
    param([string]$Root, [string]$RelativePath)
    if ([string]::IsNullOrWhiteSpace($Root) -or
        $RelativePath -notmatch '^(00000002|Headers)/[A-Za-z0-9._/-]+$' -or
        ($RelativePath -split '/') -contains '..' -or
        ($RelativePath -split '/') -contains '.') {
        throw 'Invalid DLC manifest path.'
    }
    $rootFull = [IO.Path]::GetFullPath($Root).TrimEnd([IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
    $relativeNative = $RelativePath.Replace('/', [IO.Path]::DirectorySeparatorChar)
    $full = [IO.Path]::GetFullPath([IO.Path]::Combine($rootFull, $relativeNative))
    $prefix = $rootFull + [IO.Path]::DirectorySeparatorChar
    if (!$full.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'DLC manifest path escapes its root.'
    }
    return $full
}

function Assert-VenomOwnedDlcNoReparse {
    param([string]$Root, [string]$RelativePath)
    $rootFull = [IO.Path]::GetFullPath($Root)
    $rootItem = Get-Item -LiteralPath $rootFull -Force -ErrorAction SilentlyContinue
    if ($rootItem -and !$rootItem.PSIsContainer) { throw "DLC root is not a directory: $rootFull" }
    if ($rootItem -and ($rootItem.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "DLC root is a reparse point: $rootFull"
    }
    $current = $rootFull
    $parts = $RelativePath -split '/'
    for ($i = 0; $i -lt $parts.Count; $i++) {
        $current = Join-Path $current $parts[$i]
        $item = Get-Item -LiteralPath $current -Force -ErrorAction SilentlyContinue
        if ($item -and $i -lt ($parts.Count - 1) -and !$item.PSIsContainer) {
            throw "DLC path parent is not a directory: $current"
        }
        if ($item -and ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw "DLC path contains a reparse point: $current"
        }
    }
}

function Get-VenomOwnedDlcHash {
    param([string]$Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Assert-VenomOwnedDlc {
    param([string]$TitleRoot, [object[]]$Files)
    $root = [IO.Path]::GetFullPath($TitleRoot)
    foreach ($entry in $Files) {
        $file = Get-VenomOwnedDlcPath -Root $root -RelativePath $entry.Path
        if ($entry.SHA256 -notmatch '^[A-Fa-f0-9]{64}$') { throw "Invalid DLC manifest hash: $($entry.Path)" }
        Assert-VenomOwnedDlcNoReparse -Root $root -RelativePath $entry.Path
        if (!(Test-Path -LiteralPath $file -PathType Leaf)) {
            throw "Missing required owned DLC file: $($entry.Path)"
        }
        if ((Get-VenomOwnedDlcHash $file) -ne $entry.SHA256.ToUpperInvariant()) {
            throw "Owned DLC file hash differs: $($entry.Path)"
        }
    }
}

function Install-VenomOwnedDlc {
    param([string]$Source, [string]$UserDataRoot, [object[]]$Files)

    $sourceRoot = [IO.Path]::GetFullPath($Source)
    if (!(Test-Path -LiteralPath $sourceRoot -PathType Container)) { throw "Owned DLC source is missing: $sourceRoot" }
    $publicProfile = Join-Path ([IO.Path]::GetFullPath($UserDataRoot)) '0000000000000000'
    $target = Join-Path $publicProfile '45410915'

    # Validate the complete source manifest and every existing destination before
    # creating directories or publishing any file. Existing matching files are idempotent.
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    $plan = [Collections.Generic.List[object]]::new()
    foreach ($entry in $Files) {
        if (!$seen.Add([string]$entry.Path)) { throw "Duplicate DLC manifest path: $($entry.Path)" }
        $sourceFile = Get-VenomOwnedDlcPath -Root $sourceRoot -RelativePath $entry.Path
        $targetFile = Get-VenomOwnedDlcPath -Root $target -RelativePath $entry.Path
        if ($entry.SHA256 -notmatch '^[A-Fa-f0-9]{64}$') { throw "Invalid DLC manifest hash: $($entry.Path)" }
        $expected = $entry.SHA256.ToUpperInvariant()
        Assert-VenomOwnedDlcNoReparse -Root $sourceRoot -RelativePath $entry.Path
        if (!(Test-Path -LiteralPath $sourceFile -PathType Leaf)) { throw "Missing owned DLC source file: $($entry.Path)" }
        if ((Get-VenomOwnedDlcHash $sourceFile) -ne $expected) { throw "Owned DLC source hash differs: $($entry.Path)" }
        Assert-VenomOwnedDlcNoReparse -Root $target -RelativePath $entry.Path
        if (Test-Path -LiteralPath $targetFile) {
            if (!(Test-Path -LiteralPath $targetFile -PathType Leaf)) { throw "DLC destination is not a file: $($entry.Path)" }
            if ((Get-VenomOwnedDlcHash $targetFile) -ne $expected) { throw "Conflicting DLC destination file: $($entry.Path)" }
            $plan.Add([pscustomobject]@{ Source=$sourceFile; Target=$targetFile; Relative=$entry.Path; Expected=$expected })
        } else {
            $plan.Add([pscustomobject]@{ Source=$sourceFile; Target=$targetFile; Relative=$entry.Path; Expected=$expected })
        }
    }

    [IO.Directory]::CreateDirectory($publicProfile) | Out-Null
    [IO.Directory]::CreateDirectory($target) | Out-Null
    foreach ($item in $plan) {
        Assert-VenomOwnedDlcNoReparse -Root $target -RelativePath $item.Relative
        if (Test-Path -LiteralPath $item.Target) {
            if (!(Test-Path -LiteralPath $item.Target -PathType Leaf) -or
                (Get-VenomOwnedDlcHash $item.Target) -ne $item.Expected) {
                throw "Conflicting DLC destination file appeared during publication: $($item.Relative)"
            }
            continue
        }
        $parent = Split-Path -Parent $item.Target
        [IO.Directory]::CreateDirectory($parent) | Out-Null
        $temp = $item.Target + '.venom-' + [guid]::NewGuid().ToString('N') + '.tmp'
        try {
            Copy-Item -LiteralPath $item.Source -Destination $temp
            if ((Get-VenomOwnedDlcHash $temp) -ne $item.Expected) { throw "Staged DLC file hash differs: $($item.Relative)" }
            try {
                [IO.File]::Move($temp, $item.Target)
            } catch [IO.IOException] {
                # Another installer may have published the same file concurrently.
                if (!(Test-Path -LiteralPath $item.Target -PathType Leaf) -or
                    (Get-VenomOwnedDlcHash $item.Target) -ne $item.Expected) { throw }
            }
        } finally {
            if (Test-Path -LiteralPath $temp -PathType Leaf) { Remove-Item -LiteralPath $temp -Force }
        }
    }
    Assert-VenomOwnedDlc -TitleRoot $target -Files $Files
}
