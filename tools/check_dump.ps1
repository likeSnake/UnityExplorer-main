# check_dump.ps1 — dump.cs 产物质量体检
#
# 用法：
#   pwsh -File tools\check_dump.ps1 -Path F:\gua\UnityExplorer-main\dump_workspace\inject_test\out\dump.cs
#   pwsh -File tools\check_dump.ps1 -Path <super dump> -Label super
#
# 输出：规模统计 + 质量指标（修饰符分布 / 接口垃圾率 / 类型名退化率 / 参数名来源）
param(
  [Parameter(Mandatory = $true)][string]$Path,
  [string]$Label = "dump"
)

if (-not (Test-Path $Path)) { Write-Error "not found: $Path"; exit 1 }

$img = 0; $cls = 0; $fldBlock = 0; $methBlock = 0; $propBlock = 0; $lines = 0
$rvaNon = 0; $rvaEmpty = 0
$ser = 0
$clsMods = @{}
$classTypeFallback = 0   # " Class " 类型名退化
$type0 = 0               # "Type0" 类型名退化
$ifaceLines = 0; $ifaceDupLines = 0; $ifaceGarbageLines = 0
$synthParams = 0; $realParams = 0
$slotLines = 0

$sr = [System.IO.StreamReader]::new($Path)
try {
  while (($l = $sr.ReadLine()) -ne $null) {
    $lines++
    if ($l.StartsWith('// Image ')) { $img++; continue }
    if ($l.StartsWith('// Class: ')) { $cls++; continue }
    if ($l -eq '[Serializable]') { $ser++; continue }
    if ($l -match '^\s*// Fields') { $fldBlock++; continue }
    if ($l -match '^\s*// Methods:') { $methBlock++; continue }
    if ($l -match '^\s*// Properties') { $propBlock++; continue }

    if ($l.TrimStart().StartsWith('// RVA:')) {
      if ($l -match 'RVA: 0x([0-9A-Fa-f]+)') { $rvaNon++ } else { $rvaEmpty++ }
      if ($l -match 'Slot:') { $slotLines++ }
      continue
    }

    if ($l -match '^(?<m>(?:(?:public|internal|private|protected)\s+)?(?:(?:static|sealed|abstract)\s+)*)(?<k>class|struct|interface|enum)\s') {
      $key = ($matches['m'] + $matches['k']).Trim()
      if ($clsMods.ContainsKey($key)) { $clsMods[$key]++ } else { $clsMods[$key] = 1 }
      if ($l -match ' : ') {
        $ifaceLines++
        $bases = ($l -split ' : ')[1] -split ' // ' | Select-Object -First 1
        $items = $bases -split ',\s*'
        if ($items.Count -gt 1) {
          $uniq = $items | Sort-Object -Unique
          if ($uniq.Count -ne $items.Count) { $ifaceDupLines++ }
        }
        if ($bases -match '(^|,\s*)(Object|ValueType|Enum)(\s*,|$)' -or $bases -match '[^A-Za-z0-9_<>`,\.\-]') {
          $ifaceGarbageLines++
        }
      }
      continue
    }

    if ($l -match '\bClass\b' -and $l -match '^\s+(public|private|protected|internal)') { $classTypeFallback++ }
    if ($l -match '\bType0\b') { $type0++ }
    if ($l -match '\(\s*(?:ref |out )?\S+\s+p\d+') { $synthParams++ }
    elseif ($l -match '\(\s*(?:ref |out )?\S+\s+[a-zA-Z_]\w*') { $realParams++ }
  }
} finally { $sr.Close() }

$sizeMB = [math]::Round((Get-Item $Path).Length / 1MB, 1)
Write-Host "===== $Label =====" -ForegroundColor Cyan
Write-Host ("file            : {0}  ({1} MB)" -f $Path, $sizeMB)
Write-Host ("lines           : {0}" -f $lines)
Write-Host ("images          : {0}" -f $img)
Write-Host ("classes         : {0}" -f $cls)
Write-Host ("field blocks    : {0}   method blocks: {1}   property blocks: {2}" -f $fldBlock, $methBlock, $propBlock)
Write-Host ("RVA non-empty   : {0}   RVA empty: {1}   Slot lines: {2}" -f $rvaNon, $rvaEmpty, $slotLines)
Write-Host ""
Write-Host "--- 类修饰符分布（前 10）---"
$clsMods.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 10 | ForEach-Object {
  Write-Host ("  {0,-42} {1}" -f $_.Key, $_.Value)
}
Write-Host ""
Write-Host "--- 质量指标 ---"
$internalPct = if ($cls -gt 0) { [math]::Round(100.0 * ($clsMods['internal class'] + $clsMods['internal struct'] + $clsMods['internal enum'] + $clsMods['internal interface']) / $cls, 1) } else { 0 }
Write-Host ("internal 占比    : {0} %   （越低越好；旧版实测 ~93%）" -f $internalPct)
Write-Host ("[Serializable]   : {0}   （旧版为 0 = flags 偏移错）" -f $ser)
Write-Host ("基类/接口行       : {0}   其中重复项: {1}   含垃圾项: {2}" -f $ifaceLines, $ifaceDupLines, $ifaceGarbageLines)
Write-Host ("类型名退化 Class  : {0}   Type0: {1}   （旧版 219707 / 8893）" -f $classTypeFallback, $type0)
Write-Host ("参数名 合成/真实  : {0} / {1}" -f $synthParams, $realParams)
