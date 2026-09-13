param(
    [Parameter(Mandatory=$true)][string]$CatalogDirectory,
    [Parameter(Mandatory=$true)][string]$SourceDump,
    [Parameter(Mandatory=$false)][string]$OutputRoot
)
$ErrorActionPreference = 'Stop'

$catalogPath = (Resolve-Path -LiteralPath $CatalogDirectory).ProviderPath
$sourcePath = (Resolve-Path -LiteralPath $SourceDump).ProviderPath
$root = if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    [IO.Path]::GetDirectoryName($sourcePath)
} else {
    [IO.Path]::GetFullPath($OutputRoot)
}
$catalogManifestPath = Join-Path $catalogPath 'manifest.json'
if (!(Test-Path -LiteralPath $catalogManifestPath)) { throw 'Hero catalog manifest is missing.' }
$catalogManifest = Get-Content -LiteralPath $catalogManifestPath -Raw -Encoding utf8 | ConvertFrom-Json
if ($catalogManifest.status -ne 'COMPLETE_WITH_LIMITATIONS' -or $catalogManifest.source_kind -ne 'FILE_SNAPSHOT') {
    throw 'Catalog is incomplete or has an unsupported source type.'
}
$sourceName = [IO.Path]::GetFileName($sourcePath)
$sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash.ToLowerInvariant()
$sourceInfo = [ordered]@{ path = $sourcePath; file = $sourceName; sha256 = $sourceHash; bytes = (Get-Item -LiteralPath $sourcePath).Length }
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss_fff'
[void][IO.Directory]::CreateDirectory($root)
$bundle = $null
for ($suffix = 0; $suffix -lt 100; $suffix++) {
    $candidate = Join-Path $root ("{0}_runtime_info_{1}_{2}" -f [IO.Path]::GetFileNameWithoutExtension($sourcePath), $stamp, $suffix)
    if ([IO.Directory]::CreateDirectory($candidate)) { $bundle = $candidate; break }
}
if (!$bundle) { throw 'Cannot allocate a unique runtime information directory.' }

$requiredProducts = @('heroes.json','heroes.tsv','hero_skills.tsv','hero_action_links.tsv','action_states.tsv','dump_header.cs','offset_log.txt')
$products = [Collections.Generic.List[object]]::new()
try {
    foreach ($name in $requiredProducts) {
        $source = Join-Path $catalogPath $name
        if (!(Test-Path -LiteralPath $source)) { throw "Catalog product is missing: $name" }
        $destination = Join-Path $bundle $name
        Copy-Item -LiteralPath $source -Destination $destination -Force:$false
        $info = Get-Item -LiteralPath $destination
        $products.Add([ordered]@{file=$name; bytes=$info.Length; sha256=(Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()})
    }
    $readme = @"
# Runtime information bundle

This directory contains structured hero and action information associated with:

- dump file: `$sourceName`
- dump SHA-256: `$sourceHash`
- catalog source: `$catalogPath`

The original dump file was not modified. `heroes.json` records the source kind and
the limitations of the snapshot. Unknown runtime/open-state/action-hash values stay
null. Verify every file against `bundle_manifest.json` before consuming it.
"@
    $readmePath = Join-Path $bundle 'README.md'
    [IO.File]::WriteAllText($readmePath, $readme, [Text.UTF8Encoding]::new($false))
    $products.Add([ordered]@{file='README.md'; bytes=(Get-Item -LiteralPath $readmePath).Length; sha256=(Get-FileHash -LiteralPath $readmePath -Algorithm SHA256).Hash.ToLowerInvariant()})
    $bundleManifest = [ordered]@{
        schema_version = 1
        status = 'COMPLETE_WITH_LIMITATIONS'
        source_kind = 'FILE_SNAPSHOT'
        dump_modified = $false
        source_dump = $sourceInfo
        catalog_directory = $catalogPath
        catalog_manifest_sha256 = (Get-FileHash -LiteralPath $catalogManifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
        counts = $catalogManifest.counts
        products = @($products)
        exported_at_local = [DateTimeOffset]::Now.ToString('o')
    }
    $manifestPath = Join-Path $bundle 'bundle_manifest.json'
    [IO.File]::WriteAllText($manifestPath, ($bundleManifest | ConvertTo-Json -Depth 10) + "`n", [Text.UTF8Encoding]::new($false))
    [ordered]@{ bundle = $bundle; dump_modified = $false; source_dump = $sourceInfo; products = @($products); counts = $catalogManifest.counts } |
        ConvertTo-Json -Depth 10
} catch {
    try { [IO.File]::WriteAllText((Join-Path $bundle 'FAILED.txt'), "Bundle creation failed: $($_.Exception.Message)`r`n", [Text.UTF8Encoding]::new($false)) } catch {}
    throw
}
