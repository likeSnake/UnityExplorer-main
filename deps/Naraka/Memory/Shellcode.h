#pragma once
#include "pch.h"

class c_shellcode
{
private:

public:
	c_shellcode() = default;

	~c_shellcode() = default;

	/**
	* @Brief 在目标进程中查找代码洞，其大小至少为函数大小且具有读取、写入、执行权限。
	* @param 函数大小 - 我们要注入的函数的大小
	* @param 进程名称 - 我们要注入的进程的名称
	* @return 代码洞的地址，如果失败则返回 0
	**/
	uint64_t find_codecave(size_t function_size, const std::string& process_name, const std::string& module);

	/**
	* @brief 查找目标进程中的所有代码洞，其大小至少与函数大小相同，并且具有读取、写入、执行权限。
	* @param 函数大小 - 我们要注入的函数的大小
	* @param 进程名称 - 我们要注入的进程的名称
	* @return 所有具有足够大的代码洞的地址。
	**/
	std::vector<uint64_t> find_all_codecave(size_t function_size, const std::string& process_name);

	/**
	* @param hook - 我们要挂接以调用函数参数的函数
	* @param function - 要调用的函数
	* @param process name - 我们要注入的进程的名称
	* @return 如果成功则返回 true，否则返回 false。
	**/
	bool call_function(void* hook, void* function, const std::string& process_name);
	bool function(uintptr_t hook, uintptr_t function, const std::string& process_name);
	//template <typename T, typename... Args>
	//auto SysCall(uint64_t function, Args&&... args) -> std::enable_if_t<!std::is_void<std::invoke_result_t<T, Args...>>::value, std::invoke_result_t<T, Args...>>
	//{
	//	uintptr_t ntos_shutdown = mem.GetExportTableAddress("NtShutdownSystem", "csrss.exe", "ntoskrnl.exe");
	//	uint64_t nt_shutdown = (uint64_t)GetProcAddress(LoadLibraryA("ntdll.dll"), "NtShutdownSystem");

	//	T buffer { };

	//	BYTE jmp_bytes[14] = {
	//		0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp [RIP+0x00000000]
	//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // RIP value
	//	};
	//	*reinterpret_cast<uint64_t*>(jmp_bytes + 6) = function;

	//	 Save original bytes
	//	BYTE orig_bytes[sizeof(jmp_bytes)];
	//	if (!mem.Read(ntos_shutdown, (PBYTE)orig_bytes, sizeof(orig_bytes), 4))
	//		return buffer;

	//
	//	if (!mem.Write(ntos_shutdown, jmp_bytes, sizeof(jmp_bytes), 4))
	//	{
	//		LOG("[!] Failed to write memory at 0x%p\n", ntos_shutdown);
	//		return buffer;
	//	}

	//	 Call function
	//	buffer = std::invoke(reinterpret_cast<T>(nt_shutdown), std::forward<Args>(args)...);

	//	Restore function
	//	if (!mem.Write(ntos_shutdown, orig_bytes, sizeof(orig_bytes), 4))
	//		LOG("[!] Failed to write memory at 0x%p\n", ntos_shutdown);

	//	return buffer;
	//}

	//template <typename T, typename... Args>
	//void SysCall(uint64_t function, Args&&... args)
	//{
	//	uintptr_t ntos_shutdown = mem.GetExportTableAddress("NtShutdownSystem", "csrss.exe", "ntoskrnl.exe");
	//	uint64_t nt_shutdown = (uint64_t)GetProcAddress(LoadLibraryA("ntdll.dll"), "NtShutdownSystem");

	//	T buffer { };

	//	BYTE jmp_bytes[14] = {
	//		0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp [RIP+0x00000000]
	//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // RIP value
	//	};
	//	*reinterpret_cast<uint64_t*>(jmp_bytes + 6) = function;

	//	// Save original bytes
	//	BYTE orig_bytes[sizeof(jmp_bytes)];
	//	if (!mem.Read(ntos_shutdown, (PBYTE)orig_bytes, sizeof(orig_bytes), 4))
	//		return;
	//	if (!mem.Write(ntos_shutdown, jmp_bytes, sizeof(jmp_bytes), 4))
	//	{
	//		LOG("[!] Failed to write memory at 0x%p\n", ntos_shutdown);
	//		return;
	//	}

	//	std::invoke(reinterpret_cast<T>(nt_shutdown), std::forward<Args>(args)...);

	//	// Restore function
	//	if (!mem.Write(ntos_shutdown, orig_bytes, sizeof(orig_bytes), 4))
	//		LOG("[!] Failed to write memory at 0x%p\n", ntos_shutdown);
	//}
};
