#pragma once
#include "pch.h"
#include "InputManager.h"
#include "Registry.h"
#include "Shellcode.h"
#include <winternl.h>
class Memory
{
private:
	struct LibModules
	{
		HMODULE VMM = nullptr;
		HMODULE FTD3XX = nullptr;
		HMODULE LEECHCORE = nullptr;
	};

	static inline LibModules modules { };

	struct CurrentProcessInformation
	{
		int PID = 0;
		size_t base_address = 0;
		size_t base_size = 0;
		std::string process_name = "";
	};

	static inline CurrentProcessInformation current_process { };

	static inline BOOLEAN DMA_INITIALIZED = FALSE;
	static inline BOOLEAN PROCESS_INITIALIZED = FALSE;
	/**
	*转储系统当前物理内存页面
	*转储到文件中，以便我们在 DMA 中使用它（：
	*此文件被创建到 %temp% 文件夹
	*@return 如果成功则返回 true，否则返回 false。
	*/
	bool DumpMemoryMap(bool debug = false);

	/**
	* brief 删除与 FPGA 设备相关的基本信息
	* 在执行任何 DMA 操作之前都需要执行此操作。
	* 确保游戏作弊时的最佳安全性。
	* @return 如果成功则返回 true，否则返回 false。
	*/
	bool SetFPGA();

	//shared pointer
	std::shared_ptr<c_keys> key;
	c_registry registry;
	c_shellcode shellcode;

	/*this->registry_ptr = std::make_shared<c_registry>(*this);
	this->key_ptr = std::make_shared<c_keys>(*this);*/

public:
	/**
	* 简要构造函数采用该过程的宽字符串。
	* 期望所有库都在根目录中
	 */
	Memory();
	~Memory();

	/**
	* @brief 获取注册表对象
	* @return 注册表类
	*/
	c_registry GetRegistry() { return registry; }

	/**
	* @brief 获取密钥对象
	* @return key class
	*/
	c_keys* GetKeyboard() { return key.get(); }

	/**
	* @brief 获取 shellcode 对象
	* @return shellcode 类
	*/
	c_shellcode GetShellcode() { return shellcode; }

	/**
	* brief 初始化 DMA
	* 在执行任何 DMA 操作之前，这都是必需的。
	* @param process name 进程的名称
	* @param mem Map 如果为 true，则会将内存映射转储到文件并让 DMA 使用它。
	* @return 如果成功则返回 true，否则返回 false.
	*/
	bool Init(std::string process_name, bool memMap = true, bool debug = false);

	/*This part here is things related to the process information such as Base daddy, Size ect.*/

	/**
	* brief 获取进程的进程 id
	* @param process name 进程的名称
	* @return 进程的进程 id
	*/
	DWORD GetPidFromName(std::string process_name);

	/**
	* brief 获取进程的所有进程 id
	* @param process name 进程的名称
	* @returns 进程的所有进程 id
	*/
	std::vector<int> GetPidListFromName(std::string process_name);

	/**
	* \brief 获取进程的模块列表
	* \param process name 进程的名称
	* \return 进程的所有模块名称
	*/
	std::vector<std::string> GetModuleList(std::string process_name);

	/**
	* \brief 获取进程信息
	* \return 进程信息
	*/
	VMMDLL_PROCESS_INFORMATION GetProcessInformation();

	/**
	* \brief 获取进程 peb
	* \return 进程 peb
	*/
	PEB GetProcessPeb();

	/**
	* brief 获取进程的基地址
	* @param module name 模块名称
	* @return 进程的基地址
	*/
	size_t GetBaseDaddy(std::string module_name);

	/**
	* brief 获取进程的基本大小
	* @param module name 模块的名称
	* @return 进程的基本大小
	*/
	size_t GetBaseSize(std::string module_name);

