#pragma once
#include"Data.h"

namespace Hook
{
	extern uintptr_t vtbale_hook_address;
	extern uintptr_t vtbale_parry_address;
	extern uintptr_t vtbale_old_address;
	extern uintptr_t vtbale_break_address;
	extern uintptr_t vtbale_jmp_address;
	extern uintptr_t vtbale_switch_address;
	extern uintptr_t vtbale_ret_address;
	extern uintptr_t vtbale_Knifevtable;
	extern uintptr_t instruction_addr;

	VOID InitVtbaleHook();
	bool KnifeStartHook();
	bool KnifeStartHook_CrossFade();
	bool VirtualShockHOOK();
	VOID restore();
	void SkockOn();
	void SkockOf();
	void ReportMemoryShockInitFailure();
	bool InitMemoryShockHook();
	void SetMemoryShockEnabled(bool enable);
	void SetMemoryShockBerserkMode(bool enable);
	bool IsMemoryShockReady();
	bool IsMemoryShockEnabled();
	bool InitCrossFadeSpeedHook();
	void EnableCrossFadeSpeedHook(bool enable);
	bool IsCrossFadeSpeedHookEnabled();
	void SetCrossFadeSpeed(float speed);
	VOID enable_vtbale(bool enable);
	VOID vtbale(bool enable);

	// Phase A: AnimPlayable.CrossFadeInFixedTime 只读 Probe
	bool InitCrossFadeProbeHook();
	void EnableCrossFadeProbeHook(bool enable);
	bool IsCrossFadeProbeHookEnabled();
	// 临时测试 stateHash 用，当前整体注释停用。
	//void ResetCrossFadeProbeDebug();
	//bool TryReadCrossFadeProbeStateHash(int& stateHash, int* layerIdx = nullptr, unsigned long long* hitCount = nullptr);
	void PrintCrossFadeProbeDebug(const char* tag);
}
