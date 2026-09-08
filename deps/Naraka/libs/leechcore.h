// leechcore.h：Leech Core 库的外部头文件。
//
// Leech Core 是一个抽象了对各种
// 软件和硬件获取源的读写的库。来源范围从内存转储
// 文件到驱动程序支持的实时内存，再到硬件 (FPGA) DMA 支持的内存。
//
// Leech Core 内置设备支持可以通过外部插件进行扩展
// 设备驱动程序作为 .dll 或 .so 文件放置在与 Leech Core 相同的文件夹中。
//
// 有关更多信息，请查阅 Github 上的 Leech Core 信息：
// - README: https://github.com/ufrisk/LeechCore
// - GUIDE:  https://github.com/ufrisk/LeechCore/wiki
//
// (c) Ulf Frisk, 2020-2024
// Author: Ulf Frisk, pcileech@frizk.net
//
// Header Version: 2.17
//

#ifndef __LEECHCORE_H__
#define __LEECHCORE_H__
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//-----------------------------------------------------------------------------
// OS COMPATIBILITY BELOW:
//-----------------------------------------------------------------------------

#ifdef _WIN32

#include <Windows.h>
#define EXPORTED_FUNCTION                   __declspec(dllexport)
typedef unsigned __int64                    QWORD, *PQWORD;

#endif /* _WIN32 */
#ifdef LINUX

#include <inttypes.h>
#include <stdlib.h>
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
#define _In_reads_bytes_opt_(x)
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



//-----------------------------------------------------------------------------
// 创建和关闭 Leech Core 设备：
// 可以并行创建多个 Leech Core 设备，也可以创建不同类型的设备（如果底层设备允许）。Leech Core 将
// 自动处理和抽象与底层设备相关的任何硬件/软件问题。
//
// 有关受支持设备的更多信息，请查看 Leech Core
// guide at: https://github.com/ufrisk/LeechCore/wiki
//-----------------------------------------------------------------------------

#define LC_CONFIG_VERSION                       0xc0fd0002
#define LC_CONFIG_ERRORINFO_VERSION             0xc0fe0002

#define LC_CONFIG_PRINTF_ENABLED                0x01
#define LC_CONFIG_PRINTF_V                      0x02
#define LC_CONFIG_PRINTF_VV                     0x04
#define LC_CONFIG_PRINTF_VVV                    0x08

typedef struct LC_CONFIG {
    // 以下由呼叫者设置
    DWORD dwVersion;                        //必须等于 LC CREATE VERSION
    DWORD dwPrintfVerbosity;                //根据 LC PRINTF 的 printf 详细程度 *
    CHAR szDevice[MAX_PATH];                // 设备配置-请参阅 wiki 以获取更多信息。
    CHAR szRemote[MAX_PATH];                // 远程配置-请参阅 wiki 以获取更多信息。
    _Check_return_opt_ int(*pfn_printf_opt)(_In_z_ _Printf_format_string_ char const *const _Format, ...);
    //以下由调用者设置，由 Leec Core 更新
    QWORD paMax;                            // 最大物理地址（禁用任何最大地址自动检测）.
    //以下由 Leech Core 设置
    BOOL fVolatile;
    BOOL fWritable;
    BOOL fRemote;
    BOOL fRemoteDisableCompress;
    CHAR szDeviceName[MAX_PATH];            // 设备名称 - 例如“fpga”或“file”。
} LC_CONFIG, *PLC_CONFIG;

typedef struct tdLC_CONFIG_ERRORINFO {
    DWORD dwVersion;                        // must equal LC_CONFIG_ERRORINFO_VERSION
    DWORD cbStruct;
    DWORD _FutureUse[16];
    BOOL fUserInputRequest;
    DWORD cwszUserText;
    WCHAR wszUserText[];
} LC_CONFIG_ERRORINFO, *PLC_CONFIG_ERRORINFO, **PPLC_CONFIG_ERRORINFO;

/*
* 根据提供的配置创建新的 Leech Core 设备。
* CALLER Lc Mem Free: pp Lc Create Error Info
* -- p Lc Create Config
* -- pp Lc Create Error Info = ptr 用于接收函数分配的结构体，其中包含函数失败时的错误信息。此信息可能包含用户消息，例如，请求用户采取行动。任何返回的结构体都应通过调用 Lc Mem Free() 进行释放。
* -- return
*/
EXPORTED_FUNCTION _Success_(return != NULL)
HANDLE LcCreate(
    _Inout_ PLC_CONFIG pLcCreateConfig
);

