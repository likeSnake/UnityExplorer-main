param(
    [Parameter(Mandatory=$true)][ValidateRange(1,2147483647)][int]$TargetProcessId,
    [Parameter(Mandatory=$true)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
if ([IntPtr]::Size -ne 8) { throw '64-bit PowerShell is required.' }
if (Test-Path -LiteralPath $OutputPath) { throw 'Output already exists; choose a new evidence path.' }
$target = Get-CimInstance Win32_Process -Filter "ProcessId = $TargetProcessId"
if (!$target -or $target.Name -ine 'NarakaBladepoint.exe') { throw 'PID does not belong to the target game.' }
if (!('HeroCatalogReadAccess' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class HeroCatalogReadAccess {
    [DllImport("kernel32.dll", SetLastError=true)]
    public static extern IntPtr OpenProcess(uint access, bool inherit, uint pid);
    [DllImport("kernel32.dll", SetLastError=true)]
    public static extern bool CloseHandle(IntPtr process);
    [DllImport("kernel32.dll", SetLastError=true, CharSet=CharSet.Unicode)]
    public static extern bool QueryFullProcessImageName(IntPtr process, uint flags, StringBuilder path, ref uint size);
    [DllImport("psapi.dll", SetLastError=true)]
    public static extern bool EnumProcessModulesEx(IntPtr process, [Out] IntPtr[] modules, uint bytes, out uint needed, uint filter);
    [DllImport("psapi.dll", SetLastError=true, CharSet=CharSet.Unicode)]
    public static extern uint GetModuleFileNameEx(IntPtr process, IntPtr module, StringBuilder path, uint size);
    [DllImport("kernel32.dll", SetLastError=true)]
    public static extern bool ReadProcessMemory(IntPtr process, IntPtr address, [Out] byte[] buffer, UIntPtr size, out UIntPtr read);
}
'@
}
$result = [ordered]@{
    timestamp = [DateTimeOffset]::Now.ToString('o')
    pid = $TargetProcessId
    process_name = $target.Name
    process_start = $target.CreationDate.ToString('o')
    access = 'PROCESS_QUERY_INFORMATION | PROCESS_VM_READ (0x410)'
    read_only = $true
    status = 'NOT_RUN'
    modules = @()
}
$processHandle = [HeroCatalogReadAccess]::OpenProcess(0x410, $false, $TargetProcessId)
$openError = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
try {
    if ($processHandle -eq [IntPtr]::Zero) {
        $result.status = 'OPEN_FAILED'
        $result.win32_error = $openError
    } else {
        $imagePath = New-Object Text.StringBuilder 32768
        [uint32]$pathSize = $imagePath.Capacity
        if ([HeroCatalogReadAccess]::QueryFullProcessImageName($processHandle, 0, $imagePath, [ref]$pathSize)) {
            $result.image_path = $imagePath.ToString()
        } else { $result.query_image_error = [Runtime.InteropServices.Marshal]::GetLastWin32Error() }
        $moduleHandles = New-Object IntPtr[] 4096
        [uint32]$needed = 0
        $ok = [HeroCatalogReadAccess]::EnumProcessModulesEx($processHandle, $moduleHandles, 32768, [ref]$needed, 3)
        $enumError = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
        $result.module_bytes_reported = $needed
        if (!$ok) {
            $result.status = 'MODULE_ENUM_FAILED'
            $result.win32_error = $enumError
        } elseif ($needed -eq 0) {
            $result.status = 'MODULE_ENUM_EMPTY'
        } elseif ($needed -gt 32768) {
            $result.status = 'MODULE_ENUM_LIMIT'
        } else {
            for ($i=0; $i -lt ($needed / 8); $i++) {
                $modulePath = New-Object Text.StringBuilder 32768
                $length = [HeroCatalogReadAccess]::GetModuleFileNameEx($processHandle, $moduleHandles[$i], $modulePath, $modulePath.Capacity)
                if ($length -eq 0) { continue }
                $name = [IO.Path]::GetFileName($modulePath.ToString())
                if ($name -notmatch '^(GameAssembly.*\.dll|UnityPlayer.*\.dll|NarakaBladepoint\.exe)$') { continue }
                $header = New-Object byte[] 4096
                [UIntPtr]$bytesRead = [UIntPtr]::Zero
                $readOk = [HeroCatalogReadAccess]::ReadProcessMemory($processHandle, $moduleHandles[$i], $header, [UIntPtr]4096, [ref]$bytesRead)
                $readError = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
                $entry = [ordered]@{
                    name=$name; path=$modulePath.ToString(); base=('0x{0:X}' -f $moduleHandles[$i].ToInt64())
                    header_read_ok=$readOk; bytes_read=$bytesRead.ToUInt64()
                }
                if ($readOk -and $bytesRead.ToUInt64() -eq 4096 -and $header[0] -eq 77 -and $header[1] -eq 90) {
                    $nt = [BitConverter]::ToInt32($header, 60)
                    if ($nt -ge 64 -and $nt -le 4000 -and [BitConverter]::ToUInt32($header, $nt) -eq 17744) {
                        $entry.pe_timestamp = '0x{0:X8}' -f [BitConverter]::ToUInt32($header, $nt+8)
                        $entry.image_size = '0x{0:X}' -f [BitConverter]::ToUInt32($header, $nt+80)
                    }
                } else { $entry.read_error = $readError }
                $result.modules += [pscustomobject]$entry
            }
            $result.status = if (@($result.modules | Where-Object {$_.name -like 'GameAssembly*'}).Count) {'MODULE_IDENTIFIED'} else {'GAME_MODULE_NOT_FOUND'}
        }
    }
} finally {
    if ($processHandle -ne [IntPtr]::Zero) { [void][HeroCatalogReadAccess]::CloseHandle($processHandle) }
}
$destination = [IO.Path]::GetFullPath($OutputPath)
[void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($destination))
$result | ConvertTo-Json -Depth 8 | Out-File -LiteralPath $destination -Encoding utf8
$result | ConvertTo-Json -Depth 8
