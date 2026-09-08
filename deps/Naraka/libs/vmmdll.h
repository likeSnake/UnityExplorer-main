// vmmdll.h : header file to include in projects that use vmm.dll / vmm.so
// 
// Please also consult the guide at: https://github.com/ufrisk/MemProcFS/wiki
// 
// U/W functions
// =============
// Windows may access both UTF-8 *U and Wide-Char *W versions of functions
// while Linux may only access UTF-8 versions. Some functionality may also
// be degraded or unavailable on Linux.
//
// (c) Ulf Frisk, 2018-2024
// Author: Ulf Frisk, pcileech@frizk.net
//
// Header Version: 5.9
//

#include "leechcore.h"

#ifndef __VMMDLL_H__
#define __VMMDLL_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _WIN32

#include <Windows.h>
#undef EXPORTED_FUNCTION
#define EXPORTED_FUNCTION
typedef unsigned __int64                    QWORD, *PQWORD;

#endif /* _WIN32 */
#ifdef LINUX

#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#undef EXPORTED_FUNCTION
#define EXPORTED_FUNCTION                   __attribute__((visibility("default")))
typedef void                                VOID, *PVOID, *HANDLE, **PHANDLE, *HMODULE;
typedef long long unsigned int              QWORD, *PQWORD, ULONG64, *PULONG64;
typedef size_t                              SIZE_T, *PSIZE_T;
typedef uint64_t                            FILETIME, *PFILETIME;
typedef uint32_t                            DWORD, *PDWORD, *LPDWORD, BOOL, *PBOOL, NTSTATUS;
typedef uint16_t                            WORD, *PWORD;
typedef uint8_t                             BYTE, *PBYTE, *LPBYTE, UCHAR;
typedef char                                CHAR, *PCHAR, *LPSTR;
typedef const char                          *LPCSTR;
typedef uint16_t                            WCHAR, *PWCHAR, *LPWSTR;
typedef const uint16_t                      *LPCWSTR;
#define MAX_PATH                            260
#define _In_
#define _In_z_
#define _In_opt_
#define _In_reads_(x)
#define _In_reads_bytes_(x)
#define _In_reads_opt_(x)
#define _Inout_
#define _Inout_bytecount_(x)
#define _Inout_opt_
#define _Inout_updates_opt_(x)
#define _Out_
#define _Out_opt_
#define _Out_writes_(x)
#define _Out_writes_bytes_opt_(x)
#define _Out_writes_opt_(x)
#define _Out_writes_to_(x,y)
#define _When_(x,y)
#define _Frees_ptr_opt_
#define _Post_ptr_invalid_
#define _Check_return_opt_
#define _Printf_format_string_
#define _Success_(x)

#endif /* LINUX */

typedef struct tdVMM_HANDLE     *VMM_HANDLE;
typedef struct tdVMMVM_HANDLE   *VMMVM_HANDLE;
typedef BYTE                    OPAQUE_OB_HEADER[0x40];



//-----------------------------------------------------------------------------
// INITIALIZATION FUNCTIONALITY BELOW:
// Choose one way of initializing the VMM / MemProcFS.
//-----------------------------------------------------------------------------

/*
* 使用命令行参数初始化 VMM.DLL。有关参数的更多详细信息，请参阅 Mem Proc FS 和 Leech Core 的 github wiki。
* 注意！Leech Core 初始化参数也适用于此功能。
* 重要参数是：
* -printf = 显示 printf 样式输出。
* -v -vv -vvv = 额外的详细程度。
* -device = 设备与 Leech Core 的格式相同 - 请参阅 leechcore.h 或
* Github 文档以获取更多信息。一些值是：
* <file>、fpga、usb3380、hvsavedstate、totalmeltdown、pmem
* -remote = 远程 Leech Core 实例 - 请参阅 leechcore.h 或 Github
* 文档以获取更多信息。
* -norefresh = 禁用后台刷新（即使后备内存是
* 易失性内存）。
* -memmap = 指定文件提供的物理内存映射或指定“自动”。
* 示例：-memmap c:\\temp\\my custom memory map.txt
* 示例：-memmap auto
* -pagefile[0-9] = 除物理内存外要使用的页面文件。
* 通常 pagefile.sys 的索引为 0，而 swapfile.sys 的索引为 1。
* 页面文件不断变化 - 如果内存转储和页面文件之间的时间差超过几分钟，请勿使用。
* 示例：'pagefile0 swapfile.sys'
* -disable-python = 阻止加载 python 插件子系统。
* -disable-symbolserver = 禁用符号服务器，直到用户更改。
* 此参数将优先于注册表设置。
* -disable-symbols = 禁用从 .pdb 文件中查找符号。
* -disable-infodb = 禁用 infodb 以及通过它进行的任何符号查找。
* -waitinitialize = 等待初始化完成后再返回。
* 正常使用是某些初始化是异步完成的
* 并且在初始化调用完成时可能无法完成。
* 这包括虚拟内存压缩、注册表等。
* 示例：'-waitinitialize'
* -userinteract = 允许 vmm.dll 在控制台上向用户查询
* 信息，例如（但不限于）leechcore 设备选项。
* 默认值：用户交互 = 禁用。
* -vm = 虚拟机 (VM) 解析。
* -vm-basic = 虚拟机 (VM) 解析（仅限物理内存）。
* -vm-nested = 虚拟机 (VM) 解析（包括嵌套 VM）。
* -forensic-yara-rules = 使用指定规则执行取证 yara 扫描。
* 应指定源或编译的 yara 规则的完整路径。
* 示例：-forensic-yara-rules "C:\Temp\my yara rules.yar"
* -forensic = 如果可能，在启动后立即开始对物理内存进行取证扫描。允许的参数值范围为 0-4。
* 注意！取证模式不适用于实时内存。
* 1 = 取证模式，使用内存中的 sqlite 数据库。
* 2 = 取证模式，退出时删除临时 sqlite 数据库。
* 3 = 取证模式，退出时保留临时 sqlite 数据库。
* 4 = 取证模式，使用静态命名 sqlite 数据库 (vmm.sqlite3)。
* 示例 -forensic 4
*
* -- argc
* -- argv
* -- pp Lc Error Info = 可选指针，用于接收分配内存的函数
* struct LC CONFIG ERRORINFO，其中包含失败时的扩展错误信息。调用者应通过
* 调用 Lc Mem Free() 释放收到的任何内存。
* -- 如果成功，则返回 VMM HANDLE，以便在后续 API 调用中使用。NULL=失败。
*/
EXPORTED_FUNCTION _Success_(return != NULL)
VMM_HANDLE VMMDLL_Initialize(_In_ DWORD argc, _In_ LPCSTR argv[]);

EXPORTED_FUNCTION _Success_(return != NULL)
VMM_HANDLE VMMDLL_InitializeEx(_In_ DWORD argc, _In_ LPCSTR argv[], _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcErrorInfo);

/*
*关闭 VMM HANDLE 的实例化版本并释放所有资源。
* -- hVMM
*/
EXPORTED_FUNCTION
VOID VMMDLL_Close(_In_opt_ _Post_ptr_invalid_ VMM_HANDLE hVMM);

/*
* 关闭所有实例化的VMM HANDLE版本并释放所有资源。
*/
EXPORTED_FUNCTION
VOID VMMDLL_CloseAll();

/*
* 查询 VMMDLL 分配的内存大小。
* -- pv Mem
* -- 返回 = 保存内存分配所需的字节数。
*/
EXPORTED_FUNCTION _Success_(return != 0)
SIZE_T VMMDLL_MemSize(_In_ PVOID pvMem);

/*
* 释放 VMMDLL 分配的内存。
* -- pv Mem
*/
EXPORTED_FUNCTION
VOID VMMDLL_MemFree(_Frees_ptr_opt_ PVOID pvMem);



//-----------------------------------------------------------------------------
// CONFIGURATION SETTINGS BELOW:
// Configure MemProcFS or the underlying memory
// acquisition devices.
//-----------------------------------------------------------------------------

/*
* 与以下函数一起使用的选项：VMMDLL Config Get 和 VMMDLL Config Set
* 选项定义为：此头文件中的 VMMDLL OPT * 或 leechcore.h 中的
* LC OPT *
* 有关更多详细信息，请查看各个设备类型的源代码。
*/
#define VMMDLL_OPT_CORE_PRINTF_ENABLE                   0x4000000100000000  // RW
#define VMMDLL_OPT_CORE_VERBOSE                         0x4000000200000000  // RW
#define VMMDLL_OPT_CORE_VERBOSE_EXTRA                   0x4000000300000000  // RW
#define VMMDLL_OPT_CORE_VERBOSE_EXTRA_TLP               0x4000000400000000  // RW
#define VMMDLL_OPT_CORE_MAX_NATIVE_ADDRESS              0x4000000800000000  // R
#define VMMDLL_OPT_CORE_LEECHCORE_HANDLE                0x4000001000000000  // R - 底层 leechcore 句柄（不要关闭）。

#define VMMDLL_OPT_CORE_SYSTEM                          0x2000000100000000  // R
#define VMMDLL_OPT_CORE_MEMORYMODEL                     0x2000000200000000  // R

#define VMMDLL_OPT_CONFIG_IS_REFRESH_ENABLED            0x2000000300000000  // R - 1/0
#define VMMDLL_OPT_CONFIG_TICK_PERIOD                   0x2000000400000000  // RW - 基本刻度周期（以毫秒为单位）
#define VMMDLL_OPT_CONFIG_READCACHE_TICKS               0x2000000500000000  // RW - 内存缓存有效期（以刻为单位）
#define VMMDLL_OPT_CONFIG_TLBCACHE_TICKS                0x2000000600000000  // RW -页表 (tlb) 缓存有效期（以刻为单位）
#define VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_PARTIAL       0x2000000700000000  // RW - 进程刷新（部分）周期（以刻为单位）
#define VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_TOTAL         0x2000000800000000  // RW - 进程刷新（完整）周期（以刻为单位）
#define VMMDLL_OPT_CONFIG_VMM_VERSION_MAJOR             0x2000000900000000  // R
#define VMMDLL_OPT_CONFIG_VMM_VERSION_MINOR             0x2000000A00000000  // R
#define VMMDLL_OPT_CONFIG_VMM_VERSION_REVISION          0x2000000B00000000  // R
#define VMMDLL_OPT_CONFIG_STATISTICS_FUNCTIONCALL       0x2000000C00000000  // RW - 启用函数调用统计（.status/statistics fncall 文件）
#define VMMDLL_OPT_CONFIG_IS_PAGING_ENABLED             0x2000000D00000000  // RW - 1/0
#define VMMDLL_OPT_CONFIG_DEBUG                         0x2000000E00000000  // W
#define VMMDLL_OPT_CONFIG_YARA_RULES                    0x2000000F00000000  // R

#define VMMDLL_OPT_WIN_VERSION_MAJOR                    0x2000010100000000  // R
#define VMMDLL_OPT_WIN_VERSION_MINOR                    0x2000010200000000  // R
#define VMMDLL_OPT_WIN_VERSION_BUILD                    0x2000010300000000  // R
#define VMMDLL_OPT_WIN_SYSTEM_UNIQUE_ID                 0x2000010400000000  // R

#define VMMDLL_OPT_FORENSIC_MODE                        0x2000020100000000  // RW - 启用/检索取证模式类型[0-4]。

// 刷新选项：
#define VMMDLL_OPT_REFRESH_ALL                          0x2001ffff00000000  // W - 刷新所有缓存
#define VMMDLL_OPT_REFRESH_FREQ_MEM                     0x2001100000000000  // W - 刷新内存缓存（不包括 TLB）[完全]
#define VMMDLL_OPT_REFRESH_FREQ_MEM_PARTIAL             0x2001000200000000  // W - 刷新内存缓存（不包括 TLB）[部分 33%/调用]
#define VMMDLL_OPT_REFRESH_FREQ_TLB                     0x2001080000000000  // W -刷新页表（TLB）缓存[完全]
#define VMMDLL_OPT_REFRESH_FREQ_TLB_PARTIAL             0x2001000400000000  // W - 刷新页表 (TLB) 缓存 [部分 33%/调用]
#define VMMDLL_OPT_REFRESH_FREQ_FAST                    0x2001040000000000  // W - 刷新频率快 - 包括部分进程刷新
#define VMMDLL_OPT_REFRESH_FREQ_MEDIUM                  0x2001000100000000  // W - 中频刷新 - 包括全流程刷新
#define VMMDLL_OPT_REFRESH_FREQ_SLOW                    0x2001001000000000  // W - 刷新频率慢。

// 进程选项：[LO-DWORD：进程 PID]
#define VMMDLL_OPT_PROCESS_DTB                          0x2002000100000000  // W - 强制设置进程目录表基数。
#define VMMDLL_OPT_PROCESS_DTB_FAST_LOWINTEGRITY        0x2002000200000000  // W - 强制设置进程目录表基数（快速、低完整性模式、较少检查）——使用风险自负！。

static LPCSTR VMMDLL_MEMORYMODEL_TOSTRING[5] = { "N/A", "X86", "X86PAE", "X64", "ARM64" };

typedef enum tdVMMDLL_MEMORYMODEL_TP {
    VMMDLL_MEMORYMODEL_NA       = 0,
    VMMDLL_MEMORYMODEL_X86      = 1,
    VMMDLL_MEMORYMODEL_X86PAE   = 2,
    VMMDLL_MEMORYMODEL_X64      = 3,
    VMMDLL_MEMORYMODEL_ARM64    = 4,
} VMMDLL_MEMORYMODEL_TP;

typedef enum tdVMMDLL_SYSTEM_TP {
    VMMDLL_SYSTEM_UNKNOWN_PHYSICAL = 0,
    VMMDLL_SYSTEM_UNKNOWN_64    = 1,
    VMMDLL_SYSTEM_WINDOWS_64    = 2,
    VMMDLL_SYSTEM_UNKNOWN_32    = 3,
    VMMDLL_SYSTEM_WINDOWS_32    = 4,
    VMMDLL_SYSTEM_UNKNOWN_X64   = 1,    // deprecated - do not use!
    VMMDLL_SYSTEM_WINDOWS_X64   = 2,    // deprecated - do not use!
    VMMDLL_SYSTEM_UNKNOWN_X86   = 3,    // deprecated - do not use!
    VMMDLL_SYSTEM_WINDOWS_X86   = 4     // deprecated - do not use!
} VMMDLL_SYSTEM_TP;

/*
* 获取特定于设备的选项值。有关有效选项值的信息，请参阅 VMMDLL OPT * 的定义。请注意，选项值可能在不同设备类型之间重叠，含义也不同。
* -- h VMM
* -- f 选项
* -- pqw 值 = 指向 ULONG64 的指针，用于接收选项值。
* -- 返回 = 成功/失败.
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_ConfigGet(_In_ VMM_HANDLE hVMM, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue);

/*
* 设置设备特定的选项值。有关有效选项值的信息，请参阅 VMMDLL OPT * 定义。请注意，选项值可能在不同设备类型之间重叠，且含义不同。
* -- h VMM
* -- f 选项
* -- qw 值
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_ConfigSet(_In_ VMM_HANDLE hVMM, _In_ ULONG64 fOption, _In_ ULONG64 qwValue);



//-----------------------------------------------------------------------------
// FORWARD DECLARATIONS:
//-----------------------------------------------------------------------------

typedef struct tdVMMDLL_MAP_PFN *PVMMDLL_MAP_PFN;



//-----------------------------------------------------------------------------
// LINUX SPECIFIC DEFINES:
//-----------------------------------------------------------------------------
#ifdef LINUX

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
        DWORD   PhysicalAddress;
        DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _SERVICE_STATUS {
    DWORD   dwServiceType;
    DWORD   dwCurrentState;
    DWORD   dwControlsAccepted;
    DWORD   dwWin32ExitCode;
    DWORD   dwServiceSpecificExitCode;
    DWORD   dwCheckPoint;
    DWORD   dwWaitHint;
} SERVICE_STATUS, *LPSERVICE_STATUS;
#endif /* LINUX */



//-----------------------------------------------------------------------------
// VFS - VIRTUAL FILE SYSTEM FUNCTIONALITY BELOW:
// NB! VFS FUNCTIONALITY REQUIRES PLUGINS TO BE INITIALIZED
//     WITH CALL TO VMMDLL_InitializePlugins().
// This is the core of MemProcFS. All implementation and analysis towards
// the virtual file system (vfs) is possible by using functionality below. 
//-----------------------------------------------------------------------------

#define VMMDLL_STATUS_SUCCESS                       ((NTSTATUS)0x00000000L)
#define VMMDLL_STATUS_UNSUCCESSFUL                  ((NTSTATUS)0xC0000001L)
#define VMMDLL_STATUS_END_OF_FILE                   ((NTSTATUS)0xC0000011L)
#define VMMDLL_STATUS_FILE_INVALID                  ((NTSTATUS)0xC0000098L)
#define VMMDLL_STATUS_FILE_SYSTEM_LIMITATION        ((NTSTATUS)0xC0000427L)