EXPORTED_FUNCTION _Success_(return != NULL)
HANDLE LcCreateEx(
    _Inout_ PLC_CONFIG pLcCreateConfig,
    _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo
);

/*
* Close a LeechCore handle and free any resources no longer needed.
*/
EXPORTED_FUNCTION
VOID LcClose(
    _In_opt_ _Post_ptr_invalid_ HANDLE hLC
);



//-----------------------------------------------------------------------------
// 使用连续方法
// 或更推荐的分散方法从底层设备读取和写入内存。
//
// MEM SCATTER 结构允许读取和写入不连续的内存
// 块，这些块必须遵守以下规则：
// - 最大大小 = 0x1000（4096）字节 = 推荐大小。
// - 最小大小 = 2 个 DWORD（8 字节）。
// - 必须是 DWORD（4 字节）对齐。
// - 绝不能跨越 0x1000 页边界。
// - i Stack 的最大值 = MEM SCATTER STACK SIZE - 2。
//-----------------------------------------------------------------------------

#define MEM_SCATTER_VERSION                 0xc0fe0002
#define MEM_SCATTER_STACK_SIZE              12

typedef struct tdMEM_SCATTER {
    DWORD version;                          // MEM_SCATTER_VERSION
    BOOL f;                                 // TRUE = success data in pb, FALSE = fail or not yet read.
    QWORD qwA;                              // address of memory to read
    union {
        PBYTE pb;                           // buffer to hold memory contents
        QWORD _Filler;
    };
    DWORD cb;                               // size of buffer to hold memory contents.
    DWORD iStack;                           // internal stack pointer
    QWORD vStack[MEM_SCATTER_STACK_SIZE];   // internal stack
} MEM_SCATTER, *PMEM_SCATTER, **PPMEM_SCATTER;

#define MEM_SCATTER_ADDR_INVALID            ((QWORD)-1)
#define MEM_SCATTER_ADDR_ISINVALID(pMEM)    (pMEM->qwA == (QWORD)-1)
#define MEM_SCATTER_ADDR_ISVALID(pMEM)      (pMEM->qwA != (QWORD)-1)
#define MEM_SCATTER_STACK_PUSH(pMEM, v)     (pMEM->vStack[pMEM->iStack++] = (QWORD)(v))
#define MEM_SCATTER_STACK_PEEK(pMEM, i)     (pMEM->vStack[pMEM->iStack - i])
#define MEM_SCATTER_STACK_SET(pMEM, i, v)   (pMEM->vStack[pMEM->iStack - i] = (QWORD)(v))
#define MEM_SCATTER_STACK_ADD(pMEM, i, v)   (pMEM->vStack[pMEM->iStack - i] += (QWORD)(v))
#define MEM_SCATTER_STACK_POP(pMEM)         (pMEM->vStack[--pMEM->iStack])

/*
* 释放 Leech Core 分配的内存，例如由
* Lc Alloc Scatter / Lc Command 函数分配的内存。
* -- pv
*/
EXPORTED_FUNCTION
VOID LcMemFree(
    _Frees_ptr_opt_ PVOID pv
);

