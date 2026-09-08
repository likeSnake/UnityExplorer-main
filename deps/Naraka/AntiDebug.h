#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <TlHelp32.h>
using namespace std;

extern "C" bool AntiDebug();

//NtQueryInformationProcess反调试
typedef NTSTATUS(WINAPI* FnNtQueryInformationProcess)(
	HANDLE           ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID            ProcessInformation,
	ULONG            ProcessInformationLength,
	PULONG           ReturnLength
	);
bool AntiDebugNtQueryInformationProcess0x0(FnNtQueryInformationProcess func, HANDLE hProcess)
{
	PROCESS_BASIC_INFORMATION pbi = { 0 };
	DWORD dwPbiSize = sizeof(PROCESS_BASIC_INFORMATION);
	DWORD dwRetLength = 0;
	func(hProcess, ProcessBasicInformation, &pbi, dwPbiSize, &dwRetLength);
	PPEB peb = pbi.PebBaseAddress;
	if (peb->BeingDebugged == true)
		return true;
	else
		return false;
}
bool AntiDebugNtQueryInformationProcess0x7(FnNtQueryInformationProcess func, HANDLE hProcess)
{
	DWORD dwDebugPort = 0;
	func(hProcess, ProcessDebugPort, &dwDebugPort, sizeof(dwDebugPort), NULL);
	cout << dwDebugPort << endl;
	if (dwDebugPort == 0xFFFFFFFF)
		return true;
	else
		return false;
}
bool AntiDebugNtQueryInformationProcess0x1E(FnNtQueryInformationProcess func, HANDLE hProcess)
{
	HANDLE hDebugHandle = NULL;
	func(hProcess, (PROCESSINFOCLASS)0x1E, &hDebugHandle, sizeof(hDebugHandle), NULL);
	if (hDebugHandle != NULL)
		return true;
	else
		return false;
}
bool AntiDebugNtQueryInformationProcess0x1F(FnNtQueryInformationProcess func, HANDLE hProcess)
{
	BOOL bDebugFlag = false;
	func(hProcess, (PROCESSINFOCLASS)0x1F, &bDebugFlag, sizeof(bDebugFlag), NULL);
	if (bDebugFlag == false)
		return true;
	else
		return false;
}

//根据是否是顶层进行反调试
char* __cdecl stristrEx(const char* str1, const char* str2)
{
	char* cp = (char*)str1;
	char* s1, * s2;

	if (!*str2)
		return((char*)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char*)str2;

		while (*s1 && *s2 && toupper(*s1) == toupper(*s2)) {
			s1++, s2++;
		}

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);

}
bool TraversalTopWnd(vector<HWND>& vec)
{
	vec.clear();
	HWND hWnd = GetTopWindow(0);
	while (hWnd)
	{
		if (GetParent(hWnd) == 0)
			vec.push_back(hWnd);
		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}
	return vec.size() > 0;
}
bool AntiDebug2()
{
	std::vector<HWND> vec;
	TraversalTopWnd(vec);
	char szWindowName[MAX_PATH] = { 0 };
	for (HWND tmp : vec)
	{
		GetWindowTextA(tmp, szWindowName, MAX_PATH);
		if (stristrEx(szWindowName, "cheat") != NULL || stristrEx(szWindowName, "dbg") != NULL || stristrEx(szWindowName, "ntdll") != NULL)
		{
			return true;
		}
		memset(szWindowName, 0, MAX_PATH);
	}
	return false;
}

//基于父进程的反调试
DWORD GetParentProcessId(DWORD dwProcessId)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return -1;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	BOOL bMore = Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		if (pe32.th32ProcessID == dwProcessId)
		{
			return pe32.th32ParentProcessID;
		}
		bMore = Process32Next(hProcessSnap, &pe32);
	}
}
DWORD GetProcessIdForProcessName(const wchar_t* szProcessName)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return -1;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	BOOL bMore = Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		if (wcscmp(szProcessName, pe32.szExeFile) == 0)
		{
			return pe32.th32ProcessID;
		}
		bMore = Process32Next(hProcessSnap, &pe32);
	}

}
bool AntiDebug3()
{
	DWORD dwProcessId = GetProcessId(GetCurrentProcess());
	DWORD dwParentProcessId = GetParentProcessId(dwProcessId);
	DWORD NeacProcessId = GetProcessIdForProcessName(L"explorer.exe");
	if (dwParentProcessId == NeacProcessId)
		return false;
	else
		return true;
}

