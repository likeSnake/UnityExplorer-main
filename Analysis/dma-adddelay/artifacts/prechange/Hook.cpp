#include "Hook.h"
#include "memory.h"
#include "ERPort/MemoryAllocation.h"
#include "ERPort/SignatureDefs.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
	inline void OffsetToBytes(uint32_t offset, BYTE* bytes)
	{
		bytes[0] = static_cast<BYTE>(offset);
		bytes[1] = static_cast<BYTE>(offset >> 8);
		bytes[2] = static_cast<BYTE>(offset >> 16);
		bytes[3] = static_cast<BYTE>(offset >> 24);
	}

	inline void OffsetToBytes(uintptr_t offset, BYTE* bytes)
	{
		bytes[0] = static_cast<BYTE>(offset);
		bytes[1] = static_cast<BYTE>(offset >> 8);
		bytes[2] = static_cast<BYTE>(offset >> 16);
		bytes[3] = static_cast<BYTE>(offset >> 24);
		bytes[4] = static_cast<BYTE>(offset >> 32);
		bytes[5] = static_cast<BYTE>(offset >> 40);
		bytes[6] = static_cast<BYTE>(offset >> 48);
		bytes[7] = static_cast<BYTE>(offset >> 56);
	}

	inline void OffsetToBytesBE(uintptr_t offset, BYTE* bytes)
	{
		bytes[0] = static_cast<BYTE>(offset >> 56);
		bytes[1] = static_cast<BYTE>(offset >> 48);
		bytes[2] = static_cast<BYTE>(offset >> 40);
		bytes[3] = static_cast<BYTE>(offset >> 32);
		bytes[4] = static_cast<BYTE>(offset >> 24);
		bytes[5] = static_cast<BYTE>(offset >> 16);
		bytes[6] = static_cast<BYTE>(offset >> 8);
		bytes[7] = static_cast<BYTE>(offset);
	}
}

BYTE* HexChangeByte(std::string CurString, bool Flag, int StrLen, PBYTE zz)
{
	std::string aa;
	BYTE ReByte[4] = { 0 };
	//cout << "要转换的字符串:" << CurString << endl;
	//BYTE* ReByte = new BYTE;
	int Pos = 0;
	long temporary = 0;
	int n = 0;
	int size = 0;

	aa = CurString;

	if (strlen(aa.c_str()) % 2 != 0)
		aa = "0" + aa;

	//cout << "加0后的字符串" << aa << endl;
	//cout << strlen(aa.c_str()) << endl;
	//cout << hex << aa << endl;
	//cout << hex<< aa.substr(0, 2) << endl;

	//cout << strlen(aa.c_str()) - 1 << endl;

	for (int cc = (int)(strlen(aa.c_str()) - 1); cc >= 1; cc -= 2)
	{
		//temporary = hex2num(aa.substr(Pos, 2));
		zz[size] = { (BYTE)stoul(aa.substr(Pos, 2), nullptr, 16) };
		//printf("%X, ", zz[size]);
		size++;
		Pos += 2;
	}

	if (Flag)
	{
		if (size < StrLen)
		{
			int a = 0;
			for (int i = size; i >= 0; i--)
			{
				zz[StrLen - a] = zz[i];
				a++;
			}
			for (int i = 0; i < StrLen - size; i++)
			{
				zz[i] = { 0 };
			}

		}
	}
	return zz;
}
std::string n2hexstr(long long w, size_t hex_len) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}
VOID RtlCopyMemoryEx(PBYTE _Dst, PBYTE _Src, SIZE_T _Size)
{
	for (size_t i = 0; i < _Size; i++)
	{
		_Dst[i] = _Src[i];
	}
}
namespace Hook
{
	uintptr_t vtbale_hook_address = NULL;
	uintptr_t vtbale_parry_address = NULL;
	uintptr_t vtbale_old_address = NULL;
	uintptr_t vtbale_break_address = NULL;
	uintptr_t vtbale_jmp_address = NULL;
	uintptr_t vtbale_switch_address = NULL;
	uintptr_t vtbale_ret_address = NULL;
	uintptr_t vtbale_Knifevtable = NULL;
	uintptr_t instruction_addr = NULL;
	static std::atomic_bool g_memory_shock_hook_ready = false;
	static std::atomic_bool g_memory_shock_enabled = false;
	static std::atomic_bool g_memory_shock_initializing = false;
	static constexpr bool kMemoryShockVerboseLog = true;
	static constexpr bool kMemoryShockResultLog = true;
	static constexpr size_t kMemoryShockControlBytes = 8;
	static constexpr size_t kMemoryShockLegacyShellcodeSize = 299;
	static constexpr size_t kMemoryShockCrossFadeShellcodeSize = 360;
	static constexpr size_t kMemoryShockCrossFadeSwitchShellcodeSize = 373;
	static constexpr size_t kMemoryShockCrossFadeControlPatchOffset = 79;
	static constexpr uint64_t kMemoryShockLegacyShellMagic = 4707740824204951893ull;
	static constexpr uint64_t kMemoryShockLegacyMetadataMagic = 0x3147454C4B48534Dull; // MSHKLEG1
	static constexpr uint64_t kMemoryShockCrossFadeMetadataMagic = 0x314446434B48534Dull; // MSHKCFD1
	static std::atomic_bool g_memory_shock_success_reported = false;
	static std::atomic_bool g_memory_shock_failure_reported = false;
	static std::atomic_bool g_memory_shock_berserk_mode = false;
	static std::atomic_int g_memory_shock_active_mode = -1; // -1=none, 0=CrossFade, 1=Legacy, 2=Virtual

	static constexpr BYTE kCrossFadeControlGateBytes[] = {
		0x80, 0x3D, 0xA2, 0xFF, 0xFF, 0xFF, 0x01,
		0x0F, 0x85, 0xAF, 0x00, 0x00, 0x00,
	};
	static constexpr BYTE kCrossFadeControlNopBytes[] = {
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	};
	static_assert(sizeof(kCrossFadeControlGateBytes) == sizeof(kCrossFadeControlNopBytes), "CrossFade control patch size mismatch");

	struct MemoryShockLegacyMetadata
	{
		uint64_t magic = 0;
		uint64_t originalRet = 0;
		uint64_t shellEntry = 0;
		uint64_t vtableAddress = 0;
	};
	static_assert(sizeof(MemoryShockLegacyMetadata) == 32, "MemoryShockLegacyMetadata size");

	template <typename... Args>
	static void MemoryShockDebugPrintf(const char* format, Args... args)
	{
		if constexpr (kMemoryShockVerboseLog)
		{
			::printf(format, args...);
		}
	}

	static void printf(const char* format, ...)
	{
		if constexpr (kMemoryShockVerboseLog)
		{
			va_list args;
			va_start(args, format);
			::vprintf(format, args);
			va_end(args);
		}
	}

	static void MemoryShockDebugCout(const char* message)
	{
		if constexpr (kMemoryShockVerboseLog)
		{
			std::cout << message << std::endl;
		}
	}

	void ReportMemoryShockInitFailure()
	{
		if constexpr (kMemoryShockResultLog)
		{
			bool expected = false;
			if (!g_memory_shock_success_reported.load() &&
				g_memory_shock_failure_reported.compare_exchange_strong(expected, true))
			{
				std::cout << "内存振开启 失败" << std::endl;
			}
		}
	}

	static void ReportMemoryShockInitSuccess()
	{
		if constexpr (kMemoryShockResultLog)
		{
			bool expected = false;
			if (g_memory_shock_success_reported.compare_exchange_strong(expected, true))
			{
				std::cout << "内存振开启 成功" << std::endl;
			}
		}
		g_memory_shock_failure_reported.store(false);
	}

	static void ResetMemoryShockResultLog()
	{
		g_memory_shock_success_reported.store(false);
		g_memory_shock_failure_reported.store(false);
	}

	static bool EnsureInitActorKitRefer();
	static bool CommitMemoryShockHook(const char* label, uint64_t magic);
	static const char* GetCurrentGameAssemblyModuleName();
	static uintptr_t FindReadOnlyCodecave(size_t dataSize, const char* label);
	static uintptr_t FindMemoryShockCodecave(size_t dataSize, const char* label);
	static uintptr_t FindWritableCodecave(const BYTE* data, size_t dataSize, const char* label);
	static bool WriteMemoryShockShellcodeBlock(const char* label, uintptr_t baseAddress, const BYTE* shellcode, size_t shellcodeSize,
		const MemoryShockLegacyMetadata* metadata = nullptr);
	static bool TryReadMemoryShockMetadata(uintptr_t shellEntry, size_t shellcodeSize, uint64_t metadataMagic,
		uintptr_t expectedVtable, MemoryShockLegacyMetadata& metadata);
	static bool TryReadAnyMemoryShockMetadata(uintptr_t shellEntry, uintptr_t expectedVtable,
		MemoryShockLegacyMetadata& metadata);
	static bool RecoverMemoryShockOriginalRetFromMetadata(const char* label, uintptr_t currentVtableTarget,
		uintptr_t vtableAddress, uintptr_t& originalRet);
	static void MarkBadCodecave(uintptr_t address);
	static void ReleaseCodecaveReservation(uintptr_t address);

	VOID InitVtbaleHook() {

		vtbale_jmp_address = Offset::NarakaBladepoint + Offset::MemoryShockCodecaveHintRva;

		vtbale_hook_address = Offset::GameAssembly + Offset::InitActorKitRefer; //0x967F1A0

		vtbale_jmp_address = vtbale_jmp_address + 0x300;
		vtbale_switch_address = vtbale_jmp_address + 0x200;
		BYTE  code[] = {
			0x48,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//mov rax,00007FFB19EFC1AD
			0x48,0x39,0x84,0x24,0xC0,0x00,0x00,0x00,//cmp [rsp+000000C0],rax
			0x75,0x20,//jne 29E65A70334
			0x48,0xB8,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//mov rax,0000000000000006
			0x48,0x01,0x84,0x24,0xC0,0x00,0x00,0x00,//add [rsp+000000C0],rax
			0xFF,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//jmp 7FFB1BE254D0
			0x48,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//mov rax,0000029E65A70500
			0x83,0x38,0x01,//cmp dword ptr [rax],01
			0x75,0x1A,//jne 29E65A7035D
			0xC7,0x00,0x00,0x00,0x00,0x00,//mov [rax],00000000
			0x48,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//mov rax,00007FFB1BE254D0
			0x48,0xA3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//mov [29E6C5CA890],rax
			0xFF,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00//jmp 7FFB1BE254D0
		};
		*(uintptr_t*)&code[2] = vtbale_break_address;
		vtbale_parry_address = mem.Read<uintptr_t>(vtbale_hook_address);
		vtbale_parry_address = mem.Read<uintptr_t>(
			vtbale_parry_address + Offset::MemoryShockBootstrapChain0Offset);
		vtbale_parry_address = mem.Read<uintptr_t>(
			vtbale_parry_address + Offset::MemoryShockBootstrapChain1Offset);
		vtbale_parry_address = mem.Read<uintptr_t>(
			vtbale_parry_address + Offset::MemoryShockBootstrapChain2Offset);

		if (!vtbale_parry_address) {
			return;
		}
		vtbale_old_address = mem.Read<uintptr_t>(vtbale_parry_address);
		if (!vtbale_old_address) {
			return;
		}
		*(uintptr_t*)&code[44] = vtbale_old_address;
		*(uintptr_t*)&code[54] = vtbale_switch_address;
		*(uintptr_t*)&code[75] = vtbale_old_address;
		*(uintptr_t*)&code[85] = vtbale_parry_address;
		*(uintptr_t*)&code[99] = vtbale_old_address;
		mem.Write(vtbale_jmp_address, code, sizeof(code));
		//RtlCopyMemoryEx((PBYTE)vtbale_jmp_address, code, sizeof(code));
		//Dedbg("%llX", Parry::vtbale_jmp_address);
	}

	VOID enable_vtbale(bool enable) {
		mem.Write<uintptr_t>(vtbale_parry_address, enable ? vtbale_jmp_address : vtbale_old_address);
	}

	bool KnifeStartHook()
	{
		if (g_memory_shock_hook_ready)
		{
			return true;
		}
		if (!EnsureInitActorKitRefer())
		{
			ReportMemoryShockInitFailure();
			return false;
		}

		static constexpr size_t kLegacyCodecaveSize = kMemoryShockControlBytes + kMemoryShockLegacyShellcodeSize +
			sizeof(MemoryShockLegacyMetadata);

		uintptr_t temp = mem.ReadMultiLevel<uintptr_t>(
			Offset::GameAssembly,
			Offset::InitActorKitRefer,
			Offset::Il2CppClassStaticFieldsOffset,
			Offset::Il2CppStaticInstanceOffset,
			Offset::MemoryShockTransparentTagHandlerActorKitOffset,
			Offset::ActorModel.ActorKitS.InteractableDevice,
			0x0);
		if (!temp)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Legacy vtable chain failed\n");
			return false;
		}