/*
* 分配并预初始化空 MEM，包括每个 0x1000 缓冲区
* p MEM。当不再需要结果时，应由 Lc Free 释放。
* 0x1000 大小的每个 MEM 内存缓冲区按顺序在 MEM 之间连续。
* -- c MEMs
* -- ppp MEMs = 接收 pp MEMs 的指针
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcAllocScatter1(
    _In_ DWORD cMEMs,
    _Out_ PPMEM_SCATTER *pppMEMs
);

/*
* 分配并预初始化空的 MEM，但不包括 0x1000 缓冲区，
* 该缓冲区将以连续的方式计入 pb 数据缓冲区。
* 当不再需要结果时，应由 Lc Free 释放。
* -- cb 数据 = pb 数据的大小（必须是 c MEM * 0x1000）
* -- pb 数据 = 用于 MEM.pb 的缓冲区
* -- c MEM
* -- ppp MEM = 接收 pp MEM 的指针
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcAllocScatter2(
    _In_ DWORD cbData,
    _Inout_updates_opt_(cbData) PBYTE pbData,
    _In_ DWORD cMEMs,
    _Out_ PPMEM_SCATTER *pppMEMs
);

/*
* 分配并预初始化空的 MEM，但不包括 0x1000 缓冲区，这些缓冲区
* 将以连续的方式计入 pb 数据缓冲区。
* -- pb 数据第一页 = 第一页的可选缓冲区
* -- pb 数据最后一页 = 最后一页的可选缓冲区
* -- cb 数据 = pb 数据的大小
* -- pb 数据 = 用于 MEM.pb 的缓冲区，如果存在，则除第一个/最后一个之外
* -- c MEMs
* -- ppp MEMs = 接收 pp MEMs 的指针
* -- 返回
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcAllocScatter3(
    _Inout_updates_opt_(0x1000) PBYTE pbDataFirstPage,
    _Inout_updates_opt_(0x1000) PBYTE pbDataLastPage,
    _In_ DWORD cbData,
    _Inout_updates_opt_(cbData) PBYTE pbData,
    _In_ DWORD cMEMs,
    _Out_ PPMEM_SCATTER *pppMEMs
);

/*
* 以分散、不连续的方式读取内存。建议读取时采用这种方式。
* -- h LC
* -- c MEMs
* -- pp MEMs
*/
EXPORTED_FUNCTION
VOID LcReadScatter(
    _In_ HANDLE hLC,
    _In_ DWORD cMEMs,
    _Inout_ PPMEM_SCATTER ppMEMs
);

/*
* 以连续的方式读取内存。请注意，如果需要读取多个内存段
* Lc Read Scatter() 可能更有效。
* -- h LC,
* -- pa
* -- cb
* -- pb
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcRead(
    _In_ HANDLE hLC,
    _In_ QWORD pa,
    _In_ DWORD cb,
    _Out_writes_(cb) PBYTE pb
);

/*
* 以分散、不连续的方式写入内存。
* -- h LC
* -- c MEMs
* -- pp MEMs
*/
EXPORTED_FUNCTION
VOID LcWriteScatter(
    _In_ HANDLE hLC,
    _In_ DWORD cMEMs,
    _Inout_ PPMEM_SCATTER ppMEMs
);

/*
* 以连续的方式写入内存。
* -- h LC
* -- pa
* -- cb
* -- pb
* -- return
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcWrite(
    _In_ HANDLE hLC,
    _In_ QWORD pa,
    _In_ DWORD cb,
    _In_reads_(cb) PBYTE pb
);



//-----------------------------------------------------------------------------
// Get/Set/Command functionality may be used to query and/or update LeechCore
// or its devices in various ways.
//-----------------------------------------------------------------------------

/*
* 设置由 LC OPT * 定义的选项。（R 选项）。
* -- h LC
* -- f 选项 = LC OPT *
* -- cb 数据
* -- pb 数据
* -- pcb 数据
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcGetOption(
    _In_ HANDLE hLC,
    _In_ QWORD fOption,
    _Out_ PQWORD pqwValue
);

/*
* 获取由 LC OPT * 定义的选项。（W 选项）。
* -- h LC
* -- f 选项 = LC OPT *
* -- cb 数据
* -- pb 数据
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcSetOption(
    _In_ HANDLE hLC,
    _In_ QWORD fOption,
    _In_ QWORD qwValue
);

/*
* 同时执行命令并检索结果（如果有）。
* 注意！如果 *ppb Data Out 在退出时包含内存分配，则应通过调用 Lc Mem Free() 释放内存。
* CALLER Lc Free Mem: *ppb Data Out
* -- h LC
* -- f Command = LC CMD *
* -- cb Data In
* -- pb Data In
* -- ppb Data Out
* -- pcb Data Out
*/
EXPORTED_FUNCTION _Success_(return)
BOOL LcCommand(
    _In_ HANDLE hLC,
    _In_ QWORD fCommand,
    _In_ DWORD cbDataIn,
    _In_reads_opt_(cbDataIn) PBYTE pbDataIn,
    _Out_opt_ PBYTE *ppbDataOut,
    _Out_opt_ PDWORD pcbDataOut
);

