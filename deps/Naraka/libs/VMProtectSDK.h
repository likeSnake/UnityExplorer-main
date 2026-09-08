#pragma once

#if defined(__APPLE__) || defined(__unix__)
#define VMP_IMPORT 
#define VMP_API
#define VMP_WCHAR unsigned short
#else
#define VMP_IMPORT __declspec(dllimport)
#define VMP_API __stdcall
#define VMP_WCHAR wchar_t
#ifdef _WIN64
	#pragma comment(lib, "VMProtectSDK64.lib")
#else
	#pragma comment(lib, "VMProtectSDK32.lib")
#endif // _WIN64
#endif // __APPLE__ || __unix__

#ifdef __cplusplus
extern "C" {
#endif

// protection
// 导入VMP_API宏
VMP_IMPORT void VMP_API VMProtectBegin(const char *); // 开始保护
VMP_IMPORT void VMP_API VMProtectBeginVirtualization(const char *); // 开始虚拟化保护
VMP_IMPORT void VMP_API VMProtectBeginMutation(const char *); // 开始变异保护
VMP_IMPORT void VMP_API VMProtectBeginUltra(const char *); // 开始超保护
VMP_IMPORT void VMP_API VMProtectBeginVirtualizationLockByKey(const char *); // 开始虚拟化保护并锁定
VMP_IMPORT void VMP_API VMProtectBeginUltraLockByKey(const char *); // 开始超保护并锁定
VMP_IMPORT void VMP_API VMProtectEnd(void); // 结束保护
// utils
// 导入VMProtectIsProtected函数，用于判断当前程序是否被VMProtect保护
VMP_IMPORT bool VMP_API VMProtectIsProtected();
// 导入VMProtectIsDebuggerPresent函数，用于判断当前程序是否被调试器调试
VMP_IMPORT bool VMP_API VMProtectIsDebuggerPresent(bool);
// 导入VMProtectIsVirtualMachinePresent函数，用于判断当前程序是否在虚拟机中运行
VMP_IMPORT bool VMP_API VMProtectIsVirtualMachinePresent(void);
// 导入VMProtectIsValidImageCRC函数，用于判断当前程序的CRC值是否有效
VMP_IMPORT bool VMP_API VMProtectIsValidImageCRC(void);
// 导入VMProtectDecryptStringA函数，用于解密字符串
VMP_IMPORT const char * VMP_API VMProtectDecryptStringA(const char *value);
// 导入VMProtectDecryptStringW函数，用于解密宽字符字符串
VMP_IMPORT const VMP_WCHAR * VMP_API VMProtectDecryptStringW(const VMP_WCHAR *value);
// 导入VMProtectFreeString函数，用于释放字符串
VMP_IMPORT bool VMP_API VMProtectFreeString(const void *value);

// licensing
enum VMProtectSerialStateFlags
{
	SERIAL_STATE_SUCCESS				= 0,
	SERIAL_STATE_FLAG_CORRUPTED			= 0x00000001,
	SERIAL_STATE_FLAG_INVALID			= 0x00000002,
	SERIAL_STATE_FLAG_BLACKLISTED		= 0x00000004,
	SERIAL_STATE_FLAG_DATE_EXPIRED		= 0x00000008,
	SERIAL_STATE_FLAG_RUNNING_TIME_OVER	= 0x00000010,
	SERIAL_STATE_FLAG_BAD_HWID			= 0x00000020,
	SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED	= 0x00000040,
};

#pragma pack(push, 1)
typedef struct
{
	unsigned short	wYear;
	unsigned char	bMonth;
	unsigned char	bDay;
} VMProtectDate;

typedef struct
{
	int				nState;				// VMProtectSerialStateFlags
	VMP_WCHAR		wUserName[256];		// user name
	VMP_WCHAR		wEMail[256];		// email
	VMProtectDate	dtExpire;			// date of serial number expiration
	VMProtectDate	dtMaxBuild;			// max date of build, that will accept this key
	int				bRunningTime;		// running time in minutes
	unsigned char	nUserDataLength;	// length of user data in bUserData
	unsigned char	bUserData[255];		// up to 255 bytes of user data
} VMProtectSerialNumberData;
#pragma pack(pop)

// 设置序列号
VMP_IMPORT int VMP_API VMProtectSetSerialNumber(const char *serial);
// 获取序列号状态
VMP_IMPORT int VMP_API VMProtectGetSerialNumberState();
// 获取序列号数据
VMP_IMPORT bool VMP_API VMProtectGetSerialNumberData(VMProtectSerialNumberData *data, int size);
// 获取当前硬件ID
VMP_IMPORT int VMP_API VMProtectGetCurrentHWID(char *hwid, int size);
// activation
enum VMProtectActivationFlags
{
	ACTIVATION_OK = 0,
	ACTIVATION_SMALL_BUFFER,
	ACTIVATION_NO_CONNECTION,
	ACTIVATION_BAD_REPLY,
	ACTIVATION_BANNED,
	ACTIVATION_CORRUPTED,
	ACTIVATION_BAD_CODE,
	ACTIVATION_ALREADY_USED,
	ACTIVATION_SERIAL_UNKNOWN,
	ACTIVATION_EXPIRED,
	ACTIVATION_NOT_AVAILABLE
};

// 导入VMProtectActivateLicense函数，用于激活许可证
VMP_IMPORT int VMP_API VMProtectActivateLicense(const char *code, char *serial, int size);
// 导入VMProtectDeactivateLicense函数，用于注销许可证
VMP_IMPORT int VMP_API VMProtectDeactivateLicense(const char *serial);
// 导入VMProtectGetOfflineActivationString函数，用于获取离线激活字符串
VMP_IMPORT int VMP_API VMProtectGetOfflineActivationString(const char *code, char *buf, int size);
// 导入VMProtectGetOfflineDeactivationString函数，用于获取离线注销字符串
VMP_IMPORT int VMP_API VMProtectGetOfflineDeactivationString(const char *serial, char *buf, int size);

#ifdef __cplusplus
}
#endif