	/**
	* brief 获取进程的导出表地址
	* @param import 导出的名称
	* @param process 进程的名称
	* @param module 想要在其中查找导出的模块的名称
	* @return 导出的导出表地址
	*/
	uintptr_t GetExportTableAddress(std::string import, std::string process, std::string module);

	/**
	* brief 获取进程的导入表地址
	* @param import 导入的名称
	* @param process 进程的名称
	* @param module 想要在其中查找导入的模块的名称
	* @return 导入的导入表地址
	*/
	uintptr_t GetImportTableAddress(std::string import, std::string process, std::string module);

	/**
	* \brief 这修复了 EAC 造成的 CR3 混乱。
	* 它通过迭代系统中存在的所有 DTB 并查找特定的 DTB 来修复它
	* 不再分配 PID，即它们的 pid 为 0
	* 然后将其放入向量中，以便稍后尝试每个可能的 DTB 来查找进程的 DTB。
	* 注意：使用 Fix CR3 需要您拥有 symsrv.dll、dbghelp.dll 和 info.db
	 */
	bool FixCr3();

	/**
	* \brief 将地址（需要是有效的 PE Header）处的进程内存转储到以下路径
	* \param address PE Header 的地址（基址）
	* \param path 您要保存转储到的路径
	 */
	bool DumpMemory(uintptr_t address, std::string path);

	/**
	* \刷新缓存 页表
	* 
	* \
	 */
	void RefreshConfigSet();

	/**
	* \刷新 所有 缓存 页表
	*
	* \
	 */
	void RefreshConfigSetALL();

	/*This part is where all memory operations are done, such as read, write.*/

	/**
	* \brief 扫描签名的过程。
	* \param signature 签名示例“48 ? ? ?”
	* \param range start 扫描起始区域
	* \param range end 扫描至的区域
	* \param PID (可选) 读取至何处？
	* \return 签名的地址
	 */
	uint64_t FindSignature(const char* signature, uint64_t range_start, uint64_t range_end, int PID = 0);

	/**
	* \brief 将内存写入进程
	* \param address 要写入的地址
	* \param buffer 要写入的缓冲区
	* \param size 缓冲区的大小
	 * \return 
	 */
	bool Write(uintptr_t address, void* buffer, size_t size) const;
	bool Write(uintptr_t address, void* buffer, size_t size, int pid) const;

	/**
	* \brief 使用模板将内存写入进程
	* \param 要写入的地址
	* \param value 要写入地址的值
	 */
	template <typename T>
	void Write(void* address, T value)
	{
		Write(address, &value, sizeof(T));
	}

	template <typename T>
	void Write(uintptr_t address, T value)
	{
		Write(address, &value, sizeof(T));
	}

	/**
	* brief 从进程中读取内存
	* @param address 读取的地址
	* @param buffer 读取到的缓冲区
	* @param size 缓冲区的大小
	* @return 如果成功则返回 true，否则返回 false。
	*/
	bool Read(uintptr_t address, void* buffer, size_t size) const;
	bool Read(uintptr_t address, void* buffer, size_t size, int pid) const;

	/**
	* brief 使用模板从进程中读取内存
	* @param address 要读取的地址
	* @return 从进程中读取的值
	*/
	template <typename T>
	T Read(void* address)
	{
		T buffer { };
		memset(&buffer, 0, sizeof(T));
		Read(reinterpret_cast<uint64_t>(address), reinterpret_cast<void*>(&buffer), sizeof(T));

		return buffer;
	}

	template <typename T>
	T Read(uint64_t address)
	{
		return Read<T>(reinterpret_cast<void*>(address));
	}

	/**
	* brief 使用模板和 pid 从进程中读取内存
	* @param address 要读取的地址
	* @param pid 进程的进程 id
	* @return 从进程中读取的值
	*/
	template <typename T>
	T Read(void* address, int pid)
	{
		T buffer { };
		memset(&buffer, 0, sizeof(T));
		Read(reinterpret_cast<uint64_t>(address), reinterpret_cast<void*>(&buffer), sizeof(T), pid);

		return buffer;
	}