#define LC_OPT_CORE_PRINTF_ENABLE                   0x4000000100000000  // RW
#define LC_OPT_CORE_VERBOSE                         0x4000000200000000  // RW
#define LC_OPT_CORE_VERBOSE_EXTRA                   0x4000000300000000  // RW
#define LC_OPT_CORE_VERBOSE_EXTRA_TLP               0x4000000400000000  // RW
#define LC_OPT_CORE_VERSION_MAJOR                   0x4000000500000000  // R
#define LC_OPT_CORE_VERSION_MINOR                   0x4000000600000000  // R
#define LC_OPT_CORE_VERSION_REVISION                0x4000000700000000  // R
#define LC_OPT_CORE_ADDR_MAX                        0x1000000800000000  // R
#define LC_OPT_CORE_STATISTICS_CALL_COUNT           0x4000000900000000  // R [lo-dword: LC_STATISTICS_ID_*]
#define LC_OPT_CORE_STATISTICS_CALL_TIME            0x4000000a00000000  // R [lo-dword: LC_STATISTICS_ID_*]
#define LC_OPT_CORE_VOLATILE                        0x1000000b00000000  // R
#define LC_OPT_CORE_READONLY                        0x1000000c00000000  // R

#define LC_OPT_MEMORYINFO_VALID                     0x0200000100000000  // R
#define LC_OPT_MEMORYINFO_FLAG_32BIT                0x0200000300000000  // R
#define LC_OPT_MEMORYINFO_FLAG_PAE                  0x0200000400000000  // R
#define LC_OPT_MEMORYINFO_ARCH                      0x0200001200000000  // R - LC_ARCH_TP
#define LC_OPT_MEMORYINFO_OS_VERSION_MINOR          0x0200000500000000  // R
#define LC_OPT_MEMORYINFO_OS_VERSION_MAJOR          0x0200000600000000  // R
#define LC_OPT_MEMORYINFO_OS_DTB                    0x0200000700000000  // R
#define LC_OPT_MEMORYINFO_OS_PFN                    0x0200000800000000  // R
#define LC_OPT_MEMORYINFO_OS_PsLoadedModuleList     0x0200000900000000  // R
#define LC_OPT_MEMORYINFO_OS_PsActiveProcessHead    0x0200000a00000000  // R
#define LC_OPT_MEMORYINFO_OS_MACHINE_IMAGE_TP       0x0200000b00000000  // R
#define LC_OPT_MEMORYINFO_OS_NUM_PROCESSORS         0x0200000c00000000  // R
#define LC_OPT_MEMORYINFO_OS_SYSTEMTIME             0x0200000d00000000  // R
#define LC_OPT_MEMORYINFO_OS_UPTIME                 0x0200000e00000000  // R
#define LC_OPT_MEMORYINFO_OS_KERNELBASE             0x0200000f00000000  // R
#define LC_OPT_MEMORYINFO_OS_KERNELHINT             0x0200001000000000  // R
#define LC_OPT_MEMORYINFO_OS_KdDebuggerDataBlock    0x0200001100000000  // R

#define LC_OPT_FPGA_PROBE_MAXPAGES                  0x0300000100000000  // RW
#define LC_OPT_FPGA_MAX_SIZE_RX                     0x0300000300000000  // RW
#define LC_OPT_FPGA_MAX_SIZE_TX                     0x0300000400000000  // RW
#define LC_OPT_FPGA_DELAY_PROBE_READ                0x0300000500000000  // RW - uS
#define LC_OPT_FPGA_DELAY_PROBE_WRITE               0x0300000600000000  // RW - uS
#define LC_OPT_FPGA_DELAY_WRITE                     0x0300000700000000  // RW - uS
#define LC_OPT_FPGA_DELAY_READ                      0x0300000800000000  // RW - uS
#define LC_OPT_FPGA_RETRY_ON_ERROR                  0x0300000900000000  // RW
#define LC_OPT_FPGA_DEVICE_ID                       0x0300008000000000  // RW - bus:dev:fn (ex: 04:00.0 == 0x0400).
#define LC_OPT_FPGA_FPGA_ID                         0x0300008100000000  // R
#define LC_OPT_FPGA_VERSION_MAJOR                   0x0300008200000000  // R
#define LC_OPT_FPGA_VERSION_MINOR                   0x0300008300000000  // R
#define LC_OPT_FPGA_ALGO_TINY                       0x0300008400000000  // RW - 1/0 使用微小的 128 字节/tlp 读取算法.
#define LC_OPT_FPGA_ALGO_SYNCHRONOUS                0x0300008500000000  // RW - 1/0 使用同步（旧）读取算法。
#define LC_OPT_FPGA_CFGSPACE_XILINX                 0x0300008600000000  // RW - [lo-dword: register address in bytes] [bytes: 0-3: data, 4-7: byte_enable(if wr/set); top bit = cfg_mgmt_wr_rw1c_as_rw]
#define LC_OPT_FPGA_TLP_READ_CB_WITHINFO            0x0300009000000000  // RW - 1/0 调用 TLP 读取回调函数，并在 sz Info 中添加附加字符串信息
#define LC_OPT_FPGA_TLP_READ_CB_FILTERCPL           0x0300009100000000  // RW - 1/0 调用 TLP 读取回调，并从读取调用中过滤内存读取完成情况

