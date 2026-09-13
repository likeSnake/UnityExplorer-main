param(
    [Parameter(Mandatory=$true)][string[]]$CatalogDirectories,
    [Parameter(Mandatory=$true)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
if (Test-Path -LiteralPath $OutputPath) { throw 'Verification output exists.' }
$checks = [Collections.Generic.List[object]]::new()
function Add-Check([string]$Name, [bool]$Ok) {
    $checks.Add([pscustomobject]@{ name = $Name; passed = $Ok })
}
foreach ($directory in $CatalogDirectories) {
    $dir = (Resolve-Path -LiteralPath $directory).ProviderPath
    $manifest = Get-Content -LiteralPath (Join-Path $dir 'manifest.json') -Raw -Encoding utf8 | ConvertFrom-Json
    foreach ($source in $manifest.sources) {
        if ($source.status -eq 'MISSING') { continue }
        Add-Check ('source ' + $source.file) ((Get-FileHash -LiteralPath $source.file -Algorithm SHA256).Hash -ieq $source.sha256)
    }
    foreach ($product in $manifest.products) {
        $path = Join-Path $dir $product.file
        Add-Check ('product ' + $path) (((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ieq $product.sha256) -and ((Get-Item -LiteralPath $path).Length -eq $product.bytes))
    }
    $report = Get-Content -LiteralPath (Join-Path $dir 'heroes.json') -Raw -Encoding utf8 | ConvertFrom-Json
    $heroes = @(Import-Csv -LiteralPath (Join-Path $dir 'heroes.tsv') -Delimiter "`t" -Encoding utf8)
    $skills = @(Import-Csv -LiteralPath (Join-Path $dir 'hero_skills.tsv') -Delimiter "`t" -Encoding utf8)
    Add-Check ('row counts ' + $dir) (($heroes.Count -eq $report.counts.config_records) -and ($skills.Count -eq $report.counts.hero_skill_links))
    Add-Check ('identity uniqueness ' + $dir) (@($heroes | Group-Object hero_id | Where-Object Count -gt 1).Count -eq 0)
    $combinedPath = Join-Path $dir 'dump_with_hero_catalog.cs'
    if (Test-Path -LiteralPath $combinedPath) {
        $composition = Get-Content -LiteralPath ($combinedPath + '.manifest.json') -Raw -Encoding utf8 | ConvertFrom-Json
        Add-Check ('composed digest ' + $dir) ((Get-FileHash -LiteralPath $combinedPath -Algorithm SHA256).Hash -ieq $composition.output_sha256)
        $sourceStream = [IO.File]::OpenRead($composition.source_dump)
        $combinedStream = [IO.File]::OpenRead($combinedPath)
        $sha = [Security.Cryptography.SHA256]::Create()
        try {
            $prefix = New-Object byte[] 3
            $read = $sourceStream.Read($prefix, 0, 3)
            if (!($read -eq 3 -and $prefix[0] -eq 239 -and $prefix[1] -eq 187 -and $prefix[2] -eq 191)) { $sourceStream.Position = 0 }
            $headerBytes = (Get-Item -LiteralPath (Join-Path $dir 'dump_header.cs')).Length
            $combinedStream.Position = $headerBytes
            $sourceBodyHash = [BitConverter]::ToString($sha.ComputeHash($sourceStream))
            $outputBodyHash = [BitConverter]::ToString($sha.ComputeHash($combinedStream))
            Add-Check ('original dump body unchanged ' + $dir) ($sourceBodyHash -eq $outputBodyHash)
        } finally { $sha.Dispose(); $sourceStream.Dispose(); $combinedStream.Dispose() }
    }
}
$failures = @($checks | Where-Object { !$_.passed })
$result = [ordered]@{passed=$checks.Count-$failures.Count; failed=$failures.Count; checks=@($checks.ToArray())}
$text = $result | ConvertTo-Json -Depth 6
$destination = [IO.Path]::GetFullPath($OutputPath)
[void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($destination))
$bytes = [Text.UTF8Encoding]::new($false).GetBytes($text)
$stream = [IO.File]::Open($destination, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None)
try { $stream.Write($bytes, 0, $bytes.Length) } finally { $stream.Dispose() }
"passed=$($result.passed) failed=$($result.failed)"
if ($failures.Count) { throw 'Artifact verification failed.' }