		vtbale_Knifevtable = temp + Offset::MemoryShockInteractableVtableSlotOffset;
		uintptr_t currentVtableTarget = mem.Read<uintptr_t>(vtbale_Knifevtable);
		if (!currentVtableTarget)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Legacy ret read failed @ 0x%llX\n",
				(unsigned long long)vtbale_Knifevtable);
			return false;
		}
		vtbale_ret_address = currentVtableTarget;
		if (!RecoverMemoryShockOriginalRetFromMetadata(
			"Legacy",
			currentVtableTarget,
			vtbale_Knifevtable,
			vtbale_ret_address))
		{
			return false;
		}

		uint64_t lastAllocFailLogTick = 0;
		do
		{
			Sleep(100);
			vtbale_jmp_address = FindReadOnlyCodecave(
				kLegacyCodecaveSize,
				"MemoryShock Legacy reserve");
			if (!vtbale_jmp_address)
			{
				const uint64_t now = GetTickCount64();
				if (now - lastAllocFailLogTick >= 3000)
				{
					ReportMemoryShockInitFailure();
					MemoryShockDebugPrintf("[MemoryShock] Legacy read-only codecave reserve failed, retrying size=%zu\n",
						kLegacyCodecaveSize);
					lastAllocFailLogTick = now;
				}
			}
		} while (!vtbale_jmp_address);

		BYTE Char[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::InitActorKitRefer, Char);
		BYTE Refer[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyReferRva, Refer);
		BYTE Re[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyReRva, Re);
		BYTE Ref[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyRefRva, Ref);
		BYTE JMP[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyJmpRva, JMP);
		BYTE JMP2[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyJmp2Rva, JMP2);
		BYTE JMP3[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyJmp3Rva, JMP3);
		BYTE JMP4[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockLegacyJmp4Rva, JMP4);

		BYTE test_offset_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyTestFieldOffset, test_offset_bytes);

		BYTE cmp_value_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyCompareValue, cmp_value_bytes);

		BYTE mov_offset_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyMoveFieldOffset, mov_offset_bytes);

		BYTE actor_kit_offset_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKit), actor_kit_offset_bytes);

		BYTE actor_kit_BreakData_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKitS.actorKitBreakData), actor_kit_BreakData_bytes);
		BYTE il2cpp_static_fields_bytes[4] = { 0 };
		OffsetToBytes(Offset::Il2CppClassStaticFieldsOffset, il2cpp_static_fields_bytes);
		BYTE legacy_local_actor_chain0_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyLocalActorChain0Offset, legacy_local_actor_chain0_bytes);
		BYTE legacy_ref_state_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyRefStateOffset, legacy_ref_state_bytes);
		BYTE legacy_ref_object_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockLegacyRefObjectOffset, legacy_ref_object_bytes);
		static_assert(Offset::Il2CppStaticInstanceOffset <= 0x7F, "Legacy static instance requires disp8");
		static_assert(Offset::MemoryShockHookContextEntityKitOffset <= 0x7F, "Legacy hook context requires disp8");
		static_assert(Offset::MemoryShockEntityKitActorModelOffset <= 0x7F, "Legacy actor model requires disp8");
		static_assert(Offset::MemoryShockLegacyLocalActorChain1Offset <= 0x7F, "Legacy actor chain requires disp8");
		static_assert(Offset::MemoryShockCacheDelayBreakTypeOffset <= 0x7F, "Legacy break cache requires disp8");
		static_assert(Offset::MemoryShockLegacyRefCompareOffset <= 0x7F, "Legacy compare field requires disp8");

		instruction_addr = vtbale_jmp_address + 8;

		BYTE ShellCode[] = {
			0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54,
			0x56, 0x57, 0x53, 0x48, 0x81, 0xEC, 0xC8, 0x00, 0x00, 0x00,
			0x48, 0x8D, 0xAC, 0x24, 0x80, 0x00, 0x00, 0x00,
			0x48, 0xC7, 0x45, 0x38, 0xFE, 0xFF, 0xFF, 0xFF,
			0x49, 0x89, 0xCE,
			0xA0,
			Refer[7], Refer[6], Refer[5], Refer[4], Refer[3], Refer[2], Refer[1], Refer[0],
			0x3C, 0x00,
			0x0F, 0x84, 0xCA, 0x00, 0x00, 0x00,
			0x44, 0x8B, 0xAD, 0xC8, 0x00, 0x00, 0x00,
			0x44, 0x8B, 0x95, 0xC0, 0x00, 0x00, 0x00,
			0x44, 0x8B, 0xA5, 0xB8, 0x00, 0x00, 0x00,
			0xC5, 0xF8, 0x57, 0xC0,
			0xC5, 0xF8, 0x29, 0x45, 0xE0,
			0x48, 0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00,
			// 原逻辑: cmp byte ptr [control], 1; jne skip
			// 当前测试: 去掉 SkockOn/SkockOf 控制 byte 限制，保持长度不变。
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x48, 0x8B, 0x41, static_cast<BYTE>(Offset::MemoryShockHookContextEntityKitOffset),
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::MemoryShockEntityKitActorModelOffset),
			0x48, 0x8B, 0xF0,
			0x48, 0xA1,
			Char[7], Char[6], Char[5], Char[4], Char[3], Char[2], Char[1], Char[0],
			0x48, 0x8B, 0x80,
			il2cpp_static_fields_bytes[0], il2cpp_static_fields_bytes[1],
			il2cpp_static_fields_bytes[2], il2cpp_static_fields_bytes[3],
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::Il2CppStaticInstanceOffset),
			0x48, 0x8B, 0x80,
			legacy_local_actor_chain0_bytes[0], legacy_local_actor_chain0_bytes[1],
			legacy_local_actor_chain0_bytes[2], legacy_local_actor_chain0_bytes[3],
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::MemoryShockLegacyLocalActorChain1Offset),
			0x48, 0x39, 0xC6,
			0x75, 0x15,
			0x48, 0x8B, 0x80,
			actor_kit_offset_bytes[0], actor_kit_offset_bytes[1], actor_kit_offset_bytes[2], actor_kit_offset_bytes[3],
			0x48, 0x8B, 0x80,
			actor_kit_BreakData_bytes[0], actor_kit_BreakData_bytes[1], actor_kit_BreakData_bytes[2], actor_kit_BreakData_bytes[3],
			0xC7, 0x40, static_cast<BYTE>(Offset::MemoryShockCacheDelayBreakTypeOffset), 0x00, 0x30, 0x00, 0x00,
			0x48, 0xA1,
			Re[7], Re[6], Re[5], Re[4], Re[3], Re[2], Re[1], Re[0],
			0xF6, 0x80,
			test_offset_bytes[0], test_offset_bytes[1], test_offset_bytes[2], test_offset_bytes[3],
			0x02,
			0x74, 0x51,
			0x48, 0xA1,
			Ref[7], Ref[6], Ref[5], Ref[4], Ref[3], Ref[2], Ref[1], Ref[0],
			0x48, 0x8B, 0xC8,
			0x83, 0xB9,
			legacy_ref_state_bytes[0], legacy_ref_state_bytes[1], legacy_ref_state_bytes[2], legacy_ref_state_bytes[3], 0x00,
			0x74, 0x49,
			0x48, 0x8B, 0x81,
			legacy_ref_object_bytes[0], legacy_ref_object_bytes[1],
			legacy_ref_object_bytes[2], legacy_ref_object_bytes[3],
			0x48, 0x8B, 0x00,
			0x81, 0x78, static_cast<BYTE>(Offset::MemoryShockLegacyRefCompareOffset),
			cmp_value_bytes[0], cmp_value_bytes[1], cmp_value_bytes[2], cmp_value_bytes[3],
			0x7C, 0x28,
			0x48, 0x8B, 0x88,
			mov_offset_bytes[0], mov_offset_bytes[1], mov_offset_bytes[2], mov_offset_bytes[3],
			0x48, 0x85, 0xC9,
			0x74, 0x1C,
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP3[7], JMP3[6], JMP3[5], JMP3[4], JMP3[3], JMP3[2], JMP3[1], JMP3[0],
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP[7], JMP[6], JMP[5], JMP[4], JMP[3], JMP[2], JMP[1], JMP[0],
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP2[7], JMP2[6], JMP2[5], JMP2[4], JMP2[3], JMP2[2], JMP2[1], JMP2[0],
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP4[7], JMP4[6], JMP4[5], JMP4[4], JMP4[3], JMP4[2], JMP4[1], JMP4[0],
		};
		static_assert(sizeof(ShellCode) == kMemoryShockLegacyShellcodeSize, "Legacy shellcode size changed");

		MemoryShockLegacyMetadata legacyMetadata{};
		legacyMetadata.magic = kMemoryShockLegacyMetadataMagic;
		legacyMetadata.originalRet = vtbale_ret_address;
		legacyMetadata.shellEntry = instruction_addr;
		legacyMetadata.vtableAddress = vtbale_Knifevtable;

		if (!WriteMemoryShockShellcodeBlock("Legacy", vtbale_jmp_address, ShellCode, sizeof(ShellCode), &legacyMetadata))
		{
			ReleaseCodecaveReservation(vtbale_jmp_address);
			MarkBadCodecave(vtbale_jmp_address);
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Legacy shellcode write failed\n");
			return false;
		}

		bool committed = CommitMemoryShockHook("Legacy", 4707740824204951893ull);
		if (committed)
		{
			g_memory_shock_active_mode = 1;
		}
		return committed;
	}

	VOID vtbale(bool enable)
	{
		if (enable == true)
		{
			mem.GetShellcode().function(Global::WorldPtr.Knifevtable, vtbale_jmp_address, "NarakaBladepoint.exe");

		}
	}

	// =============================================================================
	// 动画过渡加速 — 自研Shellcode (DMA逆向, 2026-05-28)
	//
	// 核心逆向发现 (GameAssembly_Super.dll 现场DMA):
	//   ActorModel链:  [EntityKit+0x18] 直接指向ActorModel (3/3验证)
	//   AnimPlayable:  [ActorKit+Offset::ActorModel.ActorKitS.AnimPlayableAgent]
	//   本地玩家检测:  TransparentTagHandler2 -> CharacterManager (回退链, 验证可用)
	//   FlushState参数: rcx=this, edx=layerIdx, r8d=curTagHash, r9=pTagState,
	//                   [rsp+0x28]=lastTagHash, [rsp+0x30]=lastStateName, [rsp+0x38]=targetTagHash
	//   CrossFadeInFixedTime: rcx=AnimPlayable, edx=stateHash, xmm1=duration, r8d=layer, xmm2=offset
	// =============================================================================
	static constexpr size_t kFlushStolenSize = 15;
	static constexpr BYTE kFlushExpectedPrologue[kFlushStolenSize] = {
		0x56, 0x48, 0x83, 0xEC, 0x30,
		0x4D, 0x89, 0xC3,
		0x48, 0x89, 0xC8,
		0x49, 0x8B, 0x49, 0x18
	};
	static constexpr size_t kFlushDebugDataSize = 0x20;
	static constexpr uint64_t kFlushDebugMagic = 0x414343454C44554Full; // ODULECCA
	static constexpr uint64_t kMarkerTthSlot = 0x1111111111111111ull;
	static constexpr uint64_t kMarkerCrossFade = 0x2222222222222222ull;
	static constexpr uint64_t kMarkerEntryCounter = 0x3333333333333333ull;
	static constexpr uint64_t kMarkerCallCounter = 0x4444444444444444ull;

	uintptr_t flush_hook_trampoline = 0;
	uintptr_t flush_codecave = 0;
	static uintptr_t g_flush_debug_data = 0;
	static BYTE g_flush_orig_bytes[kFlushStolenSize] = { 0 };
	static bool g_flush_orig_saved = false;
	static bool g_anim_accel_enabled = false;
	static bool g_flush_hook_initialized = false;
	// Phase A: 只读Probe for AnimPlayable.CrossFadeInFixedTime (7 params)
	static constexpr uint64_t kCrossFadeProbeMagic = 0x414245504F524643ull; // "CFPROBEA"
	struct CrossFadeProbeDebug {
		uint64_t magic;           // 0x00
		uint64_t hitCount;        // 0x08
		uint64_t lastThis;        // 0x10  AnimPlayable*
		int32_t  lastStateHash;   // 0x18
		int32_t  lastLayerIdx;    // 0x1C
		float    lastTransitionDuration; // 0x20
		float    lastFixedTime;   // 0x24
		float    lastSpeed;       // 0x28
		int32_t  lastSourceType;  // 0x2C  candidate from [rsp+0x20]
		uint8_t  lastForceSyncLoad; // 0x30 candidate from [rsp+0x28]
		uint8_t  reserved[7];     // 0x31
		uint64_t lastRcx;         // 0x38
		uint64_t lastRdx;         // 0x40
		uint64_t lastR8;          // 0x48
		uint64_t lastR9;          // 0x50
		float    lastXmm0;        // 0x58
		float    lastXmm1;        // 0x5C
		float    lastXmm2;        // 0x60
		float    lastXmm3;        // 0x64
		uint64_t lastRsp;         // 0x68  saved original rsp at hook entry
		uint64_t lastReturnAddress; // 0x70 [original_rsp+0x00]
		uint64_t stackQword[16];  // 0x78  [original_rsp+0x00]..[original_rsp+0x78]
	};
	static_assert(sizeof(CrossFadeProbeDebug) == 0xF8, "CrossFadeProbeDebug size");

	static uintptr_t g_cf_probe_address = 0;     // AnimPlayable.CrossFadeInFixedTime VA
	static uintptr_t g_cf_probe_shellcode = 0;   // codecave for shellcode
	static uintptr_t g_cf_probe_debug = 0;       // codecave for CrossFadeProbeDebug
	static BYTE g_cf_probe_orig_bytes[32] = { 0 };
	static size_t g_cf_probe_stolen_size = 0;
	static bool g_cf_probe_initialized = false;
	static bool g_cf_probe_enabled = false;

	static constexpr uint64_t kCrossFadeSpeedMagic = 0x315053464B4F4F48ull; // HOOKFSP1
	struct CrossFadeSpeedControl {
		uint64_t magic;              // 0x00
		uint8_t enabled;             // 0x08
		uint8_t reserved0[7];        // 0x09
		float speed;                 // 0x10
		uint32_t reserved1;          // 0x14
		uint64_t localAnimPlayable;  // 0x18
		uint64_t hitCount;           // 0x20
		uint64_t selfHitCount;       // 0x28
		uint64_t lastThis;           // 0x30
		uint64_t lastLocalAnim;      // 0x38
		int32_t lastStateHash;       // 0x40
		int32_t lastLayerIdx;        // 0x44
		float lastOriginalSpeed;     // 0x48
		float lastAppliedSpeed;      // 0x4C
	};
	static_assert(sizeof(CrossFadeSpeedControl) == 0x50, "CrossFadeSpeedControl size");

	static uintptr_t g_cf_speed_address = 0;      // AnimPlayable.CrossFadeInFixedTime VA
	static uintptr_t g_cf_speed_shellcode = 0;    // 独立动画加速 shellcode
	static uintptr_t g_cf_speed_control = 0;      // CrossFadeSpeedControl
	static BYTE g_cf_speed_orig_bytes[32] = { 0 };
	static size_t g_cf_speed_stolen_size = 0;
	static bool g_cf_speed_initialized = false;
	static bool g_cf_speed_enabled = false;

	struct CodecaveAllocation
	{
		uintptr_t address = 0;
		size_t size = 0;
	};
	struct CodecaveCandidate
	{
		uintptr_t address = 0;
		size_t capacity = 0;
	};
	struct ReadOnlyCodecaveScan
	{
		std::string moduleName;
		size_t scanSize = 0;
	};
	static std::vector<CodecaveAllocation> g_codecave_allocations;
	static std::vector<uintptr_t> g_bad_codecaves;
	static std::vector<CodecaveCandidate> g_cached_codecaves;
	static std::vector<size_t> g_codecave_scan_sizes;
	static std::vector<ReadOnlyCodecaveScan> g_readonly_codecave_scans;
	static uintptr_t g_fast_codecave_next = 0;
	static constexpr uintptr_t kFastCodecaveOffset = Offset::MemoryShockCodecaveHintRva + 0x800;
	static constexpr size_t kFastCodecaveReserveSize = 0x3000;
	static constexpr size_t kFastCodecaveSlotAlign = 0x200;

	static void PrintHookBytes(const char* label, const BYTE* bytes, size_t size)
	{
		printf("%s", label);
		size_t limit = size < 32 ? size : 32;
		for (size_t i = 0; i < limit; ++i)
		{
			printf(" %02X", bytes[i]);
		}
		if (size > limit)
		{
			printf(" ...");
		}
		printf("\n");
	}

	static bool ReadBackEquals(uintptr_t address, const BYTE* expected, size_t size, const char* label)
	{
		std::vector<BYTE> readback(size, 0);
		if (!mem.Read(address, readback.data(), readback.size()))
		{
			MemoryShockDebugPrintf("[AnimationAccel] %s readback failed @ 0x%llX\n", label, (unsigned long long)address);
			return false;
		}
		if (std::memcmp(readback.data(), expected, size) != 0)
		{
			MemoryShockDebugPrintf("[AnimationAccel] %s readback mismatch @ 0x%llX\n", label, (unsigned long long)address);
			if constexpr (kMemoryShockVerboseLog)
			{
				PrintHookBytes("[AnimationAccel] expected:", expected, size);
				PrintHookBytes("[AnimationAccel] actual:  ", readback.data(), size);
			}
			return false;
		}
		return true;
	}

	static bool IsAllZeroBytes(const BYTE* bytes, size_t size)
	{
		if (!bytes || size == 0)
		{
			return false;
		}
		for (size_t i = 0; i < size; ++i)
		{
			if (bytes[i] != 0)
			{
				return false;
			}
		}
		return true;
	}

	static bool ReadMemoryEquals(uintptr_t address, const BYTE* expected, size_t size)
	{
		if (!address || !expected || size == 0)
		{
			return false;
		}
		std::vector<BYTE> bytes(size, 0);
		if (!mem.Read(address, bytes.data(), bytes.size()))
		{
			return false;
		}
		return std::memcmp(bytes.data(), expected, size) == 0;
	}

	static bool IsValidMemoryShockMetadata(const MemoryShockLegacyMetadata& metadata, uint64_t metadataMagic,
		uintptr_t shellEntry, uintptr_t expectedVtable)
	{
		return metadata.magic == metadataMagic &&
			metadata.originalRet != 0 &&
			metadata.originalRet != shellEntry &&
			metadata.shellEntry == shellEntry &&
			(!expectedVtable || metadata.vtableAddress == expectedVtable);
	}

	static bool TryReadMemoryShockMetadataAt(uintptr_t metadataAddress, size_t shellcodeSize, uint64_t metadataMagic,
		uintptr_t shellEntry, uintptr_t expectedVtable, MemoryShockLegacyMetadata& metadata)
	{
		MemoryShockLegacyMetadata candidate{};
		if (!metadataAddress || !mem.Read(metadataAddress, &candidate, sizeof(candidate)))
		{
			return false;
		}
		if (!IsValidMemoryShockMetadata(candidate, metadataMagic, shellEntry, expectedVtable))
		{
			return false;
		}
		if (metadataAddress != shellEntry + shellcodeSize)
		{
			return false;
		}
		metadata = candidate;
		return true;
	}

	static bool TryReadMemoryShockMetadata(uintptr_t shellEntry, size_t shellcodeSize, uint64_t metadataMagic,
		uintptr_t expectedVtable, MemoryShockLegacyMetadata& metadata)
	{
		if (!shellEntry || shellcodeSize == 0)
		{
			return false;
		}

		return TryReadMemoryShockMetadataAt(
			shellEntry + shellcodeSize,
			shellcodeSize,
			metadataMagic,
			shellEntry,
			expectedVtable,
			metadata);
	}

	static bool TryReadAnyMemoryShockMetadata(uintptr_t shellEntry, uintptr_t expectedVtable,
		MemoryShockLegacyMetadata& metadata)
	{
		if (TryReadMemoryShockMetadata(
			shellEntry,
			kMemoryShockLegacyShellcodeSize,
			kMemoryShockLegacyMetadataMagic,
			expectedVtable,
			metadata))
		{
			return true;
		}
		if (TryReadMemoryShockMetadata(
			shellEntry,
			kMemoryShockCrossFadeShellcodeSize,
			kMemoryShockCrossFadeMetadataMagic,
			expectedVtable,
			metadata))
		{
			return true;
		}
		if (TryReadMemoryShockMetadata(
			shellEntry,
			kMemoryShockCrossFadeSwitchShellcodeSize,
			kMemoryShockCrossFadeMetadataMagic,
			expectedVtable,
			metadata))
		{
			return true;
		}
		return false;
	}

	static bool RecoverMemoryShockOriginalRetFromMetadata(const char* label, uintptr_t currentVtableTarget,
		uintptr_t vtableAddress, uintptr_t& originalRet)
	{
		MemoryShockLegacyMetadata staleMetadata{};
		if (!TryReadAnyMemoryShockMetadata(currentVtableTarget, vtableAddress, staleMetadata))
		{
			return true;
		}

		originalRet = static_cast<uintptr_t>(staleMetadata.originalRet);
		if (!mem.Write(vtableAddress, &originalRet, sizeof(originalRet)) ||
			mem.Read<uintptr_t>(vtableAddress) != originalRet)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] %s stale hook restore failed: shell=0x%llX original=0x%llX\n",
				label,
				(unsigned long long)currentVtableTarget,
				(unsigned long long)originalRet);
			return false;
		}
		MemoryShockDebugPrintf("[MemoryShock] %s stale hook recovered: shell=0x%llX original=0x%llX\n",
			label,
			(unsigned long long)currentVtableTarget,
			(unsigned long long)originalRet);
		return true;
	}

	static bool WriteMemoryShockShellcodeBlock(const char* label, uintptr_t baseAddress, const BYTE* shellcode, size_t shellcodeSize,
		const MemoryShockLegacyMetadata* metadata)
	{
		if (!baseAddress || !shellcode || shellcodeSize == 0)
		{
			MemoryShockDebugPrintf("[MemoryShock] %s invalid shellcode block: base=0x%llX size=%zu\n",
				label,
				(unsigned long long)baseAddress,
				shellcodeSize);
			return false;
		}

		const size_t metadataSize = metadata ? sizeof(*metadata) : 0;
		std::vector<BYTE> block(kMemoryShockControlBytes + shellcodeSize + metadataSize, 0);
		std::memcpy(block.data() + kMemoryShockControlBytes, shellcode, shellcodeSize);
		if (metadata)
		{
			std::memcpy(block.data() + kMemoryShockControlBytes + shellcodeSize, metadata, sizeof(*metadata));
		}

		for (int attempt = 1; attempt <= 3; ++attempt)
		{
			bool writeOk = mem.Write(baseAddress, block.data(), block.size());
			std::vector<BYTE> readback(block.size(), 0);
			bool readRawOk = mem.Read(baseAddress, readback.data(), readback.size());
			bool readOk = readRawOk && std::memcmp(readback.data(), block.data(), block.size()) == 0;
			if (!readRawOk)
			{
				MemoryShockDebugPrintf("[AnimationAccel] %s readback failed @ 0x%llX\n", label, (unsigned long long)baseAddress);
			}
			else if (!readOk)
			{
				MemoryShockDebugPrintf("[AnimationAccel] %s readback mismatch @ 0x%llX\n", label, (unsigned long long)baseAddress);
				if constexpr (kMemoryShockVerboseLog)
				{
					PrintHookBytes("[AnimationAccel] expected:", block.data(), block.size());
					PrintHookBytes("[AnimationAccel] actual:  ", readback.data(), readback.size());
				}
			}
			MemoryShockDebugPrintf("[MemoryShock] %s shellcode write attempt=%d base=0x%llX inst=0x%llX size=%zu write=%d readback=%d\n",
				label,
				attempt,
				(unsigned long long)baseAddress,
				(unsigned long long)(baseAddress + kMemoryShockControlBytes),
				block.size(),
				writeOk ? 1 : 0,
				readOk ? 1 : 0);
			if (writeOk && readOk)
			{
				return true;
			}
			if (writeOk && readRawOk && IsAllZeroBytes(readback.data(), readback.size()))
			{
				MemoryShockDebugPrintf("[MemoryShock] %s shellcode readback stayed zero, skip retries base=0x%llX\n",
					label,
					(unsigned long long)baseAddress);
				return false;
			}

			mem.RefreshConfigSetALL();
			Sleep(50);
		}

		return false;
	}

	static bool PatchCrossFadeBerserkModeBytes(bool enable)
	{
		if (!instruction_addr)
		{
			return false;
		}

		const BYTE* patchBytes = enable ? kCrossFadeControlNopBytes : kCrossFadeControlGateBytes;
		BYTE patchCopy[sizeof(kCrossFadeControlNopBytes)] = { 0 };
		std::memcpy(patchCopy, patchBytes, sizeof(patchCopy));
		const uintptr_t patchAddress = instruction_addr + kMemoryShockCrossFadeControlPatchOffset;
		const bool writeOk = mem.Write(patchAddress, patchCopy, sizeof(patchCopy));
		if constexpr (kMemoryShockVerboseLog)
		{
			BYTE readback[sizeof(kCrossFadeControlNopBytes)] = { 0 };
			const bool readOk = mem.Read(patchAddress, readback, sizeof(readback));
			const bool match = readOk && std::memcmp(readback, patchCopy, sizeof(readback)) == 0;
			MemoryShockDebugPrintf("[MemoryShock] CrossFade berserk patch enable=%d addr=0x%llX write=%d readback=%d match=%d\n",
				enable ? 1 : 0,
				(unsigned long long)patchAddress,
				writeOk ? 1 : 0,
				readOk ? 1 : 0,
				match ? 1 : 0);
		}
		return writeOk;
	}

	static bool IsPatchedToCodecave(const BYTE* bytes)
	{
		if (bytes[0] != 0xFF || bytes[1] != 0x25 ||
			bytes[2] != 0x00 || bytes[3] != 0x00 ||
			bytes[4] != 0x00 || bytes[5] != 0x00)
		{
			return false;
		}
		uintptr_t target = 0;
		std::memcpy(&target, bytes + 6, sizeof(target));
		return target == flush_codecave;
	}

	static bool IsCodecaveRangeUsed(uintptr_t address, size_t size)
	{
		uintptr_t end = address + size;
		for (const auto& allocation : g_codecave_allocations)
		{
			uintptr_t allocationEnd = allocation.address + allocation.size;
			if (address < allocationEnd && end > allocation.address)
			{
				return true;
			}
		}
		return false;
	}

	static bool IsBadCodecave(uintptr_t address)
	{
		return std::find(g_bad_codecaves.begin(), g_bad_codecaves.end(), address) != g_bad_codecaves.end();
	}

	static void MarkBadCodecave(uintptr_t address)
	{
		if (address && !IsBadCodecave(address))
		{
			g_bad_codecaves.push_back(address);
		}
	}

	static void ReleaseCodecaveReservation(uintptr_t address)
	{
		if (!address)
		{
			return;
		}
		g_codecave_allocations.erase(
			std::remove_if(
				g_codecave_allocations.begin(),
				g_codecave_allocations.end(),
				[address](const CodecaveAllocation& allocation)
				{
					return allocation.address == address;
				}),
			g_codecave_allocations.end());
	}

	static size_t AlignUpSize(size_t value, size_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	static uintptr_t AlignUpPtr(uintptr_t value, size_t alignment)
	{
		return (value + alignment - 1) & ~(static_cast<uintptr_t>(alignment) - 1);
	}

	static bool IsReadableZeroRange(uintptr_t address, size_t size)
	{
		std::vector<BYTE> buffer(size, 0);
		if (!mem.Read(address, buffer.data(), buffer.size()))
		{
			return false;
		}
		for (BYTE value : buffer)
		{
			if (value != 0)
			{
				return false;
			}
		}
		return true;
	}

	static void AddCachedCodecave(uintptr_t address, size_t capacity, bool prefer = false)
	{
		if (!address)
		{
			return;
		}
		for (auto& candidate : g_cached_codecaves)
		{
			if (candidate.address == address)
			{
				if (candidate.capacity < capacity)
				{
					candidate.capacity = capacity;
				}
				return;
			}
		}
		CodecaveCandidate candidate{ address, capacity };
		if (prefer)
		{
			g_cached_codecaves.insert(g_cached_codecaves.begin(), candidate);
		}
		else
		{
			g_cached_codecaves.push_back(candidate);
		}
	}

	static bool HasScannedCodecaveSize(size_t scanSize)
	{
		return std::find(g_codecave_scan_sizes.begin(), g_codecave_scan_sizes.end(), scanSize) !=
			g_codecave_scan_sizes.end();
	}

	static bool HasScannedReadOnlyCodecaves(size_t scanSize, const char* moduleName)
	{
		if (!moduleName || !*moduleName)
		{
			return true;
		}
		for (const auto& scan : g_readonly_codecave_scans)
		{
			if (scan.scanSize == scanSize && scan.moduleName == moduleName)
			{
				return true;
			}
		}
		return false;
	}

	static void MarkReadOnlyCodecavesScanned(size_t scanSize, const char* moduleName)
	{
		if (!moduleName || !*moduleName || HasScannedReadOnlyCodecaves(scanSize, moduleName))
		{
			return;
		}
		g_readonly_codecave_scans.push_back({ moduleName, scanSize });
	}

	static size_t NormalizeCodecaveScanSize(size_t dataSize)
	{
		return AlignUpSize(dataSize < 0x40 ? 0x40 : dataSize, 0x40);
	}

	static bool TryWriteCodecave(uintptr_t candidate, const BYTE* data, size_t dataSize, const char* label)
	{
		if (!candidate)
		{
			return false;
		}
		if (IsCodecaveRangeUsed(candidate, dataSize))
		{
			return false;
		}
		if (IsBadCodecave(candidate))
		{
			return false;
		}
		if (mem.Write(candidate, const_cast<BYTE*>(data), dataSize) &&
			ReadBackEquals(candidate, data, dataSize, label))
		{
			g_codecave_allocations.push_back({ candidate, dataSize });
			return true;
		}
		MarkBadCodecave(candidate);
		MemoryShockDebugPrintf("[AnimationAccel] codecave 不可用 @ 0x%llX\n", (unsigned long long)candidate);
		return false;
	}

	static bool TryReserveReadOnlyCodecave(uintptr_t candidate, size_t dataSize, const char* label)
	{
		if (!candidate || dataSize == 0)
		{
			return false;
		}
		if (IsCodecaveRangeUsed(candidate, dataSize) || IsBadCodecave(candidate))
		{
			return false;
		}
		if (!IsReadableZeroRange(candidate, dataSize))
		{
			MarkBadCodecave(candidate);
			MemoryShockDebugPrintf("[MemoryShock] %s read-only zero check failed @ 0x%llX\n",
				label,
				(unsigned long long)candidate);
			return false;
		}

		g_codecave_allocations.push_back({ candidate, dataSize });
		MemoryShockDebugPrintf("[MemoryShock] %s reserved read-only codecave @ 0x%llX size=%zu\n",
			label,
			(unsigned long long)candidate,
			dataSize);
		return true;
	}

	static uintptr_t TryFastReadOnlyCodecave(size_t dataSize, const char* label)
	{
		if (!Offset::NarakaBladepoint || dataSize == 0 || dataSize > kFastCodecaveSlotAlign)
		{
			return 0;
		}

		uintptr_t base = Offset::NarakaBladepoint + kFastCodecaveOffset;
		uintptr_t end = base + kFastCodecaveReserveSize;
		if (g_fast_codecave_next < base || g_fast_codecave_next >= end)
		{
			g_fast_codecave_next = base;
		}

		size_t slotSpan = AlignUpSize(dataSize, kFastCodecaveSlotAlign);
		for (uintptr_t candidate = g_fast_codecave_next;
			candidate + dataSize <= end;
			candidate += slotSpan)
		{
			if (TryReserveReadOnlyCodecave(candidate, dataSize, label))
			{
				g_fast_codecave_next = candidate + slotSpan;
				return candidate;
			}
			g_fast_codecave_next = candidate + slotSpan;
		}

		return 0;
	}

	static uintptr_t TryCachedReadOnlyCodecaves(size_t dataSize, const char* label)
	{
		for (const auto& candidate : g_cached_codecaves)
		{
			if (candidate.capacity < dataSize)
			{
				continue;
			}
			if (TryReserveReadOnlyCodecave(candidate.address, dataSize, label))
			{
				return candidate.address;
			}
		}
		return 0;
	}

	static void ScanModuleSectionSlackCodecaves(size_t scanSize, const char* moduleName, bool preferCandidates = false)
	{
		if (!moduleName || !*moduleName)
		{
			return;
		}
		if (HasScannedReadOnlyCodecaves(scanSize, moduleName))
		{
			return;
		}
		MarkReadOnlyCodecavesScanned(scanSize, moduleName);

		int pid = mem.GetCurrentProcessPID();
		uint64_t moduleBase = VMMDLL_ProcessGetModuleBaseU(mem.vHandle, pid, const_cast<LPSTR>(moduleName));
		if (!moduleBase)
		{
			MemoryShockDebugPrintf("[MemoryShock] module base failed for %s\n", moduleName);
			return;
		}

		DWORD sectionCount = 0;
		if (!VMMDLL_ProcessGetSectionsU(mem.vHandle, pid, const_cast<LPSTR>(moduleName), nullptr, 0, &sectionCount) ||
			!sectionCount)
		{
			MemoryShockDebugPrintf("[MemoryShock] section count failed for %s\n", moduleName);
			return;
		}

		PIMAGE_SECTION_HEADER sections = static_cast<PIMAGE_SECTION_HEADER>(
			LocalAlloc(LMEM_ZEROINIT, sectionCount * sizeof(IMAGE_SECTION_HEADER)));
		if (!sections)
		{
			return;
		}

		if (!VMMDLL_ProcessGetSectionsU(mem.vHandle, pid, const_cast<LPSTR>(moduleName), sections, sectionCount, &sectionCount) ||
			!sectionCount)
		{
			LocalFree(sections);
			MemoryShockDebugPrintf("[MemoryShock] sections failed for %s\n", moduleName);
			return;
		}

		size_t foundCount = 0;
		for (DWORD i = 0; i < sectionCount; ++i)
		{
			const DWORD need = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE;
			if ((sections[i].Characteristics & need) != need)
			{
				continue;
			}

			uintptr_t sectionEndRva =
				static_cast<uintptr_t>(sections[i].VirtualAddress) +
				static_cast<uintptr_t>(sections[i].Misc.VirtualSize);
			uintptr_t alignedEndRva = AlignUpPtr(sectionEndRva, 0x1000);
			if (alignedEndRva <= sectionEndRva)
			{
				continue;
			}

			size_t slackSize = static_cast<size_t>(alignedEndRva - sectionEndRva);
			if (slackSize < scanSize)
			{
				continue;
			}

			uintptr_t cave = static_cast<uintptr_t>(moduleBase) + sectionEndRva;
			if (IsReadableZeroRange(cave, scanSize))
			{
				AddCachedCodecave(cave, slackSize, preferCandidates);
				++foundCount;
			}
		}

		LocalFree(sections);
		MemoryShockDebugPrintf("[MemoryShock] read-only section slack scan %s size=%zu caves=%zu\n",
			moduleName,
			scanSize,
			foundCount);
	}

	static uintptr_t TryFastCodecave(const BYTE* data, size_t dataSize, const char* label)
	{
		if (!Offset::NarakaBladepoint || dataSize == 0 || dataSize > kFastCodecaveSlotAlign)
		{
			return 0;
		}

		uintptr_t base = Offset::NarakaBladepoint + kFastCodecaveOffset;
		uintptr_t end = base + kFastCodecaveReserveSize;
		if (g_fast_codecave_next < base || g_fast_codecave_next >= end)
		{
			g_fast_codecave_next = base;
		}

		size_t slotSpan = AlignUpSize(dataSize, kFastCodecaveSlotAlign);
		for (uintptr_t candidate = g_fast_codecave_next;
			candidate + dataSize <= end;
			candidate += slotSpan)
		{
			if (IsCodecaveRangeUsed(candidate, dataSize))
			{
				continue;
			}
			if (IsBadCodecave(candidate))
			{
				continue;
			}
			if (!IsReadableZeroRange(candidate, dataSize))
			{
				continue;
			}
			if (TryWriteCodecave(candidate, data, dataSize, label))
			{
				g_fast_codecave_next = candidate + slotSpan;
				return candidate;
			}
			g_fast_codecave_next = candidate + slotSpan;
		}

		return 0;
	}

	static uintptr_t TryCachedCodecaves(const BYTE* data, size_t dataSize, const char* label)
	{
		for (const auto& candidate : g_cached_codecaves)
		{
			if (candidate.capacity < dataSize)
			{
				continue;
			}
			if (TryWriteCodecave(candidate.address, data, dataSize, label))
			{
				return candidate.address;
			}
		}
		return 0;
	}

	static void ScanCodecavesOnce(size_t scanSize)
	{
		if (HasScannedCodecaveSize(scanSize))
		{
			return;
		}

		ULONGLONG startTick = GetTickCount64();
		auto discovered = mem.GetShellcode().find_all_codecave(scanSize, "NarakaBladepoint.exe");
		for (uint64_t cave : discovered)
		{
			AddCachedCodecave(static_cast<uintptr_t>(cave), scanSize);
		}
		g_codecave_scan_sizes.push_back(scanSize);
		MemoryShockDebugPrintf("[AnimationAccel] codecave scan size=%zu caves=%zu cost=%llums\n",
			scanSize,
			discovered.size(),
			(unsigned long long)(GetTickCount64() - startTick));
	}

	static uintptr_t FindWritableCodecave(const BYTE* data, size_t dataSize, const char* label)
	{
		uintptr_t fast = TryFastCodecave(data, dataSize, label);
		if (fast)
		{
			return fast;
		}

		uintptr_t cached = TryCachedCodecaves(data, dataSize, label);
		if (cached)
		{
			return cached;
		}

		size_t scanSize = NormalizeCodecaveScanSize(dataSize);
		ScanCodecavesOnce(scanSize);
		return TryCachedCodecaves(data, dataSize, label);
	}

	static uintptr_t FindReadOnlyCodecave(size_t dataSize, const char* label)
	{
		uintptr_t cached = TryCachedReadOnlyCodecaves(dataSize, label);
		if (cached)
		{
			return cached;
		}

		size_t scanSize = NormalizeCodecaveScanSize(dataSize);
		ScanModuleSectionSlackCodecaves(scanSize, GetCurrentGameAssemblyModuleName(), true);
		cached = TryCachedReadOnlyCodecaves(dataSize, label);
		if (cached)
		{
			return cached;
		}

		ScanModuleSectionSlackCodecaves(scanSize, "NarakaBladepoint.exe");
		cached = TryCachedReadOnlyCodecaves(dataSize, label);
		if (cached)
		{
			return cached;
		}

		return TryFastReadOnlyCodecave(dataSize, label);
	}

	static uintptr_t TryAllocatedMemoryShockCodecave(size_t dataSize, const char* label)
	{
		if (dataSize == 0)
		{
			return 0;
		}
		uintptr_t allocated = DMA_AllocateMemory(dataSize, mem.GetCurrentProcessPID());
		if (!allocated)
		{
			MemoryShockDebugPrintf("[MemoryShock] %s DMA_AllocateMemory fallback failed size=%zu\n",
				label,
				dataSize);
			return 0;
		}
		if (IsBadCodecave(allocated))
		{
			MemoryShockDebugPrintf("[MemoryShock] %s DMA_AllocateMemory fallback returned bad codecave @ 0x%llX\n",
				label,
				(unsigned long long)allocated);
			return 0;
		}
		g_codecave_allocations.push_back({ allocated, dataSize });
		MemoryShockDebugPrintf("[MemoryShock] %s reserved allocated codecave @ 0x%llX size=%zu\n",
			label,
			(unsigned long long)allocated,
			dataSize);
		return allocated;
	}

	static uintptr_t FindMemoryShockCodecave(size_t dataSize, const char* label)
	{
		uintptr_t address = FindReadOnlyCodecave(dataSize, label);
		if (address)
		{
			return address;
		}
		return TryAllocatedMemoryShockCodecave(dataSize, label);
	}

	static void PatchShellcodeImm64(BYTE* shellcode, size_t shellcodeSize, uint64_t marker, uintptr_t value)
	{
		for (size_t i = 0; i + sizeof(uint64_t) <= shellcodeSize; ++i)
		{
			uint64_t current = 0;
			std::memcpy(&current, shellcode + i, sizeof(current));
			if (current == marker)
			{
				std::memcpy(shellcode + i, &value, sizeof(value));
				return;
			}
		}
		printf("[AnimationAccel] shellcode marker not found: 0x%llX\n", (unsigned long long)marker);
	}

	static void PrintAnimationAccelCounters(const char* label)
	{
		if (!g_flush_debug_data)
		{
			return;
		}

		uint64_t debugData[4] = { 0 };
		if (!mem.Read(g_flush_debug_data, debugData, sizeof(debugData)))
		{
			printf("[AnimationAccel] %s DebugData read failed @ 0x%llX\n", label, (unsigned long long)g_flush_debug_data);
			return;
		}

		printf("[AnimationAccel] %s counters: magic=0x%llX entry=%llu call=%llu debug=0x%llX\n",
			label,
			(unsigned long long)debugData[0],
			(unsigned long long)debugData[1],
			(unsigned long long)debugData[2],
			(unsigned long long)g_flush_debug_data);
	}

	static uintptr_t g_game_assembly_size = 0;
	static uintptr_t g_actor_kit_cross_fade_in_fixed_time = 0;

	static bool IsCanonicalUserPtr(uintptr_t address)
	{
		return address >= 0x10000ull && address < 0x0000800000000000ull;
	}

	static uintptr_t GetGameAssemblySizeCached()
	{
		if (g_game_assembly_size)
		{
			return g_game_assembly_size;
		}

		const char* names[] = {
			"GameAssembly.dll",
			"GameAssembly_Super.dll",
			"GameAssembly_Super_IBT.dll"
		};
		for (const char* name : names)
		{
			size_t size = mem.GetBaseSize(name);
			if (size)
			{
				g_game_assembly_size = static_cast<uintptr_t>(size);
				return g_game_assembly_size;
			}
		}

		g_game_assembly_size = 0x20000000ull;
		return g_game_assembly_size;
	}

	static bool IsGameAssemblyCodePtr(uintptr_t address)
	{
		const uintptr_t size = GetGameAssemblySizeCached();
		return Offset::GameAssembly &&
			address >= Offset::GameAssembly &&
			address < Offset::GameAssembly + size;
	}

	static uint64_t GetAnimPlayableCrossFadeInFixedTimeFallbackRva()
	{
		// Fallback values are maintained in Offset.h; runtime MethodInfo resolution remains preferred.
		if (Offset::IsGameAssemblySuper() || Offset::IsGameAssemblySuperIBT())
		{
			return Offset::AnimPlayableCrossFadeInFixedTimeSuper;
		}
		return Offset::AnimPlayableCrossFadeInFixedTimeNormal;
	}

	static bool ReadCStringSafe(uintptr_t address, std::string& out, size_t maxLen = 192)
	{
		out.clear();
		if (!IsCanonicalUserPtr(address) || maxLen == 0)
		{
			return false;
		}

		for (size_t i = 0; i < maxLen; ++i)
		{
			char ch = '\0';
			if (!mem.Read(address + i, &ch, sizeof(ch)))
			{
				return false;
			}
			if (ch == '\0')
			{
				return true;
			}
			if (static_cast<unsigned char>(ch) < 0x20 ||
				static_cast<unsigned char>(ch) > 0x7E)
			{
				return false;
			}
			out.push_back(ch);
		}
		return false;
	}

	static bool ReadKlassName(uintptr_t klass, std::string& className)
	{
		className.clear();
		if (!IsCanonicalUserPtr(klass))
		{
			return false;
		}

		uintptr_t namePtr = mem.Read<uintptr_t>(klass + Offset::Il2CppClassNameOffset);
		return ReadCStringSafe(namePtr, className) && !className.empty();
	}

	static bool ReadMethodName(uintptr_t methodInfo, std::string& methodName)
	{
		methodName.clear();
		if (!IsCanonicalUserPtr(methodInfo))
		{
			return false;
		}

		uintptr_t namePtr = mem.Read<uintptr_t>(methodInfo + Offset::Il2CppMethodNameOffset);
		return ReadCStringSafe(namePtr, methodName) && !methodName.empty();
	}

	static uintptr_t ReadObjectKlass(uintptr_t object)
	{
		if (!IsCanonicalUserPtr(object))
		{
			return 0;
		}

		uintptr_t klass = mem.Read<uintptr_t>(object);
		if (!IsCanonicalUserPtr(klass))
		{
			return 0;
		}
		return klass;
	}

	static uintptr_t GetLocalActorModelForResolve()
	{
		uintptr_t actorModel = reinterpret_cast<uintptr_t>(Global::Cache_LocalPlayer.L_ActorModel);
		if (IsCanonicalUserPtr(actorModel))
		{
			return actorModel;
		}

		if (Global::WorldPtr.Character && Global::WorldPtr.Character->CharacterManagerPtr)
		{
			actorModel = mem.Read<uintptr_t>(Global::WorldPtr.Character->CharacterManagerPtr +
				Offset::MemoryShockCharacterManagerMainPlayerOffset);
			if (IsCanonicalUserPtr(actorModel))
			{
				return actorModel;
			}
		}

		uintptr_t managerClass = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_CharacterManager);
		uintptr_t staticFields = managerClass ?
			mem.Read<uintptr_t>(managerClass + Offset::Il2CppClassStaticFieldsOffset) : 0;
		uintptr_t manager = staticFields ?
			mem.Read<uintptr_t>(staticFields + Offset::Il2CppStaticInstanceOffset) : 0;
		actorModel = manager ?
			mem.Read<uintptr_t>(manager + Offset::MemoryShockCharacterManagerMainPlayerOffset) : 0;
		if (IsCanonicalUserPtr(actorModel))
		{
			return actorModel;
		}

		if (Offset::m_TransparentTagHandler)
		{
			uintptr_t handlerClass = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_TransparentTagHandler);
			uintptr_t handlerStatic = handlerClass ?
				mem.Read<uintptr_t>(handlerClass + Offset::Il2CppClassStaticFieldsOffset) : 0;
			uintptr_t handler = handlerStatic ?
				mem.Read<uintptr_t>(handlerStatic + Offset::Il2CppStaticInstanceOffset) : 0;
			manager = handler ? mem.Read<uintptr_t>(handler +
				Offset::MemoryShockTransparentTagHandlerCharacterManagerOffset) : 0;
			actorModel = manager ? mem.Read<uintptr_t>(manager +
				Offset::MemoryShockCharacterManagerMainPlayerOffset) : 0;
			if (IsCanonicalUserPtr(actorModel))
			{
				return actorModel;
			}
		}

		return 0;
	}

	static uintptr_t GetLocalActorKitForResolve()
	{
		uintptr_t actorKit = reinterpret_cast<uintptr_t>(Global::Cache_LocalPlayer.L_ActorKit);
		if (IsCanonicalUserPtr(actorKit))
		{
			return actorKit;
		}

		uintptr_t actorModel = GetLocalActorModelForResolve();
		if (!actorModel)
		{
			return 0;
		}

		actorKit = mem.Read<uintptr_t>(actorModel + Offset::ActorModel.ActorKit);
		return IsCanonicalUserPtr(actorKit) ? actorKit : 0;
	}

	static uintptr_t GetLocalAnimPlayableForResolve()
	{
		uintptr_t actorKit = GetLocalActorKitForResolve();
		if (!actorKit)
		{
			return 0;
		}

		uintptr_t animPlayable =
			mem.Read<uintptr_t>(actorKit + Offset::ActorModel.ActorKitS.AnimPlayableAgent);
		return IsCanonicalUserPtr(animPlayable) ? animPlayable : 0;
	}

	static uintptr_t FindMethodPointerByName(uintptr_t klass, const char* classLabel,
		const char* methodName, int expectedParamCount = -1)
	{
		if (!IsCanonicalUserPtr(klass))
		{
			return 0;
		}

		uint16_t methodCount = mem.Read<uint16_t>(klass + Offset::Il2CppClassMethodCountOffset);
		uintptr_t methods = mem.Read<uintptr_t>(klass + Offset::Il2CppClassMethodsOffset);
		if (!methods || methodCount == 0 || methodCount > 4096)
		{
			printf("[AnimationProbe] MethodTable invalid %s klass=0x%llX methods=0x%llX count=%u\n",
				classLabel,
				(unsigned long long)klass,
				(unsigned long long)methods,
				methodCount);
			return 0;
		}

		uintptr_t fallbackMethodPointer = 0;
		uintptr_t fallbackMethodInfo = 0;
		uint8_t fallbackParamCount = 0;
		for (uint16_t i = 0; i < methodCount; ++i)
		{
			uintptr_t methodInfo = mem.Read<uintptr_t>(methods + i * sizeof(uintptr_t));
			if (!IsCanonicalUserPtr(methodInfo))
			{
				continue;
			}

			std::string currentName;
			if (!ReadMethodName(methodInfo, currentName) || currentName != methodName)
			{
				continue;
			}

			uint8_t paramCount = mem.Read<uint8_t>(methodInfo + Offset::Il2CppMethodParameterCountOffset);
			uintptr_t methodPointer = mem.Read<uintptr_t>(methodInfo + Offset::Il2CppMethodPointerOffset);
			if (!IsGameAssemblyCodePtr(methodPointer))
			{
				printf("[AnimationProbe] Method %s.%s code ptr invalid: MethodInfo=0x%llX ptr=0x%llX params=%u\n",
					classLabel,
					methodName,
					(unsigned long long)methodInfo,
					(unsigned long long)methodPointer,
					paramCount);
				continue;
			}

			if (expectedParamCount >= 0 &&
				paramCount != static_cast<uint8_t>(expectedParamCount) &&
				!fallbackMethodPointer)
			{
				fallbackMethodPointer = methodPointer;
				fallbackMethodInfo = methodInfo;
				fallbackParamCount = paramCount;
				continue;
			}

			printf("[AnimationProbe] Method %s.%s => 0x%llX RVA=0x%llX MethodInfo=0x%llX params=%u slot=%u\n",
				classLabel,
				methodName,
				(unsigned long long)methodPointer,
				(unsigned long long)(methodPointer - Offset::GameAssembly),
				(unsigned long long)methodInfo,
				paramCount,
				mem.Read<uint16_t>(methodInfo + Offset::Il2CppMethodSlotOffset));
			return methodPointer;
		}

		if (fallbackMethodPointer)
		{
			printf("[AnimationProbe] Method %s.%s => 0x%llX RVA=0x%llX MethodInfo=0x%llX params=%u expected=%d (param fallback)\n",
				classLabel,
				methodName,
				(unsigned long long)fallbackMethodPointer,
				(unsigned long long)(fallbackMethodPointer - Offset::GameAssembly),
				(unsigned long long)fallbackMethodInfo,
				fallbackParamCount,
				expectedParamCount);
			return fallbackMethodPointer;
		}

		printf("[AnimationProbe] Method not found %s.%s klass=0x%llX count=%u\n",
			classLabel, methodName, (unsigned long long)klass, methodCount);
		return 0;
	}

	static uintptr_t FindMethodInParentChain(uintptr_t klass, const char* rootLabel,
		const char* className, const char* methodName, int expectedParamCount = -1)
	{
		uintptr_t current = klass;
		for (int depth = 0; depth < 16 && IsCanonicalUserPtr(current); ++depth)
		{
			std::string currentName;
			ReadKlassName(current, currentName);
			if (currentName == className)
			{
				return FindMethodPointerByName(current, className, methodName, expectedParamCount);
			}

			current = mem.Read<uintptr_t>(current + Offset::Il2CppClassParentOffset);
		}

		printf("[AnimationProbe] Class %s not found in parent chain of %s klass=0x%llX\n",
			className, rootLabel, (unsigned long long)klass);
		return 0;
	}

	static uintptr_t ResolveActorKitMethod(const char* methodName, int expectedParamCount)
	{
		uintptr_t actorKit = GetLocalActorKitForResolve();
		uintptr_t klass = ReadObjectKlass(actorKit);
		if (!klass)
		{
			printf("[AnimationProbe] Resolve ActorKit.%s failed: ActorKit object/class unreadable object=0x%llX\n",
				methodName, (unsigned long long)actorKit);
			return 0;
		}

		std::string className;
		ReadKlassName(klass, className);
		printf("[AnimationProbe] ActorKit resolve object=0x%llX klass=0x%llX name=%s\n",
			(unsigned long long)actorKit,
			(unsigned long long)klass,
			className.empty() ? "?" : className.c_str());

		if (className == "ActorKit")
		{
			return FindMethodPointerByName(klass, "ActorKit", methodName, expectedParamCount);
		}

		return FindMethodInParentChain(klass, "ActorKitObject", "ActorKit", methodName, expectedParamCount);
	}

	static uintptr_t ResolveAnimPlayableMethod(const char* methodName, int expectedParamCount)
	{
		uintptr_t animPlayable = GetLocalAnimPlayableForResolve();
		uintptr_t klass = ReadObjectKlass(animPlayable);
		if (!klass)
		{
			printf("[AnimationProbe] Resolve AnimPlayable.%s failed: AnimPlayable object/class unreadable object=0x%llX\n",
				methodName, (unsigned long long)animPlayable);
			return 0;
		}

		std::string className;
		ReadKlassName(klass, className);
		printf("[AnimationProbe] AnimPlayable resolve object=0x%llX klass=0x%llX name=%s\n",
			(unsigned long long)animPlayable,
			(unsigned long long)klass,
			className.empty() ? "?" : className.c_str());

		if (className == "AnimPlayable")
		{
			return FindMethodPointerByName(klass, "AnimPlayable", methodName, expectedParamCount);
		}

		return FindMethodInParentChain(klass, "AnimPlayableObject", "AnimPlayable", methodName, expectedParamCount);
	}

	struct AnimationProbeCandidate
	{
		const char* name;
		const char* methodName;
		int expectedParamCount;
		uintptr_t(*resolver)(const char*, int);
		uint64_t rva;
		bool allowDumpFallback;
		uintptr_t address = 0;
		uintptr_t shellcode = 0;
		uintptr_t counter = 0;
		size_t stolenSize = 0;
		BYTE original[32] = { 0 };
		bool initialized = false;
		bool enabled = false;
	};

	static constexpr size_t kProbePatchMinSize = 14;
	static constexpr size_t kProbeMaxStolenSize = 32;
	static AnimationProbeCandidate g_animation_probe_candidates[] = {
		{ "ActorKit.FlushState", "FlushState", 8, ResolveActorKitMethod, Offset::FlushState, true },
		{ "ActorKit.ResetAgentInputParams", "ResetAgentInputParams", 0, ResolveActorKitMethod, 0x1F32A50, false },
		{ "ActorKit.SetInputAction", "SetInputAction", 1, ResolveActorKitMethod, 0x1F33EF0, false },
		{ "ActorKit.AnimatorCrossFadeInFixTime", "AnimatorCrossFadeInFixTime", 4, ResolveActorKitMethod, 0x3967E60, false },
		{ "ActorKit.AnimatorCrossFade", "AnimatorCrossFade", 3, ResolveActorKitMethod, 0x1F271B0, false },
		{ "ActorKit.NoticeLayerCrossFade", "NoticeLayerCrossFade", 1, ResolveActorKitMethod, 0x1F2F820, false },
		{ "ActorKit.ChangeActorLayer", "ChangeActorLayer", 2, ResolveActorKitMethod, 0x1F27B30, false },
		{ "ActorKit.SetDefaultLayer", "SetDefaultLayer", 0, ResolveActorKitMethod, 0x1F33B90, false },
		{ "ActorKit.CheckDodgeSprintEvt", "CheckDodgeSprintEvt", 0, ResolveActorKitMethod, 0x1F27EB0, false },
		{ "ActorKit.CheckSimpleAction", "CheckSimpleAction", 3, ResolveActorKitMethod, 0x1F28DC0, false },
		{ "ActorKit.TrySimpleAction", "TrySimpleAction", 4, ResolveActorKitMethod, 0x1F36060, false },
		{ "ActorKit.GetRealSkillInput", "GetRealSkillInput", 1, ResolveActorKitMethod, 0x1F2BE40, false },
		{ "ActorKit.IsQuickShockBanned", "IsQuickShockBanned", 0, ResolveActorKitMethod, 0x1F2ECC0, false },
		{ "ActorKit.FrameBeginTick", "FrameBeginTick", 2, ResolveActorKitMethod, 0x1F29710, false },
		{ "ActorKit.FrameEndTick", "FrameEndTick", 2, ResolveActorKitMethod, 0x1F2A4C0, false },
		{ "AnimPlayable.CrossFadeInFixedTime", "CrossFadeInFixedTime", 7, ResolveAnimPlayableMethod, Offset::AnimPlayableCrossFadeInFixedTimeNormal, false },
		{ "AnimPlayable.CrossFade", "CrossFade", 7, ResolveAnimPlayableMethod, 0x39B7620, false },
		{ "AnimPlayable.CrossFadeContext", "CrossFadeContext", 8, ResolveAnimPlayableMethod, 0x39B4220, false },
		{ "AnimPlayable.CrossFadeInternal", "CrossFadeInternal", 9, ResolveAnimPlayableMethod, 0x39B4870, false },
		{ "AnimPlayable.DoTransition", "DoTransition", 4, ResolveAnimPlayableMethod, 0x39B7940, false },
		{ "AnimPlayable.HandleStateTransition", "HandleStateTransition", 2, ResolveAnimPlayableMethod, 0x39BA2E0, false },
	};
	static constexpr size_t kAnimationProbeCandidateCount =
		sizeof(g_animation_probe_candidates) / sizeof(g_animation_probe_candidates[0]);
	static bool g_animation_probe_initialized = false;
	static bool g_animation_probe_enabled = false;
	static bool g_animation_probe_runtime_resolved = false;

	static bool IsRelControlOpcode(BYTE opcode)
	{
		return opcode == 0xE8 || opcode == 0xE9 || opcode == 0xEB ||
			(opcode >= 0x70 && opcode <= 0x7F);
	}

	static bool DecodeModRmLength(const BYTE* bytes, size_t maxSize, size_t* length, bool* ripRelative)
	{
		if (maxSize < 1)
		{
			return false;
		}

		BYTE modrm = bytes[0];
		BYTE mod = (modrm >> 6) & 0x3;
		BYTE rm = modrm & 0x7;
		size_t len = 1;
		if (mod != 3 && rm == 4)
		{
			if (maxSize < len + 1)
			{
				return false;
			}
			BYTE sib = bytes[len++];
			BYTE base = sib & 0x7;
			if (mod == 0 && base == 5)
			{
				if (ripRelative)
				{
					*ripRelative = true;
				}
				len += 4;
			}
		}
		else if (mod == 0 && rm == 5)
		{
			if (ripRelative)
			{
				*ripRelative = true;
			}
			len += 4;
		}
		else if (mod == 1)
		{
			len += 1;
		}
		else if (mod == 2)
		{
			len += 4;
		}

		if (len > maxSize)
		{
			return false;
		}
		*length = len;
		return true;
	}

	static bool DecodeOneInstructionLength(const BYTE* bytes, size_t maxSize, size_t* length)
	{
		if (!bytes || maxSize == 0)
		{
			return false;
		}

		size_t i = 0;
		bool rexW = false;
		while (i < maxSize)
		{
			BYTE b = bytes[i];
			if (b >= 0x40 && b <= 0x4F)
			{
				rexW = (b & 0x08) != 0;
				++i;
				continue;
			}
			if (b == 0x66 || b == 0x67 || b == 0xF0 || b == 0xF2 || b == 0xF3 ||
				b == 0x2E || b == 0x36 || b == 0x3E || b == 0x26 || b == 0x64 || b == 0x65)
			{
				++i;
				continue;
			}
			break;
		}
		if (i >= maxSize)
		{
			return false;
		}

		BYTE opcode = bytes[i];
		if (opcode == 0xCC || opcode == 0xC3 || opcode == 0xC2 || opcode == 0xCB || opcode == 0xCA ||
			IsRelControlOpcode(opcode))
		{
			return false;
		}

		if ((opcode >= 0x50 && opcode <= 0x5F) || opcode == 0x90 || opcode == 0x9C || opcode == 0x9D)
		{
			*length = i + 1;
			return true;
		}
		if (opcode == 0x6A)
		{
			*length = i + 2;
			return *length <= maxSize;
		}
		if (opcode == 0x68)
		{
			*length = i + 5;
			return *length <= maxSize;
		}
		if (opcode >= 0xB8 && opcode <= 0xBF)
		{
			*length = i + 1 + (rexW ? 8 : 4);
			return *length <= maxSize;
		}
		if (opcode == 0xA0 || opcode == 0xA1 || opcode == 0xA2 || opcode == 0xA3)
		{
			*length = i + 1 + (rexW ? 8 : 4);
			return *length <= maxSize;
		}

		if (opcode == 0x0F)
		{
			if (i + 1 >= maxSize)
			{
				return false;
			}
			BYTE op2 = bytes[i + 1];
			if (op2 >= 0x80 && op2 <= 0x8F)
			{
				return false;
			}
			if (op2 == 0x1E && i + 2 < maxSize && bytes[i + 2] == 0xFA)
			{
				*length = i + 3;
				return true;
			}

			bool ripRelative = false;
			size_t modLen = 0;
			if (!DecodeModRmLength(bytes + i + 2, maxSize - i - 2, &modLen, &ripRelative) || ripRelative)
			{
				return false;
			}
			*length = i + 2 + modLen;
			return true;
		}

		if (opcode == 0xC5 || opcode == 0xC4 || opcode == 0x62)
		{
			size_t prefixLen = opcode == 0xC5 ? 2 : (opcode == 0xC4 ? 3 : 4);
			if (i + prefixLen + 2 > maxSize)
			{
				return false;
			}

			bool ripRelative = false;
			size_t modLen = 0;
			if (!DecodeModRmLength(bytes + i + prefixLen + 1, maxSize - i - prefixLen - 1, &modLen, &ripRelative) || ripRelative)
			{
				return false;
			}
			*length = i + prefixLen + 1 + modLen;
			return true;
		}

		auto decodeModRmOpcode = [&](size_t immSize) -> bool
			{
				bool ripRelative = false;
				size_t modLen = 0;
				if (i + 1 >= maxSize ||
					!DecodeModRmLength(bytes + i + 1, maxSize - i - 1, &modLen, &ripRelative) ||
					ripRelative)
				{
					return false;
				}
				*length = i + 1 + modLen + immSize;
				return *length <= maxSize;
			};

		switch (opcode)
		{
		case 0x80:
		case 0x82:
		case 0x83:
		case 0xC0:
		case 0xC1:
		case 0xC6:
			return decodeModRmOpcode(1);
		case 0x81:
		case 0xC7:
			return decodeModRmOpcode(4);
		case 0xF6:
		case 0xF7:
		case 0xFF:
		case 0x8B:
		case 0x89:
		case 0x8A:
		case 0x88:
		case 0x8D:
		case 0x85:
		case 0x84:
		case 0x31:
		case 0x33:
		case 0x39:
		case 0x3B:
		case 0x01:
		case 0x03:
		case 0x09:
		case 0x0B:
		case 0x21:
		case 0x23:
		case 0x29:
		case 0x2B:
			return decodeModRmOpcode(0);
		default:
			break;
		}

		return false;
	}

	static bool CalcProbeStolenSize(const BYTE* bytes, size_t maxSize, size_t* stolenSize)
	{
		size_t total = 0;
		while (total < kProbePatchMinSize)
		{
			size_t len = 0;
			if (!DecodeOneInstructionLength(bytes + total, maxSize - total, &len) || len == 0)
			{
				return false;
			}
			total += len;
			if (total > kProbeMaxStolenSize)
			{
				return false;
			}
		}
		*stolenSize = total;
		return true;
	}

	static bool IsPatchedToAddress(const BYTE* bytes, uintptr_t target);

	// ===== Phase A: BuildCrossFadeProbeShellcode (READ-ONLY, no param modification) =====
	// Stack layout after sub rsp,0x40 (verified by PM audit):
	//   [rsp+0x00] xmm0 save (16B)   [rsp+0x10] xmm1 save (16B)
	//   [rsp+0x20] xmm2 save (16B)   [rsp+0x30] xmm3 save (16B)
	//   [rsp+0x40] push r11 = original_rsp   [rsp+0x48] push r10
	//   [rsp+0x50] push r9                   [rsp+0x58] push r8
	//   [rsp+0x60] push rdx                  [rsp+0x68] push rcx
	//   [rsp+0x70] push rax                  [rsp+0x78] pushfq (rflags)
	static std::vector<BYTE> BuildCrossFadeProbeShellcode(uintptr_t targetAddress,
		uintptr_t debugAddress,
		const BYTE* originalBytes,
		size_t stolenSize)
	{
		std::vector<BYTE> sc;
		BYTE placeholder[8] = { 0 };

		// ---- save original_rsp (before any push) ----
		sc.insert(sc.end(), { 0x49, 0x89, 0xE3 });        // mov r11, rsp  ; r11 = original_rsp

		// ---- save volatile regs (push order: top of stack = last pushed) ----
		sc.insert(sc.end(), { 0x9C });                    // pushfq          → [rsp+0x78]
		sc.insert(sc.end(), { 0x50 });                    // push rax        → [rsp+0x70]
		sc.insert(sc.end(), { 0x51 });                    // push rcx        → [rsp+0x68]
		sc.insert(sc.end(), { 0x52 });                    // push rdx        → [rsp+0x60]
		sc.insert(sc.end(), { 0x41, 0x50 });              // push r8         → [rsp+0x58]
		sc.insert(sc.end(), { 0x41, 0x51 });              // push r9         → [rsp+0x50]
		sc.insert(sc.end(), { 0x41, 0x52 });              // push r10        → [rsp+0x48]
		sc.insert(sc.end(), { 0x41, 0x53 });              // push r11        → [rsp+0x40] (original_rsp)

		// ---- save xmm0-xmm3 on stack (16 bytes each, aligned) ----
		sc.insert(sc.end(), { 0x48, 0x83, 0xEC, 0x40 }); // sub rsp, 0x40
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x7F, 0x04, 0x24 });       // movdqu [rsp+0x00], xmm0
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x10 }); // movdqu [rsp+0x10], xmm1
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x20 }); // movdqu [rsp+0x20], xmm2
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x30 }); // movdqu [rsp+0x30], xmm3

		// ---- record debug: mov rax, debugAddress ----
		sc.insert(sc.end(), { 0x48, 0xB8 });
		size_t debugOff = sc.size();
		sc.insert(sc.end(), placeholder, placeholder + 8); // debug addr placeholder

		// inc hitCount = inc qword ptr [rax+0x08]
		sc.insert(sc.end(), { 0x48, 0xFF, 0x40, 0x08 });   // inc qword ptr [rax+0x08]

		// lastThis = saved rcx @ [rsp+0x68]
		sc.insert(sc.end(), { 0x48, 0x8B, 0x4C, 0x24, 0x68 }); // mov rcx, [rsp+0x68]
		sc.insert(sc.end(), { 0x48, 0x89, 0x48, 0x10 });   // mov [rax+0x10], rcx

		// lastStateHash = saved edx @ [rsp+0x60] low32
		sc.insert(sc.end(), { 0x8B, 0x54, 0x24, 0x60 });   // mov edx, [rsp+0x60]
		sc.insert(sc.end(), { 0x89, 0x50, 0x18 });         // mov [rax+0x18], edx

		// lastLayerIdx = saved r8d @ [rsp+0x58] low32
		sc.insert(sc.end(), { 0x44, 0x8B, 0x44, 0x24, 0x58 }); // mov r8d, [rsp+0x58]
		sc.insert(sc.end(), { 0x44, 0x89, 0x40, 0x1C });   // mov [rax+0x1C], r8d

		// lastTransitionDuration = xmm1 from save area [rsp+0x10]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x4C, 0x24, 0x10 }); // movss xmm1, [rsp+0x10]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x48, 0x20 }); // movss [rax+0x20], xmm1

		// lastFixedTime = xmm2 from save area [rsp+0x20]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x54, 0x24, 0x20 }); // movss xmm2, [rsp+0x20]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x50, 0x24 }); // movss [rax+0x24], xmm2

		// lastSpeed = xmm3 from save area [rsp+0x30]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x5C, 0x24, 0x30 }); // movss xmm3, [rsp+0x30]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x58, 0x28 }); // movss [rax+0x28], xmm3

		// ---- read original_rsp to access stack params ----
		// original_rsp = saved r11 @ [rsp+0x40]
		sc.insert(sc.end(), { 0x4C, 0x8B, 0x5C, 0x24, 0x40 }); // mov r11, [rsp+0x40] ; r11=original_rsp

		// lastSourceType candidate = dword from [original_rsp + 0x20]
		sc.insert(sc.end(), { 0x41, 0x8B, 0x4B, 0x20 });   // mov ecx, [r11+0x20]
		sc.insert(sc.end(), { 0x89, 0x48, 0x2C });         // mov [rax+0x2C], ecx

		// lastForceSyncLoad candidate = byte from [original_rsp + 0x28]
		sc.insert(sc.end(), { 0x41, 0x0F, 0xB6, 0x4B, 0x28 }); // movzx ecx, byte ptr [r11+0x28]
		sc.insert(sc.end(), { 0x88, 0x48, 0x30 });         // mov [rax+0x30], cl

		// lastRcx/rdx/r8/r9 (full 64-bit from saved regs on stack)
		sc.insert(sc.end(), { 0x48, 0x8B, 0x4C, 0x24, 0x68 }); // mov rcx, [rsp+0x68] (saved rcx)
		sc.insert(sc.end(), { 0x48, 0x89, 0x48, 0x38 });   // mov [rax+0x38], rcx
		sc.insert(sc.end(), { 0x48, 0x8B, 0x54, 0x24, 0x60 }); // mov rdx, [rsp+0x60] (saved rdx)
		sc.insert(sc.end(), { 0x48, 0x89, 0x50, 0x40 });   // mov [rax+0x40], rdx
		sc.insert(sc.end(), { 0x4C, 0x8B, 0x44, 0x24, 0x58 }); // mov r8, [rsp+0x58] (saved r8)
		sc.insert(sc.end(), { 0x4C, 0x89, 0x40, 0x48 });   // mov [rax+0x48], r8
		sc.insert(sc.end(), { 0x4C, 0x8B, 0x4C, 0x24, 0x50 }); // mov r9, [rsp+0x50] (saved r9)
		sc.insert(sc.end(), { 0x4C, 0x89, 0x48, 0x50 });   // mov [rax+0x50], r9

		// lastXmm0-3 from stack save areas
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x04, 0x24 });       // movss xmm0, [rsp+0x00]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x40, 0x58 }); // movss [rax+0x58], xmm0
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x4C, 0x24, 0x10 }); // movss xmm1, [rsp+0x10]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x48, 0x5C }); // movss [rax+0x5C], xmm1
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x54, 0x24, 0x20 }); // movss xmm2, [rsp+0x20]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x50, 0x60 }); // movss [rax+0x60], xmm2
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x5C, 0x24, 0x30 }); // movss xmm3, [rsp+0x30]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x58, 0x64 }); // movss [rax+0x64], xmm3

		// lastRsp = original_rsp (from saved r11 @ [rsp+0x40])
		sc.insert(sc.end(), { 0x4C, 0x8B, 0x5C, 0x24, 0x40 }); // mov r11, [rsp+0x40]
		sc.insert(sc.end(), { 0x4C, 0x89, 0x58, 0x68 });   // mov [rax+0x68], r11

		// lastReturnAddress = [original_rsp]
		sc.insert(sc.end(), { 0x4D, 0x8B, 0x1B });         // mov r11, [r11]
		sc.insert(sc.end(), { 0x4C, 0x89, 0x58, 0x70 });   // mov [rax+0x70], r11

		// stackQword[0..15] from original_rsp + 0,8,16,...0x78
		for (int si = 0; si < 16; ++si) {
			int stackOff = si * 8;
			// mov r11, original_rsp (saved @ [rsp+0x40])
			sc.insert(sc.end(), { 0x4C, 0x8B, 0x5C, 0x24, 0x40 }); // mov r11, [rsp+0x40]
			// mov r11, [r11 + stackOff]
			if (stackOff < 0x80) {
				sc.insert(sc.end(), { 0x4D, 0x8B, 0x9B }); // mov r11, [r11+disp32]
				uint32_t disp = static_cast<uint32_t>(stackOff);
				sc.insert(sc.end(), (BYTE*)&disp, ((BYTE*)&disp) + 4);
			}
			// mov [rax + 0x78 + si*8], r11
			sc.insert(sc.end(), { 0x4C, 0x89, 0x98 });     // mov [rax+disp32], r11
			uint32_t dstOff = static_cast<uint32_t>(0x78 + si * 8);
			sc.insert(sc.end(), (BYTE*)&dstOff, ((BYTE*)&dstOff) + 4);
		}

		// ---- restore xmm0-xmm3 (correct, inverse order) ----
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x6F, 0x04, 0x24 });       // movdqu xmm0, [rsp+0x00]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x10 }); // movdqu xmm1, [rsp+0x10]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x20 }); // movdqu xmm2, [rsp+0x20]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x30 }); // movdqu xmm3, [rsp+0x30]
		sc.insert(sc.end(), { 0x48, 0x83, 0xC4, 0x40 });             // add rsp, 0x40

		// ---- restore volatile regs (reverse push order) ----
		sc.insert(sc.end(), { 0x41, 0x5B });              // pop r11
		sc.insert(sc.end(), { 0x41, 0x5A });              // pop r10
		sc.insert(sc.end(), { 0x41, 0x59 });              // pop r9
		sc.insert(sc.end(), { 0x41, 0x58 });              // pop r8
		sc.insert(sc.end(), { 0x5A });                    // pop rdx
		sc.insert(sc.end(), { 0x59 });                    // pop rcx
		sc.insert(sc.end(), { 0x58 });                    // pop rax
		sc.insert(sc.end(), { 0x9D });                    // popfq

		// ---- execute stolen bytes ----
		sc.insert(sc.end(), originalBytes, originalBytes + stolenSize);

		// ---- jmp back ----
		sc.insert(sc.end(), { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 });
		uintptr_t retAddr = targetAddress + stolenSize;
		sc.insert(sc.end(), (BYTE*)&retAddr, ((BYTE*)&retAddr) + 8);

		// Patch debug address placeholder
		*(uintptr_t*)&sc[debugOff] = debugAddress;

		return sc;
	}

	// ===== Phase A: InitCrossFadeProbeHook =====
	// NOTE: magic is written during init (debug struct), not in shellcode
	bool InitCrossFadeProbeHook()
	{
		g_cf_probe_initialized = false;
		g_cf_probe_enabled = false;

		g_cf_probe_address = ResolveAnimPlayableMethod("CrossFadeInFixedTime", 7);
		if (!g_cf_probe_address)
		{
			printf("[CrossFadeProbe] resolve FAILED\n");
			return false;
		}

		BYTE current[kProbeMaxStolenSize] = { 0 };
		if (!mem.Read(g_cf_probe_address, current, sizeof(current)))
		{
			printf("[CrossFadeProbe] read FAILED @ 0x%llX\n", (unsigned long long)g_cf_probe_address);
			return false;
		}

		size_t stolenSize = 0;
		if (!CalcProbeStolenSize(current, sizeof(current), &stolenSize))
		{
			printf("[CrossFadeProbe] prologue unsafe @ 0x%llX\n", (unsigned long long)g_cf_probe_address);
			return false;
		}

		std::memcpy(g_cf_probe_orig_bytes, current, stolenSize);
		g_cf_probe_stolen_size = stolenSize;

		// Write CrossFadeProbeDebug with magic to codecave (magic set once here)
		CrossFadeProbeDebug dbgInit{};
		dbgInit.magic = kCrossFadeProbeMagic;
		g_cf_probe_debug = FindWritableCodecave(
			reinterpret_cast<const BYTE*>(&dbgInit), sizeof(dbgInit), "CFProbeDebug");
		if (!g_cf_probe_debug)
		{
			printf("[CrossFadeProbe] debug alloc FAILED\n");
			return false;
		}

		// Build and write shellcode
		std::vector<BYTE> shellcode = BuildCrossFadeProbeShellcode(
			g_cf_probe_address, g_cf_probe_debug, g_cf_probe_orig_bytes, stolenSize);
		g_cf_probe_shellcode = FindWritableCodecave(
			shellcode.data(), shellcode.size(), "CFProbeShellcode");
		if (!g_cf_probe_shellcode)
		{
			printf("[CrossFadeProbe] shellcode alloc FAILED\n");
			return false;
		}

		g_cf_probe_initialized = true;
		printf("[CrossFadeProbe] READY RVA=0x%llX VA=0x%llX stolen=%zu shell=0x%llX debug=0x%llX size=%zu\n",
			(unsigned long long)(g_cf_probe_address - Offset::GameAssembly),
			(unsigned long long)g_cf_probe_address, stolenSize,
			(unsigned long long)g_cf_probe_shellcode,
			(unsigned long long)g_cf_probe_debug, shellcode.size());
		return true;
	}

	// ===== Phase A: PrintCrossFadeProbeDebug =====
	void PrintCrossFadeProbeDebug(const char* tag)
	{
		if (!g_cf_probe_debug) return;
		CrossFadeProbeDebug dbg{};
		mem.Read(g_cf_probe_debug, &dbg, sizeof(dbg));
		printf("[CrossFadeProbe] %s magic=0x%llX hit=%llu\n", tag,
			(unsigned long long)dbg.magic, (unsigned long long)dbg.hitCount);
		printf("[CrossFadeProbe] last this=0x%llX stateHash=0x%X(%d) layer=%d\n",
			(unsigned long long)dbg.lastThis,
			(unsigned int)dbg.lastStateHash, dbg.lastStateHash,
			dbg.lastLayerIdx);
		printf("[CrossFadeProbe] args duration=%.4f fixedTime=%.4f speed=%.4f srcTypeCand=0x%X(%d) forceSyncCand=%u\n",
			dbg.lastTransitionDuration, dbg.lastFixedTime, dbg.lastSpeed,
			(unsigned int)dbg.lastSourceType, dbg.lastSourceType,
			(unsigned int)dbg.lastForceSyncLoad);
		printf("[CrossFadeProbe] regs rcx=0x%llX rdx=0x%llX r8=0x%llX r9=0x%llX rsp=0x%llX ret=0x%llX\n",
			(unsigned long long)dbg.lastRcx, (unsigned long long)dbg.lastRdx,
			(unsigned long long)dbg.lastR8, (unsigned long long)dbg.lastR9,
			(unsigned long long)dbg.lastRsp, (unsigned long long)dbg.lastReturnAddress);
		printf("[CrossFadeProbe] xmm0=%.4f xmm1=%.4f xmm2=%.4f xmm3=%.4f\n",
			dbg.lastXmm0, dbg.lastXmm1, dbg.lastXmm2, dbg.lastXmm3);
		for (int i = 0; i < 16; ++i)
		{
			printf("[CrossFadeProbe] stack[%02d]=0x%016llX", i, (unsigned long long)dbg.stackQword[i]);
			if (i % 2 == 1) printf("\n"); else printf("  ");
		}
	}

	// 临时测试 stateHash 用，当前整体注释停用。
	//void ResetCrossFadeProbeDebug()
	//{
	//	if (!g_cf_probe_debug)
	//	{
	//		return;
	//	}
	//	CrossFadeProbeDebug dbg{};
	//	dbg.magic = kCrossFadeProbeMagic;
	//	mem.Write(g_cf_probe_debug, &dbg, sizeof(dbg));
	//}

	//bool TryReadCrossFadeProbeStateHash(int& stateHash, int* layerIdx, unsigned long long* hitCount)
	//{
	//	stateHash = 0;
	//	if (layerIdx)
	//	{
	//		*layerIdx = 0;
	//	}
	//	if (hitCount)
	//	{
	//		*hitCount = 0;
	//	}
	//	if (!g_cf_probe_debug)
	//	{
	//		return false;
	//	}
	//
	//	CrossFadeProbeDebug dbg{};
	//	if (!mem.Read(g_cf_probe_debug, &dbg, sizeof(dbg)))
	//	{
	//		return false;
	//	}
	//	if (dbg.magic != kCrossFadeProbeMagic || dbg.hitCount == 0)
	//	{
	//		return false;
	//	}
	//
	//	stateHash = dbg.lastStateHash;
	//	if (layerIdx)
	//	{
	//		*layerIdx = dbg.lastLayerIdx;
	//	}
	//	if (hitCount)
	//	{
	//		*hitCount = static_cast<unsigned long long>(dbg.hitCount);
	//	}
	//	return true;
	//}

	// ===== Phase A: EnableCrossFadeProbeHook =====
	void EnableCrossFadeProbeHook(bool enable)
	{
		if (enable && (!g_cf_probe_initialized || !g_cf_probe_shellcode))
		{
			InitCrossFadeProbeHook();
		}
		if (!g_cf_probe_initialized || !g_cf_probe_shellcode || !g_cf_probe_stolen_size)
		{
			return;
		}

		if (enable && !g_cf_probe_enabled)
		{
			uint64_t zero = 0;
			mem.Write(g_cf_probe_debug + 8, &zero, sizeof(zero));

			BYTE current[kProbeMaxStolenSize] = { 0 };
			if (!mem.Read(g_cf_probe_address, current, g_cf_probe_stolen_size))
			{
				printf("[CrossFadeProbe] enable read FAILED\n");
				return;
			}
			if (IsPatchedToAddress(current, g_cf_probe_shellcode))
			{
				g_cf_probe_enabled = true;
				printf("[CrossFadeProbe] ON (already patched)\n");
				return;
			}
			if (std::memcmp(current, g_cf_probe_orig_bytes, g_cf_probe_stolen_size) != 0)
			{
				printf("[CrossFadeProbe] enable skipped: prologue changed\n");
				return;
			}

			std::vector<BYTE> patch(g_cf_probe_stolen_size, 0x90);
			patch[0] = 0xFF; patch[1] = 0x25;
			patch[2] = 0x00; patch[3] = 0x00;
			patch[4] = 0x00; patch[5] = 0x00;
			*(uintptr_t*)&patch[6] = g_cf_probe_shellcode;
			if (mem.Write(g_cf_probe_address, patch.data(), patch.size()) &&
				ReadBackEquals(g_cf_probe_address, patch.data(), patch.size(), "CFProbe patch"))
			{
				g_cf_probe_enabled = true;
				printf("[CrossFadeProbe] ON\n");
			}
		}
		else if (!enable && g_cf_probe_enabled)
		{
			if (mem.Write(g_cf_probe_address, g_cf_probe_orig_bytes, g_cf_probe_stolen_size) &&
				ReadBackEquals(g_cf_probe_address, g_cf_probe_orig_bytes, g_cf_probe_stolen_size, "CFProbe restore"))
			{
				g_cf_probe_enabled = false;
				printf("[CrossFadeProbe] OFF\n");
				// [PhaseA isolation] PrintCrossFadeProbeDebug removed
			}
		}
	}

	bool IsCrossFadeProbeHookEnabled() { return g_cf_probe_enabled; }

	static float NormalizeCrossFadeSpeed(float speed)
	{
		if (speed >= 0.1f && speed <= 50.0f)
		{
			return speed;
		}
		if (speed < 0.1f)
		{
			return 0.1f;
		}
		if (speed > 50.0f)
		{
			return 50.0f;
		}
		return 10.0f;
	}

	static uintptr_t ResolveCrossFadeSpeedAddress()
	{
		uintptr_t address = ResolveAnimPlayableMethod("CrossFadeInFixedTime", 7);
		if (address && IsGameAssemblyCodePtr(address))
		{
			printf("[CrossFadeSpeed] resolver RVA=0x%llX VA=0x%llX\n",
				(unsigned long long)(address - Offset::GameAssembly),
				(unsigned long long)address);
			return address;
		}

		const uint64_t fallbackRva = GetAnimPlayableCrossFadeInFixedTimeFallbackRva();
		address = Offset::GameAssembly + fallbackRva;
		printf("[CrossFadeSpeed] fallback RVA=0x%llX VA=0x%llX\n",
			(unsigned long long)fallbackRva,
			(unsigned long long)address);
		return address;
	}

	static uintptr_t GetLocalAnimPlayableForSpeed()
	{
		uintptr_t actorKit = GetLocalActorKitForResolve();
		if (!actorKit)
		{
			return 0;
		}

		// 独立动画加速只用于 self 过滤；优先使用最新 dump 偏移，保留旧偏移兜底。
		const uint32_t offsets[] = {
			static_cast<uint32_t>(Offset::ActorModel.ActorKitS.AnimPlayableAgent),
			0x2D0u,
			0x2C8u,
			0x2C0u
		};
		for (uint32_t offset : offsets)
		{
			uintptr_t animPlayable = mem.Read<uintptr_t>(actorKit + offset);
			if (!IsCanonicalUserPtr(animPlayable))
			{
				continue;
			}

			uintptr_t klass = ReadObjectKlass(animPlayable);
			std::string className;
			if (ReadKlassName(klass, className) && className == "AnimPlayable")
			{
				return animPlayable;
			}
		}

		uintptr_t animPlayable = GetLocalAnimPlayableForResolve();
		if (IsCanonicalUserPtr(animPlayable))
		{
			return animPlayable;
		}
		return 0;
	}

	static bool WriteCrossFadeSpeedControl(bool enable)
	{
		if (!g_cf_speed_control)
		{
			return false;
		}

		CrossFadeSpeedControl control{};
		if (!mem.Read(g_cf_speed_control, &control, sizeof(control)) ||
			control.magic != kCrossFadeSpeedMagic)
		{
			control = {};
			control.magic = kCrossFadeSpeedMagic;
		}

		control.enabled = enable ? 1 : 0;
		control.speed = NormalizeCrossFadeSpeed(Function::Shock::动画加速倍率);
		control.localAnimPlayable = GetLocalAnimPlayableForSpeed();
		control.lastLocalAnim = control.localAnimPlayable;

		const bool writeOk = mem.Write(g_cf_speed_control, &control, sizeof(control));
		printf("[CrossFadeSpeed] control enable=%d speed=%.2f localAnim=0x%llX write=%d\n",
			enable ? 1 : 0,
			control.speed,
			(unsigned long long)control.localAnimPlayable,
			writeOk ? 1 : 0);
		return writeOk;
	}

	static void PatchRel32(std::vector<BYTE>& code, size_t instructionOffset, size_t targetOffset)
	{
		const int32_t rel = static_cast<int32_t>(targetOffset - (instructionOffset + 6));
		std::memcpy(code.data() + instructionOffset + 2, &rel, sizeof(rel));
	}

	static std::vector<BYTE> BuildCrossFadeSpeedShellcode(uintptr_t targetAddress,
		uintptr_t controlAddress,
		const BYTE* originalBytes,
		size_t stolenSize)
	{
		std::vector<BYTE> sc;
		BYTE placeholder[8] = { 0 };

		sc.insert(sc.end(), { 0x9C });                         // pushfq
		sc.insert(sc.end(), { 0x50 });                         // push rax
		sc.insert(sc.end(), { 0x41, 0x52 });                   // push r10

		sc.insert(sc.end(), { 0x48, 0xB8 });                   // mov rax, controlAddress
		size_t controlOff = sc.size();
		sc.insert(sc.end(), placeholder, placeholder + 8);
		*(uintptr_t*)&sc[controlOff] = controlAddress;

		sc.insert(sc.end(), { 0x48, 0xFF, 0x40, 0x20 });       // inc qword ptr [rax+0x20]
		sc.insert(sc.end(), { 0x48, 0x89, 0x48, 0x30 });       // mov [rax+0x30], rcx
		sc.insert(sc.end(), { 0x89, 0x50, 0x40 });             // mov [rax+0x40], edx
		sc.insert(sc.end(), { 0x44, 0x89, 0x40, 0x44 });       // mov [rax+0x44], r8d

		sc.insert(sc.end(), { 0x80, 0x78, 0x08, 0x01 });       // cmp byte ptr [rax+0x08], 1
		size_t jneDisabled = sc.size();
		sc.insert(sc.end(), { 0x0F, 0x85, 0, 0, 0, 0 });       // jne done

		sc.insert(sc.end(), { 0x4C, 0x8B, 0x50, 0x18 });       // mov r10, [rax+0x18]
		sc.insert(sc.end(), { 0x4D, 0x85, 0xD2 });             // test r10, r10
		size_t jeNoLocal = sc.size();
		sc.insert(sc.end(), { 0x0F, 0x84, 0, 0, 0, 0 });       // je done

		sc.insert(sc.end(), { 0x4C, 0x89, 0x50, 0x38 });       // mov [rax+0x38], r10
		sc.insert(sc.end(), { 0x4C, 0x39, 0xD1 });             // cmp rcx, r10
		size_t jneNotSelf = sc.size();
		sc.insert(sc.end(), { 0x0F, 0x85, 0, 0, 0, 0 });       // jne done

		sc.insert(sc.end(), { 0x48, 0xFF, 0x40, 0x28 });       // inc qword ptr [rax+0x28]

		// 独立动画加速：只改 AnimPlayable.CrossFadeInFixedTime 的 speed 参数。
		// 已验证 Probe 中 speed 对应 xmm3；这里不写栈参数，避免影响 sourceType/forceSyncLoad。
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x58, 0x48 });       // movss [rax+0x48], xmm3
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x10, 0x58, 0x10 });       // movss xmm3, [rax+0x10]
		sc.insert(sc.end(), { 0xF3, 0x0F, 0x11, 0x58, 0x4C });       // movss [rax+0x4C], xmm3

		size_t doneOffset = sc.size();
		PatchRel32(sc, jneDisabled, doneOffset);
		PatchRel32(sc, jeNoLocal, doneOffset);
		PatchRel32(sc, jneNotSelf, doneOffset);

		sc.insert(sc.end(), { 0x41, 0x5A });                   // pop r10
		sc.insert(sc.end(), { 0x58 });                         // pop rax
		sc.insert(sc.end(), { 0x9D });                         // popfq

		sc.insert(sc.end(), originalBytes, originalBytes + stolenSize);

		sc.insert(sc.end(), { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 });
		uintptr_t retAddr = targetAddress + stolenSize;
		sc.insert(sc.end(), (BYTE*)&retAddr, ((BYTE*)&retAddr) + 8);

		return sc;
	}

	bool InitCrossFadeSpeedHook()
	{
		if (g_cf_speed_initialized && g_cf_speed_shellcode && g_cf_speed_control)
		{
			return true;
		}

		g_cf_speed_initialized = false;
		g_cf_speed_enabled = false;
		g_cf_speed_address = ResolveCrossFadeSpeedAddress();
		if (!g_cf_speed_address || !IsGameAssemblyCodePtr(g_cf_speed_address))
		{
			printf("[CrossFadeSpeed] resolve FAILED\n");
			return false;
		}

		BYTE current[kProbeMaxStolenSize] = { 0 };
		if (!mem.Read(g_cf_speed_address, current, sizeof(current)))
		{
			printf("[CrossFadeSpeed] read FAILED @ 0x%llX\n",
				(unsigned long long)g_cf_speed_address);
			return false;
		}

		size_t stolenSize = 0;
		if (!CalcProbeStolenSize(current, sizeof(current), &stolenSize))
		{
			printf("[CrossFadeSpeed] prologue unsafe @ 0x%llX\n",
				(unsigned long long)g_cf_speed_address);
			PrintHookBytes("[CrossFadeSpeed] bytes:", current, sizeof(current));
			return false;
		}

		std::memcpy(g_cf_speed_orig_bytes, current, stolenSize);
		g_cf_speed_stolen_size = stolenSize;

		CrossFadeSpeedControl control{};
		control.magic = kCrossFadeSpeedMagic;
		control.enabled = 0;
		control.speed = NormalizeCrossFadeSpeed(Function::Shock::动画加速倍率);
		control.localAnimPlayable = GetLocalAnimPlayableForSpeed();
		control.lastLocalAnim = control.localAnimPlayable;
		g_cf_speed_control = FindWritableCodecave(
			reinterpret_cast<const BYTE*>(&control), sizeof(control), "CFSpeedControl");
		if (!g_cf_speed_control)
		{
			printf("[CrossFadeSpeed] control alloc FAILED\n");
			return false;
		}

		std::vector<BYTE> shellcode = BuildCrossFadeSpeedShellcode(
			g_cf_speed_address, g_cf_speed_control, g_cf_speed_orig_bytes, stolenSize);
		g_cf_speed_shellcode = FindWritableCodecave(
			shellcode.data(), shellcode.size(), "CFSpeedShellcode");
		if (!g_cf_speed_shellcode)
		{
			printf("[CrossFadeSpeed] shellcode alloc FAILED\n");
			return false;
		}

		g_cf_speed_initialized = true;
		printf("[CrossFadeSpeed] READY RVA=0x%llX VA=0x%llX stolen=%zu shell=0x%llX control=0x%llX size=%zu\n",
			(unsigned long long)(g_cf_speed_address - Offset::GameAssembly),
			(unsigned long long)g_cf_speed_address,
			stolenSize,
			(unsigned long long)g_cf_speed_shellcode,
			(unsigned long long)g_cf_speed_control,
			shellcode.size());
		return true;
	}

	void SetCrossFadeSpeed(float speed)
	{
		Function::Shock::动画加速倍率 = NormalizeCrossFadeSpeed(speed);
		if (g_cf_speed_control)
		{
			WriteCrossFadeSpeedControl(g_cf_speed_enabled);
		}
	}

	void EnableCrossFadeSpeedHook(bool enable)
	{
		if (enable && (!g_cf_speed_initialized || !g_cf_speed_shellcode))
		{
			InitCrossFadeSpeedHook();
		}

		if (!g_cf_speed_initialized || !g_cf_speed_shellcode || !g_cf_speed_stolen_size)
		{
			Function::Shock::动画加速 = false;
			return;
		}

		if (enable)
		{
			WriteCrossFadeSpeedControl(true);
		}

		if (enable && !g_cf_speed_enabled)
		{
			BYTE current[kProbeMaxStolenSize] = { 0 };
			if (!mem.Read(g_cf_speed_address, current, g_cf_speed_stolen_size))
			{
				printf("[CrossFadeSpeed] enable read FAILED\n");
				Function::Shock::动画加速 = false;
				return;
			}
			if (IsPatchedToAddress(current, g_cf_speed_shellcode))
			{
				g_cf_speed_enabled = true;
				Function::Shock::动画加速 = true;
				printf("[CrossFadeSpeed] ON (already patched)\n");
				return;
			}
			if (std::memcmp(current, g_cf_speed_orig_bytes, g_cf_speed_stolen_size) != 0)
			{
				printf("[CrossFadeSpeed] enable skipped: prologue changed\n");
				Function::Shock::动画加速 = false;
				return;
			}

			std::vector<BYTE> patch(g_cf_speed_stolen_size, 0x90);
			patch[0] = 0xFF;
			patch[1] = 0x25;
			patch[2] = 0x00;
			patch[3] = 0x00;
			patch[4] = 0x00;
			patch[5] = 0x00;
			*(uintptr_t*)&patch[6] = g_cf_speed_shellcode;
			if (mem.Write(g_cf_speed_address, patch.data(), patch.size()) &&
				ReadBackEquals(g_cf_speed_address, patch.data(), patch.size(), "CFSpeed patch"))
			{
				g_cf_speed_enabled = true;
				Function::Shock::动画加速 = true;
				printf("[CrossFadeSpeed] ON\n");
			}
			else
			{
				Function::Shock::动画加速 = false;
			}
		}
		else if (!enable && g_cf_speed_enabled)
		{
			if (mem.Write(g_cf_speed_address, g_cf_speed_orig_bytes, g_cf_speed_stolen_size) &&
				ReadBackEquals(g_cf_speed_address, g_cf_speed_orig_bytes, g_cf_speed_stolen_size, "CFSpeed restore"))
			{
				g_cf_speed_enabled = false;
				WriteCrossFadeSpeedControl(false);
				Function::Shock::动画加速 = false;
				printf("[CrossFadeSpeed] OFF\n");
			}
		}
		else if (!enable)
		{
			WriteCrossFadeSpeedControl(false);
			Function::Shock::动画加速 = false;
		}
	}

	bool IsCrossFadeSpeedHookEnabled()
	{
		return g_cf_speed_enabled;
	}

	static bool IsPatchedToAddress(const BYTE* bytes, uintptr_t target)
	{
		if (bytes[0] != 0xFF || bytes[1] != 0x25 ||
			bytes[2] != 0x00 || bytes[3] != 0x00 ||
			bytes[4] != 0x00 || bytes[5] != 0x00)
		{
			return false;
		}
		uintptr_t currentTarget = 0;
		std::memcpy(&currentTarget, bytes + 6, sizeof(currentTarget));
		return currentTarget == target;
	}

	static std::vector<BYTE> BuildProbeShellcode(const AnimationProbeCandidate& candidate)
	{
		std::vector<BYTE> shellcode = {
			0x9C,                                           // pushfq
			0x50,                                           // push rax
			0x48, 0xB8,                                     // mov rax, counter
			0,0,0,0,0,0,0,0,
			0x48, 0xFF, 0x00,                               // inc qword ptr [rax]
			0x58,                                           // pop rax
			0x9D,                                           // popfq
		};
		*(uintptr_t*)&shellcode[4] = candidate.counter;
		shellcode.insert(shellcode.end(), candidate.original, candidate.original + candidate.stolenSize);
		BYTE jmpBack[] = {
			0xFF,0x25,0x00,0x00,0x00,0x00,
			0,0,0,0,0,0,0,0
		};
		uintptr_t retAddr = candidate.address + candidate.stolenSize;
		*(uintptr_t*)&jmpBack[6] = retAddr;
		shellcode.insert(shellcode.end(), jmpBack, jmpBack + sizeof(jmpBack));
		return shellcode;
	}

	static void ResetProbeCounter(const AnimationProbeCandidate& candidate)
	{
		if (!candidate.counter)
		{
			return;
		}
		uint64_t zero = 0;
		mem.Write(candidate.counter, &zero, sizeof(zero));
	}

	static uint64_t ReadProbeCounter(const AnimationProbeCandidate& candidate)
	{
		uint64_t value = 0;
		if (candidate.counter)
		{
			mem.Read(candidate.counter, &value, sizeof(value));
		}
		return value;
	}

	VOID InitAnimationAccelProbe()
	{
		ULONGLONG initStartTick = GetTickCount64();
		g_animation_probe_initialized = false;
		g_animation_probe_enabled = false;
		g_animation_probe_runtime_resolved = false;
		for (auto& candidate : g_animation_probe_candidates)
		{
			candidate.address = 0;
			candidate.shellcode = 0;
			candidate.counter = 0;
			candidate.stolenSize = 0;
			candidate.initialized = false;
			candidate.enabled = false;
			std::memset(candidate.original, 0, sizeof(candidate.original));
		}

		g_actor_kit_cross_fade_in_fixed_time =
			ResolveActorKitMethod("AnimatorCrossFadeInFixTime", 4);
		if (g_actor_kit_cross_fade_in_fixed_time)
		{
			Offset::CrossFadeInFixedTime =
				g_actor_kit_cross_fade_in_fixed_time - Offset::GameAssembly;
			printf("[AnimationAccel] CrossFadeInFixedTime runtime RVA updated: 0x%llX\n",
				(unsigned long long)Offset::CrossFadeInFixedTime);
		}

		std::vector<BYTE> counters(kAnimationProbeCandidateCount * sizeof(uint64_t), 0);
		uintptr_t counterBase = FindWritableCodecave(counters.data(), counters.size(), "ProbeCounters");
		if (!counterBase)
		{
			printf("[AnimationProbe] 初始化失败: 无法分配 ProbeCounters cost=%llums\n",
				(unsigned long long)(GetTickCount64() - initStartTick));
			return;
		}

		size_t readyCount = 0;
		size_t runtimeResolvedCount = 0;
		for (size_t i = 0; i < kAnimationProbeCandidateCount; ++i)
		{
			auto& candidate = g_animation_probe_candidates[i];
			if (candidate.resolver)
			{
				candidate.address =
					candidate.resolver(candidate.methodName, candidate.expectedParamCount);
				if (candidate.address)
				{
					++runtimeResolvedCount;
				}
			}
			if (!candidate.address)
			{
				if (!candidate.allowDumpFallback)
				{
					printf("[AnimationProbe] SKIP %s no live address (dump RVA=0x%llX disabled)\n",
						candidate.name,
						(unsigned long long)candidate.rva);
					continue;
				}
				candidate.address = Offset::GameAssembly + candidate.rva;
				printf("[AnimationProbe] FALLBACK %s dump RVA=0x%llX VA=0x%llX\n",
					candidate.name,
					(unsigned long long)candidate.rva,
					(unsigned long long)candidate.address);
			}

			if (!IsGameAssemblyCodePtr(candidate.address))
			{
				printf("[AnimationProbe] SKIP %s RVA=0x%llX VA=0x%llX 不在 GameAssembly 模块范围\n",
					candidate.name,
					(unsigned long long)(candidate.address - Offset::GameAssembly),
					(unsigned long long)candidate.address);
				continue;
			}

			BYTE current[kProbeMaxStolenSize] = { 0 };
			if (!mem.Read(candidate.address, current, sizeof(current)))
			{
				printf("[AnimationProbe] SKIP %s RVA=0x%llX VA=0x%llX 读取失败\n",
					candidate.name,
					(unsigned long long)(candidate.address - Offset::GameAssembly),
					(unsigned long long)candidate.address);
				continue;
			}

			size_t stolenSize = 0;
			if (!CalcProbeStolenSize(current, sizeof(current), &stolenSize))
			{
				printf("[AnimationProbe] SKIP %s RVA=0x%llX VA=0x%llX 前导指令不可安全搬运\n",
					candidate.name,
					(unsigned long long)(candidate.address - Offset::GameAssembly),
					(unsigned long long)candidate.address);
				PrintHookBytes("[AnimationProbe] bytes:", current, sizeof(current));
				continue;
			}

			candidate.stolenSize = stolenSize;
			candidate.counter = counterBase + i * sizeof(uint64_t);
			std::memcpy(candidate.original, current, stolenSize);
			std::vector<BYTE> shellcode = BuildProbeShellcode(candidate);
			candidate.shellcode = FindWritableCodecave(shellcode.data(), shellcode.size(), candidate.name);
			if (!candidate.shellcode)
			{
				continue;
			}
			candidate.initialized = true;
			++readyCount;
			printf("[AnimationProbe] READY %s RVA=0x%llX VA=0x%llX stolen=%zu shell=0x%llX counter=0x%llX\n",
				candidate.name,
				(unsigned long long)(candidate.address - Offset::GameAssembly),
				(unsigned long long)candidate.address,
				candidate.stolenSize,
				(unsigned long long)candidate.shellcode,
				(unsigned long long)candidate.counter);
		}

		g_animation_probe_initialized = readyCount > 0;
		g_animation_probe_runtime_resolved = runtimeResolvedCount > 0;
		printf("[AnimationProbe] Init ready=%zu/%zu runtime=%zu region=0x%llX cost=%llums\n",
			readyCount,
			kAnimationProbeCandidateCount,
			runtimeResolvedCount,
			(unsigned long long)counterBase,
			(unsigned long long)(GetTickCount64() - initStartTick));
	}

	VOID EnableAnimationAccelProbe(bool enable)
	{
		if (enable && (g_anim_accel_enabled || g_cf_probe_enabled))
		{
			printf("[AnimationProbe] skipping old accel disable (not active)\n");
		}

		if (enable && (!g_animation_probe_initialized || !g_animation_probe_runtime_resolved))
		{
			printf("[AnimationProbe] Lazy init before enable\n");
			InitAnimationAccelProbe();
		}

		if (!g_animation_probe_initialized)
		{
			return;
		}

		if (enable && !g_animation_probe_enabled)
		{
			size_t enabledCount = 0;
			for (auto& candidate : g_animation_probe_candidates)
			{
				if (!candidate.initialized || !candidate.shellcode || !candidate.stolenSize)
				{
					continue;
				}

				BYTE current[kProbeMaxStolenSize] = { 0 };
				if (!mem.Read(candidate.address, current, candidate.stolenSize))
				{
					printf("[AnimationProbe] Enable 读取失败 %s @ 0x%llX\n",
						candidate.name, (unsigned long long)candidate.address);
					continue;
				}
				if (IsPatchedToAddress(current, candidate.shellcode))
				{
					candidate.enabled = true;
					++enabledCount;
					continue;
				}
				if (std::memcmp(current, candidate.original, candidate.stolenSize) != 0)
				{
					printf("[AnimationProbe] Enable 跳过 %s，入口字节已变化\n", candidate.name);
					PrintHookBytes("[AnimationProbe] expected:", candidate.original, candidate.stolenSize);
					PrintHookBytes("[AnimationProbe] actual:  ", current, candidate.stolenSize);
					continue;
				}

				ResetProbeCounter(candidate);
				std::vector<BYTE> patch(candidate.stolenSize, 0x90);
				patch[0] = 0xFF;
				patch[1] = 0x25;
				patch[2] = 0x00;
				patch[3] = 0x00;
				patch[4] = 0x00;
				patch[5] = 0x00;
				*(uintptr_t*)&patch[6] = candidate.shellcode;
				if (mem.Write(candidate.address, patch.data(), patch.size()) &&
					ReadBackEquals(candidate.address, patch.data(), patch.size(), candidate.name))
				{
					candidate.enabled = true;
					++enabledCount;
				}
				else
				{
					printf("[AnimationProbe] Enable patch failed %s\n", candidate.name);
				}
			}

			g_animation_probe_enabled = enabledCount > 0;
			printf("[AnimationProbe] ON candidates=%zu\n", enabledCount);
		}
		else if (!enable && g_animation_probe_enabled)
		{
			size_t restoredCount = 0;
			size_t hitCount = 0;
			for (auto& candidate : g_animation_probe_candidates)
			{
				if (!candidate.enabled)
				{
					continue;
				}
				if (mem.Write(candidate.address, candidate.original, candidate.stolenSize) &&
					ReadBackEquals(candidate.address, candidate.original, candidate.stolenSize, candidate.name))
				{
					++restoredCount;
				}
				else
				{
					printf("[AnimationProbe] Restore failed %s\n", candidate.name);
				}
				candidate.enabled = false;

				uint64_t count = ReadProbeCounter(candidate);
				if (count > 0)
				{
					++hitCount;
					printf("[AnimationProbe] HIT %s RVA=0x%llX count=%llu\n",
						candidate.name,
						(unsigned long long)(candidate.address - Offset::GameAssembly),
						(unsigned long long)count);
				}
			}

			g_animation_probe_enabled = false;
			if (hitCount == 0)
			{
				printf("[AnimationProbe] OFF restored=%zu hit=0\n", restoredCount);
			}
			else
			{
				printf("[AnimationProbe] OFF restored=%zu hit=%zu\n", restoredCount, hitCount);
			}
		}
	}

	bool IsAnimationAccelProbeEnabled() { return g_animation_probe_enabled; }

	VOID InitFlushStateHook()
	{
		g_flush_hook_initialized = false;
		g_anim_accel_enabled = false;
		g_flush_orig_saved = false;
		g_flush_debug_data = 0;
		// 避开 vtbale_jmp_address(+0x300) 和 vtbale_switch_address(+0x500) 使用的区域。
		flush_codecave = 0;

		uintptr_t tthSlotVA = Offset::GameAssembly + Offset::m_TransparentTagHandler;
		uintptr_t crossFadeVA = g_actor_kit_cross_fade_in_fixed_time ?
			g_actor_kit_cross_fade_in_fixed_time :
			Offset::GameAssembly + Offset::CrossFadeInFixedTime;
		uintptr_t flushAddr = Offset::GameAssembly + Offset::FlushState;
		uintptr_t retAddr = flushAddr + kFlushStolenSize;
		flush_hook_trampoline = retAddr;
		printf("[AnimationAccel] Init FlushState=0x%llX CrossFade=0x%llX RVA=0x%llX\n",
			(unsigned long long)flushAddr,
			(unsigned long long)crossFadeVA,
			(unsigned long long)(crossFadeVA - Offset::GameAssembly));

		BYTE currentPrologue[kFlushStolenSize] = { 0 };
		if (!mem.Read(flushAddr, currentPrologue, sizeof(currentPrologue)))
		{
			printf("[AnimationAccel] 读取 FlushState 原始字节失败 @ 0x%llX\n", (unsigned long long)flushAddr);
			return;
		}
		if (std::memcmp(currentPrologue, kFlushExpectedPrologue, sizeof(currentPrologue)) != 0)
		{
			printf("[AnimationAccel] FlushState 原始字节不匹配，停止初始化 @ 0x%llX\n", (unsigned long long)flushAddr);
			PrintHookBytes("[AnimationAccel] expected:", kFlushExpectedPrologue, sizeof(kFlushExpectedPrologue));
			PrintHookBytes("[AnimationAccel] actual:  ", currentPrologue, sizeof(currentPrologue));
			return;
		}
		std::memcpy(g_flush_orig_bytes, currentPrologue, sizeof(g_flush_orig_bytes));
		g_flush_orig_saved = true;

		BYTE il2cpp_static_fields_bytes[4] = { 0 };
		OffsetToBytes(Offset::Il2CppClassStaticFieldsOffset, il2cpp_static_fields_bytes);
		BYTE flush_identity_chain0_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockFlushIdentityChain0Offset, flush_identity_chain0_bytes);
		BYTE anim_playable_agent_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockFlushAnimPlayableOffset, anim_playable_agent_bytes);
		static_assert(Offset::MemoryShockEntityKitActorModelOffset <= 0x7F, "Flush actor model requires disp8");
		static_assert(Offset::Il2CppStaticInstanceOffset <= 0x7F, "Flush static instance requires disp8");
		static_assert(Offset::MemoryShockFlushIdentityChain1Offset <= 0x7F, "Flush identity chain1 requires disp8");
		static_assert(Offset::MemoryShockFlushIdentityChain2Offset <= 0x7F, "Flush identity chain2 requires disp8");
		static_assert(Offset::MemoryShockFlushIdentityChain3Offset <= 0x7F, "Flush identity chain3 requires disp8");

		// 自研Shellcode — rbp帧指针 + 被偷字节执行 + jmp返回
		BYTE ShellCode[] = {
			// Prologue: 帧指针 + 保存全寄存器 + 工作区
			0x55, 0x48, 0x89, 0xE5,                         // push rbp; mov rbp, rsp
			0x50, 0x51, 0x52,                               // push rax, rcx, rdx
			0x41,0x50,0x41,0x51,0x41,0x52,0x41,0x53,        // push r8-r11
			0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,        // push r12-r15
			0x56, 0x57,                                     // push rsi, rdi
			0x48,0x81,0xEC,0x80,0x00,0x00,0x00,             // sub rsp, 0x80

			// Debug: shellcode entry counter
			0x48,0xB8, 0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33, // mov rax, entryCounter
			0x48,0xFF,0x00,                                      // inc qword ptr [rax]

			// 身份: EntityKit.actorModel vs 本地 ActorModel
			0x48,0x8B,0x5D,0xF0, 0x48,0x8B,0x5B,
			static_cast<BYTE>(Offset::MemoryShockEntityKitActorModelOffset),
			// TTH->CM->alive[0] 本地玩家ActorModel
			0x48,0xB8, 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, // TTH slot VA placeholder
			0x48,0x8B,0x00,                                 // mov rax,[rax]
			0x48,0x8B,0x80,
			il2cpp_static_fields_bytes[0], il2cpp_static_fields_bytes[1],
			il2cpp_static_fields_bytes[2], il2cpp_static_fields_bytes[3],
			0x48,0x8B,0x40,static_cast<BYTE>(Offset::Il2CppStaticInstanceOffset),
			0x48,0x8B,0x80,
			flush_identity_chain0_bytes[0], flush_identity_chain0_bytes[1],
			flush_identity_chain0_bytes[2], flush_identity_chain0_bytes[3],
			0x48,0x8B,0x40,static_cast<BYTE>(Offset::MemoryShockFlushIdentityChain1Offset),
			0x48,0x8B,0x40,static_cast<BYTE>(Offset::MemoryShockFlushIdentityChain2Offset),
			0x48,0x8B,0x40,static_cast<BYTE>(Offset::MemoryShockFlushIdentityChain3Offset),
			0x48,0x39,0xD8,                                 // cmp rax, rbx
			0x0F,0x85,0x3B,0x00,0x00,0x00,                 // jne .skip

			// AnimPlayable: 旧 FlushState hook 上下文中的观察字段
			0x48,0x8B,0x5D,0xF0,                             // mov rbx,[rbp-0x10]
			0x48,0x8B,0x9B,
			anim_playable_agent_bytes[0], anim_playable_agent_bytes[1],
			anim_playable_agent_bytes[2], anim_playable_agent_bytes[3],
			0x48,0x85,0xDB, 0x74,0x2B,                      // test rbx,rbx; jz .skip

			// Debug: CrossFade call path counter
			0x48,0xB8, 0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44, // mov rax, callCounter
			0x48,0xFF,0x00,                                      // inc qword ptr [rax]

			// CrossFadeInFixedTime(AnimPlayable, targetTagHash, 0, layerIdx, 0)
			0x48,0x89,0xD9,                                 // mov rcx, rbx
			0x8B,0x55,0x40,                                 // mov edx,[rbp+0x40] = targetTagHash
			0x44,0x8B,0x45,0xE8,                           // mov r8d,[rbp-0x18] = layerIdx
			0x66,0x0F,0xEF,0xC9, 0x66,0x0F,0xEF,0xD2,      // pxor xmm1,xmm1; pxor xmm2,xmm2
			0x48,0xB8, 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22, // CrossFadeInFixedTime VA
			0xFF,0xD0,                                     // call rax

			// Epilogue: 恢复工作区 + 寄存器
			0x48,0x81,0xC4,0x80,0x00,0x00,0x00,             // add rsp, 0x80
			0x5F,0x5E,                                     // pop rdi, rsi
			0x41,0x5F,0x41,0x5E,0x41,0x5D,0x41,0x5C,        // pop r15-r12
			0x41,0x5B,0x41,0x5A,0x41,0x59,0x41,0x58,        // pop r11-r8
			0x5A,0x59,0x58, 0x5D,                           // pop rdx,rcx,rax,rbp

			// 执行被偷字节 (FlushState原始前15字节，避免截断 49 8B 49 18)
			0x56, 0x48,0x83,0xEC,0x30,                       // push rsi; sub rsp,0x30
			0x4D,0x89,0xC3,                                 // mov r11, r8
			0x48,0x89,0xC8,                                 // mov rax, rcx
			0x49,0x8B,0x49,0x18,                             // mov rcx,[r9+0x18]

			// jmp [rip+0]; return_addr
			0xFF,0x25,0x00,0x00,0x00,0x00,
			0,0,0,0,0,0,0,0,                               // return addr placeholder
		};

		*(uintptr_t*)&ShellCode[sizeof(ShellCode) - 8] = retAddr;

		BYTE debugInit[kFlushDebugDataSize] = { 0 };
		std::memcpy(debugInit, &kFlushDebugMagic, sizeof(kFlushDebugMagic));
		g_flush_debug_data = FindWritableCodecave(debugInit, sizeof(debugInit), "DebugData");
		if (!g_flush_debug_data)
		{
			printf("[AnimationAccel] 失败 FAILED to allocate debug data!\n");
			return;
		}

		if (!ReadBackEquals(g_flush_debug_data, debugInit, sizeof(debugInit), "DebugData"))
		{
			printf("[AnimationAccel] 失败 FAILED to init debug data!\n");
			g_flush_debug_data = 0;
			return;
		}

		PatchShellcodeImm64(ShellCode, sizeof(ShellCode), kMarkerTthSlot, tthSlotVA);
		PatchShellcodeImm64(ShellCode, sizeof(ShellCode), kMarkerCrossFade, crossFadeVA);
		PatchShellcodeImm64(ShellCode, sizeof(ShellCode), kMarkerEntryCounter, g_flush_debug_data + 0x08);
		PatchShellcodeImm64(ShellCode, sizeof(ShellCode), kMarkerCallCounter, g_flush_debug_data + 0x10);

		flush_codecave = FindWritableCodecave(ShellCode, sizeof(ShellCode), "Shellcode");
		if (flush_codecave)
		{
			g_flush_hook_initialized = true;
			printf("[AnimationAccel] 成功 Shellcode @ 0x%llX (%zu bytes, self-reversed), DebugData @ 0x%llX\n",
				(unsigned long long)flush_codecave, sizeof(ShellCode), (unsigned long long)g_flush_debug_data);
			PrintAnimationAccelCounters("INIT");
		}
		else
		{
			printf("[AnimationAccel] 失败 FAILED to write shellcode!\n");
		}
	}

	VOID EnableAnimationAccel(bool enable)
	{
		if (!enable)
		{
			// [PhaseA isolation] EnableCrossFadeProbeHook removed
		}
		if (enable && !g_actor_kit_cross_fade_in_fixed_time)
		{
			g_actor_kit_cross_fade_in_fixed_time =
				ResolveActorKitMethod("AnimatorCrossFadeInFixTime", 4);
			if (g_actor_kit_cross_fade_in_fixed_time)
			{
				Offset::CrossFadeInFixedTime =
					g_actor_kit_cross_fade_in_fixed_time - Offset::GameAssembly;
				printf("[AnimationAccel] CrossFadeInFixedTime lazy RVA updated: 0x%llX\n",
					(unsigned long long)Offset::CrossFadeInFixedTime);
			}
		}
		if (enable && g_actor_kit_cross_fade_in_fixed_time &&
			Offset::CrossFadeInFixedTime + Offset::GameAssembly != g_actor_kit_cross_fade_in_fixed_time)
		{
			Offset::CrossFadeInFixedTime =
				g_actor_kit_cross_fade_in_fixed_time - Offset::GameAssembly;
		}

		uintptr_t flushAddr = Offset::GameAssembly + Offset::FlushState;
		if (!flush_codecave || !g_flush_hook_initialized)
		{
			if (enable)
			{
				printf("[AnimationAccel] FlushState hook not inited, enabling CrossFade hook only\n");
				// [PhaseA isolation] EnableCrossFadeProbeHook removed
			}
			return;
		}

		if (enable && !g_anim_accel_enabled)
		{
			PrintAnimationAccelCounters("BEFORE_ON");
			// [PhaseA isolation] EnableCrossFadeProbeHook removed
			if (!g_flush_orig_saved)
			{
				if (!mem.Read(flushAddr, g_flush_orig_bytes, sizeof(g_flush_orig_bytes)))
				{
					printf("[AnimationAccel] 读取 FlushState 原始字节失败\n");
					return;
				}
				g_flush_orig_saved = true;
			}

			BYTE current[kFlushStolenSize] = { 0 };
			if (!mem.Read(flushAddr, current, sizeof(current)))
			{
				printf("[AnimationAccel] Enable 前读取 FlushState 失败\n");
				return;
			}
			if (IsPatchedToCodecave(current))
			{
				g_anim_accel_enabled = true;
				printf("[AnimationAccel] ON (already patched)\n");
				return;
			}
			if (std::memcmp(current, g_flush_orig_bytes, sizeof(current)) != 0)
			{
				printf("[AnimationAccel] Enable 前 FlushState 字节异常，停止 patch\n");
				PrintHookBytes("[AnimationAccel] expected:", g_flush_orig_bytes, sizeof(g_flush_orig_bytes));
				PrintHookBytes("[AnimationAccel] actual:  ", current, sizeof(current));
				return;
			}

			BYTE jmp[kFlushStolenSize] = { 0xFF,0x25,0x00,0x00,0x00,0x00,0,0,0,0,0,0,0,0,0x90 };
			*(uintptr_t*)&jmp[6] = flush_codecave;
			if (mem.Write(flushAddr, jmp, sizeof(jmp)) &&
				ReadBackEquals(flushAddr, jmp, sizeof(jmp), "FlushState patch"))
			{
				{ g_anim_accel_enabled = true; printf("[AnimationAccel] ON\n"); PrintAnimationAccelCounters("ON"); }
				// [PhaseA isolation] PrintCrossFadeProbeDebug removed
			}
			else
			{
				printf("[AnimationAccel] ON failed\n");
			}
		}
		else if (!enable && g_anim_accel_enabled)
		{
			if (mem.Write(flushAddr, g_flush_orig_bytes, sizeof(g_flush_orig_bytes)) &&
				ReadBackEquals(flushAddr, g_flush_orig_bytes, sizeof(g_flush_orig_bytes), "FlushState restore"))
			{
				{ g_anim_accel_enabled = false; printf("[AnimationAccel] OFF\n"); PrintAnimationAccelCounters("OFF"); }
				// [PhaseA isolation] PrintCrossFadeProbeDebug removed
			}
			else
			{
				printf("[AnimationAccel] OFF failed\n");
			}
		}
	}

	bool IsAnimationAccelEnabled() { return g_anim_accel_enabled; }

	static const char* GetCurrentGameAssemblyModuleName()
	{
		if (Offset::IsGameAssemblySuper())
		{
			return "GameAssembly_Super.dll";
		}
		if (Offset::IsGameAssemblySuperIBT())
		{
			return "GameAssembly_Super_IBT.dll";
		}
		return "GameAssembly.dll";
	}

	static bool EnsureInitActorKitRefer()
	{
		if (Offset::InitActorKitRefer && Offset::InitActorKitRefer != Offset::MemoryShockUnresolvedRva)
		{
			return true;
		}
		if (!Offset::GameAssembly)
		{
			MemoryShockDebugPrintf("[MemoryShock] GameAssembly base is empty\n");
			return false;
		}

		// CrossFade 内存振入口实际走 TransparentTagHandler TypeInfo 链：
		// GameAssembly + TTH -> klass -> static -> s_instance -> refCharacterMgr -> localActorModel。
		// 版本更新时优先复用 Offset.h 中已维护的 m_TransparentTagHandler，旧签名扫描只做兜底。
		if (Offset::m_TransparentTagHandler)
		{
			const uintptr_t tthClass =
				mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_TransparentTagHandler);
			std::string className;
			if (IsCanonicalUserPtr(tthClass) &&
				ReadKlassName(tthClass, className) &&
				className == "TransparentTagHandler")
			{
				Offset::InitActorKitRefer = Offset::m_TransparentTagHandler;
				MemoryShockDebugPrintf("[MemoryShock] InitActorKitRefer using TransparentTagHandler RVA=0x%llX\n",
					(unsigned long long)Offset::InitActorKitRefer);
				return true;
			}

			MemoryShockDebugPrintf("[MemoryShock] TransparentTagHandler candidate invalid: RVA=0x%llX klass=0x%llX name=%s\n",
				(unsigned long long)Offset::m_TransparentTagHandler,
				(unsigned long long)tthClass,
				className.empty() ? "<unreadable>" : className.c_str());
		}

		const char* moduleName = GetCurrentGameAssemblyModuleName();
		size_t moduleSize = mem.GetBaseSize(moduleName);
		if (!moduleSize)
		{
			MemoryShockDebugPrintf("[MemoryShock] module size failed: %s\n", moduleName);
			return false;
		}

		uint64_t rva = mem.ScanPointer(
			Signature::INIT_ACTOR_KIT_REFER_SIG,
			Offset::GameAssembly,
			moduleSize,
			3);
		if (!rva)
		{
			MemoryShockDebugPrintf("[MemoryShock] InitActorKitRefer signature not found in %s\n", moduleName);
			return false;
		}

		Offset::InitActorKitRefer = rva;
		MemoryShockDebugPrintf("[MemoryShock] InitActorKitRefer RVA updated: 0x%llX\n",
			(unsigned long long)Offset::InitActorKitRefer);
		return true;
	}

	static uintptr_t ReserveMemoryShockCodecave(size_t size, const char* label)
	{
		std::vector<BYTE> zero(size, 0);
		uintptr_t address = FindWritableCodecave(zero.data(), zero.size(), label);
		if (!address)
		{
			MemoryShockDebugPrintf("[MemoryShock] codecave reserve failed: %s size=%zu\n", label, size);
		}
		return address;
	}

	static uintptr_t ReadPointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets)
	{
		uintptr_t current = base;
		for (uintptr_t offset : offsets)
		{
			current = mem.Read<uintptr_t>(current + offset);
			if (!current)
			{
				return 0;
			}
		}
		return current;
	}

	static bool CommitMemoryShockHook(const char* label, uint64_t magic)
	{
		if (!instruction_addr || !vtbale_Knifevtable || !vtbale_ret_address)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] %s invalid hook addresses: inst=0x%llX vtable=0x%llX ret=0x%llX\n",
				label,
				(unsigned long long)instruction_addr,
				(unsigned long long)vtbale_Knifevtable,
				(unsigned long long)vtbale_ret_address);
			return false;
		}

		BYTE magicBytes[16] = { 0 };
		bool magicReadOk = mem.Read(instruction_addr, magicBytes, sizeof(magicBytes));
		std::memcpy(&vtbale_parry_address, magicBytes, sizeof(vtbale_parry_address));
		if (vtbale_parry_address != magic)
		{
			MemoryShockDebugCout("无法注入 请重新尝试");
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] %s magic mismatch: 0x%llX expected=0x%llX readOk=%d base=0x%llX inst=0x%llX\n",
				label,
				(unsigned long long)vtbale_parry_address,
				(unsigned long long)magic,
				magicReadOk ? 1 : 0,
				(unsigned long long)vtbale_jmp_address,
				(unsigned long long)instruction_addr);
			if constexpr (kMemoryShockVerboseLog)
			{
				PrintHookBytes("[MemoryShock] magic bytes:", magicBytes, sizeof(magicBytes));
			}
			return false;
		}

		if (!mem.Write(vtbale_Knifevtable, &instruction_addr, sizeof(instruction_addr)))
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] %s vtable write failed @ 0x%llX\n",
				label,
				(unsigned long long)vtbale_Knifevtable);
			return false;
		}

		uintptr_t patched = mem.Read<uintptr_t>(vtbale_Knifevtable);
		if (patched != instruction_addr)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] %s vtable readback mismatch: 0x%llX expected=0x%llX\n",
				label,
				(unsigned long long)patched,
				(unsigned long long)instruction_addr);
			return false;
		}

		g_memory_shock_hook_ready = true;
		g_memory_shock_enabled = false;
		SkockOf();
		ReportMemoryShockInitSuccess();
		MemoryShockDebugPrintf("[MemoryShock] %s ready: vtable=0x%llX shell=0x%llX ret=0x%llX\n",
			label,
			(unsigned long long)vtbale_Knifevtable,
			(unsigned long long)instruction_addr,
			(unsigned long long)vtbale_ret_address);
		return true;
	}

	bool KnifeStartHook_CrossFade()
	{
		if (g_memory_shock_hook_ready)
		{
			return true;
		}
		if (!EnsureInitActorKitRefer())
		{
			ReportMemoryShockInitFailure();
			return false;
		}

		static constexpr size_t kCrossFadeCodecaveSize = kMemoryShockControlBytes + kMemoryShockCrossFadeShellcodeSize +
			sizeof(MemoryShockLegacyMetadata);

		uintptr_t temp = mem.ReadMultiLevel<uintptr_t>(
			Offset::GameAssembly,
			Offset::InitActorKitRefer,
			Offset::Il2CppClassStaticFieldsOffset,
			Offset::Il2CppStaticInstanceOffset,
			Offset::MemoryShockTransparentTagHandlerActorKitOffset,
			Offset::ActorModel.ActorKitS.InteractableDevice,
			0x0);
		if (!temp)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] CrossFade vtable chain failed\n");
			return false;
		}

		vtbale_Knifevtable = temp + Offset::MemoryShockInteractableVtableSlotOffset;
		uintptr_t currentVtableTarget = mem.Read<uintptr_t>(vtbale_Knifevtable);
		if (!currentVtableTarget)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] CrossFade ret read failed @ 0x%llX\n",
				(unsigned long long)vtbale_Knifevtable);
			return false;
		}
		vtbale_ret_address = currentVtableTarget;

		MemoryShockLegacyMetadata existingCrossFadeMetadata{};
		if (TryReadMemoryShockMetadata(
			currentVtableTarget,
			kMemoryShockCrossFadeShellcodeSize,
			kMemoryShockCrossFadeMetadataMagic,
			vtbale_Knifevtable,
			existingCrossFadeMetadata) ||
			TryReadMemoryShockMetadata(
				currentVtableTarget,
				kMemoryShockCrossFadeSwitchShellcodeSize,
				kMemoryShockCrossFadeMetadataMagic,
				vtbale_Knifevtable,
				existingCrossFadeMetadata))
		{
			// 退出辅助但不退出游戏时，vtable 仍然指向上一次写入的 CrossFade shellcode。
			// 这里直接接管旧 hook，避免恢复后重新扫描/写入 codecave。
			instruction_addr = static_cast<uintptr_t>(existingCrossFadeMetadata.shellEntry);
			vtbale_jmp_address = instruction_addr >= kMemoryShockControlBytes
				? instruction_addr - kMemoryShockControlBytes
				: 0;
			vtbale_ret_address = static_cast<uintptr_t>(existingCrossFadeMetadata.originalRet);
			if (!instruction_addr || !vtbale_jmp_address || !vtbale_ret_address)
			{
				ReportMemoryShockInitFailure();
				MemoryShockDebugPrintf("[MemoryShock] CrossFade existing hook metadata invalid: shell=0x%llX base=0x%llX ret=0x%llX\n",
					(unsigned long long)instruction_addr,
					(unsigned long long)vtbale_jmp_address,
					(unsigned long long)vtbale_ret_address);
				return false;
			}

			g_memory_shock_hook_ready = true;
			g_memory_shock_enabled = false;
			g_memory_shock_active_mode = 0;
			SkockOf();
			if (g_memory_shock_berserk_mode.load())
			{
				PatchCrossFadeBerserkModeBytes(true);
			}
			ReportMemoryShockInitSuccess();
			MemoryShockDebugPrintf("[MemoryShock] CrossFade reused existing hook: vtable=0x%llX shell=0x%llX base=0x%llX ret=0x%llX\n",
				(unsigned long long)vtbale_Knifevtable,
				(unsigned long long)instruction_addr,
				(unsigned long long)vtbale_jmp_address,
				(unsigned long long)vtbale_ret_address);
			return true;
		}

		if (!RecoverMemoryShockOriginalRetFromMetadata(
			"CrossFade",
			currentVtableTarget,
			vtbale_Knifevtable,
			vtbale_ret_address))
		{
			return false;
		}

		BYTE Char[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::InitActorKitRefer, Char);

		uintptr_t crossFadeAddress = ResolveAnimPlayableMethod("CrossFadeInFixedTime", 7);
		if (!IsGameAssemblyCodePtr(crossFadeAddress))
		{
			const uint64_t fallbackRva = GetAnimPlayableCrossFadeInFixedTimeFallbackRva();
			crossFadeAddress = Offset::GameAssembly + fallbackRva;
			MemoryShockDebugPrintf("[MemoryShock] CrossFade fallback RVA=0x%llX VA=0x%llX\n",
				(unsigned long long)fallbackRva,
				(unsigned long long)crossFadeAddress);
		}
		else
		{
			MemoryShockDebugPrintf("[MemoryShock] CrossFade resolver RVA=0x%llX VA=0x%llX\n",
				(unsigned long long)(crossFadeAddress - Offset::GameAssembly),
				(unsigned long long)crossFadeAddress);
		}
		Offset::CrossFadeInFixedTime = crossFadeAddress - Offset::GameAssembly;

		BYTE CrossFadeInFixedTime[8] = { 0 };
		OffsetToBytesBE(crossFadeAddress, CrossFadeInFixedTime);

		BYTE JMP[8] = { 0 };
		OffsetToBytesBE(vtbale_ret_address, JMP);

		BYTE actor_kit_offset_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKit), actor_kit_offset_bytes);

		BYTE actor_kit_BreakData_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKitS.actorKitBreakData), actor_kit_BreakData_bytes);

		BYTE actor_kit_AnimPlayable_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKitS.AnimPlayableAgent), actor_kit_AnimPlayable_bytes);
		BYTE il2cpp_static_fields_bytes[4] = { 0 };
		OffsetToBytes(Offset::Il2CppClassStaticFieldsOffset, il2cpp_static_fields_bytes);
		BYTE tth_character_manager_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockTransparentTagHandlerCharacterManagerOffset, tth_character_manager_bytes);
		static_assert(Offset::MemoryShockHookContextEntityKitOffset <= 0x7F, "CrossFade hook context requires disp8");
		static_assert(Offset::MemoryShockEntityKitActorModelOffset <= 0x7F, "CrossFade actor model requires disp8");
		static_assert(Offset::Il2CppStaticInstanceOffset <= 0x7F, "CrossFade static instance requires disp8");
		static_assert(Offset::MemoryShockCharacterManagerMainPlayerOffset <= 0x7F, "CrossFade main player requires disp8");
		static_assert(Offset::MemoryShockCacheDelayBreakTypeOffset <= 0x7F, "CrossFade break cache requires disp8");

		const BYTE* crossFadeControlPatch = g_memory_shock_berserk_mode.load()
			? kCrossFadeControlNopBytes
			: kCrossFadeControlGateBytes;

		BYTE ShellCode[] = {
			0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57,
			0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,
			0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
			0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00,
			0x48, 0x89, 0x0C, 0x24,
			0x48, 0x89, 0x54, 0x24, 0x08,
			0x4C, 0x89, 0x44, 0x24, 0x10,
			0x4C, 0x89, 0x4C, 0x24, 0x18,
			0xC5, 0xF8, 0x29, 0x74, 0x24, 0x40,
			0xC5, 0xF8, 0x29, 0x7C, 0x24, 0x50,
			0xC5, 0x78, 0x29, 0x84, 0x24, 0x60, 0x00, 0x00, 0x00,
			0xC5, 0x78, 0x29, 0x8C, 0x24, 0x70, 0x00, 0x00, 0x00,
			// CrossFade动作加速开关控制：
			// Hook::SkockOn() 写 vtbale_jmp_address[0] = 1 时开启；
			// Hook::SkockOf() 写 vtbale_jmp_address[0] = 0 时关闭。
			// 这里通过 RIP 相对负偏移读取 codecave 头部 control byte，
			// 不命中时直接跳到恢复现场，跳过 CacheDelayBreakType 写入和 CrossFadeInFixedTime 调用。
			// 变态模式开启时，这 13 字节会被替换为 NOP，动态去掉 SkockOn/SkockOf 限制。
			crossFadeControlPatch[0], crossFadeControlPatch[1], crossFadeControlPatch[2],
			crossFadeControlPatch[3], crossFadeControlPatch[4], crossFadeControlPatch[5],
			crossFadeControlPatch[6], crossFadeControlPatch[7], crossFadeControlPatch[8],
			crossFadeControlPatch[9], crossFadeControlPatch[10], crossFadeControlPatch[11],
			crossFadeControlPatch[12],
			0x4C, 0x8B, 0x59, static_cast<BYTE>(Offset::MemoryShockHookContextEntityKitOffset),
			0x4D, 0x8B, 0x5B, static_cast<BYTE>(Offset::MemoryShockEntityKitActorModelOffset),
			0x48, 0xB8,
			Char[7], Char[6], Char[5], Char[4], Char[3], Char[2], Char[1], Char[0],
			0x48, 0x8B, 0x00,
			0x48, 0x8B, 0x80,
			il2cpp_static_fields_bytes[0], il2cpp_static_fields_bytes[1],
			il2cpp_static_fields_bytes[2], il2cpp_static_fields_bytes[3],
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::Il2CppStaticInstanceOffset),
			// TransparentTagHandler.refCharacterMgr -> CharacterManager.mainPlayerActorModel.
			0x48, 0x8B, 0x80,
			tth_character_manager_bytes[0], tth_character_manager_bytes[1],
			tth_character_manager_bytes[2], tth_character_manager_bytes[3],
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::MemoryShockCharacterManagerMainPlayerOffset),
			0x4C, 0x39, 0xD8,
			0x75, 0x7F,
			0x48, 0x8B, 0x80,
			actor_kit_offset_bytes[0], actor_kit_offset_bytes[1], actor_kit_offset_bytes[2], actor_kit_offset_bytes[3],
			0x48, 0x8B, 0x80,
			actor_kit_BreakData_bytes[0], actor_kit_BreakData_bytes[1], actor_kit_BreakData_bytes[2], actor_kit_BreakData_bytes[3],
			0xC7, 0x40, static_cast<BYTE>(Offset::MemoryShockCacheDelayBreakTypeOffset), 0x00, 0x30, 0x00, 0x00,
			0x4D, 0x8B, 0x9B,
			actor_kit_offset_bytes[0], actor_kit_offset_bytes[1], actor_kit_offset_bytes[2], actor_kit_offset_bytes[3],
			0x4D, 0x8B, 0x9B,
			actor_kit_AnimPlayable_bytes[0], actor_kit_AnimPlayable_bytes[1], actor_kit_AnimPlayable_bytes[2], actor_kit_AnimPlayable_bytes[3],
			0x48, 0x81, 0xEC, 0x50, 0x00, 0x00, 0x00,
			// CrossFadeInFixedTime stack args:
			// [rsp+0x28] = 50.0f，原为 5.0f，推断为播放速度/过渡速度类参数。
			//0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00,  // 旧 fixedTime = 0.0f
			0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x3F,  // fixedTime = 0.5f
			//0xC7, 0x44, 0x24, 0x28, 0x00, 0x00, 0x20, 0x41,  // 这一段是10倍加速
			0xC7, 0x44, 0x24, 0x28, 0x00, 0x00, 0xA0, 0x40, // 这一段是五倍加速
			//0xC7, 0x44, 0x24, 0x28, 0x00, 0x00, 0x48, 0x42,  // 这一段是50倍加速
			0x48, 0xC7, 0x44, 0x24, 0x30, 0x00, 0x00, 0x00, 0x00,  // sourceType = 0
			0xC6, 0x44, 0x24, 0x38, 0x01,                          // forceSyncLoad = true
			0x4C, 0x89, 0xD9,
			0x44, 0x89, 0xEA,
			0xC7, 0x44, 0x24, 0x48, 0xCD, 0xCC, 0x4C, 0x3D,
			0xF3, 0x0F, 0x10, 0x7C, 0x24, 0x48,
			0x41, 0xB9, 0x00, 0x00, 0x00, 0x00,
			0x41, 0xB8, 0x12, 0x00, 0x00, 0x00,
			0xC5, 0xF8, 0x28, 0xD7,
			0x48, 0xB8,
			CrossFadeInFixedTime[7], CrossFadeInFixedTime[6], CrossFadeInFixedTime[5], CrossFadeInFixedTime[4],
			CrossFadeInFixedTime[3], CrossFadeInFixedTime[2], CrossFadeInFixedTime[1], CrossFadeInFixedTime[0],
			0xFF, 0xD0,
			0x48, 0x81, 0xC4, 0x50, 0x00, 0x00, 0x00,
			0x48, 0x8B, 0x0C, 0x24,
			0x48, 0x8B, 0x54, 0x24, 0x08,
			0x4C, 0x8B, 0x44, 0x24, 0x10,
			0x4C, 0x8B, 0x4C, 0x24, 0x18,
			0xC5, 0xF8, 0x28, 0x74, 0x24, 0x40,
			0xC5, 0xF8, 0x28, 0x7C, 0x24, 0x50,
			0xC5, 0x78, 0x28, 0x84, 0x24, 0x60, 0x00, 0x00, 0x00,
			0xC5, 0x78, 0x28, 0x8C, 0x24, 0x70, 0x00, 0x00, 0x00,
			0x48, 0x81, 0xC4, 0x80, 0x00, 0x00, 0x00,
			0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
			0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58,
			0x5F, 0x5E, 0x5D, 0x5B, 0x5A, 0x59, 0x58,
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP[7], JMP[6], JMP[5], JMP[4], JMP[3], JMP[2], JMP[1], JMP[0],
		};
		static_assert(sizeof(ShellCode) == kMemoryShockCrossFadeShellcodeSize, "CrossFade shellcode size changed");

		static constexpr int kCrossFadeMaxCodecaveAttempts = 24;
		uint64_t lastAllocFailLogTick = 0;
		bool shellcodeWritten = false;
		for (int attempt = 0; attempt < kCrossFadeMaxCodecaveAttempts; ++attempt)
		{
			Sleep(100);
			vtbale_jmp_address = FindMemoryShockCodecave(
				kCrossFadeCodecaveSize,
				"MemoryShock CrossFade reserve");
			if (!vtbale_jmp_address)
			{
				const uint64_t now = GetTickCount64();
				if (now - lastAllocFailLogTick >= 3000)
				{
					ReportMemoryShockInitFailure();
					MemoryShockDebugPrintf("[MemoryShock] CrossFade codecave reserve failed, retrying size=%zu\n",
						kCrossFadeCodecaveSize);
					lastAllocFailLogTick = now;
				}
				continue;
			}

			instruction_addr = vtbale_jmp_address + kMemoryShockControlBytes;

			MemoryShockLegacyMetadata crossFadeMetadata{};
			crossFadeMetadata.magic = kMemoryShockCrossFadeMetadataMagic;
			crossFadeMetadata.originalRet = vtbale_ret_address;
			crossFadeMetadata.shellEntry = instruction_addr;
			crossFadeMetadata.vtableAddress = vtbale_Knifevtable;

			if (WriteMemoryShockShellcodeBlock("CrossFade", vtbale_jmp_address, ShellCode, sizeof(ShellCode), &crossFadeMetadata))
			{
				shellcodeWritten = true;
				break;
			}

			ReleaseCodecaveReservation(vtbale_jmp_address);
			MarkBadCodecave(vtbale_jmp_address);
			MemoryShockDebugPrintf("[MemoryShock] CrossFade shellcode write failed, trying next codecave\n");
			vtbale_jmp_address = 0;
		}
		if (!shellcodeWritten)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] CrossFade shellcode write failed after %d attempts\n",
				kCrossFadeMaxCodecaveAttempts);
			return false;
		}
		bool committed = CommitMemoryShockHook("CrossFade", 4708326859909255504ull);
		if (committed)
		{
			g_memory_shock_active_mode = 0;
		}
		return committed;
	}

	bool VirtualShockHOOK()
	{
		if (g_memory_shock_hook_ready)
		{
			return true;
		}
		if (!EnsureInitActorKitRefer())
		{
			ReportMemoryShockInitFailure();
			return false;
		}

		uint64_t lastAllocFailLogTick = 0;
		do
		{
			Sleep(100);
			vtbale_jmp_address = DMA_AllocateMemory(400, mem.GetCurrentProcessPID());
			if (!vtbale_jmp_address)
			{
				const uint64_t now = GetTickCount64();
				if (now - lastAllocFailLogTick >= 3000)
				{
					ReportMemoryShockInitFailure();
					MemoryShockDebugPrintf("[MemoryShock] Virtual DMA_AllocateMemory failed, retrying\n");
					lastAllocFailLogTick = now;
				}
			}
		} while (!vtbale_jmp_address);

		uintptr_t refer = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::InitActorKitRefer);
		if (!refer)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Virtual refer read failed\n");
			return false;
		}

		vtbale_Knifevtable = refer + Offset::MemoryShockVirtualVtableSlotOffset;
		vtbale_ret_address = mem.Read<uintptr_t>(vtbale_Knifevtable);
		if (!vtbale_ret_address)
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Virtual ret read failed @ 0x%llX\n",
				(unsigned long long)vtbale_Knifevtable);
			return false;
		}

		BYTE Char[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::m_CharacterManager, Char);
		BYTE Refer[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::InitActorKitRefer, Refer);
		BYTE CALL_A[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockVirtualCallARva, CALL_A);
		BYTE CALL_B[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockVirtualCallBRva, CALL_B);
		BYTE CALL_C[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockVirtualCallCRva, CALL_C);
		BYTE JMP[8] = { 0 };
		OffsetToBytesBE(Offset::GameAssembly + Offset::MemoryShockVirtualReturnRva, JMP);

		BYTE actor_kit_offset_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKit), actor_kit_offset_bytes);
		BYTE actor_kit_break_data_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(Offset::ActorModel.ActorKitS.actorKitBreakData), actor_kit_break_data_bytes);
		BYTE il2cpp_static_fields_bytes[4] = { 0 };
		OffsetToBytes(Offset::Il2CppClassStaticFieldsOffset, il2cpp_static_fields_bytes);
		BYTE virtual_vtable_slot_bytes[4] = { 0 };
		OffsetToBytes(Offset::MemoryShockVirtualVtableSlotOffset, virtual_vtable_slot_bytes);
		static_assert(Offset::MemoryShockHookContextEntityKitOffset <= 0x7F, "Virtual hook context requires disp8");
		static_assert(Offset::MemoryShockEntityKitActorModelOffset <= 0x7F, "Virtual actor model requires disp8");
		static_assert(Offset::Il2CppStaticInstanceOffset <= 0x7F, "Virtual static instance requires disp8");
		static_assert(Offset::MemoryShockCharacterManagerMainPlayerOffset <= 0x7F, "Virtual main player requires disp8");
		static_assert(Offset::MemoryShockVirtualBreakCacheOffset <= 0x7F, "Virtual break cache requires disp8");

		instruction_addr = vtbale_jmp_address + 0x16;
		uintptr_t target_addr = vtbale_jmp_address;
		uintptr_t relative_offset = target_addr - (instruction_addr + 7);
		BYTE offset_bytes[4] = { 0 };
		OffsetToBytes(static_cast<uint32_t>(relative_offset), offset_bytes);

		BYTE ShellCode[] = {
			0x4C, 0x8B, 0x25,
			offset_bytes[0], offset_bytes[1], offset_bytes[2], offset_bytes[3],
			0x49, 0x83, 0xFC, 0x01,
			0x75, 0x3E,
			0x48, 0x8B, 0x41, static_cast<BYTE>(Offset::MemoryShockHookContextEntityKitOffset),
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::MemoryShockEntityKitActorModelOffset),
			0x4C, 0x8B, 0xE0,
			0x48, 0xA1,
			Char[7], Char[6], Char[5], Char[4], Char[3], Char[2], Char[1], Char[0],
			0x48, 0x8B, 0x80,
			il2cpp_static_fields_bytes[0], il2cpp_static_fields_bytes[1],
			il2cpp_static_fields_bytes[2], il2cpp_static_fields_bytes[3],
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::Il2CppStaticInstanceOffset),
			0x48, 0x8B, 0x40, static_cast<BYTE>(Offset::MemoryShockCharacterManagerMainPlayerOffset),
			0x49, 0x39, 0xC4,
			0x75, 0x15,
			0x48, 0x8B, 0x80,
			actor_kit_offset_bytes[0], actor_kit_offset_bytes[1], actor_kit_offset_bytes[2], actor_kit_offset_bytes[3],
			0x48, 0x8B, 0x80,
			actor_kit_break_data_bytes[0], actor_kit_break_data_bytes[1],
			actor_kit_break_data_bytes[2], actor_kit_break_data_bytes[3],
			0xC7, 0x40, static_cast<BYTE>(Offset::MemoryShockVirtualBreakCacheOffset), 0x00, 0x30, 0x00, 0x00,
			0x48, 0xA1,
			Refer[7], Refer[6], Refer[5], Refer[4], Refer[3], Refer[2], Refer[1], Refer[0],
			0x48, 0x8D, 0x80,
			virtual_vtable_slot_bytes[0], virtual_vtable_slot_bytes[1],
			virtual_vtable_slot_bytes[2], virtual_vtable_slot_bytes[3],
			0x4D, 0x31, 0xE4,
			0x44, 0x89, 0x4C, 0x24, 0x20,
			0x44, 0x89, 0x44, 0x24, 0x18,
			0x89, 0x54, 0x24, 0x10,
			0x48, 0x89, 0x4C, 0x24, 0x08,
			0x55, 0x53, 0x56, 0x57,
			0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57,
			0x48, 0x81, 0xEC, 0xB8, 0x00, 0x00, 0x00,
			0x48, 0x8D, 0x6C, 0x24, 0x50,
			0x41, 0x8B, 0xF9,
			0x45, 0x8B, 0xF0,
			0x8B, 0xF2,
			0x48, 0x8B, 0xD9,
			0x8B, 0x04, 0x24,
			0x48, 0x83, 0xEC, 0x10,
			0x4C, 0x8D, 0x7C, 0x24, 0x50,
			0x41, 0x8B, 0x07,
			0x45, 0x33, 0xC0,
			0xBA, 0x35, 0x00, 0x00, 0x00,
			0xB9, 0x82, 0x00, 0x00, 0x00,
			0xFF, 0x15, 0x02, 0x00, 0x00, 0x00,
			0xEB, 0x08,
			CALL_A[7], CALL_A[6], CALL_A[5], CALL_A[4], CALL_A[3], CALL_A[2], CALL_A[1], CALL_A[0],
			0x48, 0x85, 0xC0,
			0x74, 0x7D,
			0x33, 0xD2,
			0xB9, 0xB5, 0x20, 0x00, 0x00,
			0xFF, 0x15, 0x02, 0x00, 0x00, 0x00,
			0xEB, 0x08,
			CALL_B[7], CALL_B[6], CALL_B[5], CALL_B[4], CALL_B[3], CALL_B[2], CALL_B[1], CALL_B[0],
			0x48, 0x85, 0xC0,
			0x74, 0x61,
			0x45, 0x33, 0xE4,
			0x4C, 0x89, 0x64, 0x24, 0x48,
			0x8B, 0x8D, 0xE8, 0x00, 0x00, 0x00,
			0x89, 0x4C, 0x24, 0x40,
			0x8B, 0x8D, 0xE0, 0x00, 0x00, 0x00,
			0x89, 0x4C, 0x24, 0x38,
			0x8B, 0x8D, 0xD8, 0x00, 0x00, 0x00,
			0x89, 0x4C, 0x24, 0x30,
			0x8B, 0x8D, 0xD0, 0x00, 0x00, 0x00,
			0x89, 0x4C, 0x24, 0x28,
			0x89, 0x7C, 0x24, 0x20,
			0x45, 0x8B, 0xCE,
			0x44, 0x8B, 0xC6,
			0x48, 0x8B, 0xD3,
			0x48, 0x8B, 0xC8,
			0xFF, 0x15, 0x02, 0x00, 0x00, 0x00,
			0xEB, 0x08,
			CALL_C[7], CALL_C[6], CALL_C[5], CALL_C[4], CALL_C[3], CALL_C[2], CALL_C[1], CALL_C[0],
			0x48, 0x8D, 0x65, 0x68,
			0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C,
			0x5F, 0x5E, 0x5B, 0x5D,
			0xC3,
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			JMP[7], JMP[6], JMP[5], JMP[4], JMP[3], JMP[2], JMP[1], JMP[0],
			0x24, 0x20,
			0x44, 0x89, 0x44, 0x24, 0x18,
			0x89, 0x54, 0x24, 0x10,
			0x48, 0x89, 0x4C, 0x24, 0x08,
		};

		if (!mem.Write(instruction_addr, ShellCode, sizeof(ShellCode)))
		{
			ReportMemoryShockInitFailure();
			MemoryShockDebugPrintf("[MemoryShock] Virtual shellcode write failed\n");
			return false;
		}
		bool committed = CommitMemoryShockHook("Virtual", 5332261958322588492ull);
		if (committed)
		{
			g_memory_shock_active_mode = 2;
		}
		return committed;
	}

	VOID restore()
	{
		if (vtbale_Knifevtable && vtbale_ret_address)
		{
			mem.Write<uintptr_t>(vtbale_Knifevtable, vtbale_ret_address);
		}
		if (instruction_addr)
		{
			std::vector<BYTE> zero(0x600, 0);
			mem.Write(instruction_addr, zero.data(), zero.size());
		}
		g_memory_shock_hook_ready = false;
		g_memory_shock_enabled = false;
		g_memory_shock_active_mode = -1;
		ResetMemoryShockResultLog();
	}

	void SkockOn()
	{
		if (vtbale_jmp_address)
		{
			mem.Write<BYTE>(vtbale_jmp_address, 1);
		}
	}

	void SkockOf()
	{
		if (vtbale_jmp_address)
		{
			mem.Write<BYTE>(vtbale_jmp_address, 0);
		}
	}

	bool InitMemoryShockHook()
	{
		if (g_memory_shock_hook_ready.load())
		{
			return true;
		}
		bool expected = false;
		if (!g_memory_shock_initializing.compare_exchange_strong(expected, true))
		{
			return false;
		}

		bool ok = false;
		Function::Shock::MemoryShockMode = 0;
		if (Offset::IsGameAssemblyDefault())
		{
			ok = VirtualShockHOOK();
		}
		else
		{
			ok = KnifeStartHook_CrossFade();
		}
		g_memory_shock_initializing.store(false);
		return ok && g_memory_shock_hook_ready.load();
	}

	void SetMemoryShockEnabled(bool enable)
	{
		if (enable && !InitMemoryShockHook())
		{
			g_memory_shock_enabled = false;
			return;
		}

		if (!g_memory_shock_hook_ready)
		{
			g_memory_shock_enabled = false;
			return;
		}

		g_memory_shock_enabled = enable;
		if (enable)
		{
			SkockOn();
		}
		else
		{
			SkockOf();
		}
	}

	void SetMemoryShockBerserkMode(bool enable)
	{
		if (enable && !Function::Shock::M_ShockKnife)
		{
			enable = false;
		}
		g_memory_shock_berserk_mode = enable;
		Function::Shock::内存振变态模式 = enable;

		// 只对 CrossFade动作加速生效；Legacy常驻写入和普通 GameAssembly Virtual 模式不 patch。
		// 如果内存振已经初始化，直接动态替换 shellcode 中的 13 字节控制判断。
		// enable=true  写 NOP，去掉 SkockOn/SkockOf 限制；
		// enable=false 写回 cmp control byte + jne，恢复 SkockOn/SkockOf 限制。
		if (g_memory_shock_active_mode.load() == 0 &&
			(Offset::IsGameAssemblySuper() || Offset::IsGameAssemblySuperIBT()) &&
			g_memory_shock_hook_ready.load())
		{
			PatchCrossFadeBerserkModeBytes(enable);
		}
	}

	bool IsMemoryShockReady()
	{
		return g_memory_shock_hook_ready.load();
	}

	bool IsMemoryShockEnabled()
	{
		return g_memory_shock_enabled.load();
	}
}