#define LC_CMD_FPGA_PCIECFGSPACE                    0x0000010300000000  // R
#define LC_CMD_FPGA_CFGREGPCIE                      0x0000010400000000  // RW - [lo-dword:寄存器地址]
#define LC_CMD_FPGA_CFGREGCFG                       0x0000010500000000  // RW - [lo-dword:寄存器地址]
#define LC_CMD_FPGA_CFGREGDRP                       0x0000010600000000  // RW - [lo-dword:寄存器地址]
#define LC_CMD_FPGA_CFGREGCFG_MARKWR                0x0000010700000000  // W  - 用掩码写入 [lo-dword：寄存器地址] [字节：0-1：数据，2-3：掩码]
#define LC_CMD_FPGA_CFGREGPCIE_MARKWR               0x0000010800000000  // W  - 用掩码写入 [lo-dword：寄存器地址] [字节：0-1：数据，2-3：掩码]
#define LC_CMD_FPGA_CFGREG_DEBUGPRINT               0x0000010a00000000  // N/A
#define LC_CMD_FPGA_PROBE                           0x0000010b00000000  // RW
#define LC_CMD_FPGA_CFGSPACE_SHADOW_RD              0x0000010c00000000  // R
#define LC_CMD_FPGA_CFGSPACE_SHADOW_WR              0x0000010d00000000  // W  - [lo-dword: config space write base address]
#define LC_CMD_FPGA_TLP_WRITE_SINGLE                0x0000011000000000  // W  - write single tlp BYTE:s
#define LC_CMD_FPGA_TLP_WRITE_MULTIPLE              0x0000011100000000  // W  - write multiple LC_TLP:s
#define LC_CMD_FPGA_TLP_TOSTRING                    0x0000011200000000  // RW - convert single TLP to LPSTR; *pcbDataOut includes NULL terminator.

#define LC_CMD_FPGA_TLP_CONTEXT                     0x2000011400000000  // W - set/unset TLP user-defined context to be passed to callback function. (pbDataIn == LPVOID user context). [not remote].
#define LC_CMD_FPGA_TLP_CONTEXT_RD                  0x2000011b00000000  // R - 获取要传递给回调函数的 TLP 用户定义上下文。[非远程]。
#define LC_CMD_FPGA_TLP_FUNCTION_CALLBACK           0x2000011500000000  // W - 设置/取消设置 TLP 回调函数（pb Data In == PLC TLP CALLBACK）。[非远程]。
#define LC_CMD_FPGA_TLP_FUNCTION_CALLBACK_RD        0x2000011c00000000  // R - get TLP callback function. [not remote].
#define LC_CMD_FPGA_BAR_CONTEXT                     0x2000012000000000  // W - set/unset BAR user-defined context to be passed to callback function. (pbDataIn == LPVOID user context). [not remote].
#define LC_CMD_FPGA_BAR_CONTEXT_RD                  0x2000012100000000  // R - get BAR user-defined context to be passed to callback function. [not remote].
#define LC_CMD_FPGA_BAR_FUNCTION_CALLBACK           0x2000012200000000  // W - set/unset BAR callback function (pbDataIn == PLC_BAR_CALLBACK). [not remote].
#define LC_CMD_FPGA_BAR_FUNCTION_CALLBACK_RD        0x2000012300000000  // R - get BAR callback function. [not remote].
#define LC_CMD_FPGA_BAR_INFO                        0x0000012400000000  // R - get BAR info (pbDataOut == LC_BAR_INFO[6]).

