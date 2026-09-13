param(
    [Parameter(Mandatory=$true)][string]$CatalogDirectory,
    [Parameter(Mandatory=$true)][string]$SourceDump,
    [Parameter(Mandatory=$true)][string]$OutputPath,
    [switch]$Legacy
)
$ErrorActionPreference = 'Stop'
if (!$Legacy) {
    throw '该脚本会把摘要写入 dump 副本，已停用默认行为。请使用 Export-HeroCatalogBundle.ps1；只有读取旧流程时才显式传入 -Legacy。'
}
$catalogPath = (Resolve-Path -LiteralPath $CatalogDirectory).ProviderPath
$sourcePath = (Resolve-Path -LiteralPath $SourceDump).ProviderPath
$destination = [IO.Path]::GetFullPath($OutputPath)
$sidecar = $destination + '.manifest.json'
if ((Test-Path -LiteralPath $destination) -or (Test-Path -LiteralPath $sidecar)) {
    throw 'Output exists; choose a new path.'
}
$manifest = Get-Content -LiteralPath (Join-Path $catalogPath 'manifest.json') -Raw -Encoding utf8 | ConvertFrom-Json
if ($manifest.status -ne 'COMPLETE_WITH_LIMITATIONS' -or $manifest.source_kind -ne 'FILE_SNAPSHOT') {
    throw 'Catalog is incomplete or has an unsupported source type.'
}
$battle = @($manifest.sources | Where-Object { [IO.Path]::GetFileName($_.file) -eq 'HeroConfig_HeroBattleConfig.tsv' })
if ($battle.Count -ne 1) { throw 'Missing or ambiguous source identity.' }
# Require the dump beside the config folder, so two different exports are not
# silently merged. This verifies file grouping, not game-version equivalence.
$sourceGroup = [IO.Directory]::GetParent([IO.Path]::GetDirectoryName($battle[0].file)).FullName
if ([IO.Path]::GetDirectoryName($sourcePath) -ine $sourceGroup) {
    throw 'Dump and configuration must come from the same export directory.'
}
$headerPath = Join-Path $catalogPath 'dump_header.cs'
$header = [IO.File]::ReadAllBytes($headerPath)
$headerRecord = @($manifest.products | Where-Object file -eq 'dump_header.cs')
$hasher = [Security.Cryptography.SHA256]::Create()
try { $headerHash = [BitConverter]::ToString($hasher.ComputeHash($header)).Replace('-', '').ToLowerInvariant() }
finally { $hasher.Dispose() }
if ($headerRecord.Count -ne 1 -or $headerRecord[0].sha256 -ne $headerHash) { throw 'Header digest mismatch.' }
$source = [IO.File]::Open($sourcePath, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
$destinationStream = $null
try {
    $sourceHash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($destination))
    $destinationStream = [IO.File]::Open($destination, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
    $destinationStream.Write($header, 0, $header.Length)
    $prefix = New-Object byte[] 3
    $read = $source.Read($prefix, 0, 3)
    if (!($read -eq 3 -and $prefix[0] -eq 239 -and $prefix[1] -eq 187 -and $prefix[2] -eq 191)) { $source.Position = 0 }
    $source.CopyTo($destinationStream)
    $destinationStream.Flush($true)
} finally {
    if ($destinationStream) { $destinationStream.Dispose() }
    $source.Dispose()
}
$result = [ordered]@{
    status = 'COMPOSED_FILE_SNAPSHOT'
    live_process_verified = $false
    source_dump = $sourcePath
    source_dump_sha256 = $sourceHash
    source_catalog = $catalogPath
    header_sha256 = $headerHash
    output = $destination
    output_sha256 = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash
    counts = $manifest.counts
}
$json = $result | ConvertTo-Json -Depth 8
$bytes = [Text.UTF8Encoding]::new($false).GetBytes($json)
$manifestStream = [IO.File]::Open($sidecar, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
try { $manifestStream.Write($bytes, 0, $bytes.Length); $manifestStream.Flush($true) }
finally { $manifestStream.Dispose() }
$json
