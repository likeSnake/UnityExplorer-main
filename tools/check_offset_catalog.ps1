param(
  [string]$OffsetHeader = 'F:\gua\myCode\Naraka\Offset.h',
  [string]$Catalog = "$PSScriptRoot\..\App\Il2CppRuntimeDumper\naraka_offset_catalog.hpp",
  [string]$Dump = "$PSScriptRoot\..\dump_workspace\inject_test\out_super\dump.cs",
  [string]$Report = "$PSScriptRoot\..\dump_workspace\validation\v7-offset-catalog.md"
)
$ErrorActionPreference = 'Stop'
$source = Get-Content -LiteralPath $OffsetHeader -Encoding UTF8
$ranges = [Collections.Generic.List[object]]::new()
$stack = [Collections.Generic.Stack[int]]::new()
for ($i = 0; $i -lt $source.Count; $i++) {
  if ($source[$i] -match '^\s*struct\s*$') { $stack.Push($i) }
  if ($source[$i] -match '^\s*}\s*(\w+)\s*;' -and $stack.Count) {
    $ranges.Add(@{Start=$stack.Pop(); End=$i; Name=$Matches[1]})
  }
}
$sourceKeys = [Collections.Generic.List[string]]::new()
$old = [Collections.Hashtable]::new([StringComparer]::Ordinal)
for ($i = 0; $i -lt $source.Count; $i++) {
  if ($source[$i] -match '^\s*(?:inline\s+(?:constexpr\s+)?(?:uint64_t|uint32_t)|DWORD)\s+(\w+)\s*=') {
    $name=$Matches[1]
    $parents=@($ranges | Where-Object { $_.Start -lt $i -and $_.End -gt $i } | Sort-Object Start | ForEach-Object { $_.Name })
    $key = (@($parents)+@($name)) -join '.'
    $sourceKeys.Add($key)
    $value = ''; $mapping = ''
    if ($source[$i] -match '=\s*([^;]+);') { $value=$Matches[1].Trim() }
    if ($source[$i] -match 'Class:\s*(.+?)\s*$') { $mapping=$Matches[1].Trim() }
    $old[$key]=@{Value=$value;Mapping=$mapping;Line=$i+1}
  }
}
$specs = @(Get-Content -LiteralPath $Catalog -Encoding UTF8 | ForEach-Object {
  if ($_ -match '^\s*\{"([^"]+)", OffsetKind::(\w+), "([^"]*)", "([^"]*)", "([^"]*)", (\d+), (-?\d+),') {
    [PSCustomObject]@{Key=$Matches[1];Kind=$Matches[2];Class=$Matches[3];NS=$Matches[4];Member=$Matches[5];Variant=[int]$Matches[6];Arg=[int]$Matches[7]}
  }
})
$diff = @(Compare-Object @($sourceKeys) @($specs.Key) -CaseSensitive)
if ($diff.Count -or (@($specs.Key | Sort-Object -Unique).Count -ne $specs.Count)) {
  throw "Offset.h/catalog coverage mismatch: $($diff | Out-String)"
}
$wanted = [Collections.Hashtable]::new([StringComparer]::Ordinal)
foreach ($s in $specs) { if ($s.Class) { $wanted[$s.Class]=$true } }
$classes = [Collections.Hashtable]::new([StringComparer]::Ordinal)
$current = $null
$previousRva = ''
$active = if ((Get-Content -LiteralPath $Dump -Encoding UTF8 -TotalCount 20) -match '^// Module: GameAssembly_Super') { 2 } else { 1 }
$reader = [IO.StreamReader]::new((Resolve-Path -LiteralPath $Dump), [Text.Encoding]::UTF8)
try {
  while (($line = $reader.ReadLine()) -ne $null) {
    if ($line -match '^// Class: (.+?)\s*$') {
      $current=$null
      if ($wanted.ContainsKey($Matches[1])) {
        $current=@{Name=$Matches[1];NS='';Fields=[Collections.Hashtable]::new([StringComparer]::Ordinal);Methods=[Collections.Hashtable]::new([StringComparer]::Ordinal);Parent='';Rva=''}
      }
      continue
    }
    if (!$current) { continue }
    if ($line -match '^// Namespace: (.*)$') {
      $current.NS=$Matches[1].Trim()
      $id=$current.NS+'|'+$current.Name
      if (!$classes.ContainsKey($id)) { $classes[$id]=[Collections.Generic.List[object]]::new() }
      $classes[$id].Add($current)
    }
    if ($line -match '^(?:\w+ )*(?:class|struct) \S+ : ([^\s,]+)') { $current.Parent=$Matches[1] }
    if ($line -match '^// RVA: 0x([0-9a-fA-F]+)\s*$') { $current.Rva='0x'+$Matches[1].ToUpperInvariant() }
    if ($line -match '^\s+(.+?) (\S+); // offset:0x([0-9a-fA-F]+)') {
      $current.Fields[$Matches[2]]=@{Value='0x'+$Matches[3].ToUpperInvariant();Declaration=$Matches[1]+' '+$Matches[2];Member=$Matches[2]}
    }
    if ($line -match '^\s+// RVA: 0x([0-9a-fA-F]*) VA:') { $previousRva=$Matches[1] }
    if ($line -match '^\s+.+? (\S+)\((.*?)\) \{ \}') {
      $name=$Matches[1]; $params=$Matches[2]; $count=0
      if ($params) { $count=($params -split ',').Count }
      $id=$name+'|'+$count
      if (!$current.Methods.ContainsKey($id)) { $current.Methods[$id]=[Collections.Generic.List[object]]::new() }
      $types = @($params -split ',\s*' | ForEach-Object { ($_ -split '\s+')[0] }) -join ','
      $current.Methods[$id].Add(@{Value='0x'+$previousRva.ToUpperInvariant();Types=$types})
      $previousRva=''
    }
  }
} finally { $reader.Dispose() }