#define VMMDLL_VFS_FILELIST_EXINFO_VERSION          1
#define VMMDLL_VFS_FILELIST_VERSION                 2
#define VMMDLL_VFS_FILELISTBLOB_VERSION             0xf88f0001

typedef struct tdVMMDLL_VFS_FILELIST_EXINFO {
    DWORD dwVersion;
    BOOL fCompressed;                   // set flag FILE_ATTRIBUTE_COMPRESSED - (no meaning but shows gui artifact in explorer.exe)
    union {
        FILETIME ftCreationTime;        // 0 = default time
        QWORD qwCreationTime;
    };
    union {
        FILETIME ftLastAccessTime;      // 0 = default time
        QWORD qwLastAccessTime;
    };
    union {
        FILETIME ftLastWriteTime;       // 0 = default time
        QWORD qwLastWriteTime;
    };
} VMMDLL_VFS_FILELIST_EXINFO, *PVMMDLL_VFS_FILELIST_EXINFO;

typedef struct tdVMMDLL_VFS_FILELIST2 {
    DWORD dwVersion;
    VOID(*pfnAddFile)     (_Inout_ HANDLE h, _In_ LPCSTR uszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
    VOID(*pfnAddDirectory)(_Inout_ HANDLE h, _In_ LPCSTR uszName, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
    HANDLE h;
} VMMDLL_VFS_FILELIST2, *PVMMDLL_VFS_FILELIST2;

typedef struct tdVMMDLL_VFS_FILELISTBLOB_ENTRY {
    ULONG64 ouszName;                       // byte offset to string from VMMDLL_VFS_FILELISTBLOB.uszMultiText
    ULONG64 cbFileSize;                     // -1 == directory
    VMMDLL_VFS_FILELIST_EXINFO ExInfo;      // optional ExInfo
} VMMDLL_VFS_FILELISTBLOB_ENTRY, *PVMMDLL_VFS_FILELISTBLOB_ENTRY;

typedef struct tdVMMDLL_VFS_FILELISTBLOB {
    DWORD dwVersion;                        // VMMDLL_VFS_FILELISTBLOB_VERSION
    DWORD cbStruct;
    DWORD cFileEntry;
    DWORD cbMultiText;
    union {
        LPSTR uszMultiText;
        QWORD _Reserved;
    };
    DWORD _FutureUse[8];
    VMMDLL_VFS_FILELISTBLOB_ENTRY FileEntry[0];
} VMMDLL_VFS_FILELISTBLOB, *PVMMDLL_VFS_FILELISTBLOB;

/*
* 用于回调 VMM VFS FILELIST2 结构的辅助函数。
*/
EXPORTED_FUNCTION
VOID VMMDLL_VfsList_AddFile(_In_ HANDLE pFileList, _In_ LPCSTR uszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
VOID VMMDLL_VfsList_AddFileW(_In_ HANDLE pFileList, _In_ LPCWSTR wszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
EXPORTED_FUNCTION
VOID VMMDLL_VfsList_AddDirectory(_In_ HANDLE pFileList, _In_ LPCSTR uszName, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
VOID VMMDLL_VfsList_AddDirectoryW(_In_ HANDLE pFileList, _In_ LPCWSTR wszName, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo);
EXPORTED_FUNCTION BOOL VMMDLL_VfsList_IsHandleValid(_In_ HANDLE pFileList);

/*
* 列出 Mem Proc FS 中的文件目录。目录和文件将通过回调列出
* p 文件列表参数中提供的函数。
* 如果需要单个文件的信息，则必须列出其目录中的所有
* 文件。
* -- h VMM
* -- [uw]sz Path
* -- p 文件列表
* -- 返回
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_VfsListU(_In_ VMM_HANDLE hVMM, _In_ LPCSTR  uszPath, _Inout_ PVMMDLL_VFS_FILELIST2 pFileList);
_Success_(return) BOOL VMMDLL_VfsListW(_In_ VMM_HANDLE hVMM, _In_ LPCWSTR wszPath, _Inout_ PVMMDLL_VFS_FILELIST2 pFileList);

/*
* 列出 Mem Proc FS 中的文件目录并返回 VMMDLL VFS FILELISTBLOB。
* CALLER FREE: VMMDLL Mem Free(return)
* -- h VMM
* -- usz Path
* -- return
*/
EXPORTED_FUNCTION
_Success_(return != NULL) PVMMDLL_VFS_FILELISTBLOB VMMDLL_VfsListBlobU(_In_ VMM_HANDLE hVMM, _In_ LPCSTR uszPath);

/*
* 读取 Mem Proc FS 中文件的选定部分。
* -- h VMM
* -- [uw]sz 文件名
* -- pb
* -- cb
* -- pcb 读取
* -- cb 偏移量
* -- 返回
*
*/
EXPORTED_FUNCTION
NTSTATUS VMMDLL_VfsReadU(_In_ VMM_HANDLE hVMM, _In_ LPCSTR  uszFileName, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset);
NTSTATUS VMMDLL_VfsReadW(_In_ VMM_HANDLE hVMM, _In_ LPCWSTR wszFileName, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset);

/*
* 将选定的部分写入 Mem Proc FS 中的文件。
* -- h VMM
* -- [uw]sz 文件名
* -- pb
* -- cb
* -- pcb 写入
* -- cb 偏移量
* -- 返回
*/
EXPORTED_FUNCTION
NTSTATUS VMMDLL_VfsWriteU(_In_ VMM_HANDLE hVMM, _In_ LPCSTR  uszFileName, _In_reads_(cb) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbWrite, _In_ ULONG64 cbOffset);
NTSTATUS VMMDLL_VfsWriteW(_In_ VMM_HANDLE hVMM, _In_ LPCWSTR wszFileName, _In_reads_(cb) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbWrite, _In_ ULONG64 cbOffset);

/*
* Mem Proc FS 对不同底层数据进行读/写的实用函数
* 表示。
*/
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsReadFile_FromPBYTE(_In_ PBYTE pbFile, _In_ ULONG64 cbFile, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset);
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsReadFile_FromQWORD(_In_ ULONG64 qwValue, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset, _In_ BOOL fPrefix);
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsReadFile_FromDWORD(_In_ DWORD dwValue, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset, _In_ BOOL fPrefix);
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsReadFile_FromBOOL(_In_ BOOL fValue, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead, _In_ ULONG64 cbOffset);
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsWriteFile_BOOL(_Inout_ PBOOL pfTarget, _In_reads_(cb) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbWrite, _In_ ULONG64 cbOffset);
EXPORTED_FUNCTION NTSTATUS VMMDLL_UtilVfsWriteFile_DWORD(_Inout_ PDWORD pdwTarget, _In_reads_(cb) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbWrite, _In_ ULONG64 cbOffset, _In_ DWORD dwMinAllow);



//-----------------------------------------------------------------------------
// PLUGIN MANAGER FUNCTIONALITY BELOW:
// Function and structures to initialize and use MemProcFS plugin functionality.
// The plugin manager is started by a call to function:
// VMM_VfsInitializePlugins. Each built-in plugin and external plugin of which
// the DLL name matches m_*.dll will receive a call to its InitializeVmmPlugin
// function. The plugin/module may decide to call pfnPluginManager_Register to
// register plugins in the form of different names one or more times.
// Example of registration function in a plugin DLL below: 
// 'VOID InitializeVmmPlugin(_In_ VMM_HANDLE H, _In_ PVMM_PLUGIN_REGINFO pRegInfo)'
//-----------------------------------------------------------------------------

/*
* 初始化所有映射到 Mem Proc FS 的潜在插件（内置和外部）。请注意，默认情况下不会加载插件 - 必须通过调用此函数显式加载它们。它们将在 vmm dll 的一般关闭时卸载。
* -- h VMM
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_InitializePlugins(_In_ VMM_HANDLE hVMM);

#define VMMDLL_PLUGIN_CONTEXT_MAGIC                 0xc0ffee663df9301c
#define VMMDLL_PLUGIN_CONTEXT_VERSION               5
#define VMMDLL_PLUGIN_REGINFO_MAGIC                 0xc0ffee663df9301d
#define VMMDLL_PLUGIN_REGINFO_VERSION               18
#define VMMDLL_FORENSIC_JSONDATA_VERSION            0xc0ee0002
#define VMMDLL_FORENSIC_INGEST_VIRTMEM_VERSION      0xc0dd0001
#define VMMDLL_FORENSIC_INGEST_OBJECT_VERSION       0xc0de0001

#define VMMDLL_PLUGIN_NOTIFY_VERBOSITYCHANGE        0x01
#define VMMDLL_PLUGIN_NOTIFY_REFRESH_FAST           0x05    // refresh fast event   - at partial process refresh.
#define VMMDLL_PLUGIN_NOTIFY_REFRESH_MEDIUM         0x02    // refresh medium event - at full process refresh.
#define VMMDLL_PLUGIN_NOTIFY_REFRESH_SLOW           0x04    // refresh slow event   - at registry refresh.

#define VMMDLL_PLUGIN_NOTIFY_FORENSIC_INIT          0x01000100
#define VMMDLL_PLUGIN_NOTIFY_FORENSIC_INIT_COMPLETE 0x01000200
#define VMMDLL_PLUGIN_NOTIFY_VM_ATTACH_DETACH       0x01000400

typedef DWORD                                       VMMDLL_MODULE_ID;
typedef HANDLE                                      *PVMMDLL_PLUGIN_INTERNAL_CONTEXT;
typedef struct tdVMMDLL_CSV_HANDLE                  *VMMDLL_CSV_HANDLE;

#define VMMDLL_MID_MAIN                             ((VMMDLL_MODULE_ID)0x80000001)
#define VMMDLL_MID_PYTHON                           ((VMMDLL_MODULE_ID)0x80000002)
#define VMMDLL_MID_DEBUG                            ((VMMDLL_MODULE_ID)0x80000003)
#define VMMDLL_MID_RUST                             ((VMMDLL_MODULE_ID)0x80000004)

typedef struct tdVMMDLL_PLUGIN_CONTEXT {
    ULONG64 magic;
    WORD wVersion;
    WORD wSize;
    DWORD dwPID;
    PVOID pProcess;
    LPSTR uszModule;
    LPSTR uszPath;
    PVOID pvReserved1;
    PVMMDLL_PLUGIN_INTERNAL_CONTEXT ctxM;       // optional internal module context.
    VMMDLL_MODULE_ID MID;
} VMMDLL_PLUGIN_CONTEXT, *PVMMDLL_PLUGIN_CONTEXT;

typedef struct tdVMMDLL_FORENSIC_JSONDATA {
    DWORD dwVersion;        // must equal VMMDLL_FORENSIC_JSONDATA_VERSION
    DWORD _FutureUse;
    LPSTR szjType;          // log type/name (json encoded)
    DWORD i;
    DWORD dwPID;
    QWORD vaObj;
    BOOL fva[2];            // log va even if zero
    QWORD va[2];
    BOOL fNum[2];           // log num even if zero
    QWORD qwNum[2];
    BOOL fHex[2];           // log hex even if zero
    QWORD qwHex[2];
    // str: will be prioritized in order: szu > wsz.
    LPCSTR usz[2];          // str: utf-8 encoded
    LPCWSTR wsz[2];         // str: wide
    BYTE _Reserved[0x4000+256];
} VMMDLL_FORENSIC_JSONDATA, *PVMMDLL_FORENSIC_JSONDATA;

typedef enum tdVMMDLL_FORENSIC_INGEST_OBJECT_TYPE {
    VMMDLL_FORENSIC_INGEST_OBJECT_TYPE_FILE = 1,
} VMMDLL_FORENSIC_INGEST_OBJECT_TYPE;

typedef struct tdVMMDLL_FORENSIC_INGEST_OBJECT {
    OPAQUE_OB_HEADER _Reserved;
    DWORD dwVersion;        // must equal VMMDLL_FORENSIC_INGEST_OBJECT_VERSION
    VMMDLL_FORENSIC_INGEST_OBJECT_TYPE tp;
    QWORD vaObject;
    LPSTR uszText;
    PBYTE pb;
    DWORD cb;
    DWORD cbReadActual;     // actual bytes read (may be spread out in pb)
} VMMDLL_FORENSIC_INGEST_OBJECT, *PVMMDLL_FORENSIC_INGEST_OBJECT;

typedef struct tdVMMDLL_FORENSIC_INGEST_PHYSMEM {
    BOOL fValid;
    QWORD pa;
    DWORD cb;
    PBYTE pb;
    DWORD cMEMs;
    PPMEM_SCATTER ppMEMs;
    PVMMDLL_MAP_PFN pPfnMap;
} VMMDLL_FORENSIC_INGEST_PHYSMEM, *PVMMDLL_FORENSIC_INGEST_PHYSMEM;

typedef struct tdVMMDLL_FORENSIC_INGEST_VIRTMEM {
    OPAQUE_OB_HEADER _Reserved;
    DWORD dwVersion;        // must equal VMMDLL_FORENSIC_INGEST_VIRTMEM_VERSION
    BOOL fPte;
    BOOL fVad;
    PVOID pvProcess;
    DWORD dwPID;
    QWORD va;
    PBYTE pb;
    DWORD cb;
    DWORD cbReadActual;     // actual bytes read (may be spread out in pb)
} VMMDLL_FORENSIC_INGEST_VIRTMEM, *PVMMDLL_FORENSIC_INGEST_VIRTMEM;

typedef struct tdVMMDLL_PLUGIN_REGINFO {
    ULONG64 magic;                          // VMMDLL_PLUGIN_REGINFO_MAGIC
    WORD wVersion;                          // VMMDLL_PLUGIN_REGINFO_VERSION
    WORD wSize;                             // size of struct
    VMMDLL_MEMORYMODEL_TP tpMemoryModel;
    VMMDLL_SYSTEM_TP tpSystem;
    HMODULE hDLL;
    BOOL(*pfnPluginManager_Register)(_In_ VMM_HANDLE H, struct tdVMMDLL_PLUGIN_REGINFO *pPluginRegInfo);
    LPSTR uszPathVmmDLL;
    DWORD _Reserved[30];
    // python plugin information - not for general use
    struct {
        BOOL fPythonStandalone;
        DWORD _Reserved;
        HMODULE hReservedDllPython3;
        HMODULE hReservedDllPython3X;
    } python;
    // general plugin registration info to be filled out by the plugin below:
    struct {
        PVMMDLL_PLUGIN_INTERNAL_CONTEXT ctxM;   // optional internal module context [must be cleaned by pfnClose() call].
        CHAR uszPathName[128];
        BOOL fRootModule;
        BOOL fProcessModule;
        BOOL fRootModuleHidden;
        BOOL fProcessModuleHidden;
        CHAR sTimelineNameShort[6];
        CHAR _Reserved[2];
        CHAR uszTimelineFile[32];
        CHAR _Reserved2[32];
    } reg_info;
    // function plugin registration info to be filled out by the plugin below:
    struct {
        BOOL(*pfnList)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _Inout_ PHANDLE pFileList);
        NTSTATUS(*pfnRead)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _Out_writes_to_(cb, *pcbRead) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbRead,  _In_ ULONG64 cbOffset);
        NTSTATUS(*pfnWrite)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _In_reads_(cb) PBYTE pb, _In_ DWORD cb, _Out_ PDWORD pcbWrite, _In_ ULONG64 cbOffset);
        VOID(*pfnNotify)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _In_ DWORD fEvent, _In_opt_ PVOID pvEvent, _In_opt_ DWORD cbEvent);
        VOID(*pfnClose)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP);
        BOOL(*pfnVisibleModule)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP);
        PVOID pvReserved[10];
    } reg_fn;
   // 可选的取证插件功能，用于取证（更全面）
// 分析各种数据。函数是可选的。
// 函数按以下顺序和方式调用。
// 1：pfn Initialize() - 多线程（插件之间）。
// 2：（多种类型见下文）- 多线程（插件之间）。
// pfn Log CSV()
// pfn Log JSON()
// pfn Find Evil()
// pfn Ingest Physmem()
// pfn Ingest Virtmem()
// 3. pfn Ingest Finalize() - 单线程。（pfn Log CSV/pfn Log JSON/pfn Find Evil 可能仍处于活动状态）。
// 4. pfn Timeline() - 单线程。（pfn Log CSV/pfn Log JSON/pfn Find Evil 可能仍处于活动状态）。
// 5. pfn Finalize() - 单线程。
    struct {
        PVOID(*pfnInitialize)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP);
        VOID(*pfnFinalize)(_In_ VMM_HANDLE H, _In_opt_ PVOID ctxfc);
        VOID(*pfnTimeline)(
            _In_ VMM_HANDLE H,
            _In_opt_ PVOID ctxfc,
            _In_ HANDLE hTimeline,
            _In_ VOID(*pfnAddEntry)(_In_ VMM_HANDLE H, _In_ HANDLE hTimeline, _In_ QWORD ft, _In_ DWORD dwAction, _In_ DWORD dwPID, _In_ DWORD dwData32, _In_ QWORD qwData64, _In_ LPCSTR uszText),
            _In_ VOID(*pfnEntryAddBySql)(_In_ VMM_HANDLE H, _In_ HANDLE hTimeline, _In_ DWORD cEntrySql, _In_ LPCSTR *pszEntrySql));
        VOID(*pfnIngestObject)(_In_ VMM_HANDLE H, _In_opt_ PVOID ctxfc, _In_ PVMMDLL_FORENSIC_INGEST_OBJECT pIngestObject);
        VOID(*pfnIngestPhysmem)(_In_ VMM_HANDLE H, _In_opt_ PVOID ctxfc, _In_ PVMMDLL_FORENSIC_INGEST_PHYSMEM pIngestPhysmem);
        VOID(*pfnIngestVirtmem)(_In_ VMM_HANDLE H, _In_opt_ PVOID ctxfc, _In_ PVMMDLL_FORENSIC_INGEST_VIRTMEM pIngestVirtmem);
        VOID(*pfnIngestFinalize)(_In_ VMM_HANDLE H, _In_opt_ PVOID ctxfc);
        VOID(*pfnFindEvil)(_In_ VMM_HANDLE H, _In_ VMMDLL_MODULE_ID MID, _In_opt_ PVOID ctxfc);
        PVOID pvReserved[6];
        VOID(*pfnLogCSV)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _In_ VMMDLL_CSV_HANDLE hCSV);
        VOID(*pfnLogJSON)(_In_ VMM_HANDLE H, _In_ PVMMDLL_PLUGIN_CONTEXT ctxP, _In_ VOID(*pfnLogJSON)(_In_ VMM_HANDLE H, _In_ PVMMDLL_FORENSIC_JSONDATA pData));
    } reg_fnfc;
    // Additional system information - read/only by the plugins.
    struct {
        BOOL f32;
        DWORD dwVersionMajor;
        DWORD dwVersionMinor;
        DWORD dwVersionBuild;
        DWORD _Reserved[32];
    } sysinfo;
} VMMDLL_PLUGIN_REGINFO, *PVMMDLL_PLUGIN_REGINFO;



//-----------------------------------------------------------------------------
// FORENSIC-MODE SPECIFIC FUNCTIONALITY BELOW:
//-----------------------------------------------------------------------------

/*
* 将文本数据附加到内存支持的取证文件。
* 所有文本都应采用 UTF-8 编码。
* -- H
* -- usz 文件名
* -- usz 格式
* -- ..
* -- 返回 = 附加的字节数（不包括终止空值）。
*/
EXPORTED_FUNCTION _Success_(return != 0)
SIZE_T VMMDLL_ForensicFileAppend(
    _In_ VMM_HANDLE H,
    _In_ LPCSTR uszFileName,
    _In_z_ _Printf_format_string_ LPCSTR uszFormat,
    ...
);



//-----------------------------------------------------------------------------
// VMM 日志功能如下：
// 外部代码（主要是外部插件）可以使用
// Mem Proc FS 日志系统。
// ----------------------------------------------------------------------------

typedef enum tdVMMDLL_LOGLEVEL {
    VMMDLL_LOGLEVEL_CRITICAL = 1,  // critical stopping error
    VMMDLL_LOGLEVEL_WARNING  = 2,  // severe warning error
    VMMDLL_LOGLEVEL_INFO     = 3,  // normal/info message
    VMMDLL_LOGLEVEL_VERBOSE  = 4,  // verbose message (visible with -v)
    VMMDLL_LOGLEVEL_DEBUG    = 5,  // debug message (visible with -vv)
    VMMDLL_LOGLEVEL_TRACE    = 6,  // trace message
    VMMDLL_LOGLEVEL_NONE     = 7,  // do not use!
} VMMDLL_LOGLEVEL;

/*
* 使用内部 Mem Proc FS vmm 日志系统记录消息。日志消息
* 将根据当前日志配置显示/隐藏。
* -- h VMM
* -- MID = 插件上下文 PVMMDLL PLUGIN CONTEXT 提供的模块 ID 或
* VMMDLL MID 提供的 ID *。
* -- dw 日志级别
* -- usz 格式
* -- ...
*/
EXPORTED_FUNCTION
VOID VMMDLL_Log(
    _In_ VMM_HANDLE hVMM,
    _In_opt_ VMMDLL_MODULE_ID MID,
    _In_ VMMDLL_LOGLEVEL dwLogLevel,
    _In_z_ _Printf_format_string_ LPCSTR uszFormat,
    ...
);

/*
* 使用内部 Mem Proc FS vmm 日志系统记录消息。日志消息
* 将根据当前日志配置显示/隐藏。
* -- h VMM
* -- MID = 插件上下文 PVMMDLL PLUGIN CONTEXT 提供的模块 ID 或
* VMMDLL MID 提供的 ID *。
* -- dw 日志级别
* -- usz 格式
* -- arglist
*/
EXPORTED_FUNCTION
VOID VMMDLL_LogEx(
    _In_ VMM_HANDLE hVMM,
    _In_opt_ VMMDLL_MODULE_ID MID,
    _In_ VMMDLL_LOGLEVEL dwLogLevel,
    _In_z_ _Printf_format_string_ LPCSTR uszFormat,
    va_list arglist
);



//-----------------------------------------------------------------------------
// VMM 核心功能如下：
// Vmm 核心功能，如读取（和写入）虚拟和物理
// 内存。注意！只有目标受支持时写入才会起作用 - 即不是
// 内存转储文件...
// 要读取物理内存，请将 dw PID 指定为 (DWORD)-1
//-----------------------------------------------------------------------------

#define VMMDLL_PID_PROCESS_WITH_KERNELMEMORY        0x80000000      // Combine with dwPID to enable process kernel memory (NB! use with extreme care).

//用于抑制对 VMM Mem Read Ex() 的调用中的默认读取缓存的标志
// 这将导致始终从目标系统获取读取。
// 缓存页表（用于将 virtual2physical 进行转换）仍在使用。
#define VMMDLL_FLAG_NOCACHE                         0x0001  //不使用数据缓存（强制从内存采集设备读取）
#define VMMDLL_FLAG_ZEROPAD_ON_FAIL                 0x0002  // 零填充失败的物理内存读取，如果在物理内存范围内读取则报告成功。
#define VMMDLL_FLAG_FORCECACHE_READ                 0x0008  // 强制使用缓存 - 非缓存页面失败 - 仅对读取有效，对 VMM FLAG NOCACHE/VMM FLAG ZEROPAD ON FAIL 无效。
#define VMMDLL_FLAG_NOPAGING                        0x0010  // 不要尝试从页面文件/压缩文件中检索分页内存（即使可能）
#define VMMDLL_FLAG_NOPAGING_IO                     0x0020  // 如果读取会产生额外的 I/O（即使可能），请不要尝试从分页内存中检索内存。
#define VMMDLL_FLAG_NOCACHEPUT                      0x0100  //从内存采集设备成功读取后不要写回数据缓存。
#define VMMDLL_FLAG_CACHE_RECENT_ONLY               0x0200  // 读取时仅从最近的活动缓存区域获取。
#define VMMDLL_FLAG_NO_PREDICTIVE_READ              0x0400  //不执行额外的预测页面读取（较小请求的默认设置）。
#define VMMDLL_FLAG_FORCECACHE_READ_DISABLE         0x0800  //禁用/覆盖任何 VMM FLAG FORCECACHE READ 的使用。仅推荐用于本地文件。改进法医工件顺序。

/*
* 读取由指向 pp MEMs 数组中项目的指针指定的各种非连续位置的内存。将单独给出每个工作单元的结果。读取的项目数量没有上限，但如果超过硬件限制，则不会提高性能。每个工作单元的最大大小为 4k 页（4096 字节）。读取不得跨越 4k 页边界。读取必须从偶数 DWORD（4 字节）开始。
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于读取物理内存。
* -- pp MEMs = 分散读取标头数组。
* -- cp MEMs = pp MEMs 的数量。
* -- flags = VMMDLL FLAG 给出的可选标志 *
* -- return = 成功读取的项目数。
*/
EXPORTED_FUNCTION
DWORD VMMDLL_MemReadScatter(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Inout_ PPMEM_SCATTER ppMEMs, _In_ DWORD cpMEMs, _In_ DWORD flags);

/*
* 将内存写入由指向 pp MEMs 数组中项目的指针指定的各种不连续位置。每个工作单元的结果将单独给出。要写入的项目数没有上限。每个工作单元的最大大小为一个 4k 页（4096 字节）。写入不得跨越 4k 页边界。
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于写入物理内存。
* -- pp MEMs = 分散读取标头数组。
* -- cp MEMs = pp MEMs 的数量。
* -- 返回 = 希望成功写入的项目数。
*/
EXPORTED_FUNCTION
DWORD VMMDLL_MemWriteScatter(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Inout_ PPMEM_SCATTER ppMEMs, _In_ DWORD cpMEMs);

/*
* 读取单个 4096 字节的内存页。
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于读取物理内存。
* -- qw A
* -- pb Page
* -- 返回 = 成功/失败（取决于是否读取了所有请求的字节）。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemReadPage(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ ULONG64 qwA, _Inout_bytecount_(4096) PBYTE pbPage);

/*
* 读取连续的任意数量的内存。
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 读取物理内存。
* -- qw A
* -- pb
* -- cb
* -- 返回 = 成功/失败（取决于是否读取了所有请求的字节）。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemRead(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ ULONG64 qwA, _Out_writes_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 读取连续的内存量，并在 pcb Read 中报告读取的字节数。
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于读取物理内存。
* -- qw A
* -- pb
* -- cb
* -- pcb Read
* -- flags = 与 VMMDLL FLAG 中的标志相同 *
* -- return = 成功/失败。注意！即使读取了 0 个字节，读取也可能报告为成功
* - 建议验证 pcb Read Opt 参数。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemReadEx(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ ULONG64 qwA, _Out_writes_(cb) PBYTE pb, _In_ DWORD cb, _Out_opt_ PDWORD pcbReadOpt, _In_ ULONG64 flags);

/*
* 将多个地址（在 p A 数组中指定）预取到内存
* 缓存中。此函数用于将较大的已知读取批量放入本地
* 缓存中，然后再进行多次较小的读取 - 然后从
* 缓存中进行读取。出于性能原因，此函数存在。
* -- h VMM
* -- dw PID = 目标进程的 PID，(DWORD)-1 表示物理内存。
* -- p 预取地址 = 要读入缓存的地址数组。
* -- c 预取地址
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemPrefetchPages(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_reads_(cPrefetchAddresses) PULONG64 pPrefetchAddresses, _In_ DWORD cPrefetchAddresses);

/*
* 写入连续的任意数量的内存。请注意，某些虚拟内存
*（例如可执行文件页面（例如 DLL））可能在不同进程的不同虚拟内存之间共享。例如，在一个进程中写入 kernel32.dll
* 可能会影响整个系统中的 kernel32 - 所有进程。堆和堆栈以及其他内存通常可以安全写入。
* 写入内存时请小心！
* -- h VMM
* -- dw PID = 目标进程的 PID，(DWORD)-1 读取物理内存。
* -- qw A
* -- pb
* -- cb
* -- return = 成功时返回 TRUE，部分或零写入时返回 FALSE。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemWrite(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ ULONG64 qwA, _In_reads_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 通过遍历指定进程的页表
*，将虚拟地址转换为物理地址。
* -- h VMM
* -- dw PID
* -- qw VA
* -- pqw PA
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemVirt2Phys(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ ULONG64 qwVA, _Out_ PULONG64 pqwPA);



//-----------------------------------------------------------------------------
//简化了更易于使用的读取分散内存功能如下：
// 流程如下：
// 1. 调用 VMMDLL Scatter Initialize 来初始化句柄。
// 2. 通过多次调用 VMMDLL Scatter Prepare
// 和/或 VMMDLL Scatter Prepare Ex 函数来填充内存范围。提供给
// VMMDLL Scatter Prepare Ex 的内存缓冲区将填充步骤 (3) 中的内容。
// 3. 通过调用 VMMDLL Scatter Execute 函数来检索内存。
// 4. 如果使用了 VMMDLL Scatter Prepare（即不是 VMMDLL Scatter Prepare Ex）
// 则检索 (3) 中读取的内存。
// 5. 或者通过调用 VMMDLL Scatter Clear 来清除句柄以供重用
// 使用 VMMDLL Scatter Close Handle 关闭句柄以释放资源。
// 注意！在关闭句柄之前，不得释放提供给 VMMDLL Scatter Prepare Ex 的缓冲区，因为它可能在内部使用。
// 注意！VMMDLL Scatter Execute Read 可能会在稍后的某个时间点被调用，以
// 更新（重新读取）先前读取的数据。
// 注意！支持更大的读取（最大 1 GB），但不建议这样做。
//-----------------------------------------------------------------------------
typedef HANDLE      VMMDLL_SCATTER_HANDLE;

/*
* 初始化用于调用 VMMDLL Scatter * 函数的散射句柄。
* CALLER CLOSE: VMMDLL Scatter Close Handle(return)
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于读取物理内存。
* -- flags = 由 VMMDLL FLAG 提供的可选标志 *
* -- return = 在 VMMDLL Scatter * 函数中使用的句柄。
*/
EXPORTED_FUNCTION _Success_(return != NULL)
VMMDLL_SCATTER_HANDLE VMMDLL_Scatter_Initialize(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ DWORD flags);

/*
* 准备（添加）要读取的内存范围。调用
* VMMDLL Scatter Execute*() 后，可以使用 VMMDLL Scatter Read() 检索内存。
* -- h S
* -- va = 要读取的内存范围的起始地址。
* -- cb = 要读取的内存范围的大小。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_Prepare(_In_ VMMDLL_SCATTER_HANDLE hS, _In_ QWORD va, _In_ DWORD cb);

/*
* 准备（添加）一个用于读取的内存范围。缓冲区 pb 和读取长度
* *稍后调用 VMMDLL Scatter Execute*() 时，将填充 pcb Read。
* 注意！在调用 VMMDLL Scatter Close Handle()
* 之前，不能释放缓冲区 pb，因为它由散射功能内部使用！
* -- h S
* -- va = 要读取的内存范围的起始地址。
* -- cb = 要读取的内存范围的大小。
* -- pb = 调用 VMMDLL Scatter Execute Read() 时要用读取内存填充的缓冲区
* -- pcb Read = 可选指针，用于填充成功读取的字节数。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_PrepareEx(_In_ VMMDLL_SCATTER_HANDLE hS, _In_ QWORD va, _In_ DWORD cb, _Out_writes_opt_(cb) PBYTE pb, _Out_opt_ PDWORD pcbRead);

/*
* 准备（添加）要写入的内存范围。
* 调用此函数时，将处理要写入的内存内容。
* 调用此函数后对 va/pb/cb 的任何更改都不会反映在写入中。
* 稍后在调用 VMMDLL Scatter Execute() 时写入内存。
* 写入先于读取。
* -- h S
* -- va = 要写入的内存范围的起始地址。
* -- pb = 要写入的数据。
* -- cb = 要写入的内存范围的大小。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_PrepareWrite(_In_ VMMDLL_SCATTER_HANDLE hS, _In_ QWORD va, _In_reads_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 准备（添加）要写入的内存范围。
* 调用 VMMDLL Scatter Execute() 时处理要写入的内存内容。
* 调用 VMMDLL Scatter Execute() 时缓冲区 pb 必须有效。
* 调用 VMMDLL Scatter Execute() 时，稍后写入内存。
* 写入先于读取。
* -- h S
* -- va = 要写入的内存范围的起始地址。
* -- pb = 要写入的数据。调用 VMMDLL Scatter Execute() 时缓冲区必须有效。
* -- cb = 要写入的内存范围的大小。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_PrepareWriteEx(_In_ VMMDLL_SCATTER_HANDLE hS, _In_ QWORD va, _In_reads_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 检索并写入先前填充的内存。
* 写入使用 VMMDLL Scatter Prepare Write 函数（第一个）准备的任何内存。
* 检索先前通过调用
* VMMDLL Scatter Prepare* 函数填充的内存范围（第二个）。
* -- h S
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_Execute(_In_ VMMDLL_SCATTER_HANDLE hS);

/*
* 检索先前通过调用 VMMDLL Scatter Prepare* 函数填充的内存范围.
* -- hS
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_ExecuteRead(_In_ VMMDLL_SCATTER_HANDLE hS);

/*
* 读取先前填充范围内的内存。仅应在使用 VMMDLL Scatter Execute Read() 检索内存后调用此函数。
* -- h S
* -- va
* -- cb
* -- pb
* -- pcb Read
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_Read(_In_ VMMDLL_SCATTER_HANDLE hS, _In_ QWORD va, _In_ DWORD cb, _Out_writes_opt_(cb) PBYTE pb, _Out_opt_ PDWORD pcbRead);

/*
* 清除/重置句柄，以便在另一个后续读取散射操作中使用。
* -- h S = 要清除以供重复使用的散射句柄。
* -- dw PID = 可选的 PID 更改。
* -- 标志
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Scatter_Clear(_In_ VMMDLL_SCATTER_HANDLE hS, _In_opt_ DWORD dwPID, _In_ DWORD flags);

/*
* 关闭分散句柄并释放其使用的资源。
* -- h S = 要关闭的分散句柄。
*/
EXPORTED_FUNCTION
VOID VMMDLL_Scatter_CloseHandle(_In_opt_ _Post_ptr_invalid_ VMMDLL_SCATTER_HANDLE hS);



//-----------------------------------------------------------------------------
// VMM PROCESS MAP FUNCTIONALITY BELOW:
// Functionality for retrieving process related collections of items such as
// page table map (PTE), virtual address descriptor map (VAD), loaded modules,
// heaps and threads.
//-----------------------------------------------------------------------------

#define VMMDLL_MAP_PTE_VERSION              2
#define VMMDLL_MAP_VAD_VERSION              6
#define VMMDLL_MAP_VADEX_VERSION            4
#define VMMDLL_MAP_MODULE_VERSION           6
#define VMMDLL_MAP_UNLOADEDMODULE_VERSION   2
#define VMMDLL_MAP_EAT_VERSION              3
#define VMMDLL_MAP_IAT_VERSION              2
#define VMMDLL_MAP_HEAP_VERSION             4
#define VMMDLL_MAP_HEAPALLOC_VERSION        1
#define VMMDLL_MAP_THREAD_VERSION           4
#define VMMDLL_MAP_HANDLE_VERSION           3
#define VMMDLL_MAP_POOL_VERSION             2
#define VMMDLL_MAP_NET_VERSION              3
#define VMMDLL_MAP_PHYSMEM_VERSION          2
#define VMMDLL_MAP_USER_VERSION             2
#define VMMDLL_MAP_VM_VERSION               2
#define VMMDLL_MAP_SERVICE_VERSION          3

// flags to check for existence in the fPage field of VMMDLL_MAP_PTEENTRY
#define VMMDLL_MEMMAP_FLAG_PAGE_W           0x0000000000000002
#define VMMDLL_MEMMAP_FLAG_PAGE_NS          0x0000000000000004
#define VMMDLL_MEMMAP_FLAG_PAGE_NX          0x8000000000000000
#define VMMDLL_MEMMAP_FLAG_PAGE_MASK        0x8000000000000006

#define VMMDLL_POOLMAP_FLAG_ALL             0
#define VMMDLL_POOLMAP_FLAG_BIG             1

#define VMMDLL_MODULE_FLAG_NORMAL           0
#define VMMDLL_MODULE_FLAG_DEBUGINFO        1
#define VMMDLL_MODULE_FLAG_VERSIONINFO      2

typedef enum tdVMMDLL_PTE_TP {
    VMMDLL_PTE_TP_NA = 0,
    VMMDLL_PTE_TP_HARDWARE = 1,
    VMMDLL_PTE_TP_TRANSITION = 2,
    VMMDLL_PTE_TP_PROTOTYPE = 3,
    VMMDLL_PTE_TP_DEMANDZERO = 4,
    VMMDLL_PTE_TP_COMPRESSED = 5,
    VMMDLL_PTE_TP_PAGEFILE = 6,
    VMMDLL_PTE_TP_FILE = 7,
} VMMDLL_PTE_TP, *PVMMDLL_PTE_TP;

typedef struct tdVMMDLL_MAP_PTEENTRY {
    QWORD vaBase;
    QWORD cPages;
    QWORD fPage;
    BOOL  fWoW64;
    DWORD _FutureUse1;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _Reserved1;
    DWORD cSoftware;    // # software (non active) PTEs in region
} VMMDLL_MAP_PTEENTRY, *PVMMDLL_MAP_PTEENTRY;

typedef struct tdVMMDLL_MAP_VADENTRY {
    QWORD vaStart;
    QWORD vaEnd;
    QWORD vaVad;
    // DWORD 0
    DWORD VadType           : 3;   // Pos 0
    DWORD Protection        : 5;   // Pos 3
    DWORD fImage            : 1;   // Pos 8
    DWORD fFile             : 1;   // Pos 9
    DWORD fPageFile         : 1;   // Pos 10
    DWORD fPrivateMemory    : 1;   // Pos 11
    DWORD fTeb              : 1;   // Pos 12
    DWORD fStack            : 1;   // Pos 13
    DWORD fSpare            : 2;   // Pos 14
    DWORD HeapNum           : 7;   // Pos 16
    DWORD fHeap             : 1;   // Pos 23
    DWORD cwszDescription   : 8;   // Pos 24
    // DWORD 1
    DWORD CommitCharge      : 31;   // Pos 0
    DWORD MemCommit         : 1;    // Pos 31
    DWORD u2;
    DWORD cbPrototypePte;
    QWORD vaPrototypePte;
    QWORD vaSubsection;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _FutureUse1;
    DWORD _Reserved1;
    QWORD vaFileObject;             // only valid if fFile/fImage _and_ after wszText is initialized
    DWORD cVadExPages;              // number of "valid" VadEx pages in this VAD.
    DWORD cVadExPagesBase;          // number of "valid" VadEx pages in "previous" VADs
    QWORD _Reserved2;
} VMMDLL_MAP_VADENTRY, *PVMMDLL_MAP_VADENTRY;

#define VMMDLL_VADEXENTRY_FLAG_HARDWARE     0x01
#define VMMDLL_VADEXENTRY_FLAG_W            0x10
#define VMMDLL_VADEXENTRY_FLAG_K            0x40
#define VMMDLL_VADEXENTRY_FLAG_NX           0x80

typedef struct tdVMMDLL_MAP_VADEXENTRY {
    VMMDLL_PTE_TP tp;
    BYTE iPML;
    BYTE pteFlags;
    WORD _Reserved2;
    QWORD va;
    QWORD pa;
    QWORD pte;
    struct {
        DWORD _Reserved1;
        VMMDLL_PTE_TP tp;
        QWORD pa;
        QWORD pte;
    } proto;
    QWORD vaVadBase;
} VMMDLL_MAP_VADEXENTRY, *PVMMDLL_MAP_VADEXENTRY;

typedef enum tdVMMDLL_MODULE_TP {
    VMMDLL_MODULE_TP_NORMAL = 0,
    VMMDLL_MODULE_TP_DATA = 1,
    VMMDLL_MODULE_TP_NOTLINKED = 2,
    VMMDLL_MODULE_TP_INJECTED = 3,
} VMMDLL_MODULE_TP;

typedef struct tdVMMDLL_MAP_MODULEENTRY_DEBUGINFO {
    DWORD dwAge;
    DWORD _Reserved;
    BYTE Guid[16];
    union { LPSTR  uszGuid;             LPWSTR wszGuid;                 };
    union { LPSTR  uszPdbFilename;      LPWSTR wszPdbFilename;          };
} VMMDLL_MAP_MODULEENTRY_DEBUGINFO, *PVMMDLL_MAP_MODULEENTRY_DEBUGINFO;

typedef struct tdVMMDLL_MAP_MODULEENTRY_VERSIONINFO {
    union { LPSTR  uszCompanyName;      LPWSTR wszCompanyName;          };
    union { LPSTR  uszFileDescription;  LPWSTR wszFileDescription;      };
    union { LPSTR  uszFileVersion;      LPWSTR wszFileVersion;          };
    union { LPSTR  uszInternalName;     LPWSTR wszInternalName;         };
    union { LPSTR  uszLegalCopyright;   LPWSTR wszLegalCopyright;       };
    union { LPSTR  uszOriginalFilename; LPWSTR wszFileOriginalFilename; };
    union { LPSTR  uszProductName;      LPWSTR wszProductName;          };
    union { LPSTR  uszProductVersion;   LPWSTR wszProductVersion;       };
} VMMDLL_MAP_MODULEENTRY_VERSIONINFO, *PVMMDLL_MAP_MODULEENTRY_VERSIONINFO;

typedef struct tdVMMDLL_MAP_MODULEENTRY {
    QWORD vaBase;
    QWORD vaEntry;
    DWORD cbImageSize;
    BOOL  fWoW64;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _Reserved3;
    DWORD _Reserved4;
    union { LPSTR  uszFullName; LPWSTR wszFullName; };      // U/W dependant
    VMMDLL_MODULE_TP tp;
    DWORD cbFileSizeRaw;
    DWORD cSection;
    DWORD cEAT;
    DWORD cIAT;
    DWORD _Reserved2;
    QWORD _Reserved1[3];
    PVMMDLL_MAP_MODULEENTRY_DEBUGINFO pExDebugInfo;         // not included by default - use VMMDLL_MODULE_FLAG_DEBUGINFO to include.
    PVMMDLL_MAP_MODULEENTRY_VERSIONINFO pExVersionInfo;     // not included by default - use VMMDLL_MODULE_FLAG_VERSIONINFO to include.
} VMMDLL_MAP_MODULEENTRY, *PVMMDLL_MAP_MODULEENTRY;

typedef struct tdVMMDLL_MAP_UNLOADEDMODULEENTRY {
    QWORD vaBase;
    DWORD cbImageSize;
    BOOL  fWoW64;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _FutureUse1;
    DWORD dwCheckSum;               // user-mode only
    DWORD dwTimeDateStamp;          // user-mode only
    DWORD _Reserved1;
    QWORD ftUnload;                 // kernel-mode only
} VMMDLL_MAP_UNLOADEDMODULEENTRY, *PVMMDLL_MAP_UNLOADEDMODULEENTRY;

typedef struct tdVMMDLL_MAP_EATENTRY {
    QWORD vaFunction;
    DWORD dwOrdinal;
    DWORD oFunctionsArray;          // PIMAGE_EXPORT_DIRECTORY->AddressOfFunctions[oFunctionsArray]
    DWORD oNamesArray;              // PIMAGE_EXPORT_DIRECTORY->AddressOfNames[oNamesArray]
    DWORD _FutureUse1;
    union { LPSTR  uszFunction; LPWSTR wszFunction; };      // U/W dependant
    union { LPSTR  uszForwardedFunction; LPWSTR wszForwardedFunction; };    // U/W dependant (function or ordinal name if exists).
} VMMDLL_MAP_EATENTRY, *PVMMDLL_MAP_EATENTRY;

typedef struct tdVMMDLL_MAP_IATENTRY {
    QWORD vaFunction;
    union { LPSTR  uszFunction; LPWSTR wszFunction; };      // U/W dependant
    DWORD _FutureUse1;
    DWORD _FutureUse2;
    union { LPSTR  uszModule; LPWSTR wszModule; };          // U/W dependant
    struct {
        BOOL f32;
        WORD wHint;
        WORD _Reserved1;
        DWORD rvaFirstThunk;
        DWORD rvaOriginalFirstThunk;
        DWORD rvaNameModule;
        DWORD rvaNameFunction;
    } Thunk;
} VMMDLL_MAP_IATENTRY, *PVMMDLL_MAP_IATENTRY;

typedef enum tdVMMDLL_HEAP_TP {
    VMMDLL_HEAP_TP_NA   = 0,
    VMMDLL_HEAP_TP_NT   = 1,
    VMMDLL_HEAP_TP_SEG  = 2,
} VMMDLL_HEAP_TP, *PVMMDLL_HEAP_TP;

typedef enum tdVMMDLL_HEAP_SEGMENT_TP {
    VMMDLL_HEAP_SEGMENT_TP_NA           = 0,
    VMMDLL_HEAP_SEGMENT_TP_NT_SEGMENT   = 1,
    VMMDLL_HEAP_SEGMENT_TP_NT_LFH       = 2,
    VMMDLL_HEAP_SEGMENT_TP_NT_LARGE     = 3,
    VMMDLL_HEAP_SEGMENT_TP_NT_NA        = 4,
    VMMDLL_HEAP_SEGMENT_TP_SEG_HEAP     = 5,
    VMMDLL_HEAP_SEGMENT_TP_SEG_SEGMENT  = 6,
    VMMDLL_HEAP_SEGMENT_TP_SEG_LARGE    = 7,
    VMMDLL_HEAP_SEGMENT_TP_SEG_NA       = 8,
} VMMDLL_HEAP_SEGMENT_TP, *PVMMDLL_HEAP_SEGMENT_TP;

typedef struct tdVMMDLL_MAP_HEAP_SEGMENTENTRY {
    QWORD va;
    DWORD cb;
    VMMDLL_HEAP_SEGMENT_TP tp : 16;
    DWORD iHeap : 16;
} VMMDLL_MAP_HEAP_SEGMENTENTRY, *PVMMDLL_MAP_HEAP_SEGMENTENTRY;

typedef struct tdVMMDLL_MAP_HEAPENTRY {
    QWORD va;
    VMMDLL_HEAP_TP tp;
    BOOL f32;
    DWORD iHeap;
    DWORD dwHeapNum;
} VMMDLL_MAP_HEAPENTRY, *PVMMDLL_MAP_HEAPENTRY;

typedef enum tdVMMDLL_HEAPALLOC_TP {
    VMMDLL_HEAPALLOC_TP_NA          = 0,
    VMMDLL_HEAPALLOC_TP_NT_HEAP     = 1,
    VMMDLL_HEAPALLOC_TP_NT_LFH      = 2,
    VMMDLL_HEAPALLOC_TP_NT_LARGE    = 3,
    VMMDLL_HEAPALLOC_TP_NT_NA       = 4,
    VMMDLL_HEAPALLOC_TP_SEG_VS      = 5,
    VMMDLL_HEAPALLOC_TP_SEG_LFH     = 6,
    VMMDLL_HEAPALLOC_TP_SEG_LARGE   = 7,
    VMMDLL_HEAPALLOC_TP_SEG_NA      = 8,
} VMMDLL_HEAPALLOC_TP, *PVMMDLL_HEAPALLOC_TP;

typedef struct tdVMMDLL_MAP_HEAPALLOCENTRY {
    QWORD va;
    DWORD cb;
    VMMDLL_HEAPALLOC_TP tp;
} VMMDLL_MAP_HEAPALLOCENTRY, *PVMMDLL_MAP_HEAPALLOCENTRY;

typedef struct tdVMMDLL_MAP_THREADENTRY {
    DWORD dwTID;
    DWORD dwPID;
    DWORD dwExitStatus;
    UCHAR bState;
    UCHAR bRunning;
    UCHAR bPriority;
    UCHAR bBasePriority;
    QWORD vaETHREAD;
    QWORD vaTeb;
    QWORD ftCreateTime;
    QWORD ftExitTime;
    QWORD vaStartAddress;
    QWORD vaStackBaseUser;          // value from _NT_TIB / _TEB
    QWORD vaStackLimitUser;         // value from _NT_TIB / _TEB
    QWORD vaStackBaseKernel;
    QWORD vaStackLimitKernel;
    QWORD vaTrapFrame;
    QWORD vaRIP;                    // RIP register (if user mode)
    QWORD vaRSP;                    // RSP register (if user mode)
    QWORD qwAffinity;
    DWORD dwUserTime;
    DWORD dwKernelTime;
    UCHAR bSuspendCount;
    UCHAR bWaitReason;
    UCHAR _FutureUse1[2];
    DWORD _FutureUse2[11];
    QWORD vaImpersonationToken;
    QWORD vaWin32StartAddress;
} VMMDLL_MAP_THREADENTRY, *PVMMDLL_MAP_THREADENTRY;

typedef struct tdVMMDLL_MAP_HANDLEENTRY {
    QWORD vaObject;
    DWORD dwHandle;
    DWORD dwGrantedAccess : 24;
    DWORD iType : 8;
    QWORD qwHandleCount;
    QWORD qwPointerCount;
    QWORD vaObjectCreateInfo;
    QWORD vaSecurityDescriptor;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _FutureUse2;
    DWORD dwPID;
    DWORD dwPoolTag;
    DWORD _FutureUse[7];
    union { LPSTR  uszType; LPWSTR wszType; QWORD _Pad1; }; // U/W dependant
} VMMDLL_MAP_HANDLEENTRY, *PVMMDLL_MAP_HANDLEENTRY;

typedef enum tdVMMDLL_MAP_POOL_TYPE {
    VMMDLL_MAP_POOL_TYPE_Unknown         = 0,
    VMMDLL_MAP_POOL_TYPE_NonPagedPool    = 1,
    VMMDLL_MAP_POOL_TYPE_NonPagedPoolNx  = 2,
    VMMDLL_MAP_POOL_TYPE_PagedPool       = 3
} VMMDLL_MAP_POOL_TYPE;

typedef enum tdVMM_MAP_POOL_TYPE_SUBSEGMENT {
    VMM_MAP_POOL_TYPE_SUBSEGMENT_UNKNOWN = 0,
    VMM_MAP_POOL_TYPE_SUBSEGMENT_NA      = 1,
    VMM_MAP_POOL_TYPE_SUBSEGMENT_BIG     = 2,
    VMM_MAP_POOL_TYPE_SUBSEGMENT_LARGE   = 3,
    VMM_MAP_POOL_TYPE_SUBSEGMENT_VS      = 4,
    VMM_MAP_POOL_TYPE_SUBSEGMENT_LFH     = 5
} VMM_MAP_POOL_TYPE_SUBSEGMENT;

typedef struct tdVMMDLL_MAP_POOLENTRYTAG {
    union {
        CHAR szTag[5];
        struct {
            DWORD dwTag;
            DWORD _Filler;
            DWORD cEntry;
            DWORD iTag2Map;
        };
    };
} VMMDLL_MAP_POOLENTRYTAG, *PVMMDLL_MAP_POOLENTRYTAG;

typedef struct tdVMMDLL_MAP_POOLENTRY {
    QWORD va;
    union {
        CHAR szTag[5];
        struct {
            DWORD dwTag;
            BYTE _ReservedZero;
            BYTE fAlloc;
            BYTE tpPool;    // VMMDLL_MAP_POOL_TYPE
            BYTE tpSS;      // VMMDLL_MAP_POOL_TYPE_SUBSEGMENT
        };
    };
    DWORD cb;
    DWORD _Filler;
} VMMDLL_MAP_POOLENTRY, *PVMMDLL_MAP_POOLENTRY;

typedef struct tdVMMDLL_MAP_NETENTRY {
    DWORD dwPID;
    DWORD dwState;
    WORD _FutureUse3[3];
    WORD AF;                        // address family (IPv4/IPv6)
    struct {
        BOOL fValid;
        WORD _Reserved;
        WORD port;
        BYTE pbAddr[16];            // ipv4 = 1st 4 bytes, ipv6 = all bytes
        union { LPSTR  uszText; LPWSTR wszText; };          // U/W dependant
    } Src;
    struct {
        BOOL fValid;
        WORD _Reserved;
        WORD port;
        BYTE pbAddr[16];            // ipv4 = 1st 4 bytes, ipv6 = all bytes
        union { LPSTR  uszText; LPWSTR wszText; };          // U/W dependant
    } Dst;
    QWORD vaObj;
    QWORD ftTime;
    DWORD dwPoolTag;
    DWORD _FutureUse4;
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    DWORD _FutureUse2[4];
} VMMDLL_MAP_NETENTRY, *PVMMDLL_MAP_NETENTRY;

typedef struct tdVMMDLL_MAP_PHYSMEMENTRY {
    QWORD pa;
    QWORD cb;
} VMMDLL_MAP_PHYSMEMENTRY, *PVMMDLL_MAP_PHYSMEMENTRY;

typedef struct tdVMMDLL_MAP_USERENTRY {
    DWORD _FutureUse1[2];
    union { LPSTR  uszText; LPWSTR wszText; };              // U/W dependant
    ULONG64 vaRegHive;
    union { LPSTR  uszSID; LPWSTR wszSID; };                // U/W dependant
    DWORD _FutureUse2[2];
} VMMDLL_MAP_USERENTRY, *PVMMDLL_MAP_USERENTRY;

typedef enum tdVMMDLL_VM_TP {
    VMMDLL_VM_TP_UNKNOWN = 0,
    VMMDLL_VM_TP_HV      = 1,
    VMMDLL_VM_TP_HV_WHVP = 2
} VMMDLL_VM_TP;

typedef struct tdVMMDLL_MAP_VMENTRY {
    VMMVM_HANDLE hVM;
    union { LPSTR  uszName; LPWSTR wszName; };              // U/W dependant
    QWORD gpaMax;
    VMMDLL_VM_TP tp;
    BOOL fActive;
    BOOL fReadOnly;
    BOOL fPhysicalOnly;
    DWORD dwPartitionID;
    DWORD dwVersionBuild;
    VMMDLL_SYSTEM_TP tpSystem;
    DWORD dwParentVmmMountID;
    DWORD dwVmMemPID;
} VMMDLL_MAP_VMENTRY, *PVMMDLL_MAP_VMENTRY;

typedef struct tdVMMDLL_MAP_SERVICEENTRY {
    QWORD vaObj;
    DWORD dwOrdinal;
    DWORD dwStartType;
    SERVICE_STATUS ServiceStatus;
    union { LPSTR  uszServiceName; LPWSTR wszServiceName; QWORD _Reserved1; };  // U/W dependant
    union { LPSTR  uszDisplayName; LPWSTR wszDisplayName; QWORD _Reserved2; };  // U/W dependant
    union { LPSTR  uszPath;        LPWSTR wszPath;        QWORD _Reserved3; };  // U/W dependant
    union { LPSTR  uszUserTp;      LPWSTR wszUserTp;      QWORD _Reserved4; };  // U/W dependant
    union { LPSTR  uszUserAcct;    LPWSTR wszUserAcct;    QWORD _Reserved5; };  // U/W dependant
    union { LPSTR  uszImagePath;   LPWSTR wszImagePath;   QWORD _Reserved6; };  // U/W dependant
    DWORD dwPID;
    DWORD _FutureUse1;
    QWORD _FutureUse2;
} VMMDLL_MAP_SERVICEENTRY, *PVMMDLL_MAP_SERVICEENTRY;

typedef struct tdVMMDLL_MAP_PTE {
    DWORD dwVersion;                // VMMDLL_MAP_PTE_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // NULL or multi-wstr pointed into by VMMDLL_MAP_VADENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_PTEENTRY pMap[];     // map entries.
} VMMDLL_MAP_PTE, *PVMMDLL_MAP_PTE;

typedef struct tdVMMDLL_MAP_VAD {
    DWORD dwVersion;                // VMMDLL_MAP_VAD_VERSION
    DWORD _Reserved1[4];
    DWORD cPage;                    // # pages in vad map.
    PBYTE pbMultiText;              // NULL or multi-wstr pointed into by VMMDLL_MAP_VADENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_VADENTRY pMap[];     // map entries.
} VMMDLL_MAP_VAD, *PVMMDLL_MAP_VAD;

typedef struct tdVMMDLL_MAP_VADEX {
    DWORD dwVersion;                // VMMDLL_MAP_VADEX_VERSION
    DWORD _Reserved1[4];
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_VADEXENTRY pMap[];   // map entries.
} VMMDLL_MAP_VADEX, *PVMMDLL_MAP_VADEX;

typedef struct tdVMMDLL_MAP_MODULE {
    DWORD dwVersion;                // VMMDLL_MAP_MODULE_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_MODULEENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_MODULEENTRY pMap[];  // map entries.
} VMMDLL_MAP_MODULE, *PVMMDLL_MAP_MODULE;

typedef struct tdVMMDLL_MAP_UNLOADEDMODULE {
    DWORD dwVersion;                // VMMDLL_MAP_UNLOADEDMODULE_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_MODULEENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_UNLOADEDMODULEENTRY pMap[];  // map entries.
} VMMDLL_MAP_UNLOADEDMODULE, *PVMMDLL_MAP_UNLOADEDMODULE;

typedef struct tdVMMDLL_MAP_EAT {
    DWORD dwVersion;                // VMMDLL_MAP_EAT_VERSION
    DWORD dwOrdinalBase;
    DWORD cNumberOfNames;
    DWORD cNumberOfFunctions;
    DWORD cNumberOfForwardedFunctions;
    DWORD _Reserved1[3];
    QWORD vaModuleBase;
    QWORD vaAddressOfFunctions;
    QWORD vaAddressOfNames;
    PBYTE pbMultiText;              // multi-str pointed into by VMM_MAP_EATENTRY.wszFunction
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_EATENTRY pMap[];     // map entries.
} VMMDLL_MAP_EAT, *PVMMDLL_MAP_EAT;

typedef struct tdVMMDLL_MAP_IAT {
    DWORD dwVersion;                // VMMDLL_MAP_IAT_VERSION
    DWORD _Reserved1[5];
    QWORD vaModuleBase;
    PBYTE pbMultiText;              // multi-str pointed into by VMM_MAP_EATENTRY.[wszFunction|wszModule]
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_IATENTRY pMap[];     // map entries.
} VMMDLL_MAP_IAT, *PVMMDLL_MAP_IAT;

typedef struct tdVMMDLL_MAP_HEAP {
    DWORD dwVersion;                            // VMMDLL_MAP_HEAP_VERSION
    DWORD _Reserved1[7];
    PVMMDLL_MAP_HEAP_SEGMENTENTRY pSegments;    // heap segment entries.
    DWORD cSegments;                            // # heap segment entries.
    DWORD cMap;                                 // # map entries.
    VMMDLL_MAP_HEAPENTRY pMap[];                // map entries.
} VMMDLL_MAP_HEAP, *PVMMDLL_MAP_HEAP;

typedef struct tdVMMDLL_MAP_HEAPALLOC {
    DWORD dwVersion;                    // VMMDLL_MAP_HEAPALLOC_VERSION
    DWORD _Reserved1[7];
    PVOID _Reserved2[2];
    DWORD cMap;                         // # map entries.
    VMMDLL_MAP_HEAPALLOCENTRY pMap[];   // map entries.
} VMMDLL_MAP_HEAPALLOC, *PVMMDLL_MAP_HEAPALLOC;

typedef struct tdVMMDLL_MAP_THREAD {
    DWORD dwVersion;                // VMMDLL_MAP_THREAD_VERSION
    DWORD _Reserved[8];
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_THREADENTRY pMap[];  // map entries.
} VMMDLL_MAP_THREAD, *PVMMDLL_MAP_THREAD;

typedef struct tdVMMDLL_MAP_HANDLE {
    DWORD dwVersion;                // VMMDLL_MAP_HANDLE_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_HANDLEENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_HANDLEENTRY pMap[];  // map entries.
} VMMDLL_MAP_HANDLE, *PVMMDLL_MAP_HANDLE;

typedef struct tdVMMDLL_MAP_POOL {
    DWORD dwVersion;                // VMMDLL_MAP_POOL_VERSION
    DWORD _Reserved1[6];
    DWORD cbTotal;                  // # bytes to represent this pool map object
    PDWORD piTag2Map;               // dword map array (size: cMap): tag index to map index.
    PVMMDLL_MAP_POOLENTRYTAG pTag;  // tag entries.
    DWORD cTag;                     // # tag entries.
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_POOLENTRY pMap[];    // map entries.
} VMMDLL_MAP_POOL, *PVMMDLL_MAP_POOL;

typedef struct tdVMMDLL_MAP_NET {
    DWORD dwVersion;                // VMMDLL_MAP_NET_VERSION
    DWORD _Reserved1;
    PBYTE pbMultiText;              // multi-wstr pointed into by VMM_MAP_NETENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_NETENTRY pMap[];     // map entries.
} VMMDLL_MAP_NET, *PVMMDLL_MAP_NET;

typedef struct tdVMMDLL_MAP_PHYSMEM {
    DWORD dwVersion;                // VMMDLL_MAP_PHYSMEM_VERSION
    DWORD _Reserved1[5];
    DWORD cMap;                     // # map entries.
    DWORD _Reserved2;
    VMMDLL_MAP_PHYSMEMENTRY pMap[]; // map entries.
} VMMDLL_MAP_PHYSMEM, *PVMMDLL_MAP_PHYSMEM;

typedef struct tdVMMDLL_MAP_USER {
    DWORD dwVersion;                // VMMDLL_MAP_USER_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_USERENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_USERENTRY pMap[];    // map entries.
} VMMDLL_MAP_USER, *PVMMDLL_MAP_USER;

typedef struct tdVMMDLL_MAP_VM {
    DWORD dwVersion;                // VMMDLL_MAP_VM_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_VMENTRY.wszText
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_VMENTRY pMap[];      // map entries.
} VMMDLL_MAP_VM, *PVMMDLL_MAP_VM;

typedef struct tdVMMDLL_MAP_SERVICE {
    DWORD dwVersion;                // VMMDLL_MAP_SERVICE_VERSION
    DWORD _Reserved1[5];
    PBYTE pbMultiText;              // multi-wstr pointed into by VMMDLL_MAP_SERVICEENTRY.wsz*
    DWORD cbMultiText;
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_SERVICEENTRY pMap[]; // map entries.
} VMMDLL_MAP_SERVICE, *PVMMDLL_MAP_SERVICE;

/*
* 根据进程的硬件页表 (PTE) 检索内存映射条目。
* 返回的条目按 VMMDLL MAP PTEENTRY.va 排序
* CALLER FREE: VMMDLL Mem Free(*pp Vad Map)
* -- h VMM
* -- dw PID
* -- f 识别模块 = 尝试识别模块（= 更慢）
* -- pp Pte Map = ptr 成功后接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetPteU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ BOOL fIdentifyModules, _Out_ PVMMDLL_MAP_PTE *ppPteMap);
_Success_(return) BOOL VMMDLL_Map_GetPteW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ BOOL fIdentifyModules, _Out_ PVMMDLL_MAP_PTE *ppPteMap);

/*
* 根据进程的虚拟地址描述符 (VAD) 检索内存映射条目。
* 返回的条目按 VMMDLL MAP VADENTRY.va Start 排序
* CALLER FREE: VMMDLL Mem Free(*pp Vad Map)
* -- h VMM
* -- dw PID
* -- f 识别模块 = 尝试识别模块（= 更慢）
* -- pp Vad Map = ptr 成功后接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetVadU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ BOOL fIdentifyModules, _Out_ PVMMDLL_MAP_VAD *ppVadMap);
_Success_(return) BOOL VMMDLL_Map_GetVadW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ BOOL fIdentifyModules, _Out_ PVMMDLL_MAP_VAD *ppVadMap);

/*
* 检索有关内存映射子集的扩展内存映射信息。
* CALLER FREE: VMMDLL Mem Free(*pp Vad Ex Map)
* -- h VMM
* -- o Page = 从进程开始的页面数偏移量。
* -- c Page = 要从 o Pages 基数开始处理的页面数。
* -- pp Vad Ex Map = 成功时接收结果的 ptr。必须使用 VMMDLL Mem Free() 释放。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Map_GetVadEx(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ DWORD oPage, _In_ DWORD cPage, _Out_ PVMMDLL_MAP_VADEX *ppVadExMap);

/*
* 检索指定进程的模块 (.dll)。
* CALLER FREE: VMMDLL Mem Free(*pp Module Map)
* -- h VMM
* -- dw PID
* -- pp Module Map = ptr，用于在成功时接收结果。必须使用 VMMDLL Mem Free() 进行释放。
* -- flags = 由 VMMDLL MODULE FLAG 指定的可选标志 *
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetModuleU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_MODULE *ppModuleMap, _In_ DWORD flags);
_Success_(return) BOOL VMMDLL_Map_GetModuleW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_MODULE *ppModuleMap, _In_ DWORD flags);

/*
* 根据给定的进程和模块名称检索模块 (.dll) 条目。
* CALLER FREE: VMMDLL Mem Free(*pp Module Map Entry)
* -- h VMM
* -- dw PID
* -- [uw]sz Module Name = 模块名称（或第一个模块条目为 ""/NULL）。
* -- pp Module Map Entry = ptr，成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- flags = VMMDLL MODULE FLAG 指定的可选标志 *
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetModuleFromNameU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_opt_ LPCSTR  uszModuleName, _Out_ PVMMDLL_MAP_MODULEENTRY *ppModuleMapEntry, _In_ DWORD flags);
_Success_(return) BOOL VMMDLL_Map_GetModuleFromNameW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_opt_ LPCWSTR wszModuleName, _Out_ PVMMDLL_MAP_MODULEENTRY *ppModuleMapEntry, _In_ DWORD flags);

/*
* 检索指定进程的已卸载模块 (.dll/.sys)。
* CALLER FREE: VMMDLL Mem Free(*pp Unloaded Module Map)
* -- h VMM
* -- dw PID
* -- pp Unloaded Module Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetUnloadedModuleU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_UNLOADEDMODULE *ppUnloadedModuleMap);
_Success_(return) BOOL VMMDLL_Map_GetUnloadedModuleW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_UNLOADEDMODULE *ppUnloadedModuleMap);

/*
* 从导出地址表 (EAT) 中检索模块导出的函数。
* CALLER FREE: VMMDLL Mem Free(*pp Eat Map)
* -- h VMM
* -- dw PID
* -- [uw]sz 模块名称
* -- pp Eat Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败.
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetEATU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModuleName, _Out_ PVMMDLL_MAP_EAT *ppEatMap);
_Success_(return) BOOL VMMDLL_Map_GetEATW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModuleName, _Out_ PVMMDLL_MAP_EAT *ppEatMap);

/*
* 从导入地址表 (IAT) 检索模块导入的函数。
* CALLER FREE: VMMDLL Mem Free(*pp Iat Map)
* -- h VMM
* -- dw PID
* -- [uw]sz 模块名称
* -- pp Iat Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetIATU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModuleName, _Out_ PVMMDLL_MAP_IAT *ppIatMap);
_Success_(return) BOOL VMMDLL_Map_GetIATW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModuleName, _Out_ PVMMDLL_MAP_IAT *ppIatMap);

/*
* 检索指定进程的堆。
* CALLER FREE: VMMDLL Mem Free(*pp Heap Map)
* -- h VMM
* -- dw PID
* -- pp Heap Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetHeap(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_HEAP *ppHeapMap);

/*
* 检索指定进程堆的堆分配。
* CALLER FREE: VMMDLL Mem Free(*pp Heap Alloc Map)
* -- h VMM
* -- dw PID
* -- qw Heap Num Or Address = 要从中检索分配的堆的编号或虚拟地址。
* -- pp Heap Alloc Map = 成功时接收结果的 ptr。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetHeapAlloc(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ QWORD qwHeapNumOrAddress, _Out_ PVMMDLL_MAP_HEAPALLOC *ppHeapAllocMap);

/*
* 检索指定进程的线程。
* 返回的条目按 VMMDLL MAP THREADENTRY.dw TID 排序
* CALLER FREE: VMMDLL Mem Free(*pp Thread Map)
* -- h VMM
* -- dw PID
* -- pp Thread Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetThread(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_THREAD *ppThreadMap);

/*
* 检索指定进程的句柄。
* 返回的条目按 VMMDLL MAP HANDLEENTRY.dw Handle 排序
* CALLER FREE: VMMDLL Mem Free(*pp Handle Map)
* -- h VMM
* -- dw PID
* -- pp Handle Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetHandleU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_HANDLE *ppHandleMap);
_Success_(return) BOOL VMMDLL_Map_GetHandleW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _Out_ PVMMDLL_MAP_HANDLE *ppHandleMap);

/*
* 从操作系统物理内存映射中检索物理内存范围。
* CALLER FREE: VMMDLL Mem Free(*pp Phys Mem Map)
* -- h VMM
* -- pp Phys Mem Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetPhysMem(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_PHYSMEM *ppPhysMemMap);

/*
* 检索池映射 - 由内核分配的池条目组成。
* 池映射 p Map 按分配虚拟地址排序。
* 池映射 p Tag 按池标签排序。
* 注意！池映射可能包含误报/阳性。
* 注意！池映射依赖于调试符号。请确保支持文件
* symsrv.dll、dbghelp.dll 和 info.db（在二进制分发中找到）
* 与 vmm.dll 放在一起。（在 Linux 上，.dll 文件不是必需的）。
* CALLER FREE：VMMDLL Mem Free（*pp Pool Map）
* -- h VMM
* -- pp Pool Map = ptr 成功后接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- flags = VMMDLL POOLMAP FLAG*
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetPool(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_POOL *ppPoolMap, _In_ DWORD flags);

/*
* 检索网络连接图 - 包括活动网络连接、
* 监听套接字和其他网络功能。
* CALLER FREE：VMMDLL Mem Free(*pp Net Map)
* -- h VMM
* -- pp Net Map = ptr 成功时接收结果。必须使用 VMMDLL Mem Free() 释放。
* -- return = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetNetU(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_NET *ppNetMap);
_Success_(return) BOOL VMMDLL_Map_GetNetW(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_NET *ppNetMap);

/*
* Retrieve the non well known users that are detected in the target system.
* NB! There may be more users in the system than the ones that are detected,
* only users with mounted registry hives may currently be detected - this is
* the normal behaviour for users with active processes.
* CALLER FREE: VMMDLL_MemFree(*ppUserMap)
* -- hVMM
* -- ppUserMap = ptr to receive result on success. must be free'd with VMMDLL_MemFree().
* -- return = success/fail.
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetUsersU(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_USER *ppUserMap);
_Success_(return) BOOL VMMDLL_Map_GetUsersW(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_USER *ppUserMap);

/*
* Retrieve a map of detected child virtual machines (VMs).
* NB! May fail if called shortly after vmm init unless option: -waitinitialize
* CALLER FREE: VMMDLL_MemFree(*ppVmMap)
* -- hVMM
* -- ppVmMap = ptr to receive result on success. must be free'd with VMMDLL_MemFree().
* -- return = success/fail.
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetVMU(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_VM *ppVmMap);
_Success_(return) BOOL VMMDLL_Map_GetVMW(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_VM *ppVmMap);

/*
* Retrieve the services currently known by the service control manager (SCM).
* CALLER FREE: VMMDLL_MemFree(*ppServiceMap)
* -- hVMM
* -- ppServiceMap = ptr to receive result on success. must be free'd with VMMDLL_MemFree().
* -- return = success/fail.
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_Map_GetServicesU(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_SERVICE *ppServiceMap);
_Success_(return) BOOL VMMDLL_Map_GetServicesW(_In_ VMM_HANDLE hVMM, _Out_ PVMMDLL_MAP_SERVICE *ppServiceMap);



//-----------------------------------------------------------------------------
// MEMORY SEARCH FUNCTIONALITY:
//-----------------------------------------------------------------------------

#define VMMDLL_MEM_SEARCH_VERSION           0xfe3e0002
#define VMMDLL_MEM_SEARCH_MAX               16
#define VMMDLL_MEM_SEARCH_MAXLENGTH         32

typedef struct tdVMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY {
    DWORD cbAlign;                                  // byte-align at 2^x - 0, 1, 2, 4, 8, 16, .. bytes.
    DWORD cb;                                       // number of bytes to search (1-32).
    BYTE pb[VMMDLL_MEM_SEARCH_MAXLENGTH];
    BYTE pbSkipMask[VMMDLL_MEM_SEARCH_MAXLENGTH];   // skip bitmask '0' = match, '1' = wildcard.
} VMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY, *PVMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY;

/*
* Context to populate and use in the VMMDLL_MemSearch() function.
*/
typedef struct tdVMMDLL_MEM_SEARCH_CONTEXT {
    DWORD dwVersion;
    DWORD _Filler[2];
    BOOL fAbortRequested;       // may be set by caller to abort processing prematurely.
    DWORD cMaxResult;           // # max result entries. '0' = 1 entry. max 0x10000 entries.
    DWORD cSearch;              // number of search entries.
    VMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY search[VMMDLL_MEM_SEARCH_MAX];
    QWORD vaMin;                // min address to search (page-aligned).
    QWORD vaMax;                // max address to search (page-aligned), if 0 max memory is assumed.
    QWORD vaCurrent;            // current address (may be read by caller).
    DWORD _Filler2;
    DWORD cResult;              // number of search hits.
    QWORD cbReadTotal;          // total number of bytes read.
    PVOID pvUserPtrOpt;         // optional pointer set by caller (used for context passing to callbacks)
    // optional result callback function.
    // use of callback function disable ordinary result in ppObAddressResult.
    // return = continue search(TRUE), abort search(FALSE).
    BOOL(*pfnResultOptCB)(_In_ struct tdVMMDLL_MEM_SEARCH_CONTEXT *ctx, _In_ QWORD va, _In_ DWORD iSearch);
    // non-recommended features:
    QWORD ReadFlags;            // read flags as in VMMDLL_FLAG_*
    BOOL fForcePTE;             // force PTE method for virtual address reads.
    BOOL fForceVAD;             // force VAD method for virtual address reads.
    // optional filter callback function for virtual address reads:
    // for ranges inbetween vaMin:vaMax callback with pte or vad entry.
    // return: read from range(TRUE), do not read from range(FALSE).
    BOOL(*pfnFilterOptCB)(_In_ struct tdVMMDLL_MEM_SEARCH_CONTEXT *ctx, _In_opt_ PVMMDLL_MAP_PTEENTRY pePte, _In_opt_ PVMMDLL_MAP_VADENTRY peVad);
} VMMDLL_MEM_SEARCH_CONTEXT, *PVMMDLL_MEM_SEARCH_CONTEXT;

/*
* 在提供的上下文指定的地址空间中搜索二进制数据。
* 有关不同搜索参数的更多信息，请参阅
* 结构定义：VMMDLL MEM SEARCH CONTEXT
* 搜索可能需要很长时间。不建议以交互方式运行。
* 要提前取消搜索，请在上下文中设置 f Abort Requested 标志
* 并等待片刻。
* CALLER FREE：VMMDLL Mem Free(*ppva)
* -- h VMM
* -- dw PID - 目标进程的 PID，(DWORD)-1 用于读取物理内存。
* -- ctx
* -- ppva = 指向找到的接收地址的指针。使用 VMMDLL Mem Free() 释放。
* -- pcva = 指向 ppva 中地址数量的指针。不是字节！
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_MemSearch(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _Inout_ PVMMDLL_MEM_SEARCH_CONTEXT ctx,
    _Out_opt_ PQWORD *ppva,
    _Out_opt_ PDWORD pcva
);



//-----------------------------------------------------------------------------
// MEMORY YARA SEARCH FUNCTIONALITY:
// The yara search functionality requires that vmmyara.[dll|so] is present.
// The vmmyara project is found at: https://github.com/ufrisk/vmmyara
//-----------------------------------------------------------------------------

// =========== START SHARED STRUCTS WITH <vmmdll.h/vmmyara.h> ===========
#ifndef VMMYARA_RULE_MATCH_DEFINED
#define VMMYARA_RULE_MATCH_DEFINED

#define VMMYARA_RULE_MATCH_VERSION          0xfedc0003
#define VMMYARA_RULE_MATCH_TAG_MAX          8
#define VMMYARA_RULE_MATCH_META_MAX         16
#define VMMYARA_RULE_MATCH_STRING_MAX       8
#define VMMYARA_RULE_MATCH_OFFSET_MAX       16

/*
* Struct with match information upon a match in VmmYara_RulesScanMemory().
*/
typedef struct tdVMMYARA_RULE_MATCH {
    DWORD dwVersion;                    // VMMYARA_RULE_MATCH_VERSION
    DWORD flags;
    LPSTR szRuleIdentifier;
    DWORD cTags;
    LPSTR szTags[VMMYARA_RULE_MATCH_TAG_MAX];
    DWORD cMeta;
    struct {
        LPSTR szIdentifier;
        LPSTR szString;
    } Meta[VMMYARA_RULE_MATCH_META_MAX];
    DWORD cStrings;
    struct {
        LPSTR szString;
        DWORD cMatch;
        SIZE_T cbMatchOffset[VMMYARA_RULE_MATCH_OFFSET_MAX];
    } Strings[VMMYARA_RULE_MATCH_STRING_MAX];
} VMMYARA_RULE_MATCH, *PVMMYARA_RULE_MATCH;

#endif /* VMMYARA_RULE_MATCH_DEFINED */

#ifndef VMMYARA_SCAN_MEMORY_CALLBACK_DEFINED
#define VMMYARA_SCAN_MEMORY_CALLBACK_DEFINED

/*
* 回调函数，在 VmmYara_RulesScanMemory() 匹配时被调用。
* -- pvContext = 在调用 VmmYara_ScanMemory() 时设置的用户上下文。
* -- pRuleMatch = 指向匹配信息的指针。
* -- pbBuffer = 被扫描的内存缓冲区。
* -- cbBuffer = 被扫描的内存缓冲区大小。
* -- return = 返回 TRUE 继续扫描，返回 FALSE 停止扫描。
*/
typedef BOOL(*VMMYARA_SCAN_MEMORY_CALLBACK)(
    _In_ PVOID pvContext,
    _In_ PVMMYARA_RULE_MATCH pRuleMatch,
    _In_reads_bytes_(cbBuffer) PBYTE pbBuffer,
    _In_ SIZE_T cbBuffer
);

#endif /* VMMYARA_SCAN_MEMORY_CALLBACK_DEFINED */
// =========== END SHARED STRUCTS WITH <vmmdll.h/vmmyara.h> ===========

#define VMMDLL_YARA_CONFIG_VERSION                  0xdec30001
#define VMMDLL_YARA_MEMORY_CALLBACK_CONTEXT_VERSION 0xdec40002
#define VMMDLL_YARA_CONFIG_MAX_RESULT               0x00010000      // max 65k results.

typedef struct tdVMMDLL_YARA_CONFIG *PVMMDLL_YARA_CONFIG;           // forward declaration.

/*
* 回调函数，用于判断某个内存区域是否应被扫描。
* -- ctx = 指向 PVMMDLL_YARA_CONFIG 上下文的指针。
* -- pePte = 如果内存区域由 PTE 映射支持，则为 PTE 条目的指针，否则为 NULL。
* -- peVad = 如果内存区域由 VAD 映射支持，则为 VAD 条目的指针，否则为 NULL。
* -- return = 返回 TRUE 表示扫描该内存区域，返回 FALSE 表示跳过。
*/
typedef BOOL(*VMMYARA_SCAN_FILTER_CALLBACK)(
    _In_ PVMMDLL_YARA_CONFIG ctx,
    _In_opt_ PVMMDLL_MAP_PTEENTRY pePte,
    _In_opt_ PVMMDLL_MAP_VADENTRY peVad
);

/*
* Yara search configuration struct.
*/
typedef struct tdVMMDLL_YARA_CONFIG {
    DWORD dwVersion;            // VMMDLL_YARA_CONFIG_VERSION
    DWORD _Filler[2];
    BOOL fAbortRequested;       // may be set by caller to abort processing prematurely.
    DWORD cMaxResult;           // # max result entries. max 0x10000 entries. 0 = max entries.
    DWORD cRules;               // number of rules to use - if compiled rules only 1 is allowed.
    LPSTR *pszRules;            // array of rules to use - either filenames or in-memory rules.
    QWORD vaMin;
    QWORD vaMax;
    QWORD vaCurrent;            // current address (may be read by caller).
    DWORD _Filler2;
    DWORD cResult;              // number of search hits.
    QWORD cbReadTotal;          // total number of bytes read.
    PVOID pvUserPtrOpt;         // optional pointer set by caller (used for context passing to callbacks)
    // match callback function (recommended but optional).
    // return = continue search(TRUE), abort search(FALSE).
    VMMYARA_SCAN_MEMORY_CALLBACK pfnScanMemoryCB;
    // non-recommended features:
    QWORD ReadFlags;            // read flags as in VMMDLL_FLAG_*
    BOOL fForcePTE;             // force PTE method for virtual address reads.
    BOOL fForceVAD;             // force VAD method for virtual address reads.
    // optional filter callback function for virtual address reads:
    // for ranges inbetween vaMin:vaMax callback with pte or vad entry.
    // return: read from range(TRUE), do not read from range(FALSE).
    VMMYARA_SCAN_FILTER_CALLBACK pfnFilterOptCB;
    PVOID pvUserPtrOpt2;        // optional pointer set by caller (not used by MemProcFS).
    QWORD _Reserved;
} VMMDLL_YARA_CONFIG, *PVMMDLL_YARA_CONFIG;

/*
* Yara search callback struct which created by MemProcFS internally and is
* passed to the callback function supplied by the caller in VMMDLL_YaraSearch().
*/
typedef struct tdVMMDLL_YARA_MEMORY_CALLBACK_CONTEXT {
    DWORD dwVersion;
    DWORD dwPID;
    PVOID pUserContext;
    QWORD vaObject;
    QWORD va;
    PBYTE pb;
    DWORD cb;
    LPSTR uszTag[1];    // min 1 char (but may be more).
} VMMDLL_YARA_MEMORY_CALLBACK_CONTEXT, *PVMMDLL_YARA_MEMORY_CALLBACK_CONTEXT;

/*
* Perform a yara search in the address space of a process.
* NB! it may take a long time for this function to return.
* -- hVMM
* -- dwPID - PID of target process, (DWORD)-1 to read physical memory.
* -- pYaraConfig
* -- ppva = pointer to receive addresses found. Free'd with VMMDLL_MemFree().
* -- pcva = pointer to receive number of addresses in ppva. not bytes!
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_YaraSearch(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _In_ PVMMDLL_YARA_CONFIG pYaraConfig,
    _Out_opt_ PQWORD *ppva,
    _Out_opt_ PDWORD pcva
);



//-----------------------------------------------------------------------------
// WINDOWS SPECIFIC PAGE FRAME NUMBER (PFN) FUNCTIONALITY BELOW
//-----------------------------------------------------------------------------

#define VMMDLL_MAP_PFN_VERSION              1

#define VMMDLL_PFN_FLAG_NORMAL              0
#define VMMDLL_PFN_FLAG_EXTENDED            1

static LPCSTR VMMDLL_PFN_TYPE_TEXT[] = { "Zero", "Free", "Standby", "Modifiy", "ModNoWr", "Bad", "Active", "Transit" };
static LPCSTR VMMDLL_PFN_TYPEEXTENDED_TEXT[] = { "-", "Unused", "ProcPriv", "PageTable", "LargePage", "DriverLock", "Shareable", "File" };

typedef enum tdVMMDLL_MAP_PFN_TYPE {
    VmmDll_PfnTypeZero = 0,
    VmmDll_PfnTypeFree = 1,
    VmmDll_PfnTypeStandby = 2,
    VmmDll_PfnTypeModified = 3,
    VmmDll_PfnTypeModifiedNoWrite = 4,
    VmmDll_PfnTypeBad = 5,
    VmmDll_PfnTypeActive = 6,
    VmmDll_PfnTypeTransition = 7
} VMMDLL_MAP_PFN_TYPE;

typedef enum tdVMMDLL_MAP_PFN_TYPEEXTENDED {
    VmmDll_PfnExType_Unknown = 0,
    VmmDll_PfnExType_Unused = 1,
    VmmDll_PfnExType_ProcessPrivate = 2,
    VmmDll_PfnExType_PageTable = 3,
    VmmDll_PfnExType_LargePage = 4,
    VmmDll_PfnExType_DriverLocked = 5,
    VmmDll_PfnExType_Shareable = 6,
    VmmDll_PfnExType_File = 7,
} VMMDLL_MAP_PFN_TYPEEXTENDED;

typedef struct tdVMMDLL_MAP_PFNENTRY {
    DWORD dwPfn;
    VMMDLL_MAP_PFN_TYPEEXTENDED tpExtended;
    struct {        // Only valid if active non-prototype PFN
        union {
            DWORD dwPid;
            DWORD dwPfnPte[5];  // PFN of paging levels 1-4 (x64)
        };
        QWORD va;               // valid if non-zero
    } AddressInfo;
    QWORD vaPte;
    QWORD OriginalPte;
    union {
        DWORD _u3;
        struct {
            WORD ReferenceCount;
            // MMPFNENTRY
            BYTE PageLocation       : 3;    // Pos 0  - VMMDLL_MAP_PFN_TYPE
            BYTE WriteInProgress    : 1;    // Pos 3
            BYTE Modified           : 1;    // Pos 4
            BYTE ReadInProgress     : 1;    // Pos 5
            BYTE CacheAttribute     : 2;    // Pos 6
            BYTE Priority           : 3;    // Pos 0
            BYTE Rom_OnProtectedStandby : 1;// Pos 3
            BYTE InPageError        : 1;    // Pos 4
            BYTE KernelStack_SystemChargedPage : 1; // Pos 5
            BYTE RemovalRequested   : 1;    // Pos 6
            BYTE ParityError        : 1;    // Pos 7
        };
    };
    union {
        QWORD _u4;
        struct {
            DWORD PteFrame;
            DWORD PteFrameHigh      : 4;    // Pos 32
            DWORD _Reserved         : 21;   // Pos 36
            DWORD PrototypePte      : 1;    // Pos 57
            DWORD PageColor         : 6;    // Pos 58
        };
    };
    DWORD _FutureUse[6];
} VMMDLL_MAP_PFNENTRY, *PVMMDLL_MAP_PFNENTRY;

typedef struct tdVMMDLL_MAP_PFN {
    DWORD dwVersion;
    DWORD _Reserved1[5];
    DWORD cMap;                     // # map entries.
    VMMDLL_MAP_PFNENTRY pMap[];     // map entries.
} VMMDLL_MAP_PFN, *PVMMDLL_MAP_PFN;

/*
* Retrieve information about scattered PFNs. The PFNs are returned in order of
* in which they are stored in the pPfns set.
* -- hVMM
* -- pPfns
* -- cPfns
* -- pPfnMap = buffer of minimum byte length *pcbPfnMap or NULL.
* -- pcbPfnMap = pointer to byte count of pPhysMemMap buffer.
* -- return = success/fail.
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Map_GetPfn(
    _In_ VMM_HANDLE hVMM,
    _In_reads_(cPfns) DWORD pPfns[],
    _In_ DWORD cPfns,
    _Out_writes_bytes_opt_(*pcbPfnMap) PVMMDLL_MAP_PFN pPfnMap,
    _Inout_ PDWORD pcbPfnMap
);

/*
* Retrieve PFN information:
* CALLER FREE: VMMDLL_MemFree(*ppPfnMap)
* -- hVMM
* -- pPfns = PFNs to retrieve.
* -- cPfns = number of PFNs to retrieve.
* -- ppPfnMap =  ptr to receive result on success. must be free'd with VMMDLL_MemFree().
* -- flags = optional flags as specified by VMMDLL_PFN_FLAG_*
* -- return = success/fail.
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_Map_GetPfnEx(
    _In_ VMM_HANDLE hVMM,
    _In_reads_(cPfns) DWORD pPfns[],
    _In_ DWORD cPfns,
    _Out_ PVMMDLL_MAP_PFN *ppPfnMap,
    _In_ DWORD flags
);



//-----------------------------------------------------------------------------
// VMM PROCESS FUNCTIONALITY BELOW:
// Functionality below is mostly relating to Windows processes.
//-----------------------------------------------------------------------------

/*
* Retrieve an active process given it's name. Please note that if multiple
* processes with the same name exists only one will be returned. If required to
* parse all processes with the same name please iterate over the PID list by
* calling VMMDLL_PidList together with VMMDLL_ProcessGetInformation.
* -- hVMM
* -- szProcName = process name case insensitive.
* -- pdwPID = pointer that will receive PID on success.
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PidGetFromName(_In_ VMM_HANDLE hVMM, _In_ LPCSTR szProcName, _Out_ PDWORD pdwPID);

/*
* List the PIDs in the system.
* -- hVMM
* -- pPIDs = DWORD array of at least number of PIDs in system, or NULL.
* -- pcPIDs = size of (in number of DWORDs) pPIDs array on entry, number of PIDs in system on exit.
* -- return = success/fail.
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PidList(_In_ VMM_HANDLE hVMM, _Out_writes_opt_(*pcPIDs) PDWORD pPIDs, _Inout_ PSIZE_T pcPIDs);

#define VMMDLL_PROCESS_INFORMATION_MAGIC        0xc0ffee663df9301e
#define VMMDLL_PROCESS_INFORMATION_VERSION      7

typedef enum tdVMMDLL_PROCESS_INTEGRITY_LEVEL {
    VMMDLL_PROCESS_INTEGRITY_LEVEL_UNKNOWN      = 0,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_UNTRUSTED    = 1,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_LOW          = 2,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_MEDIUM       = 3,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_MEDIUMPLUS   = 4,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_HIGH         = 5,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_SYSTEM       = 6,
    VMMDLL_PROCESS_INTEGRITY_LEVEL_PROTECTED    = 7,
} VMMDLL_PROCESS_INTEGRITY_LEVEL;

typedef struct tdVMMDLL_PROCESS_INFORMATION {
    ULONG64 magic;
    WORD wVersion;
    WORD wSize;
    VMMDLL_MEMORYMODEL_TP tpMemoryModel;    // as given by VMMDLL_MEMORYMODEL_* enum
    VMMDLL_SYSTEM_TP tpSystem;              // as given by VMMDLL_SYSTEM_* enum
    BOOL fUserOnly;                         // only user mode pages listed
    DWORD dwPID;
    DWORD dwPPID;
    DWORD dwState;
    CHAR szName[16];
    CHAR szNameLong[64];
    ULONG64 paDTB;
    ULONG64 paDTB_UserOpt;                  // may not exist
    struct {
        ULONG64 vaEPROCESS;
        ULONG64 vaPEB;
        ULONG64 _Reserved1;
        BOOL fWow64;
        DWORD vaPEB32;                  // WoW64 only
        DWORD dwSessionId;
        ULONG64 qwLUID;
        CHAR szSID[MAX_PATH];
        VMMDLL_PROCESS_INTEGRITY_LEVEL IntegrityLevel;
    } win;
} VMMDLL_PROCESS_INFORMATION, *PVMMDLL_PROCESS_INFORMATION;

/*
* 从 PID 检索各种进程信息。可以检索进程信息，例如
* 名称、页面目录基和进程状态。
* -- h VMM
* -- dw PID
* -- p 进程信息 = 如果为空，则大小在 *pcb 进程信息中给出
* -- pcb 进程信息 = 进入和退出时 p 进程信息的大小（以字节为单位）
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_ProcessGetInformation(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _Inout_opt_ PVMMDLL_PROCESS_INFORMATION pProcessInformation,
    _In_ PSIZE_T pcbProcessInformation
);

/*
* 从所有进程（包括已终止的进程）检索各种信息。
* CALLER FREE : VMMDLL Mem Free(*pp Process Information All)
* -- h VMM
* -- 成功时，ptr 接收 pc Process Information 项目的结果数组。
* 必须使用 VMMDLL Mem Free() 进行释放。
* -- 成功时，ptr 指向 DWORD，用于接收项目进程数。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_ProcessGetInformationAll(
    _In_ VMM_HANDLE hVMM,
    _Out_ PVMMDLL_PROCESS_INFORMATION *ppProcessInformationAll,
    _Out_ PDWORD pcProcessInformation
);

#define VMMDLL_PROCESS_INFORMATION_OPT_STRING_PATH_KERNEL           1
#define VMMDLL_PROCESS_INFORMATION_OPT_STRING_PATH_USER_IMAGE       2
#define VMMDLL_PROCESS_INFORMATION_OPT_STRING_CMDLINE               3

/*
* Retrieve a string value belonging to a process. The function allocates a new
* string buffer and returns the requested string in it. The string is always
* NULL terminated. On failure NULL is returned.
* NB! CALLER IS RESPONSIBLE FOR VMMDLL_MemFree return value!
* CALLER FREE: VMMDLL_MemFree(return)
* -- hVMM
* -- dwPID
* -- fOptionString = string value to retrieve as given by VMMDLL_PROCESS_INFORMATION_OPT_STRING_*
* -- return - fail: NULL, success: the string - NB! must be VMMDLL_MemFree'd by caller!
*/
EXPORTED_FUNCTION _Success_(return != NULL)
LPSTR VMMDLL_ProcessGetInformationString(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ DWORD fOptionString);

/*
* 检索有关以下项的信息：??数据目录、部分、导出地址表
* 和导入地址表 (IAT)。
* 如果 p 数据在输入时 == NULL，则 p 数据数组的条目数必须
* 具有才能保存返回的数据。
* -- h VMM
* -- dw PID
* -- [uw]sz 模块
* -- p 数据
* -- c 数据
* -- pc 数据
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_ProcessGetDirectoriesU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModule, _Out_writes_(16) PIMAGE_DATA_DIRECTORY pDataDirectories);
_Success_(return) BOOL VMMDLL_ProcessGetDirectoriesW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModule, _Out_writes_(16) PIMAGE_DATA_DIRECTORY pDataDirectories);
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_ProcessGetSectionsU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModule, _Out_writes_opt_(cSections) PIMAGE_SECTION_HEADER pSections, _In_ DWORD cSections, _Out_ PDWORD pcSections);
_Success_(return) BOOL VMMDLL_ProcessGetSectionsW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModule, _Out_writes_opt_(cSections) PIMAGE_SECTION_HEADER pSections, _In_ DWORD cSections, _Out_ PDWORD pcSections);

/*
* 检索进程/模块内给定函数的虚拟地址。
* -- h VMM
* -- dw PID
* -- [uw]sz 模块名称
* -- sz 函数名称
* -- 返回 = 函数的虚拟地址，失败时返回零。
*/
EXPORTED_FUNCTION
_Success_(return != 0) ULONG64 VMMDLL_ProcessGetProcAddressU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModuleName, _In_ LPCSTR szFunctionName);
_Success_(return != 0) ULONG64 VMMDLL_ProcessGetProcAddressW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModuleName, _In_ LPCSTR szFunctionName);

/*
* 检索给定模块的基地址。
* -- h VMM
* -- dw PID
* -- [uw]sz 模块名称
* -- 返回 = 模块基址的虚拟地址，失败时返回零。
*/
EXPORTED_FUNCTION
_Success_(return != 0) ULONG64 VMMDLL_ProcessGetModuleBaseU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModuleName);
_Success_(return != 0) ULONG64 VMMDLL_ProcessGetModuleBaseW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModuleName);



//-----------------------------------------------------------------------------
// WINDOWS SPECIFIC DEBUGGING / SYMBOL FUNCTIONALITY BELOW:
//-----------------------------------------------------------------------------

/*
* 加载 .pdb 符号文件并在成功时返回其关联的模块名称。
* -- h VMM
* -- dw PID
* -- va 模块库
* -- sz 模块名称 = 成功后接收模块名称的缓冲区。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PdbLoad(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _In_ ULONG64 vaModuleBase,
    _Out_writes_(MAX_PATH) LPSTR szModuleName
);

/*
* 给定模块名称和符号名称，检索符号虚拟地址。
* 注意！并非所有模块都存在 - 最初只有模块“nt”可用。
* 注意！如果多个模块具有相同的名称，则将使用第一个要添加的模块。
* -- h VMM
* -- sz 模块
* -- cb 符号地址或偏移量 = 符号虚拟地址或符号偏移量。
* -- sz 符号名称 = 成功后接收符号名称的缓冲区。
* -- pdw 符号位移 = 从符号开头的位移。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PdbSymbolName(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR szModule,
    _In_ QWORD cbSymbolAddressOrOffset,
    _Out_writes_(MAX_PATH) LPSTR szSymbolName,
    _Out_opt_ PDWORD pdwSymbolDisplacement
);

/*
* 给定模块名称和符号名称，检索符号虚拟地址。
* 注意！并非所有模块都存在 - 最初只有模块“nt”可用。
* 注意！如果多个模块具有相同的名称，则将使用第一个要添加的模块。
* -- h VMM
* -- sz 模块
* -- sz 符号名称
* -- pva 符号地址
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PdbSymbolAddress(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR szModule,
    _In_ LPCSTR szSymbolName,
    _Out_ PULONG64 pvaSymbolAddress
);

/*
* 给定模块名称和类型名称，检索类型大小。
* 注意！并非所有模块都存在 - 最初只有模块“nt”可用。
* 注意！如果多个模块具有相同的名称，则将使用第一个要添加的模块。
* -- h VMM
* -- sz 模块
* -- sz 类型名称
* -- pcb 类型大小
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PdbTypeSize(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR szModule,
    _In_ LPCSTR szTypeName,
    _Out_ PDWORD pcbTypeSize
);

/*
* 找到类型子项的偏移量 - 通常是结构内的子项。
* 注意！并非所有模块都存在 - 最初只有模块“nt”可用。
* 注意！如果多个模块具有相同的名称，则将使用第一个要添加的模块。
* -- h VMM
* -- sz 模块
* -- usz 类型名称
* -- usz 类型子项名称
* -- pcb 类型子项偏移量
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_PdbTypeChildOffset(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR szModule,
    _In_ LPCSTR uszTypeName,
    _In_ LPCSTR uszTypeChildName,
    _Out_ PDWORD pcbTypeChildOffset
);



//-----------------------------------------------------------------------------
// WINDOWS SPECIFIC REGISTRY FUNCTIONALITY BELOW:
//-----------------------------------------------------------------------------

#define VMMDLL_REGISTRY_HIVE_INFORMATION_MAGIC      0xc0ffee653df8d01e
#define VMMDLL_REGISTRY_HIVE_INFORMATION_VERSION    4

typedef struct td_VMMDLL_REGISTRY_HIVE_INFORMATION {
    ULONG64 magic;
    WORD wVersion;
    WORD wSize;
    BYTE _FutureReserved1[0x34];
    ULONG64 vaCMHIVE;
    ULONG64 vaHBASE_BLOCK;
    DWORD cbLength;
    CHAR uszName[128];
    CHAR uszNameShort[32 + 1];
    CHAR uszHiveRootPath[MAX_PATH];
    QWORD _FutureReserved[0x10];
} VMMDLL_REGISTRY_HIVE_INFORMATION, *PVMMDLL_REGISTRY_HIVE_INFORMATION;

/*
* 检索有关目标系统中的注册表配置单元的信息。
* -- p Hives = c Hives 的缓冲区 * sizeof(VMMDLL REGISTRY HIVE INFORMATION) 接收有关所有配置单元的信息。NULL 接收 pc Hives 中的 # 个配置单元。
* -- c Hives
* -- pc Hives = if p Hives == NULL: # 总配置单元。如果 p Hives: # 读取配置单元。
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_HiveList(
    _In_ VMM_HANDLE hVMM,
    _Out_writes_(cHives) PVMMDLL_REGISTRY_HIVE_INFORMATION pHives,
    _In_ DWORD cHives,
    _Out_ PDWORD pcHives
);

/*
* 读取任意数量的连续注册表配置单元内存并报告
* 在 pcb Read 中读取的字节数。
* 注意！地址空间不包括 regf 注册表配置单元文件头！
* -- h VMM
* -- va CMHive
* -- ra
* -- pb
* -- cb
* -- pcb Read Opt
* -- flags = 标志，如 VMMDLL FLAG 中所示 *
* -- return = 成功/失败。注意！即使读取了 0 个字节，读取也可能报告为成功 - 建议验证 pcb Read Opt 参数。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_HiveReadEx(
    _In_ VMM_HANDLE hVMM,
    _In_ ULONG64 vaCMHive,
    _In_ DWORD ra,
    _Out_ PBYTE pb,
    _In_ DWORD cb,
    _Out_opt_ PDWORD pcbReadOpt,
    _In_ ULONG64 flags
);

/*
* 将几乎连续的任意数量的内存写入注册表配置单元。
* 注意！地址空间不包括 regf 注册表配置单元文件头！
* -- h VMM
* -- va CMHive
* -- ra
* -- pb
* -- cb
* -- return = 成功时返回 TRUE，部分或零写入时返回 FALSE。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_HiveWrite(
    _In_ VMM_HANDLE hVMM,
    _In_ ULONG64 vaCMHive,
    _In_ DWORD ra,
    _In_ PBYTE pb,
    _In_ DWORD cb
);

/*
* 枚举注册表子键 - 类似于 WINAPI 函数“Reg Enum Key Ex W”。
* 有关信息，请参阅 WINAPI 函数文档。
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) “HKLM\SOFTWARE\Key\Sub Key”
* 2) “HKLM\ORPHAN\SAM\Key\Sub Key”（孤立键）
* 3) “0x<va CMHIVE>\ROOT\Key\Sub Key”
* 4) “0x<va CMHIVE>\ORPHAN\Key\Sub Key”（孤立键）
* -- h VMM
* -- usz 完整路径键
* -- dw 索引 = 子键索引 0..N（键为 -1）。
* -- lp 名称
* -- lpcch 名称
* -- lpft 上次写入时间
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_EnumKeyExU(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR uszFullPathKey,
    _In_ DWORD dwIndex,
    _Out_writes_opt_(*lpcchName) LPSTR lpName,
    _Inout_ LPDWORD lpcchName,
    _Out_opt_ PFILETIME lpftLastWriteTime
);

/*
* 枚举给定注册表项的注册表值 - 类似于 WINAPI 函数
* 'Enum Value W'。有关信息，请参阅 WINAPI 函数文档。
* 可以通过两种方式调用：
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) 'HKLM\SOFTWARE\Key\Sub Key'
* 2) 'HKLM\ORPHAN\SAM\Key\Sub Key'（孤立键）
* 3) '0x<va CMHIVE>\ROOT\Key\Sub Key'
* 4) '0x<va CMHIVE>\ORPHAN\Key\Sub Key'（孤立键）
* -- h VMM
* -- usz 完整路径键
* -- dw 索引
* -- lp 值名称
* -- lpcch 值名称
* -- lp 类型
* -- lp 数据
* -- lpcb 数据
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_EnumValueU(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR uszFullPathKey,
    _In_ DWORD dwIndex,
    _Out_writes_opt_(*lpcchValueName) LPSTR lpValueName,
    _Inout_ LPDWORD lpcchValueName,
    _Out_opt_ LPDWORD lpType,
    _Out_writes_opt_(*lpcbData) LPBYTE lpData,
    _Inout_opt_ LPDWORD lpcbData
);

/*
* 根据给定的注册表项/值路径查询注册表值 - 类似于 WINAPI
* 函数“Reg Query Value Ex”。
* 有关信息，请参阅 WINAPI 函数文档。
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) “HKLM\SOFTWARE\Key\Sub Key\Value”
* 2) “HKLM\ORPHAN\SAM\Key\Sub Key\”（孤立键和默认值）
* 3) “0x<va CMHIVE>\ROOT\Key\Sub Key\Value”
* 4) “0x<va CMHIVE>\ORPHAN\Key\Sub Key\Value”（孤立键值）
* -- h VMM
* -- usz 完整路径键值
* -- lp 类型
* -- lp 数据
* -- lpcb 数据
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_WinReg_QueryValueExU(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR uszFullPathKeyValue,
    _Out_opt_ LPDWORD lpType,
    _Out_writes_opt_(*lpcbData) LPBYTE lpData,
    _When_(lpData == NULL, _Out_opt_) _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData
);

/*
* 枚举注册表子键 - 类似于 WINAPI 函数“Reg Enum Key Ex W”。
* 有关信息，请参阅 WINAPI 函数文档。
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) “HKLM\SOFTWARE\Key\Sub Key”
* 2) “HKLM\ORPHAN\SAM\Key\Sub Key”（孤立键）
* 3) “0x<va CMHIVE>\ROOT\Key\Sub Key”
* 4) “0x<va CMHIVE>\ORPHAN\Key\Sub Key”（孤立键）
* -- h VMM
* -- wsz 完整路径键
* -- dw 索引 = 子键索引 0..N（键为 -1）。
* -- lp 名称
* -- lpcch 名称
* -- lpft 上次写入时间
* -- 返回
*/
_Success_(return)
BOOL VMMDLL_WinReg_EnumKeyExW(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCWSTR wszFullPathKey,
    _In_ DWORD dwIndex,
    _Out_writes_opt_(*lpcchName) LPWSTR lpName,
    _Inout_ LPDWORD lpcchName,
    _Out_opt_ PFILETIME lpftLastWriteTime
);

/*
* 枚举给定注册表项的注册表值 - 类似于 WINAPI 函数
* 'Enum Value W'。有关信息，请参阅 WINAPI 函数文档。
* 可以通过两种方式调用：
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) 'HKLM\SOFTWARE\Key\Sub Key'
* 2) 'HKLM\ORPHAN\SAM\Key\Sub Key'（孤立键）
* 3) '0x<va CMHIVE>\ROOT\Key\Sub Key'
* 4) '0x<va CMHIVE>\ORPHAN\Key\Sub Key'（孤立键）
* -- h VMM
* -- wsz 完整路径键
* -- dw 索引
* -- lp 值名称
* -- lpcch 值名称
* -- lp 类型
* -- lp 数据
* -- lpcb 数据
* -- 返回
*/
_Success_(return)
BOOL VMMDLL_WinReg_EnumValueW(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCWSTR wszFullPathKey,
    _In_ DWORD dwIndex,
    _Out_writes_opt_(*lpcchValueName) LPWSTR lpValueName,
    _Inout_ LPDWORD lpcchValueName,
    _Out_opt_ LPDWORD lpType,
    _Out_writes_opt_(*lpcbData) LPBYTE lpData,
    _Inout_opt_ LPDWORD lpcbData
);

/*
* 根据给定的注册表项/值路径查询注册表值 - 类似于 WINAPI
* 函数“Reg Query Value Ex”。
* 有关信息，请参阅 WINAPI 函数文档。
* 可以使用 HKLM 基址或 CMHIVE 基址的虚拟地址调用示例：
* 1) “HKLM\SOFTWARE\Key\Sub Key\Value”
* 2) “HKLM\ORPHAN\SAM\Key\Sub Key\”（孤立键和默认值）
* 3) “0x<va CMHIVE>\ROOT\Key\Sub Key\Value”
* 4) “0x<va CMHIVE>\ORPHAN\Key\Sub Key\Value”（孤立键值）
* -- h VMM
* -- wsz 完整路径键值
* -- lp 类型
* -- lp 数据
* -- lpcb 数据
* -- 返回
*/
_Success_(return)
BOOL VMMDLL_WinReg_QueryValueExW(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCWSTR wszFullPathKeyValue,
    _Out_opt_ LPDWORD lpType,
    _Out_writes_opt_(*lpcbData) LPBYTE lpData,
    _When_(lpData == NULL, _Out_opt_) _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData
);



//-----------------------------------------------------------------------------
// WINDOWS SPECIFIC UTILITY FUNCTIONS BELOW:
//-----------------------------------------------------------------------------

typedef struct tdVMMDLL_WIN_THUNKINFO_IAT {
    BOOL fValid;
    BOOL f32;               // if TRUE fn is a 32-bit/4-byte entry, otherwise 64-bit/8-byte entry.
    ULONG64 vaThunk;        // address of import address table 'thunk'.
    ULONG64 vaFunction;     // value if import address table 'thunk' == address of imported function.
    ULONG64 vaNameModule;   // address of name string for imported module.
    ULONG64 vaNameFunction; // address of name string for imported function.
} VMMDLL_WIN_THUNKINFO_IAT, *PVMMDLL_WIN_THUNKINFO_IAT;

/*
* 检索有关导入函数的导入地址表 IAT thunk 的信息。这包括 IAT thunk 的虚拟地址，该地址对于挂钩很有用。
* -- h VMM
* -- dw PID
* -- [uw]sz 模块名称
* -- sz 导入模块名称
* -- sz 导入函数名称
* -- p Thunk IAT
* -- 返回
*/
EXPORTED_FUNCTION
_Success_(return) BOOL VMMDLL_WinGetThunkInfoIATU(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCSTR  uszModuleName, _In_ LPCSTR szImportModuleName, _In_ LPCSTR szImportFunctionName, _Out_ PVMMDLL_WIN_THUNKINFO_IAT pThunkInfoIAT);
_Success_(return) BOOL VMMDLL_WinGetThunkInfoIATW(_In_ VMM_HANDLE hVMM, _In_ DWORD dwPID, _In_ LPCWSTR wszModuleName, _In_ LPCSTR szImportModuleName, _In_ LPCSTR szImportFunctionName, _Out_ PVMMDLL_WIN_THUNKINFO_IAT pThunkInfoIAT);



//-----------------------------------------------------------------------------
// VMM VM FUNCTIONALITY BELOW:
//-----------------------------------------------------------------------------

/*
* 给定 VM 句柄，检索 VMM 句柄。
* 这在仅限物理内存的 VM 上是不允许的。
* 应通过调用 VMMDLL Close() 关闭此 VMM 句柄。
* -- h VMM
* -- h VM
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return != NULL)
VMM_HANDLE VMMDLL_VmGetVmmHandle(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM);

/*
* 初始化一个分散句柄，用于高效地读取/写入虚拟机 (VM) 中的内存。
* CALLER CLOSE: VMMDLL Scatter Close Handle(return)
* -- h VMM
* -- h VM = 虚拟机句柄；从 VMMDLL Map Get VM*) 获取
* -- flags = 由 VMMDLL FLAG 提供的可选标志 *
* -- return = 在 VMMDLL Scatter * 函数中使用的句柄。
*/
EXPORTED_FUNCTION _Success_(return != NULL)
VMMDLL_SCATTER_HANDLE VMMDLL_VmScatterInitialize(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM);

/*
* 读取虚拟机 (VM) 客户物理地址 (GPA) 内存。
* -- h VMM
* -- h VM = 虚拟机句柄。
* -- qw GPA
* -- pb
* -- cb
* -- 返回 = 成功/失败（取决于是否读取了所有请求的字节）。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_VmMemRead(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM, _In_ ULONG64 qwGPA, _Out_writes_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 写入虚拟机 (VM) 客户物理地址 (GPA) 内存。
* -- h VMM
* -- h VM = 虚拟机句柄。
* -- qw GPA
* -- pb
* -- cb
* -- 返回 = 成功时返回 TRUE，部分或零写入时返回 FALSE。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_VmMemWrite(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM, _In_ ULONG64 qwGPA, _In_reads_(cb) PBYTE pb, _In_ DWORD cb);

/*
* 分散读取虚拟机 (VM) 客户物理地址 (GPA) 内存。
* 不连续的 4096 字节页面。未缓存。
* -- h Vmm
* -- h VM = 虚拟机句柄。
* -- pp MEMs GPA
* -- cp MEMs GPA
* -- flags =（保留未来使用）。
* -- return = 成功读取的项目数。
*/
EXPORTED_FUNCTION
DWORD VMMDLL_VmMemReadScatter(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM, _Inout_ PPMEM_SCATTER ppMEMsGPA, _In_ DWORD cpMEMsGPA, _In_ DWORD flags);

/*
* 分散写入虚拟机 (VM) 客户物理地址 (GPA) 内存。
* 不连续的 4096 字节页面。未缓存。
* -- h Vmm
* -- h VM = 虚拟机句柄。
* -- pp MEMs GPA
* -- cp MEMs GPA
* -- return = 希望成功写入的项目数。
*/
EXPORTED_FUNCTION
DWORD VMMDLL_VmMemWriteScatter(_In_ VMM_HANDLE hVMM, _In_ VMMVM_HANDLE hVM, _Inout_ PPMEM_SCATTER ppMEMsGPA, _In_ DWORD cpMEMsGPA);

/*
* 将虚拟机 (VM) 客户物理地址 (GPA) 转换为：
* (1) 物理地址 (PA) 或 (2) 'vmmem' 进程中的虚拟地址 (VA)。
* -- h VMM
* -- HVM
* -- qw GPA = 要转换的客户物理地址。
* -- p PA = 转换后的物理地址（如果存在）。
* -- p VA = 'vmmem' 进程内的转换后的虚拟地址（如果存在）。
* -- 返回 = 成功/失败。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_VmMemTranslateGPA(_In_ VMM_HANDLE H, _In_ VMMVM_HANDLE hVM, _In_ ULONG64 qwGPA, _Out_opt_ PULONG64 pPA, _Out_opt_ PULONG64 pVA);



//-----------------------------------------------------------------------------
// VMM UTIL FUNCTIONALITY BELOW:
//-----------------------------------------------------------------------------

/*
* 将人类可读的十六进制 ASCII 内存转储填充到调用者提供的 sz 缓冲区中。
* -- pb
* -- cb
* -- cb 初始偏移量 = 偏移量，最大必须为 0x1000 且为 0x10 的倍数。
* -- sz = 要填充的缓冲区，NULL 用于在 pcsz 参数中检索缓冲区大小。
* -- pcsz = 如果 sz==NULL :: 退出时缓冲区的大小（包括用于终止 NULL 的空间）
* 如果 sz!=NULL :: 进入时缓冲区的大小，退出时字符的大小（不包括终止 NULL）。
*/
EXPORTED_FUNCTION _Success_(return)
BOOL VMMDLL_UtilFillHexAscii(
    _In_reads_opt_(cb) PBYTE pb,
    _In_ DWORD cb,
    _In_ DWORD cbInitialOffset,
    _Out_writes_opt_(*pcsz) LPSTR sz,
    _Inout_ PDWORD pcsz
);



//-----------------------------------------------------------------------------
// DEFAULT (WINDOWS ONLY) COMPATIBILITY FUNCTION DEFINITIONS BELOW:
//-----------------------------------------------------------------------------

#ifdef _WIN32
#define VMMDLL_VfsList                  VMMDLL_VfsListW
#define VMMDLL_VfsRead                  VMMDLL_VfsReadW
#define VMMDLL_VfsWrite                 VMMDLL_VfsWriteW
#define VMMDLL_ProcessGetDirectories    VMMDLL_ProcessGetDirectoriesW
#define VMMDLL_ProcessGetSections       VMMDLL_ProcessGetSectionsW
#define VMMDLL_ProcessGetProcAddress    VMMDLL_ProcessGetProcAddressW
#define VMMDLL_ProcessGetModuleBase     VMMDLL_ProcessGetModuleBaseW
#define VMMDLL_Map_GetPte               VMMDLL_Map_GetPteW
#define VMMDLL_Map_GetVad               VMMDLL_Map_GetVadW
#define VMMDLL_Map_GetModule            VMMDLL_Map_GetModuleW
#define VMMDLL_Map_GetModuleFromName    VMMDLL_Map_GetModuleFromNameW
#define VMMDLL_Map_GetUnloadedModule    VMMDLL_Map_GetUnloadedModuleW
#define VMMDLL_Map_GetEAT               VMMDLL_Map_GetEATW
#define VMMDLL_Map_GetIAT               VMMDLL_Map_GetIATW
#define VMMDLL_Map_GetHandle            VMMDLL_Map_GetHandleW
#define VMMDLL_Map_GetNet               VMMDLL_Map_GetNetW
#define VMMDLL_Map_GetUsers             VMMDLL_Map_GetUsersW
#define VMMDLL_Map_GetServices          VMMDLL_Map_GetServicesW
#define VMMDLL_WinGetThunkInfoIAT       VMMDLL_WinGetThunkInfoIATW
#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __VMMDLL_H__ */
