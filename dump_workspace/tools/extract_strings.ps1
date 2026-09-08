param(
  [Parameter(Mandatory=$true)][string]$Path,
  [int]$MinLen = 5,
  [string]$Out = ""
)
$bytes = [System.IO.File]::ReadAllBytes($Path)

function Get-Ptr([int]$offset) {
  return [BitConverter]::ToInt32($bytes, $offset)
}
function Get-U16([int]$offset) {
  return [BitConverter]::ToUInt16($bytes, $offset)
}
function Get-U32([int]$offset) {
  return [BitConverter]::ToUInt32($bytes, $offset)
}

# Parse PE
$lfanew = Get-Ptr 0x3C
if (-not ($bytes[0] -eq 0x4D -and $bytes[1] -eq 0x5A)) { Write-Output "no MZ"; return }
$optSize = Get-U16 ($lfanew + 20)
$nsec = Get-U16 ($lfanew + 6)
$secStart = $lfanew + 24 + $optSize

# sections
$sections = @()
for ($i = 0; $i -lt $nsec; $i++) {
  $off = $secStart + $i * 40
  $name = [System.Text.Encoding]::ASCII.GetString($bytes, $off, 8).TrimEnd([char]0)
  $vsize = Get-U32 ($off + 8)
  $vaddr = Get-U32 ($off + 12)
  $rawsize = Get-U32 ($off + 16)
  $rawptr = Get-U32 ($off + 20)
  $sections += [pscustomobject]@{ Name=$name; VA=$vaddr; VSize=$vsize; RawPtr=$rawptr; RawSize=$rawsize }
}
Write-Output "=== SECTIONS ==="
$sections | ForEach-Object { Write-Output ("  {0} VA=0x{1:X} VSize=0x{2:X} RawPtr=0x{3:X} RawSize=0x{4:X}" -f $_.Name,$_.VA,$_.VSize,$_.RawPtr,$_.RawSize) }

# Extract printable strings (pure byte scan, no regex filtering that breaks PS)
$results = @()
foreach ($sec in $sections) {
  if ($sec.RawSize -eq 0 -or $sec.RawPtr -lt 0 -or $sec.RawPtr + $sec.RawSize -gt $bytes.Length) { continue }
  $slice = New-Object 'System.Byte[]' $sec.RawSize
  [Array]::Copy($bytes, $sec.RawPtr, $slice, 0, $sec.RawSize)

  # ASCII strings
  $cur = New-Object System.Text.StringBuilder
  $count = 0
  foreach ($b in $slice) {
    if ($b -ge 0x20 -and $b -lt 0x7F) {
      [void]$cur.Append([char]$b)
    } else {
      if ($cur.Length -ge $MinLen) {
        $results += [pscustomobject]@{ Sec=$sec.Name; Kind="ASCII"; Str=$cur.ToString() }
        $count++
      }
      [void]$cur.Clear()
    }
  }
  if ($cur.Length -ge $MinLen) {
    $results += [pscustomobject]@{ Sec=$sec.Name; Kind="ASCII"; Str=$cur.ToString() }
  }

  # UTF-16 strings (little endian, printable in high byte 0)
  $cur = New-Object System.Text.StringBuilder
  $n = $slice.Length - 1
  for ($i = 0; $i -lt $n; $i += 2) {
    $lo = $slice[$i]; $hi = $slice[$i+1]
    if ($hi -eq 0 -and $lo -ge 0x20 -and $lo -lt 0x7F) {
      [void]$cur.Append([char]$lo)
    } else {
      if ($cur.Length -ge $MinLen) {
        $results += [pscustomobject]@{ Sec=$sec.Name; Kind="UTF16"; Str=$cur.ToString() }
      }
      [void]$cur.Clear()
    }
  }
}

Write-Output ""
Write-Output "=== STRINGS (dedup, len<=300) ==="
$results | Where-Object { $_.Str.Length -le 300 } | Sort-Object Sec,Str -Unique | ForEach-Object {
  Write-Output ("  [{0}/{1}] {2}" -f $_.Sec, $_.Kind, $_.Str)
}

if ($Out) {
  $results | Where-Object { $_.Str.Length -le 300 } | Sort-Object Sec,Str -Unique |
    ForEach-Object { "{0}`t{1}`t{2}" -f $_.Sec, $_.Kind, $_.Str } | Set-Content -Path $Out -Encoding UTF8
  Write-Output ""
  Write-Output "Wrote strings to $Out"
}