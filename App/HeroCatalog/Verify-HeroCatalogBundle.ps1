param(
    [Parameter(Mandatory=$true)][string]$BundleDirectory,
    [Parameter(Mandatory=$true)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
if (Test-Path -LiteralPath $OutputPath) { throw 'Verification output exists.' }
$bundle = (Resolve-Path -LiteralPath $BundleDirectory).ProviderPath
$manifest = Get-Content -LiteralPath (Join-Path $bundle 'bundle_manifest.json') -Raw -Encoding utf8 | ConvertFrom-Json
$checks = [Collections.Generic.List[object]]::new()
function Check([string]$Name, [bool]$Passed) { $checks.Add([pscustomobject]@{name=$Name;passed=$Passed}) }
Check 'dump_modified=false' ($manifest.dump_modified -eq $false)
$dump = Get-Item -LiteralPath $manifest.source_dump.path -ErrorAction SilentlyContinue
Check 'source dump exists' ($null -ne $dump)
if ($dump) {
    Check 'source dump size' ($dump.Length -eq $manifest.source_dump.bytes)
    Check 'source dump sha256' ((Get-FileHash -LiteralPath $dump.FullName -Algorithm SHA256).Hash -ieq $manifest.source_dump.sha256)
} else {
    Check 'source dump size' $false
    Check 'source dump sha256' $false
}
foreach ($product in $manifest.products) {
    $path = Join-Path $bundle $product.file
    $exists = Test-Path -LiteralPath $path
    $same = $false
    if ($exists) { $info = Get-Item -LiteralPath $path; $same = $info.Length -eq $product.bytes -and (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ieq $product.sha256 }
    Check ('product ' + $product.file) ($exists -and $same)
}
$result = [ordered]@{passed=@($checks | Where-Object passed).Count;failed=@($checks | Where-Object { !$_.passed }).Count;checks=@($checks.ToArray())}
$destination = [IO.Path]::GetFullPath($OutputPath)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($destination)) | Out-Null
[IO.File]::WriteAllText($destination, ($result | ConvertTo-Json -Depth 8) + "`n", [Text.UTF8Encoding]::new($false))
"passed=$($result.passed) failed=$($result.failed)"
if ($result.failed) { throw 'Bundle verification failed.' }