#define LC_CMD_FILE_DUMPHEADER_GET                  0x0000020100000000  // R

#define LC_CMD_STATISTICS_GET                       0x4000010000000000  // R
#define LC_CMD_MEMMAP_GET                           0x4000020000000000  // R  - MEMMAP as LPSTR
#define LC_CMD_MEMMAP_SET                           0x4000030000000000  // W  - MEMMAP as LPSTR
#define LC_CMD_MEMMAP_GET_STRUCT                    0x4000040000000000  // R  - MEMMAP as LC_MEMMAP_ENTRY[]
#define LC_CMD_MEMMAP_SET_STRUCT                    0x4000050000000000  // W  - MEMMAP as LC_MEMMAP_ENTRY[]

#define LC_CMD_AGENT_EXEC_PYTHON                    0x8000000100000000  // RW - [lo-dword: optional timeout in ms]
#define LC_CMD_AGENT_EXIT_PROCESS                   0x8000000200000000  //    - [lo-dword: process exit code]
#define LC_CMD_AGENT_VFS_LIST                       0x8000000300000000  // RW
#define LC_CMD_AGENT_VFS_READ                       0x8000000400000000  // RW
#define LC_CMD_AGENT_VFS_WRITE                      0x8000000500000000  // RW
#define LC_CMD_AGENT_VFS_OPT_GET                    0x8000000600000000  // RW
#define LC_CMD_AGENT_VFS_OPT_SET                    0x8000000700000000  // RW
#define LC_CMD_AGENT_VFS_INITIALIZE                 0x8000000800000000  // RW
#define LC_CMD_AGENT_VFS_CONSOLE                    0x8000000900000000  // RW

#define LC_CMD_AGENT_VFS_REQ_VERSION                0xfeed0001
#define LC_CMD_AGENT_VFS_RSP_VERSION                0xfeee0001

#define LC_STATISTICS_VERSION                       0xe1a10002
#define LC_STATISTICS_ID_OPEN                       0x00
#define LC_STATISTICS_ID_READ                       0x01
#define LC_STATISTICS_ID_READSCATTER                0x02
#define LC_STATISTICS_ID_WRITE                      0x03
#define LC_STATISTICS_ID_WRITESCATTER               0x04
#define LC_STATISTICS_ID_GETOPTION                  0x05
#define LC_STATISTICS_ID_SETOPTION                  0x06
#define LC_STATISTICS_ID_COMMAND                    0x07
#define LC_STATISTICS_ID_MAX                        0x07

typedef struct tdLC_CMD_AGENT_VFS_REQ {
    DWORD dwVersion;
    DWORD _FutureUse;
    CHAR uszPathFile[2*MAX_PATH];   // file path to list/read/write
    union {
        QWORD qwOffset;             // offset to read/write
        QWORD fOption;              // option to get/set (qword data in *pb)
    };
    DWORD dwLength;                 // length to read
    DWORD cb;
    BYTE pb[0];
} LC_CMD_AGENT_VFS_REQ, *PLC_CMD_AGENT_VFS_REQ;

typedef struct tdLC_CMD_AGENT_VFS_RSP {
    DWORD dwVersion;
    DWORD dwStatus;                 // ntstatus of read/write
    DWORD cbReadWrite;              // number of bytes read/written
    DWORD _FutureUse[2];
    DWORD cb;
    BYTE pb[0];
} LC_CMD_AGENT_VFS_RSP, *PLC_CMD_AGENT_VFS_RSP;

static LPCSTR LC_STATISTICS_NAME[] = {
    "LcOpen",
    "LcRead",
    "LcReadScatter",
    "LcWrite",
    "LcWriteScatter",
    "LcGetOption",
    "LcSetOption",
    "LcCommand",
};

typedef struct tdLC_STATISTICS {
    DWORD dwVersion;
    DWORD _Reserved;
    QWORD qwFreq;
    struct {
        QWORD c;
        QWORD tm;   // total time in qwFreq ticks
    } Call[LC_STATISTICS_ID_MAX + 1];
} LC_STATISTICS, *PLC_STATISTICS;

