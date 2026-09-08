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

	VOID InitVtbaleHook();
	VOID KnifeStartHook();
	VOID enable_vtbale(bool enable);
	VOID vtbale(bool enable);
}
