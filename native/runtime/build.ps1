$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$workspace = Resolve-Path (Join-Path $root '..\..')
$ninja = Join-Path $workspace 'research\toolchain\ninja\ninja.exe'
$clang = 'C:\LLVM\bin\clang++.exe'
$rc = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\rc.exe'
$build = Join-Path $PSScriptRoot 'build-ninja'
foreach ($tool in @($ninja, $clang, $rc)) { if (-not (Test-Path $tool)) { throw "Missing required tool: $tool" } }
$ninjaForward = $ninja -replace '\\','/'
$clangForward = $clang -replace '\\','/'
$rcForward = $rc -replace '\\','/'
& cmake -S $PSScriptRoot -B $build -G Ninja `
  "-DCMAKE_MAKE_PROGRAM=$ninjaForward" `
  "-DCMAKE_CXX_COMPILER=$clangForward" `
  "-DCMAKE_RC_COMPILER=$rcForward"
cmake --build $build
ctest --test-dir $build --output-on-failure
cargo build --manifest-path (Join-Path $PSScriptRoot 'Cargo.toml') --release