typedef struct tdLC_MEMMAP_ENTRY {
    QWORD pa;
    QWORD cb;
    QWORD paRemap;
} LC_MEMMAP_ENTRY, *PLC_MEMMAP_ENTRY;

typedef enum tdLC_ARCH_TP {
    LC_ARCH_NA      = 0,
    LC_ARCH_X86     = 1,
    LC_ARCH_X86PAE  = 2,
    LC_ARCH_X64     = 3,
    LC_ARCH_ARM64   = 4,
} LC_ARCH_TP;



//-----------------------------------------------------------------------------
// RAW TLP READ/WRITE SUPPORT:
//-----------------------------------------------------------------------------

/*
* TLP structure to be used with LC_CMD_FPGA_TLP_WRITE_MULTIPLE.
*/
typedef struct tdLC_TLP {
    DWORD cb;
    DWORD _Reserved1;
    PBYTE pb;
} LC_TLP, *PLC_TLP;

/*
* 收到 TLP 时调用的自定义 FPGA 回调函数。
* 通过命令 LC CMD FPGA TLP FUNCTION CALLBACK 设置的回调函数。
* 通过命令设置用户定义的上下文：LC CMD FPGA TLP CONTEXT。
*/
typedef VOID(*PLC_TLP_FUNCTION_CALLBACK)(
    _In_opt_ PVOID ctx,
    _In_ DWORD cbTlp,
    _In_ PBYTE pbTlp,
    _In_opt_ DWORD cbInfo,
    _In_opt_ LPSTR szInfo
);

#define LC_TLP_FUNCTION_CALLBACK_DISABLE        (PLC_TLP_FUNCTION_CALLBACK)(NULL)
#define LC_TLP_FUNCTION_CALLBACK_DUMMY          (PLC_TLP_FUNCTION_CALLBACK)(-1)



//-----------------------------------------------------------------------------
// PCIE BAR SUPPORT:
//-----------------------------------------------------------------------------

typedef struct tdLC_BAR {
    BOOL fValid;
    BOOL fIO;
    BOOL f64Bit;
    BOOL fPrefetchable;
    DWORD _Filler[3];
    DWORD iBar;
    QWORD pa;
    QWORD cb;
} LC_BAR, *PLC_BAR;

typedef struct tdLC_BAR_REQUEST {
    PVOID ctx;              // user context (set by command LC_CMD_FPGA_BAR_CONTEXT)
    PLC_BAR pBar;           // BAR info
    BYTE bTag;              // TLP tag (0-255)
    BYTE bFirstBE;          // First byte enable (0-3) [relevant for writes]
    BYTE bLastBE;           // Last byte enable (0-3) [relevant for writes]
    BYTE _Filler;
    BOOL f64;               // 64-bit bar access (false = 32-bit)
    BOOL fRead;             // BAR read request, called function should update pbData with read data and set fReadReply = TRUE on success.
    BOOL fReadReply;        // Read success - should be updated by called function upon read success (after updating pbData).
    BOOL fWrite;            // BAR write request (no reply should be sent, check byte-enables bFirstBE/bLastBE)
    DWORD cbData;           // number of bytes to read/write
    QWORD oData;            // data offset in BAR.
    BYTE pbData[4096];      // bytes to write or read data (to be updated by called function).
} LC_BAR_REQUEST, *PLC_BAR_REQUEST;

/*
*收到 BAR 读/写时调用的自定义 FPGA 回调函数。
* 通过命令 LC CMD FPGA BAR FUNCTION CALLBACK 设置回调函数。
* 通过命令设置用户定义上下文：LC CMD FPGA BAR CONTEXT。
* 通过使用读取数据更新 pb Data 和 f Read Reply = TRUE 来发送读取回复。
* 要返回不支持的请求 (UR)，请在 MRd 请求上设置 f Read Reply = FALSE.
*/
typedef VOID(*PLC_BAR_FUNCTION_CALLBACK)(_Inout_ PLC_BAR_REQUEST pBarRequest);

#define LC_BAR_FUNCTION_CALLBACK_DISABLE        (PLC_BAR_FUNCTION_CALLBACK)(NULL)
#define LC_BAR_FUNCTION_CALLBACK_ZEROBAR        (PLC_BAR_FUNCTION_CALLBACK)(-1)


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LEECHCORE_H__ */
