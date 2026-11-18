# Source-built ReXGlue SDK

This recipe reconstructs the two SDK stacks from upstream Git objects at c94f5ebdcb3c9d1a460ca48e04f9758448f8d518, applies the hash-pinned source patches, and builds the runtime/CLI with LLVM23.1.4 and GPU with LLVM22.1.8. The source lock pins every submodule and patch/overlay input. No game inputs or generated title code are included.

The complete source workflow was qualified with Python 3.11.15. Use that tested version, Git, CMake, Ninja, the Windows SDK, MSVC link libraries, and the two specified LLVM toolchains; other Python versions remain unqualified. Start from a local clone of https://github.com/rexglue/rexglue-sdk.git. Check out the locked revision and populate its submodules. The recipe checks each required Git tree object before writing its fresh output directory; it copies no dirty working files. Use a local build volume with enough space for two complete source/build trees. Keep all upstream and dependency license notices with any redistributed source or binary.

```powershell
python -B native/sdk/build-sdk.py `
  --upstream C:/BuildCache/rexglue-sdk `
  --output D:/VenomBuild/sdk-r1 `
  --clang23 C:/LLVM23/bin/clang++.exe `
  --clang22 C:/LLVM22/bin/clang++.exe `
  --jobs 2
```

The source-cache directory is read only. Each output source tree references its pinned object store through Git alternates, so retain the source cache while using the build. Source files are exported using git archive, checked against the patch lock, and given pinned Git metadata because upstream CMake reads Git revisions. Windows symlinks named by the dependency audit are materialized before patching. GPU overlays are separate source headers with the relative CMake include fragment; all47overlay inputs are hash checked.

Use --stage-only to verify and reconstruct sources without configuring, or --configure-only to also configure both stacks without compiling. Neither option produces an SDK build receipt. An existing output is refused and preserved; choose a fresh namespace for retries. Logs and command arguments are retained beneath the output; completed receipts and copied artifacts are flushed and independently reopened.

A full run writes sdk-build-receipt.json, the runtime source/build/library locations, the fresh CLI hash, and sdk-bundle containing rexruntimerd.dll, rexgpu-xenosrd.dll, and TracyClientrd.dll. Bind the title recipe to these fresh hashes only after its codegen and static ABI checks pass. A source build does not establish gameplay, save, DLC, performance, SBS, browser, or release acceptance.

The exact patch baseline retains historical diagnostic paths and default-off diagnostic controls. Those controls have narrowly bound research paths and are not portable features. The separate DRED experiment is outside this baseline. No *.local audit/provenance files belong in this recipe.