# Case-sensitive member matching and bounded parent traversal, just like the DLL.
function Find-Member($c, $s, $visited) {
  $id=$c.NS+'|'+$c.Name
  if ($visited.Count -ge 32 -or $visited.Contains($id)) { return $null }
  $null=$visited.Add($id)
  if ($s.Kind -eq 'TypeInfo') {
    if ($c.Rva -and (!$s.Member -or $c.Fields.ContainsKey($s.Member))) {
      return @{Value=$c.Rva;Member='TypeInfo RVA';Owner=$c.Name;Declaration=''}
    }
    return $null
  } elseif ($s.Kind -eq 'Field') {
    foreach ($candidate in @($s.Member,('<'+$s.Member+'>k__BackingField'))) {
      if ($c.Fields.ContainsKey($candidate)) {
        $f=$c.Fields[$candidate]
        return @{Value=$f.Value;Member=$f.Member;Owner=$c.Name;Declaration=$f.Declaration}
      }
    }
  } else {
    $mid=$s.Member+'|'+$s.Arg
    if ($c.Methods.ContainsKey($mid)) {
      $methods=@($c.Methods[$mid] | Where-Object {
        $s.Member -cne 'CrossFadeInFixedTime' -or $_.Types -ceq 'Int32,Single,Int32,Single,Single,TransitionSourceType,Boolean'
      })
      if ($methods.Count) {
        return @{Value=$(if ($methods.Count -eq 1) {$methods[0].Value} else {'AMBIGUOUS_METHOD'});Member=$s.Member;Owner=$c.Name;Declaration=$s.Member}
      }
    }
  }
  if ($c.Parent) {
    $parentKey=$c.NS+'|'+$c.Parent
    if ($classes.ContainsKey($parentKey) -and $classes[$parentKey].Count -eq 1) { return Find-Member $classes[$parentKey][0] $s $visited }
  }
  return $null
}
function Normalize-Hex([string]$v) {
  if (!$v) { return '' }
  if ($v -match '^0x([0-9a-fA-F]+)$') { return '0x'+([Convert]::ToUInt64($Matches[1],16)).ToString('X') }
  if ($v -match '^\d+$') { return '0x'+([uint64]$v).ToString('X') }
  return $v
}
$lines=[Collections.Generic.List[string]]::new()
$lines.Add('# v7 Offset.h 映射核对')
$lines.Add('')
$lines.Add('这是对既有 dump 的离线映射审计，不是 v7 DLL 实机运行结果。')
$lines.Add('')
$lines.Add("Source: $Dump")
$lines.Add("Inventory: $($sourceKeys.Count) / $($specs.Count), exact coverage.")
$lines.Add('')
$lines.Add('大小写严格匹配；同名类按字段唯一命中消歧；CrossFade 校验七参数类型；另一变体的方法不作比较。')
$lines.Add('')
$lines.Add('| Offset.h 变量 | 当前 dump 类/成员 | dump 值 | Offset.h 原值 | 对比 |')
$lines.Add('|---|---|---|---|---|')
$matched=0; $missing=0; $unavailable=0
$mismatches=[Collections.Generic.List[string]]::new()
$mappingChanges=[Collections.Generic.List[string]]::new()
foreach ($s in $specs | Where-Object { $_.Kind -in 'Field','Method','TypeInfo' }) {
  $id=$s.NS+'|'+$s.Class
  $result='NOT_FOUND'; $member=$s.Member
  $declaration=''; $owner=$s.Class; $comparison='unresolved'
  if ($s.Variant -and $s.Variant -ne $active) { $result='OTHER_VARIANT'; $unavailable++ }
  elseif ($classes.ContainsKey($id)) {
    $hits=@(foreach ($c in $classes[$id]) {
      $hit=Find-Member $c $s ([Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal))
      if ($hit) { $hit }
    })
    if ($hits.Count -gt 1) { $result='AMBIGUOUS_CLASS' }
    elseif ($hits.Count -eq 1) {
      $result=$hits[0].Value; $member=$hits[0].Member; $owner=$hits[0].Owner; $declaration=$hits[0].Declaration
    }
  }
  $oldValue=Normalize-Hex $old[$s.Key].Value
  if ($result -match '^0x[0-9A-F]+$') {
    $matched++; $result=Normalize-Hex $result
    $comparison=if($oldValue -notmatch '^0x[0-9A-F]+$'){'SYMBOLIC_INITIALIZER'}elseif($result -ceq $oldValue){'SAME'}else{'CHANGED'}
    $changed=$comparison -eq 'CHANGED'
    if ($s.Kind -eq 'Field' -and $owner.Contains([string][char]96) -and
        $declaration -cnotmatch '\bstatic\b' -and [Convert]::ToUInt64($result.Substring(2),16) -lt 16) {
      $comparison='UNRESOLVED_LAYOUT (raw dump value; do not apply)'; $matched--; $missing++
    }
    if ($changed) { $mismatches.Add('| '+$s.Key+' | '+$oldValue+' | '+$result+' | '+$owner+' -> '+$member+' | '+$comparison+' |') }
    if ($s.Kind -eq 'Field' -and $old[$s.Key].Mapping) {
      $actual=$owner+'->'+($declaration -replace '^(?:(?:public|private|protected|internal|static|readonly|const)\s+)+','')
      if ($actual -cne $old[$s.Key].Mapping) { $mappingChanges.Add('| '+$s.Key+' | '+$old[$s.Key].Mapping+' | '+$actual+' |') }
    }
  } elseif ($result -ne 'OTHER_VARIANT') { $missing++; Write-Output "$($s.Key): $result $($s.Class).$($s.Member)" }
  $lines.Add('| '+$s.Key+' | '+$s.NS+'.'+$owner+' -> '+$member+' | '+$result+' | '+$oldValue+' | '+$comparison+' |')
}
$lines.Add('')
$lines.Add("Managed mappings: matched=$matched missing/ambiguous=$missing other-variant=$unavailable.")
$lines.Add('')
$lines.Add('## 数值不符（列出 dump 原值；有效性看状态，禁止跨变体复制）')
$lines.Add('')
$lines.Add('| 变量 | Offset.h 原值 | dump 值 | 依据 | 状态 |')
$lines.Add('|---|---|---|---|---|')
foreach ($row in $mismatches) { $lines.Add($row) }
$lines.Add('')
$lines.Add('## 注释映射不符（精确声明以 dump 为准）')
$lines.Add('')
$lines.Add('| 变量 | Offset.h 注释 | dump 声明 |')
$lines.Add('|---|---|---|')
foreach ($row in $mappingChanges) { $lines.Add($row) }
$reportPath=[IO.Path]::GetFullPath($Report)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($reportPath)) | Out-Null
[IO.File]::WriteAllLines($reportPath,$lines,[Text.UTF8Encoding]::new($false))
Write-Output "inventory=$($specs.Count) managed matched=$matched missing/ambiguous=$missing other-variant=$unavailable value-changes=$($mismatches.Count) mapping-changes=$($mappingChanges.Count) report=$reportPath"