	template <typename T>
	T Read(uint64_t address, int pid)
	{
		return Read<T>(reinterpret_cast<void*>(address), pid);
	}

	/**
	* brief 从地址读取偏移链
	* @param address 要读取的地址
	* @param 要读取的偏移值向量
	* @return 从链中读取的值
	*/
	uint64_t ReadChain(uint64_t base, const std::vector<uint64_t>& offsets)
	{
		uint64_t result = Read<uint64_t>(base + offsets.at(0));
		for (int i = 1; i < offsets.size(); i++) result = Read<uint64_t>(result + offsets.at(i));
		return result;
	}

	template <typename T>
	T ReadChain(uint64_t base, const std::vector<uint64_t>& offsets)
	{
		T result = Read<T>(base + offsets.at(0));
		for (int i = 1; i < offsets.size(); i++) result = Read<T>(result + offsets.at(i));
		return result;
	}

	/**
	 * \brief 创建分散句柄，用于分散读/写请求
	* \return 分散句柄
	 */
	VMMDLL_SCATTER_HANDLE CreateScatterHandle() const;
	VMMDLL_SCATTER_HANDLE CreateScatterHandle(int pid) const;

	/*
	* 清除/重置句柄，以便在另一个后续读取散射操作中使用。
	* -- h S = 要清除以供重复使用的散射句柄。
	* -- dw PID = 可选的 PID 更改。
	* -- 标志
	* -- 返回
	*/
	void scatterClear(VMMDLL_SCATTER_HANDLE handle, int pid = 0);
	/**
	 * \brief 关闭散点图句柄
	* \param handle
	 */
	void CloseScatterHandle(VMMDLL_SCATTER_HANDLE handle);

	/*
	* 准备（添加）要读取的内存范围。调用
	* VMMDLL Scatter Execute*() 后，可以使用 VMMDLL Scatter Read() 检索内存。
	* -- h S
	* -- va = 要读取的内存范围的起始地址。
	* -- cb = 要读取的内存范围的大小。
	* -- 返回
	*/
	void scatterPrep(VMMDLL_SCATTER_HANDLE handle, uint64_t address, int size);
	/*读取先前填充范围内的内存。
	* -- h S
	* -- va
	* -- cb
	* -- pb
	* -- pcb Read
	* -- return
	*/
	template <typename TV>
	static TV scatterRead(VMMDLL_SCATTER_HANDLE handle, uint64_t address, size_t size) {
		TV buffer;
		DWORD bytesRead = 0;
		return VMMDLL_Scatter_Read(handle, address, size, (PBYTE)&buffer, &bytesRead) ? buffer : TV();

	}

	/**
	* \brief 向句柄添加分散读/写请求
	* \param handle 句柄
	* \param address 要读/写的地址
	* \param buffer 要读/写的缓冲区
	* \param size 缓冲区的大小
	 */
	void AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size);

	template <typename T>
	void AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, T* buffer)
	{
		AddScatterReadRequest(handle, address, reinterpret_cast<void*>(buffer), sizeof(T));
	}
	
	
	void AddScatterWriteRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size);
		
	template <typename T>
	void AddScatterWriteRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, T* buffer)
	{
		AddScatterWriteRequest(handle, address, reinterpret_cast<void*>(buffer), sizeof(T));
	}
	/**
	* \brief 执行所有准备好的散射请求，注意如果你创建了一个带有 pid 的散射句柄
	* 你需要在执行函数中指定 pid。这样​​我们就可以从句柄中清除散射。
	* \param handle
	* \param pid
	 */
	void ExecuteReadScatter(VMMDLL_SCATTER_HANDLE handle, int pid = 0);
	void ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE handle, int pid = 0);
	/*the FPGA handle*/
	VMM_HANDLE vHandle;
	VMMDLL_SCATTER_HANDLE handle;
};

inline Memory mem;
