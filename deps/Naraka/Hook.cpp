#include "Hook.h"
#include "memory.h"

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

	VOID InitVtbaleHook() {
		
		vtbale_jmp_address = Offset::NarakaBladepoint + 0x55E5C; //0x565F4

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
		vtbale_parry_address = mem.Read<uintptr_t>(vtbale_parry_address + 0x18);
		vtbale_parry_address = mem.Read<uintptr_t>(vtbale_parry_address + 0xC0);
		vtbale_parry_address = mem.Read<uintptr_t>(vtbale_parry_address + 0x90);

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
	 
	void KnifeStartHook()
	{
		vtbale_jmp_address = Offset::NarakaBladepoint + 0x55E5C; 

		Global::WorldPtr.Knifevtable = Offset::GameAssembly + Offset::InitActorKitRefer;

		BYTE Char[8] = { 0 };
		HexChangeByte(n2hexstr(Offset::GameAssembly + Offset::m_CharacterManager,16Ui64), true, 8, Char);

		BYTE Refer[8] = { 0 };
		HexChangeByte(n2hexstr(Offset::GameAssembly + Offset::InitActorKitRefer, 16Ui64), true, 8, Refer);

		BYTE ShellCode[] =
		{
			0x41,0x81,0xF9,0x7E,0x29,0x25,0xAF,
			0x75,0x52,
			0x48,0x83,0xFA,0x00,
			0x75,0x4C,
			0x48,0xA1,Char[7], Char[6], Char[5], Char[4], Char[3], Char[2], Char[1], Char[0],
			0x48,0x8B,0x80,0xB8,0x00,0x00,0x00,
			0x48,0x8B,0x40,0x08,
			0x48,0x8B,0x40,0x18,
			0x48,0x8B,0x80,Offset::ActorModel.ActorKit, 0x00, 0x00,0x00,
			0x48,0x8B,0x80,Offset::ActorModel.ActorKitS.HitSimulateData, 0x00, 0x00,0x00,
			0xC7,0x40,0x10,0x00,0x30,0x00,0x00,
			0x48,0xA1,Refer[7], Refer[6], Refer[5], Refer[4], Refer[3], Refer[2], Refer[1], Refer[0],
			0x48,0x8D,0x80,0x98,0x01,0x00,0x00,
			0x4C,0x8B,0x25,0x0D,0x00,0x00,0x00,
			0x4C,0x89,0x20,
			0x4D,0x31,0xE4,
			0xFF,0x35,0x01,0x00,0x00,0x00,
			0xC3
		};
	
		if (!mem.Write(vtbale_jmp_address, ShellCode, sizeof(ShellCode)))
		{
			printf("无法写入 shellcode\n");
		}
		else
		{
			printf("成功写入 shellcode\n");
		}
	}

	VOID vtbale(bool enable)
	{
		if (enable == true)
		{
			mem.GetShellcode().function( Global::WorldPtr.Knifevtable, vtbale_jmp_address, "NarakaBladepoint.exe");
			
		}
	}
}
	