//ZwSetInformationThread反调试
typedef NTSTATUS(WINAPI* FnZwSetInformationThread)(
	HANDLE          ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID           ThreadInformation,
	ULONG           ThreadInformationLength
	);
bool GrkSetThreadInfo(HANDLE hThread, HMODULE hModule)
{
	FnZwSetInformationThread ZwSetInformationThread = (FnZwSetInformationThread)GetProcAddress(hModule, "ZwSetInformationThread");
	ZwSetInformationThread(hThread, (THREADINFOCLASS)0x11, 0, 0);
	return true;
}
bool AntiDebug4(HMODULE hModule)
{
	DWORD dwProcessId = GetProcessId(GetCurrentProcess());
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
	THREADENTRY32 th32;
	th32.dwSize = sizeof(THREADENTRY32);
	HANDLE hThread = NULL;
	while (Thread32Next(hThreadSnap, &th32))
	{
		if (dwProcessId == th32.th32OwnerProcessID)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID);
			GrkSetThreadInfo(hThread, hModule);
			CloseHandle(hThread);
		}
	}
	return false;
}

//实时调试器检测
bool AntiDebug5()
{
	char reg64[] = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug";
	char key[] = "Debugger";
	HKEY hKey = NULL;
	RegCreateKeyA(HKEY_LOCAL_MACHINE, reg64, &hKey);
	char temp[MAX_PATH] = { 0 };
	DWORD dwType;
	DWORD dwLength = MAX_PATH;
	if (ERROR_FILE_NOT_FOUND == RegQueryValueExA(hKey, key, NULL, &dwType, (LPBYTE)temp, &dwLength))
		return false;
	else
		return true;
}

//异常检测调试器
bool AntiDebug6()
{
	__try
	{
		HANDLE hTarge, hTarget2;
		DuplicateHandle((HANDLE)-1, (HANDLE)-1, (HANDLE)-1, &hTarge, NULL, FALSE, DUPLICATE_SAME_ACCESS);
		SetHandleInformation(hTarge, HANDLE_FLAG_PROTECT_FROM_CLOSE, HANDLE_FLAG_PROTECT_FROM_CLOSE);
		DuplicateHandle((HANDLE)-1, hTarge, (HANDLE)-1, &hTarget2, NULL, FALSE, DUPLICATE_CLOSE_SOURCE);
		return false;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return true;
	}
}

//检测硬件断点
bool AntiDebug7()
{
	DWORD dwProcessId = GetProcessId(GetCurrentProcess());
	HANDLE hShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
	THREADENTRY32 te32 = { 0 };
	te32.dwSize = sizeof(THREADENTRY32);
	HANDLE hThread = NULL;
	while (Thread32Next(hShot, &te32))
	{
		if (GetCurrentProcessId() == te32.th32OwnerProcessID)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			CONTEXT ctx{ 0 };
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			GetThreadContext(hThread, &ctx);
			if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0)
				return true;
			else
				false;
		}
	}
}

//CRC32检测调试器

DWORD AntiDebugCallBack(
	LPVOID lpThreadParameter
)
{
	//NtQueryInformationProcess反调试
	//HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	HANDLE hProcess = GetCurrentProcess();
	HMODULE hNtdll = LoadLibraryA("ntdll.dll");
	FnNtQueryInformationProcess func = (FnNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");

	while (true)
	{
		Sleep(1);
		if (AntiDebug2())
		{
			//cout << "Debug2" << endl;
			
			exit(0);
		}
		if (AntiDebug3())
		{
			//cout << "Debug3" << endl;
			
			exit(0);
		}
		if (AntiDebug4(hNtdll))
		{
			//cout << "Debug4" << endl;
		
			exit(0);
		}
		if (AntiDebug5())
		{
			//cout << "Debug5" << endl;
			exit(0);
		}
		if (AntiDebug6())
		{
			//cout << "Debug6" << endl;
		
			exit(0);
		}
		if (AntiDebug7())
		{
			//cout << "Debug7" << endl;
			
			exit(0);
		}
	}
}