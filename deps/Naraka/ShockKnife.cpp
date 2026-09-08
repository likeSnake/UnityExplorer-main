#include "ShockKnife.h"
#include "log.h"
#include "landing.h"
#include "Data.h"
#include "ShuangDao.h"
#include "changqiang.h"
#include "xulihou.h"
#include <cwchar>
#include <random>

namespace {
	void SendShockKeyDown(int key) {
		if (key >= 0) {
			root_sendKeyPress(key);
			return;
		}
		int vk = -key;
		switch (vk) {
		case VK_LBUTTON:
			root_kmNet_mouse_left(1);
			break;
		case VK_RBUTTON:
			root_kmNet_mouse_right(1);
			break;
		case VK_MBUTTON:
			root_kmNet_mouse_middle(1);
			break;
		case VK_XBUTTON1:
			root_kmNet_mouse_side(1);
			break;
		case VK_XBUTTON2:
			root_kmNet_mouse_side2(1);
			break;
		default:
			break;
		}
	}

	void SendShockKeyUp(int key) {
		if (key >= 0) {
			root_sendKeyUp(key);
			return;
		}
		int vk = -key;
		switch (vk) {
		case VK_LBUTTON:
			root_kmNet_mouse_left(0);
			break;
		case VK_RBUTTON:
			root_kmNet_mouse_right(0);
			break;
		case VK_MBUTTON:
			root_kmNet_mouse_middle(0);
			break;
		case VK_XBUTTON1:
			root_kmNet_mouse_side(0);
			break;
		case VK_XBUTTON2:
			root_kmNet_mouse_side2(0);
			break;
		default:
			break;
		}
	}
}


void 振刀逻辑::BlueToRed()
{
	SendShockKeyDown(Function::Shock::振刀按键);
	Sleep(10);
	SendShockKeyUp(Function::Shock::振刀按键);
	/*if (振刀按鍵 == 0)
	{
		root_sendKeyPress(KEY_G);
		Sleep(10);
		root_sendKeyUp(KEY_G);
	}
	else if (振刀按鍵 == 1)
	{
		root_kmNet_mouse_side(1);
		Sleep(20);
		root_kmNet_mouse_side(0);
	}*/
}
void 振刀逻辑::DoubileBondShock()
{
	if (IsHokeyEx(VK_LBUTTON))
	{
		root_kmNet_mouse_left(1);
		Sleep(1);
		root_kmNet_mouse_right(1);
		Sleep(30);
		root_kmNet_mouse_left(0);
		Sleep(1);
		root_kmNet_mouse_right(0);
	}
	if (IsHokeyEx(VK_RBUTTON))
	{
		root_kmNet_mouse_right(1);
		Sleep(1);
		root_kmNet_mouse_left(1);
		Sleep(30);
		root_kmNet_mouse_right(0);
		Sleep(1);
		root_kmNet_mouse_left(0);
	}
}
void 振刀逻辑::JumpShock()
{
	root_sendKeyPress(KEY_SPACEBAR);
	Sleep(15);
	BlueToRed();
	root_sendKeyUp(KEY_SPACEBAR);
}
void 振刀逻辑::长闪A2(int Atype, float mSleep) {
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	//MyLog("S闪");
	Sleep(mSleep);
	if (Atype == 0) {
		root_kmNet_mouse_left(1);
		Sleep(13);
		root_kmNet_mouse_left(0);
	}
	else {
		//MyLog("右键");
		root_kmNet_mouse_right(1);
		Sleep(30);
		root_kmNet_mouse_right(0);
	}
	Sleep(50);
	//MyLog("结束");
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
}
// 0左A 1右A
void 振刀逻辑::cA或长闪A(int type, int spTime)
{
	bool successflag = false;
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 350;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyActionName.find(L"_crouch_idle_")) {
			MyLog("进入下蹲");
			Sleep(spTime);
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_C);
			Sleep(20);
			root_sendKeyUp(KEY_C);
			Sleep(10);
			if (type == 0) {
				root_kmNet_mouse_left(1);
				Sleep(13);
				root_kmNet_mouse_left(0);
			}
			else {
				root_kmNet_mouse_right(1);
				Sleep(13);
				root_kmNet_mouse_right(0);
			}
			successflag = true;
			break;
		}
	}
	if (successflag) {
		root_sendKeyPress(KEY_C); Sleep(20);   root_sendKeyUp(KEY_C);
	}
	else {
		MyLog("起蹲1\n");
		root_sendKeyUp(KEY_C);
	}
}
void 振刀逻辑::SquatShock()
{
	root_sendKeyPress(KEY_C);
	Sleep(10);
	BlueToRed();
	root_sendKeyUp(KEY_C);
}
void 振刀逻辑::MemShock()
{
	/*Hook::vtbale(true);*/
	/*Memory::WPM(Global::WorldPtr.Knifevtable, Global::WorldPtr.FlushAddr);*/
	root_sendKeyPress(KEY_SPACEBAR);
	MouseLeftEx(true);
	MouseRightEx(true);
	Sleep(1);
	root_sendKeyUp(KEY_SPACEBAR);
	Sleep(20);
	MouseLeftEx(false);
	MouseRightEx(false);
	/*	Memory::WPM(Global::WorldPtr.Knifevtable, Global::WorldPtr.FlushAddr);*/
}
void 振刀逻辑::TianhaiF2()
{
	root_sendKeyPress(KEY_SPACEBAR);
	Sleep(1);
	root_sendKeyPress(KEY_F);
	Sleep(25);
	root_sendKeyUp(KEY_F);
	root_sendKeyUp(KEY_SPACEBAR);
}
void 振刀逻辑::左右键直接松开() {
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_right(0);
}
void 振刀逻辑::CollideKnife()
{
	MouseRelease();
}
void 振刀逻辑::ShortDodge(uint32_t Key)
{
	root_sendKeyUp(Key);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(1);
	root_sendKeyPress(Key);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(65);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(Key);
}
void 振刀逻辑::双键闪(uint32_t Key1, uint32_t Key2)
{
	root_sendKeyUp(Key1);
	root_sendKeyUp(Key2);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(1);

	root_sendKeyPress(Key1);
	root_sendKeyPress(Key2);
	Sleep(5);

	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(65);
	root_sendKeyUp(KEY_LEFTSHIFT);

	root_sendKeyUp(Key1);
	root_sendKeyUp(Key2);
}
void 振刀逻辑::LongDodge(uint32_t Key)
{
	root_sendKeyPress(Key);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(185);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(Key);
}
void 振刀逻辑::ZhongDodge()
{
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(20);
	root_sendKeyUp(KEY_LEFTSHIFT);
}
// 后闪修刀震
void 振刀逻辑::DodgeExXiuBlueToRed(int sleep)
{
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(sleep);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(2);
	root_sendKeyUp(KEY_S);
	Sleep(220);
	root_sendKeyPress(KEY_R);
	BlueToRed();
	root_sendKeyUp(KEY_R);
	Sleep(2);
}
void 振刀逻辑::DodgeExBlueToRed(int sleep)
{
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(sleep);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(1);
	root_sendKeyUp(KEY_S);
	Sleep(370);
	MyLog("振刀");
	BlueToRed();
}
void 振刀逻辑::强后闪振() {
	string 标识 = "强行后闪振--";
	// 后闪振
	LocalPlayer_Data.上次振刀时间 = GetTickCount64() + 300;
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme10 = GetTickCount64() + 500;
	while (WaitTIme10 > GetTickCount64()) {
		Sleep(1);
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "进入闪避");
			break;
		}

	}
	root_sendKeyUp(KEY_S);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(170);
	MyLog(标识, "按下振刀");
	BlueToRed();
}
void 振刀逻辑::后闪振() {
	string 标识 = "智能后闪振--";
	if (LocalPlayer_Data.MyBlueTime <= 0.45) {
		MyLog(标识, "直接转振");
		BlueToRed();
	}
	else {
		if (FatPlayerData.O_Dis < 3) {
			// 后闪振
			LocalPlayer_Data.上次振刀时间 = GetTickCount64() + 300;
			root_sendKeyPress(KEY_S);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme10 = GetTickCount64() + 500;
			while (WaitTIme10 > GetTickCount64()) {
				Sleep(1);
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
					MyLog(标识, "进入闪避");
					break;
				}

			}
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(170);
			MyLog(标识, "按下振刀");
			BlueToRed();

		}
		else {
			{
				// AD闪振
				int keys;
				if (rand() % 2) {
					keys = KEY_A;
				}
				else {
					keys = KEY_D;
				}
				LocalPlayer_Data.上次振刀时间 = GetTickCount64() + 300;
				root_sendKeyPress(keys);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme10 = GetTickCount64() + 500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						MyLog(标识, "进入闪避");
						break;
					}
				}
				root_sendKeyUp(keys);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(170);
				MyLog(标识, "按下振刀");
				BlueToRed();
			}
		}

	}

}
void 振刀逻辑::ShortDodgeExAll()
{
	MouseReleaseNoCheck();
	int key = rand() % 4;  // 生成0到3之间的随机数

	switch (key) {
	case 0:
		ShortDodge(KEY_A);
		break;
	case 1:
		ShortDodge(KEY_D);
		break;
	case 2:
		ShortDodge(KEY_W);
		break;
	case 3:
		ShortDodge(KEY_S);
		break;
	}
}

void 振刀逻辑::ShortDodgeEx()
{
	if (rand() % 2)
		ShortDodge(KEY_A);
	else
		ShortDodge(KEY_D);
}
void 振刀逻辑::ShortDodgeWx()
{
	if (rand() % 2)
		ShortDodge(KEY_W);
	else
		ShortDodge(KEY_S);
}
void 振刀逻辑::WuTianF()
{
	root_sendKeyPress(KEY_C);
	Sleep(10);
	root_sendKeyPress(KEY_F);
	Sleep(50);
	root_sendKeyUp(KEY_F);
	root_sendKeyUp(KEY_C);
}
void 振刀逻辑::SkillF()
{
	root_sendKeyPress(KEY_F);
	Sleep(10);
	root_sendKeyUp(KEY_F);
}
void 振刀逻辑::SkillV()
{
	root_sendKeyPress(KEY_V);
	Sleep(10);
	root_sendKeyUp(KEY_V);
}
void 振刀逻辑::腾云式B()
{
	root_kmNet_mouse_right(1);
	Sleep(5);
	root_kmNet_mouse_right(0);
}
void 振刀逻辑::C滚轮切刀()
{
	/*int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;*/

	root_sendKeyPress(KEY_C);
	Sleep(30);
	root_kmNet_middle(2);
	root_sendKeyUp(KEY_C);

	/*if (武器2 == WeaponType::twinblades || 武器2 == WeaponType::dualhalberd
		|| 武器2 == WeaponType::Sw || 武器2 == WeaponType::lj) {

		root_kmNet_mouse_right(0);
		Sleep(1);
		root_kmNet_mouse_right(1);

	}*/


}
void 振刀逻辑::跳断蓄()
{
	root_sendKeyPress(KEY_SPACEBAR);
	Sleep(10);
	root_sendKeyUp(KEY_SPACEBAR);
}
void 振刀逻辑::断蓄()
{
	if (是否单刀武器() ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Blade || LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
			LocalPlayer_Data.Weapon_1 == WeaponType::saber || LocalPlayer_Data.Weapon_2 == WeaponType::saber)
		)
	{
		//通用CC断蓄力();
		CC();

	}
	else
	{
		C滚轮切刀();
	}
}
void 振刀逻辑::CC()
{
	bool successflag = false;
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 150;
	while (GetTickCount64() > WaitTIme) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.data(), L"_crouch_idle_")) {
			振刀逻辑::CollideKnife(); Sleep(200);
			root_sendKeyUp(KEY_C);
			successflag = true;
			break;
		}
	}
	if (successflag) { root_sendKeyPress(KEY_C); Sleep(20);   root_sendKeyUp(KEY_C); }
	else {
		root_sendKeyUp(KEY_C);
	}
}
void 振刀逻辑::短接长闪()
{
	ShortDodge(KEY_S); Sleep(500);
	if (LocalPlayer_Data.MyCurEnergy > 3900)
	{
		LongDodge(KEY_A);
	}
}
void 振刀逻辑::长闪绕背(uint32_t Key)
{
	root_sendKeyPress(Key);
	Sleep(20);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(340);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(Key);
}
void 振刀逻辑::跳A()
{
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_SPACEBAR);
	Sleep(20);
	平A(0);
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_SPACEBAR);
	Sleep(20);
	root_ReleaseLeftandRight();
}
void 振刀逻辑::太双切A太刀()
{
	root_BlockLeftandRight();
	Sleep(10);
	root_sendKeyPress(KEY_C);
	Sleep(40);
	root_kmNet_middle(-2);
	Sleep(20);
	root_kmNet_mouse_left(1);
	root_sendKeyUp(KEY_C);
	Sleep(10);
	root_kmNet_mouse_left(0);
	root_ReleaseLeftandRight();
}
void 振刀逻辑::太双切A()
{
	root_BlockLeftandRight();
	Sleep(75);
	root_sendKeyPress(KEY_C);
	Sleep(50);
	root_kmNet_middle(-2);
	Sleep(20);
	root_kmNet_mouse_left(1);
	root_sendKeyUp(KEY_C);
	Sleep(10);
	root_kmNet_mouse_left(0);
	root_ReleaseLeftandRight();
}
void 振刀逻辑::月闪切(float ping)
{
	int sleepTime = 20;
	if (ping > 15)
	{
		// 每增加 x ping，sleep 增加 y
		sleepTime += static_cast<int>((ping - 15) / 5) * 4;
	}
	root_BlockLeftandRight();
	Sleep(90);
	root_sendKeyPress(KEY_C);
	Sleep(50);
	root_kmNet_middle(-2);
	MyLog("暂停：", sleepTime);
	Sleep(sleepTime);
	root_kmNet_mouse_left(1);
	Sleep(10);
	root_sendKeyUp(KEY_C);
	Sleep(10);
	root_kmNet_mouse_left(0);
	root_ReleaseLeftandRight();

	Sleep(990);
}
void 振刀逻辑::月闪切右A(float ping)
{
	int sleepTime = 20;
	if (ping > 15)
	{
		// 每增加 x ping，sleep 增加 y
		sleepTime += static_cast<int>((ping - 15) / 5) * 4;
	}
	root_BlockLeftandRight();
	Sleep(90);
	root_sendKeyPress(KEY_C);
	Sleep(50);
	root_kmNet_middle(-2);
	MyLog("暂停：", sleepTime);
	Sleep(sleepTime);
	root_kmNet_mouse_right(1);
	Sleep(10);
	root_sendKeyUp(KEY_C);
	Sleep(10);
	root_kmNet_mouse_right(0);
	root_ReleaseLeftandRight();
}
void 振刀逻辑::月闪切2()
{
	root_BlockLeftandRight();
	Sleep(90);
	//root_sendKeyPress(KEY_C);
	bool successflag = false;
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 150;
	while (GetTickCount64() > WaitTIme) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.data(), L"_crouch_idle_")) {
			振刀逻辑::CollideKnife(); Sleep(200);
			root_sendKeyUp(KEY_C);
			root_kmNet_middle(-2);
			Sleep(20);
			root_kmNet_mouse_left(1);
			Sleep(15);
			root_kmNet_mouse_left(0);
			root_ReleaseLeftandRight();
			successflag = true;
			break;
		}
	}
	if (successflag) { root_sendKeyPress(KEY_C); Sleep(20);   root_sendKeyUp(KEY_C); }
	else {
		root_sendKeyUp(KEY_C);
	}
}
void 振刀逻辑::长闪A(int Key)
{
	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_LEFTSHIFT);
	Sleep(320);
	平A(Key);
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_LEFTSHIFT);
}
void 振刀逻辑::平A(int Key)
{
	if (Key == 0)
	{
		MouseLeftEx(1);
		Sleep(10);
		MouseLeftEx(0);
	}
	else if (Key == 1)
	{
		MouseRightEx(1);
		Sleep(10);
		MouseRightEx(0);
	}
}
void 振刀逻辑::拳刃升龙右A()
{
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(50);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 550;
	while (WaitTIme > GetTickCount64()) {
		MyLog("进入循环\n");
		Sleep(1);
		if (LocalPlayer_Data.MyActionName.find(L"_crouch_attack_heavy_")
			!= std::wstring::npos) {
			successflag = true;
			Sleep(50);
			MouseRightEx(0);
			Sleep(650);
			//MyLog("右键按下\n");
			MouseRightEx(1);
			Sleep(50);
			MouseRightEx(0);
			Sleep(350);
			//MyLog("右键按下\n");
			MouseRightEx(1);
			Sleep(50);
			MouseRightEx(0);
			Sleep(350);
			//MyLog("右键按下\n");
			MouseRightEx(1);
			Sleep(50);
			MouseRightEx(0);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}
void 振刀逻辑::升龙()
{
	bool successflag = false;
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 350;
	MyLog("ACA判断");
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.c_str(), L"_crouch_idle_")) {

			MyLog("进入CA");
			平A(0);

			Sleep(200);
			root_sendKeyUp(KEY_C);


			successflag = true;
			break;
		}
	}
	if (!successflag) { root_sendKeyUp(KEY_C); }
}
void 振刀逻辑::拳切百裂(int sleep) {
	int i = LocalPlayer_Data.MyWeaponIndex == 0 ? -2 : 2;
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(10);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 550;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyActionName.find(L"_crouch_attack_heavy_")
			!= std::wstring::npos) {
			successflag = true;
			Sleep(sleep);
			root_sendKeyPress(KEY_0_CPARENTHESIS);
			Sleep(10);
			root_sendKeyUp(KEY_0_CPARENTHESIS);
			Sleep(400);
			root_sendKeyPress(KEY_D);
			root_sendKeyPress(KEY_LEFTSHIFT);
			Sleep(30);
			root_kmNet_middle(i);
			Sleep(10);
			root_sendKeyUp(KEY_D);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(750);
			MouseRightEx(0);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}

void 振刀逻辑::半自动断蓄力() {
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	if (是否单刀武器() ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Blade || LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
			LocalPlayer_Data.Weapon_1 == WeaponType::saber || LocalPlayer_Data.Weapon_2 == WeaponType::saber)
		) {
		MyLog("cc断蓄力");
		通用CC断蓄进蓄力();
	}
	else {
		MyLog("c切刀");
		C滚轮切刀();
	}
}

float 振刀逻辑::重置蓄力断蓄点() {
	Function::侧键博弈::动态断蓄时间点 = 取随机Folat数(Function::侧键博弈::断续切刀最小时间, Function::侧键博弈::断续切刀时间);
	return Function::侧键博弈::动态断蓄时间点;
}

void 振刀逻辑::C滚轮切刀重置蓄力() {
	振刀逻辑::重置蓄力断蓄点();
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	if (是否单刀武器() ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Blade || LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
			LocalPlayer_Data.Weapon_1 == WeaponType::saber || LocalPlayer_Data.Weapon_2 == WeaponType::saber)
		) {
		// 后长闪进蓄力
		if (当前武器 == WeaponType::Sw) {
			振刀逻辑::长闪蓄();
		}
		else {
			MyLog("cc断蓄力");
			通用CC断蓄力();
		}
	}
	else {
		MyLog("c切刀");
		root_sendKeyPress(KEY_C);
		int WaitTImeTime = 0;
		ULONGLONG WaitTIme = GetTickCount64() + 1000;
		while (WaitTIme > GetTickCount64()) {
			//MyLog("c切刀 循环次数:", WaitTImeTime);
			if (WaitTImeTime > 40 || JudgeCrouch(LocalPlayer_Data.MyActionName)) {
				MyLog("进入下蹲状态");
				break;
			}
			else if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			WaitTImeTime++;
			Sleep(1);
		}

		振刀逻辑::CollideKnife();
		MyLog("松开鼠标左右键");
		if (当前武器位置 == 1) {
			root_sendKeyPress(KEY_2_AT);
		}
		else {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}
		MyLog("切武器中");
		Sleep(10);

		if (当前武器位置 == 1) {
			root_sendKeyUp(KEY_2_AT);
		}
		else {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}

		MyLog("切武器按键结束");
		root_sendKeyUp(KEY_C);
		MyLog("按键弹起C");
		Sleep(15);
		root_sendKeyUpAll();
	}
}

bool 振刀逻辑::长闪蓄力(int 平A方式) {
	// w长闪左蓄力
	root_kmNet_mouse_left(0);
	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 800;
	while (WaitTIme1 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return false;
		}
		changshanTemp++;
	}

	//Sleep(30);
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_LEFTSHIFT);

	Sleep(30);
	return true;
}

bool 振刀逻辑::自动选择重置蓄力方式(int 平A方式) {
	if (LocalPlayer_Data.MyCurEnergy >= 7000) {
		if (rand() % 5 < 2) {
			return 长闪蓄力(平A方式);
		}
		else {
			return 通用CC断蓄力();
		}

	}
	else {
		return 通用CC断蓄力();
	}
}
std::random_device rd;
std::mt19937 gen(rd());  // 使用随机设备初始化随机数引擎
float 振刀逻辑::取随机Folat数(float min, float max) {
	// 创建随机数生成器
	std::uniform_real_distribution<> dis(min, max); // 均匀分布
	// 生成并返回随机数
	return dis(gen);
}
bool 振刀逻辑::通用CC断蓄力() {
	MyLog("通用CC断蓄力");
	root_sendKeyUp(KEY_A);
	root_sendKeyUp(KEY_S);
	root_sendKeyUp(KEY_D);

	root_sendKeyPress(KEY_C);
	int WaitTImeTime = 0;
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		//MyLog("cc 循环次数:", WaitTImeTime);
		if (JudgeCrouch(LocalPlayer_Data.MyActionName)) {

			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return false;
		}
		WaitTImeTime++;
		Sleep(1);
	}
	Sleep(20);
	振刀逻辑::CollideKnife();
	root_sendKeyUp(KEY_C);
	Sleep(20);
	if (振刀逻辑::是否特殊状态())
	{
		root_sendKeyUpAll();
		return false;
	}
	root_sendKeyPress(KEY_C);
	Sleep(20);
	root_sendKeyUp(KEY_C);
	root_sendKeyUpAll();
	return true;
}

bool 振刀逻辑::通用CC断蓄进蓄力() {
	string 标识 = "通用CC断蓄进蓄力--";
	MyLog(标识, "开始");
	root_sendKeyUp(KEY_A);
	root_sendKeyUp(KEY_S);
	root_sendKeyUp(KEY_D);

	root_sendKeyPress(KEY_C);
	int WaitTImeTime = 0;
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		//MyLog("cc 循环次数:", WaitTImeTime);
		if (JudgeCrouch(LocalPlayer_Data.MyActionName)) {

			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return false;
		}
		WaitTImeTime++;
		Sleep(1);
	}
	Sleep(20);
	//振刀逻辑::CollideKnife();
	root_sendKeyUp(KEY_C);
	Sleep(20);
	if (振刀逻辑::是否特殊状态())
	{
		root_sendKeyUpAll();
		return false;
	}
	root_sendKeyPress(KEY_C);
	Sleep(20);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		//MyLog("cc 循环次数:", WaitTImeTime);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (JudgeZhanLi(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "站立状态");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return false;
		}
		WaitTImeTime++;
		Sleep(1);
	}
	// _idle_01
	//root_sendKeyUpAll();
	if (mem.GetKeyboard()->IsKeyDown(1)) {
		// 鼠标左键按下
		MyLog(标识, "鼠标左键按下");
		root_kmNet_mouse_left(0);
		Sleep(1);
		root_kmNet_mouse_left(1);
	}
	else if (mem.GetKeyboard()->IsKeyDown(2)) {
		// 鼠标右键按下
		MyLog(标识, "鼠标右键按下");
		root_kmNet_mouse_right(0);
		Sleep(1);
		root_kmNet_mouse_right(1);
	}
	return true;
}

void 振刀逻辑::处决后双跳A() {
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_SPACEBAR);
	Sleep(2);
	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_SPACEBAR);
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {

		if (JudgeJump(LocalPlayer_Data.MyActionName))
		{
			MyLog("起跳");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_right(0);
	Sleep(1);
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {

		if (JudgeJumpAndA(LocalPlayer_Data.MyActionName))
		{
			MyLog("跳A");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {

		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName))
		{
			MyLog("跳A进行中");
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
	while (WaitTIme4 > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (LocalPlayer_Data.MyRangeReactionType == 10)
		{
			MyLog("跳A结束");
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_SPACEBAR);

	int 临时跳A到僵直 = 0;
	ULONGLONG WaitTIme5 = GetTickCount64() + 600;
	while (WaitTIme5 > GetTickCount64()) {
		Sleep(1);
		if (!Judgefly(FatPlayerData.O_ActionName)) {
			root_sendKeyUpAll();
			return;
		}
		if (FatPlayerData.O_EndureLevel == 6 &&
			!拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("跳A到僵直");
			临时跳A到僵直 = 1;
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}

	}
	if (临时跳A到僵直 == 1) {
		ULONGLONG WaitTIme4 = GetTickCount64() + 800;
		while (WaitTIme4 > GetTickCount64()) {
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
			if (落地状态(LocalPlayer_Data.MyActionName))
			{
				MyLog("落地状态");
				break;
			}
			else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		Sleep(20);
		// 循环调用自己
		MyLog("循环调用 处决后双跳A");
		处决后双跳A();
		root_sendKeyUpAll();
		return;

	}

	root_sendKeyUpAll();
}

void 振刀逻辑::处决后跳A() {

	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_SPACEBAR);
	ULONGLONG WaitTIme = GetTickCount64() + 1200;
	while (WaitTIme > GetTickCount64()) {

		if (JudgeJump(LocalPlayer_Data.MyActionName))
		{
			MyLog("起跳");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	Sleep(1);
	root_kmNet_mouse_left(1);
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
	while (WaitTIme2 > GetTickCount64()) {

		if (JudgeJumpAndA(LocalPlayer_Data.MyActionName))
		{
			MyLog("跳A");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
	while (WaitTIme3 > GetTickCount64()) {

		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName))
		{
			MyLog("跳A进行中");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
	while (WaitTIme4 > GetTickCount64()) {

		if (LocalPlayer_Data.MyRangeReactionType == 10)
		{
			MyLog("跳A结束");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	root_sendKeyUp(KEY_W);
	root_sendKeyUp(KEY_SPACEBAR);

	ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
	while (WaitTIme5 > GetTickCount64()) {

		if (落地状态(LocalPlayer_Data.MyActionName))
		{
			MyLog("落地状态");
			break;
		}
		else if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

}

bool 振刀逻辑::侧键是否按下() {
	return 博弈键按下;
}
void 振刀逻辑::长剑A抓僵直() {
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife(); /*Sleep(300);*/

	ULONGLONG WaitTIme = GetTickCount64() + 600;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		//MyLog("我的动作名:", LocalPlayer_Data.MyActionName);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (LocalPlayer_Data.MyActionName.find(L"male_sw_attack_heavy_01") != wstring::npos ||
			LocalPlayer_Data.MyActionName.find(L"male_sw_run_attack_heavy_01") != wstring::npos ||
			LocalPlayer_Data.MyActionName.find(L"male_sw_sprint_attack_heavy_01") != wstring::npos)
		{
			MyLog("长剑出剑b");
			跳出方式 = 1;
			break;
		}

	}

	if (跳出方式 == 1)
	{
		bool 僵直抓取成功 = false;
		ULONGLONG WaitTIme = GetTickCount64() + 600;
		while (WaitTIme > GetTickCount64()) {

			//MyLog("僵直时间2:", FatPlayerData.O_ActionDuration);
			if (FatPlayerData.O_ActionDuration > 0.8f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry"))
			{
				僵直抓取成功 = true;
				MyLog("切刀985\n");
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切();
				}
				else
				{
					振刀逻辑::拳切百裂(480);
				}
				break;
			}
			Sleep(1);
		}

		if (!僵直抓取成功) {

		}

	}
	else
	{
		振刀逻辑::左右键直接松开();
		root_sendKeyUp(KEY_W);
	}
}
void 振刀逻辑::背包切() {
	//MyLog("背包切刀蓄");
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(10);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 600;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyActionName.find(L"_crouch_attack_heavy_")
			!= std::wstring::npos) {
			successflag = true;
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
			Sleep(400);
			root_kmNet_middle(-2);
			Sleep(480);
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
			Sleep(780);
			MouseRightEx(0);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}
void 振刀逻辑::拳切3A(int sleep) {
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(30);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 400;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.c_str(), L"_crouch_attack_heavy_")) {
			successflag = true;
			Sleep(sleep);
			root_kmNet_middle(2);
			Sleep(280);
			root_sendKeyPress(KEY_LEFTSHIFT);
			Sleep(40);
			root_kmNet_middle(2);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(210);
			MouseRightEx(0);
			Sleep(180);
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_SPACEBAR);
			Sleep(20);
			MouseLeftEx(1);
			Sleep(300);
			root_sendKeyUp(KEY_SPACEBAR);
			MouseLeftEx(0);
			root_sendKeyUp(KEY_W);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}
void 振刀逻辑::钩锁百裂() {
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(10);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.c_str(), L"_crouch_attack_heavy_")) {
			successflag = true;
			MouseRightEx(0);
			root_sendKeyPress(KEY_Q);
			Sleep(180);
			MouseRightEx(1);
			root_sendKeyUp(KEY_Q);
			Sleep(2150);
			MouseRightEx(0);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}
void 振刀逻辑::钩锁百裂自动钩锁() {
	bool successflag = false;
	root_BlockLeftandRight();
	root_sendKeyPress(KEY_C);
	Sleep(10);
	MouseRightEx(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (StrStrW(LocalPlayer_Data.MyActionName.c_str(), L"_crouch_attack_heavy_")) {
			successflag = true;
			MouseRightEx(0);
			Sleep(400);
			root_sendKeyPress(KEY_Q);
			Sleep(20);
			root_sendKeyUp(KEY_Q);
			Sleep(200);
			MouseRightEx(1);
			Sleep(2150);
			MouseRightEx(0);
			root_kmNet_unmask_all();
			break;
		}
	}
	if (!successflag) {
		root_sendKeyUp(KEY_C);
		MouseRightEx(0); root_kmNet_unmask_all();
	}
}

/****************武器出蓄逻辑**************/
void 振刀逻辑::双戟蓄力释放() {
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		int tempTime2 = 0;
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					双刀月影闪切太刀A僵直抓取();
					return;

				}if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					双刀月影闪切横刀A僵直抓取();
					return;
				}
				else {
					//双刀BB();
					//break;
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_kmNet_mouse_right(1);
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}

			}
			else {
				通用切刀重置蓄力();
				break;
			}

		}
		if (tempTime2 > 30) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::双刀蓄力释放() {
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTIme1 = GetTickCount64() + 800;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		int tempTime2 = 0;
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					双刀月影闪切太刀A僵直抓取();
					return;
				}else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					双刀月影闪切横刀A僵直抓取();
					return;
				}
				else {
					//双刀BB();
					//break;
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 100 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 160 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 210) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_kmNet_mouse_right(1);
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 30) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::横刀蓄力释放() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int xuType = 0; // 0左蓄力 1右蓄力
	// 判断当前是左蓄力还是右蓄力
	if (横刀左蓄状态(LocalPlayer_Data.MyActionName)) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		xuType = 0;
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		xuType = 1;
	}

	if (xuType == 0) {
		// 左蓄力逻辑
		int 左蓄力跳出方式 = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		while (WaitTIme1 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 12) {
				MyLog("横刀 --左蓄力正常释放");
				左蓄力跳出方式 = 0;
				break;
			}
			if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
				MyLog("横刀 --对面振刀，转抓");
				左蓄力跳出方式 = 1;
				break;
			}
			if (JudgeShortFlash(FatPlayerData.O_ActionName)) {
				MyLog("横刀 --对面闪了");
				break;
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		if (左蓄力跳出方式 == 0) {
			ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
			while (WaitTIme2 > GetTickCount64()) {
				if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog("横刀左蓄力命中 - 进w闪");
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUpAll();

					if (FatPlayerData.O_Dis > 6 && LocalPlayer_Data.MyCurEnergy > 5000) {
						root_sendKeyPress(KEY_W);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
						while (WaitTIme10 > GetTickCount64()) {
							Sleep(1);
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
						}
						root_sendKeyUp(KEY_W);
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						通用切刀重置蓄力();

						break;
					}

				}
				if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog("左蓄力未命中 --s闪");
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUpAll();
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
						while (WaitTIme10 > GetTickCount64()) {
							Sleep(1);
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(1);
						break;
					}
					else {
						通用切刀重置蓄力();

						break;
					}

				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		else if (左蓄力跳出方式 == 1) {
			// 对面振刀 a一下
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}
	}
	else {
		// 右蓄力逻辑 
		ULONGLONG WaitTImea1 = GetTickCount64() + 500;
		while (WaitTImea1 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 20) {
				break;
			}
			if (是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
		while (WaitTIme1 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 11 || LocalPlayer_Data.MyEndureLevel != 20) {
				break;
			}
			if (是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		int timeTemp2 = 0;
		ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
		while (WaitTIme2 > GetTickCount64()) {
			if (timeTemp2 > 10 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("横刀蓄力命中 - 进w闪");
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();

				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
			if (timeTemp2 > 35 || FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力未命中 --s闪");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}
			timeTemp2++;
			Sleep(1);
		}
	}
}
void 振刀逻辑::双截棍蓄力释放() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 20;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 800;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10 && LocalPlayer_Data.MyEndureLevel == 11 || LocalPlayer_Data.MyEndureLevel != 20) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog("双截棍蓄力释放完毕?");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("双截棍蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			Sleep(witTime);

			if (FatPlayerData.O_Dis > 6 && LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;
			}
			else {
				MyLog("精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 30) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("双截棍蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::太刀蓄力释放() {
	string 标识 = "太刀蓄力释放--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 0;
	if (LocalPlayer_Data.MyBlueTime >= 1.0 || AB3蓄力状态(LocalPlayer_Data.MyActionName)) {
		MyLog(标识, "AB3蓄力状态");
		witTime += 0;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	MyLog(标识, "太刀蓄力释放1");
	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 800;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10 && (LocalPlayer_Data.MyEndureLevel == 11 || LocalPlayer_Data.MyEndureLevel != 20)) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "太刀蓄力释放完毕?");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "太刀蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			//Sleep(witTime);
			if (FatPlayerData.O_Dis >= 4.5) {
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					MyLog(标识, "精力足够 -进w闪");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog(标识, "精力不够 -CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			else {
				MyLog(标识, "精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}


		}
		if (tempTime2 > 10) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "太刀蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::长剑蓄力释放() {
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife(); /*Sleep(300);*/

	ULONGLONG WaitTIme = GetTickCount64() + 600;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyEndureLevel == 11)
		{
			MyLog("出蓄力");
			跳出方式 = 1;
			break;
		}
		if (挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
	}
	长剑上次出蓄力时间 = GetTickCount64();
	if (跳出方式 == 1) {
		int timeTemp = 0;
		ULONGLONG WaitTIme = GetTickCount64() + 1000;
		while (WaitTIme > GetTickCount64()) {
			//wcout << "动作名=" << FatPlayerData.O_ActionName << "-人物状态=" << FatPlayerData.O_ActionName << endl;
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("命中");

				/*while (GetTickCount64() - 长剑上次出蓄力时间 < 300 ) {
					if (是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
				}*/
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
					while (WaitTIme2 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if (changshanTemp > 230 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 250 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 330) {
							MyLog("进入长闪--循环次数:", changshanTemp);
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}

					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}
			if (timeTemp > 60 && FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("未命中/拼刀/磐石");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(5);
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme2 = GetTickCount64() + 400;
					while (WaitTIme2 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							// 进入闪
							MyLog("进入闪避");
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
			}
			Sleep(1);
		}
	}
	else {
		root_sendKeyUpAll();
		if (LocalPlayer_Data.MyCurEnergy > 5000) {
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(5);
			root_sendKeyPress(KEY_S);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme2 = GetTickCount64() + 400;
			while (WaitTIme2 > GetTickCount64()) {
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
					// 进入闪
					MyLog("进入闪避");
					break;
				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(10);
		}
		return;
	}

}
void 振刀逻辑::链剑蓄力释放() {
	string 标识 = "链剑蓄力释放--";
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife(); /*Sleep(300);*/

	ULONGLONG WaitTIme = GetTickCount64() + 600;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyEndureLevel == 11)
		{
			MyLog(标识, "出蓄力");
			跳出方式 = 1;
			break;
		}
		if (挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
	}
	长剑上次出蓄力时间 = GetTickCount64();
	if (跳出方式 == 1) {
		ULONGLONG WaitTIme = GetTickCount64() + 1000;
		while (WaitTIme > GetTickCount64()) {
			//wcout << "动作名=" << FatPlayerData.O_ActionName << "-人物状态=" << FatPlayerData.O_ActionName << endl;
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "命中");
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					MyLog(标识, "精力足够进入W长闪");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme2 = GetTickCount64() + 1600;
					while (WaitTIme2 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							// 进入短闪
							MyLog("进入长闪");
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}

					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "未命中/拼刀/磐石");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(5);
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme2 = GetTickCount64() + 400;
					while (WaitTIme2 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							// 进入闪
							MyLog("进入闪避");
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
			}
			Sleep(1);
		}
	}
	else {
		root_sendKeyUpAll();
		if (LocalPlayer_Data.MyCurEnergy > 5000) {
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(5);
			root_sendKeyPress(KEY_S);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme2 = GetTickCount64() + 400;
			while (WaitTIme2 > GetTickCount64()) {
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
					// 进入闪
					MyLog("进入闪避");
					break;
				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(10);
		}
		return;
	}

}

void 振刀逻辑::斩马柄击释放() {
	string 标识 = "斩马柄击释放--";
	root_kmNet_mouse_left(1);

	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 10) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}
	MyLog(标识, "柄击释放完毕");
	int tempTime2 = 0;
	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "斩马柄击命中");
			// 接一个平A
			root_kmNet_mouse_left(1);

			ULONGLONG WaitTIm3 = GetTickCount64() + 500;
			while (WaitTIm3 > GetTickCount64()) {
				if (A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_kmNet_mouse_left(0);

			LONGLONG WaitTIm4 = GetTickCount64() + 500;
			while (WaitTIm4 > GetTickCount64()) {
				if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				if (LocalPlayer_Data.MyRangeReactionType == 10) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			// 捏住无敌磐石
			root_kmNet_mouse_left(1);
			Sleep(300);
			break;
		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "柄击未命中 ");
				// 捏住
				root_kmNet_mouse_left(1);
				Sleep(500);
				break;
			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}

}
void 振刀逻辑::斩马蓄力释放() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 20;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("斩马蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			Sleep(witTime);

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog("精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("斩马蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::长棍蓄力释放() {
	string 标识 = "长棍蓄力释放--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 80;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();

	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "MyEndureLevel == 11");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog("长棍蓄力释放完毕");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("长棍蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			Sleep(witTime);

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog("精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::长枪蓄力释放() {
	string 标识 = "长枪蓄力释放--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 80;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();

	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}


	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "MyEndureLevel == 11");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "蓄力释放完毕");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			Sleep(witTime);

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog(标识, "精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog(标识, "精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 振刀逻辑::匕首蓄力释放() {
	string 标识 = "匕首蓄力释放--";
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife();

	/*ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyEndureLevel == 11)
		{
			MyLog("出蓄力");
			跳出方式 = 1;
			break;
		}
		if (挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
	}*/

	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 800;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10 && (LocalPlayer_Data.MyEndureLevel == 11 || LocalPlayer_Data.MyEndureLevel != 20)) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "蓄力释放完毕?");

	长剑上次出蓄力时间 = GetTickCount64();
	if (跳出方式 == 1) {
		int timeTemp = 0;
		ULONGLONG WaitTIme = GetTickCount64() + 1000;
		while (WaitTIme > GetTickCount64()) {
			//wcout << "动作名=" << FatPlayerData.O_ActionName << "-人物状态=" << FatPlayerData.O_ActionName << endl;
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("命中");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme2 = GetTickCount64() + 1600;
					while (WaitTIme2 > GetTickCount64()) {
						wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							// 进入短闪
							MyLog("进入长闪");
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}

					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("未命中/拼刀/磐石");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(5);
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme2 = GetTickCount64() + 400;
					while (WaitTIme2 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							// 进入闪
							MyLog("进入闪避");
							break;
						}
						if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
			}
			Sleep(1);
		}
	}
	else {
		root_sendKeyUpAll();
		if (LocalPlayer_Data.MyCurEnergy > 5000) {
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(5);
			root_sendKeyPress(KEY_S);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme2 = GetTickCount64() + 400;
			while (WaitTIme2 > GetTickCount64()) {
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
					// 进入闪
					MyLog("进入闪避");
					break;
				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(10);
		}
		return;
	}

}
void 振刀逻辑::拳刃蓄力释放() {
	string 标识 = "拳刃蓄力释放--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 80;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();

	ULONGLONG WaitTIme12 = GetTickCount64() + 300;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10 && LocalPlayer_Data.MyEndureLevel == 11 || LocalPlayer_Data.MyEndureLevel != 20) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(10);
	}*/
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "MyEndureLevel == 11");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "蓄力释放完毕");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			Sleep(witTime);

			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog(标识, "精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog(标识, "精力不够 -CC重置");
				通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
/****************武器出蓄逻辑结束**************/

/**************武器博弈逻辑************/
void 振刀逻辑::双戟博弈() {
	int 重置方式 = 1;
	string 武器名 = "双戟博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 右蓄力起手
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入双刀 右蓄力");
				//Sleep(2);
				root_kmNet_mouse_right(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {

						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//root_sendKeyUp(KEY_W);
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		if (/*rand() % 2*/true) {
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米 按下右键");
				root_kmNet_mouse_right(1);
			}
		}
		else {
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米 按下左键");
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_A);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_D);
				root_kmNet_mouse_left(1);
			}
		}


		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {

							MyLog("进入W长闪 6-10");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 4) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 4) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于4米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();

			}

		}



	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, " 1.0前抓长闪 且距离小于4");
							振刀逻辑::执行双戟平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, " 1.0前抓 短闪且距离小于4");
							振刀逻辑::执行双戟平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.1 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}

				if (倒地状态(FatPlayerData.O_ActionName) && GetTickCount64() - 双刀上次出蓄力时间 > 1000) {
					MyLog(武器名, " 1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (JudgeYuanCheng() || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, " 1.0前抓远程/空手");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, " 1.0前抓站立/跑动/走动");
					振刀逻辑::执行双戟平A僵直抓取();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, " 1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0前抓出跳蹲");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (dualhalberdBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行双戟平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0前抓振刀");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}


				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if (被震刀僵直状态(FatPlayerData.O_ActionName) && FatPlayerData.ObjCurTime < FatPlayerData.O_ActionDuration) {
					MyLog(武器名, "蓄力1.0前抓 被震刀僵直状态");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_BlueTime > 0.2) && (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade)) {
					MyLog(武器名, "蓄力1.0前打斩马阔刀磐石");
					振刀逻辑::执行双戟平A僵直抓取();
					return;
				}
			}
		}
		if ((useMultiLogic ? LocalPlayer_Data.MyBlueTime > 0.5f : LocalPlayer_Data.MyBlueTime >= 0.5f) && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_EndureLevel == 5 || enemy.O_EndureLevel == 1; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {

						MyLog(武器名, " 1.0后抓 长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;

					}
					else if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {

						MyLog(武器名, " 1.0后抓 短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;

					}

				}

				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, " 1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, " 1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::dualhalberd)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog("蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog("蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}

				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
				/*if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0后抓出跳蹲");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}*/
			}
		}

		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}


	}
}
void 振刀逻辑::双刀博弈() {
	int 左蓄出刀补偿 = -50;
	int 重置方式 = 1;
	string 武器名 = "双刀博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 右蓄力起手
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入双刀 右蓄力");
				//Sleep(2);
				root_kmNet_mouse_right(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_kmNet_mouse_right(0);
			root_kmNet_mouse_left(0);
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//root_sendKeyUp(KEY_W);
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		if (/*rand() % 2*/true) {
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米 按下右键");
				root_kmNet_mouse_right(1);
			}
		}
		else {
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米 按下左键");
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_A);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_D);
				root_kmNet_mouse_left(1);
			}
		}


		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪 6-10");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 4) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 4) {
				//	MyLog(武器名, "蓄力到临界值:", LocalPlayer_Data.MyBlueTime, "-S闪");
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于4米-蓄力时间:", LocalPlayer_Data.MyBlueTime);
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();

			}

		}



	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		//MyLog("进入小于5距离判断");
		//MyLogTrue("进蓝时间", LocalPlayer_Data.MyBlueTime,"-MyEndureLevel", LocalPlayer_Data.MyEndureLevel,"-状态", LocalPlayer_Data.XuListate);
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, " 1.0前抓长闪 且距离小于4");
							振刀逻辑::执行双刀平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, " 1.0前抓 短闪且距离小于4");
							振刀逻辑::执行双刀平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.1 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}

				if (倒地状态(FatPlayerData.O_ActionName) && GetTickCount64() - 双刀上次出蓄力时间 > 1000) {
					MyLog(武器名, " 1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, " 1.0前抓远程/空手");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, " 1.0前抓站立/跑动/走动");
					振刀逻辑::执行双刀平A僵直抓取();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, " 1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0前抓出跳蹲");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (twinbladesBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行双刀平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0前抓振刀");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				//MyLogTrue("状态值", FatPlayerData.O_EndureLevel);
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if (被震刀僵直状态(FatPlayerData.O_ActionName) && FatPlayerData.ObjCurTime < FatPlayerData.O_ActionDuration) {
					MyLog(武器名, "蓄力1.0前抓 被震刀僵直状态");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_BlueTime > 0.2) && (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade)) {
					MyLog(武器名, "蓄力1.0前打斩马阔刀磐石");
					振刀逻辑::执行双刀平A僵直抓取();
					return;
				}
			}
		}
		if ((useMultiLogic ? LocalPlayer_Data.MyBlueTime > 0.5f : LocalPlayer_Data.MyBlueTime >= 0.5f) && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (!左蓄状态(LocalPlayer_Data.MyActionName)) {
					左蓄出刀补偿 = 0;
					if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_EndureLevel == 5 || enemy.O_EndureLevel == 1; })) {
						if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
							MyLog(武器名, " 1.0后抓 长闪");
							出蓄::通用出蓄(FatPlayerData);
							return;
						}
						else if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
							MyLog(武器名, " 1.0后抓 短闪");
							出蓄::通用出蓄(FatPlayerData);
							return;
						}

					}

					if (MatchEnemy([](const PlayerData& enemy) {
						return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
						})) {
						MyLog(武器名, " 1.0后抓远程/空手");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
						return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
						})) {
						MyLog(武器名, " 1.0后抓出刀");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::twinblades)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else {
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep((Function::侧键博弈::蓄力抓振延迟 + 25 + 左蓄出刀补偿));
									MyLog("蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟 + 左蓄出刀补偿);
								MyLog("蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}
							return;
						}

					}

				}

				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				/*if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, " 1.0后抓出跳蹲");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}*/
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间 && GetTickCount64() - 双刀上次出蓄力时间 > 800) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}
}
void 振刀逻辑::横刀博弈() {
	int 重置方式 = 1;
	string 武器名 = "--横刀博弈";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {
		string 标识 = "大于博弈距离范围--";
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "状态1");
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "状态2");
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		//MyLog("开始起手，我的精力:", LocalPlayer_Data.MyCurEnergy);
		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			//MyLog("判断起手势");
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			//MyLog("进判断前蓄力时间0:", LocalPlayer_Data.MyBlueTime);
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			Sleep(50);
			MyLog("开始滑铲");
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}
	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		string 标识 = "博弈距离范围内--";
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "状态1");
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "状态2");
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下());
		if (!判断鼠标右键是否按下() && !判断鼠标左键是否按下()) {
			if (((FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade) || 可C切武器(WeaponType::hengdao)) && (rand() % 5 < 4) || (rand() % 5 < 2)) {
				//MyLog("是否可C切:", 可C切武器(WeaponType::hadidazhao));
				if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
					MyLog("进入右蓄力小于", Function::侧键博弈::博弈距离范围);
					//root_kmNet_mouse_left(1);
					root_kmNet_mouse_right(1);
				}
			}
			else {
				if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
					MyLog("进入左蓄力小于", Function::侧键博弈::博弈距离范围);
					root_kmNet_mouse_left(1);
					//root_kmNet_mouse_right(1);
				}
			}
		}
		

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		//MyLog("进判断前蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 3) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");

		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前 抓倒地状态 跳A", 武器名);
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog("蓄力1.0前 抓壁击", 武器名);
					振刀逻辑::横刀拳切百裂();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog("蓄力1.0前抓远程/空手", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog("蓄力1.0前抓站立/跑动/走动", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog("蓄力1.0前抓出蓄力后摇", 武器名);
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前抓出跳蹲", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (JudgeLongFlash(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前抓长闪", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}
				if (JudgeDuanFlash(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前抓短闪", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (hengdaoBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行横刀平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前抓振刀", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 5) {
					MyLog("蓄力1.0前抓 状态5", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog("蓄力1.0前抓 受击站立状态", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_BlueTime > 0.2) && (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade)) {
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					MyLog("蓄力1.0前打斩马阔刀磐石", 武器名);
					振刀逻辑::执行横刀平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return 判断是否手持远程(enemy.O_ActorWeapon) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog("蓄力1.0后抓远程/空手", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog("蓄力1.0后抓出刀", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::hadidazhao)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog("蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog("蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}
							return;
						}
				}

				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog("蓄力1.0后抓倒地", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(" 1.0后抓受击状态", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog("蓄力一段后CC重置蓄力", 武器名);
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog("蓄力一段后C滚轮切刀重置蓄力", 武器名);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}
}
void 振刀逻辑::太刀博弈() {

	int 重置方式 = 1;
	string 武器名 = "太刀博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};
	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 4000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				int changshanTemp = 0;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(130);
			root_sendKeyPress(KEY_SPACEBAR);
			Sleep(10);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);

			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下());
		if ((FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade) || 可C切武器(WeaponType::Katana) && (rand() % 5 < 2)) {
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米");
				//Sleep(2);
				//root_kmNet_mouse_left(1);
				root_kmNet_mouse_right(1);
			}
		}
		else {
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力小于10米");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}


		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断,蓄力时间:", LocalPlayer_Data.MyBlueTime);
		/*if (FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
			auto O_NameHash = mem.Read<unsigned int>(FatPlayerData.Cache_NameHash + 0x18);
			float 出招后摇 = Function::侧键博弈::白刀蓝顶延迟;
			MyLog("出招后摇=", 出招后摇,"--招式哈希值=", O_NameHash);
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
				if (出招后摇 == 0) {
					break;
				}
				if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
					MyLog("后摇判断--蓝顶出刀");
					break;
				}
				Sleep(1);
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.5) {
				出蓄::通用出蓄(FatPlayerData);
			}
			else {
				振刀逻辑::执行太刀平A僵直抓取();
			}

		}*/

		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//if (FatPlayerData.O_ActorWeapon == WeaponType::punch) {
				//	// 对方拳刃 不抓闪

				//}
				//else if (FatPlayerData.O_ActorWeapon == WeaponType::Blade) {
				//	// 对方阔刀
				//}
				//else if (FatPlayerData.O_ActorWeapon == WeaponType::Knife) {
				//	// 对方飞刀
				//}
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					if (FatPlayerData.ObjCurTime > 1.5 || FatPlayerData.O_Skill.F_CD > 5 || !是否受击技能F(FatPlayerData.O_Skill.F_SkillID)) {
						// 白刀抓
						MyLog(武器名, "蓄力1.0前抓 水矛");
						振刀逻辑::执行太刀平A僵直抓取();
					}
					
					return;
					// male_all_unarmed_be_tied
				}

				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行太刀平A僵直抓取();
							return;
						}
						//else {
						//	// 距离过远长闪跟进
						//	// 距离过远长闪跟进
						//	MyLog("抓长闪过远 长闪跟进");
						//	root_sendKeyPress(KEY_W);
						//	root_sendKeyPress(KEY_LEFTSHIFT);
						//	左右键直接松开();
						//	ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						//	while (WaitTIme11 > GetTickCount64()) {
						//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName)) {
						//			break;
						//		}
						//		if (振刀逻辑::是否特殊状态())
						//		{
						//			root_sendKeyUpAll();
						//			return;
						//		}
						//		Sleep(1);
						//	}

						//	root_sendKeyUp(KEY_W);
						//	root_sendKeyUp(KEY_LEFTSHIFT);
						//	Sleep(1);
						//	return;

						//}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && 判断短闪哈希(FatPlayerData.O_NameHash)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", FatPlayerData.O_NameHash);
							振刀逻辑::执行太刀平A僵直抓取();
							return;
						}
						//else {
						//	// 距离过远长闪跟进
						//	MyLog("抓短闪过远 长闪跟进");
						//	root_sendKeyPress(KEY_W);
						//	root_sendKeyPress(KEY_LEFTSHIFT);
						//	左右键直接松开();
						//	ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						//	while (WaitTIme11 > GetTickCount64()) {
						//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName)) {
						//			break;
						//		}
						//		if (振刀逻辑::是否特殊状态())
						//		{
						//			root_sendKeyUpAll();
						//			return;
						//		}
						//		Sleep(1);
						//	}

						//	root_sendKeyUp(KEY_W);
						//	root_sendKeyUp(KEY_LEFTSHIFT);
						//	Sleep(1);
						//	return;
						//}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					// male_injured_lie_right_01
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (KatanaBlueAttackWrite2(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.505) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行太刀平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_BlueTime > 0.2) && (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade)) {
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					MyLog(武器名, "蓄力1.0前打斩马阔刀磐石");
					振刀逻辑::执行太刀平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					//蓄力抓
					MyLog(武器名, "蓄力1.0前抓 水矛");
					出蓄::通用出蓄(FatPlayerData);
					return;
					// male_all_unarmed_be_tied
				}
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::hadidazhao)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {

							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 20);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}
							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return 判断短闪哈希(enemy.O_NameHash); })) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}
				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				//std::cout << Function::侧键博弈::动态断蓄时间点 << std::endl;
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::长剑博弈() {
	int 重置方式 = 1;
	string 武器名 = "长剑博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_S);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_S);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力");
				//Sleep(2);
				//root_kmNet_mouse_left(1);
				root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1000;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_S);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_S);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下());
		if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入右蓄力小于7米");
			//Sleep(2);
			root_kmNet_mouse_right(1);
			//root_kmNet_mouse_right(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪 6-10");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
			//	通用切刀重置蓄力();

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		//if (FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
		//	/*auto O_NameHash = mem.Read<unsigned int>(FatPlayerData.Cache_NameHash + 0x18);
		//	float 出招后摇 = Function::侧键博弈::白刀蓝顶延迟;
		//	MyLog("出招后摇=", 出招后摇, "--招式哈希值=", O_NameHash);
		//	ULONGLONG WaitTIme = GetTickCount64() + 1500;
		//	while (WaitTIme > GetTickCount64()) {
		//		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		//		if (出招后摇 == 0) {
		//			break;
		//		}
		//		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
		//			MyLog("后摇判断--蓝顶出刀");
		//			break;
		//		}
		//		Sleep(1);
		//	}*/

		//	SwBlueAttackWrite(FatPlayerData);
		//	if (LocalPlayer_Data.MyBlueTime >= 0.5) {
		//		出蓄::通用出蓄(FatPlayerData);
		//	}
		//	else {
		//		振刀逻辑::执行长剑平A僵直抓取();
		//	}

		//}
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							wcout << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
							振刀逻辑::执行长剑平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							wcout << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
							振刀逻辑::执行长剑平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(false, 1, false, 1);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SwBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行长剑平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1 &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (被震刀僵直状态(FatPlayerData.O_ActionName) && FatPlayerData.ObjCurTime < FatPlayerData.O_ActionDuration) {
					MyLog(武器名, "蓄力1.0前抓 被震刀僵直状态");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_BlueTime > 0.2) && (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade)) {
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					// male_saber_attack_charge_enter_idle_05我的攻击状态
					MyLog(武器名, "蓄力1.0前打斩马阔刀磐石");
					振刀逻辑::执行长剑平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if (MatchEnemy([](const PlayerData& enemy) {
				return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
				})) {
				MyLog(武器名, "蓄力1.0后抓远程/空手");
				出蓄::通用出蓄(FatPlayerData);
				return;
			}

			if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
				return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
				})) {
				MyLog(武器名, "蓄力1.0后抓出刀");
				出蓄::通用出蓄(FatPlayerData);
				return;
			}

			if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
				if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
					return !StrStrW(enemy.O_ActionName.data(), L"jump");
					}) &&
					可C切武器(WeaponType::Sw)) {
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
				else
					if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
						if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
							if (LocalPlayer_Data.MyBlueTime < 1.01) {
								Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
								MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

						}
						else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
							Sleep(Function::侧键博弈::蓄力抓振延迟);
							MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
							出蓄::通用出蓄(FatPlayerData);
						}

						return;
					}
			}


			if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); }) && GetTickCount64() - 长剑上次出蓄力时间 > 1000) {
				MyLog(武器名, "蓄力1.0后抓倒地");
				出蓄::通用出蓄(FatPlayerData);
				return;
			}

			if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
				MyLog(武器名, " 1.0后抓受击状态");
				出蓄::通用出蓄(FatPlayerData);
				return;
			}

			if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
				if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓长闪");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
					MyLog(武器名, "蓄力1.0后抓短闪");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
					MyLog("蓄力1.0后抓跳", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
					MyLog("蓄力1.0后抓蹲", 武器名);
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}

			if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
				MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
				出蓄::通用出蓄(FatPlayerData);
				return;
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}

void 振刀逻辑::链剑博弈() {
	int 重置方式 = 1;
	string 武器名 = "链剑博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力");
				//Sleep(2);
				//root_kmNet_mouse_left(1);
				root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下());
		if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入右蓄力小于7米");
			//Sleep(2);
			root_kmNet_mouse_right(1);
			//root_kmNet_mouse_right(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪 6-10");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		//if (FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
		//	/*auto O_NameHash = mem.Read<unsigned int>(FatPlayerData.Cache_NameHash + 0x18);
		//	float 出招后摇 = Function::侧键博弈::白刀蓝顶延迟;
		//	MyLog("出招后摇=", 出招后摇, "--招式哈希值=", O_NameHash);
		//	ULONGLONG WaitTIme = GetTickCount64() + 1500;
		//	while (WaitTIme > GetTickCount64()) {
		//		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		//		if (出招后摇 == 0) {
		//			break;
		//		}
		//		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
		//			MyLog("后摇判断--蓝顶出刀");
		//			break;
		//		}
		//		Sleep(1);
		//	}*/

		//	SwBlueAttackWrite(FatPlayerData);
		//	if (LocalPlayer_Data.MyBlueTime >= 0.5) {
		//		出蓄::通用出蓄(FatPlayerData);
		//	}
		//	else {
		//		振刀逻辑::执行链剑平A僵直抓取();
		//	}

		//}
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行链剑平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行链剑平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(false, 1, false, 1);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SwBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行链剑平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}

				if (被震刀僵直状态(FatPlayerData.O_ActionName) && FatPlayerData.ObjCurTime < FatPlayerData.O_ActionDuration) {
					MyLog(武器名, "蓄力1.0前抓 被震刀僵直状态");
					振刀逻辑::执行链剑平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::lj)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); }) && GetTickCount64() - 长剑上次出蓄力时间 > 1000) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::匕首博弈() {
	int 重置方式 = 1;
	string 武器名 = "匕首博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下());
		if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入右蓄力小于10米");
			//Sleep(2);
			root_kmNet_mouse_right(1);
			//root_kmNet_mouse_right(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪且距离小于", Function::侧键博弈::白刀抓长闪最远距离);
							振刀逻辑::执行匕首扇子平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪且距离小于", Function::侧键博弈::白刀抓短闪最远距离);
							振刀逻辑::执行匕首扇子平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {

					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (DaggeBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行匕首扇子平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}

				if (被震刀僵直状态(FatPlayerData.O_ActionName) && FatPlayerData.ObjCurTime < FatPlayerData.O_ActionDuration) {
					MyLog(武器名, "蓄力1.0前抓 被震刀僵直状态");
					振刀逻辑::执行匕首扇子平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::Dagge)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}


				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}

		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::双截棍博弈() {
	int 重置方式 = 1;
	string 武器名 = "双截棍博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			int changshanTemp = 0;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 右蓄力起手
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右跑蓄");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下() && !活化闪避);
		if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入左蓄力小于10米");
			//Sleep(2);
			root_kmNet_mouse_left(1);
			//root_kmNet_mouse_right(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}

	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行双截棍平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行双截棍平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (nunchucksBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行双截棍平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行双截棍平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::nunchucks)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::斩马刀博弈() {
	int 重置方式 = 1;
	string 武器名 = "斩马刀博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};

	//MyLog("最近玩家距离：", FatPlayerData.O_Dis);
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			{
				// 左蓄力起手
				if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
					MyLog("进入左蓄");
					//Sleep(2);
					root_kmNet_mouse_left(1);
					//root_kmNet_mouse_right(1);
				}
			}


			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			root_sendKeyUp(KEY_A);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_D);
			MyLog("进入左蓄力小于7米");
			root_kmNet_mouse_left(1);
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}

	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);

		/*磐石处理*/
		if (JudgePanshi(LocalPlayer_Data.MyActionName)) {
			MyLog("磐石状态 直接捏蓝");
			root_sendKeyUpAll();

			wstring 当前磐石 = LocalPlayer_Data.MyActionName;
			bool 超时 = true;
			int WaitTIm3Temp = 0;
			ULONGLONG WaitTIm3 = GetTickCount64() + 200;
			while (WaitTIm3 > GetTickCount64()) {
				//MyLog("磐石状态循环次数:", WaitTIm3Temp);
				if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					// 对面振刀了直接出
					超时 = false;
					break;
				}

				if (当前磐石 != LocalPlayer_Data.MyActionName && JudgePanshi(LocalPlayer_Data.MyActionName)) {
					// 磐石刷新
					当前磐石 = LocalPlayer_Data.MyActionName;
					MyLog("磐石状态2");
					ULONGLONG WaitTImb3 = GetTickCount64() + 300;
					while (WaitTImb3 > GetTickCount64()) {
						if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
							// 对面振刀了直接出
							超时 = false;
							break;
						}

						if (当前磐石 != LocalPlayer_Data.MyActionName && JudgePanshi(LocalPlayer_Data.MyActionName)) {
							// 磐石刷新
							当前磐石 = LocalPlayer_Data.MyActionName;
							MyLog("磐石状态3");
							ULONGLONG WaitTImb3 = GetTickCount64() + 300;
							while (WaitTImb3 > GetTickCount64()) {
								if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
									// 对面振刀了直接出
									超时 = false;
									break;
								}

								if (当前磐石 != LocalPlayer_Data.MyActionName && JudgePanshi(LocalPlayer_Data.MyActionName)) {
									// 磐石刷新
									当前磐石 = LocalPlayer_Data.MyActionName;
									MyLog("磐石状态4");
									ULONGLONG WaitTImc3 = GetTickCount64() + 300;
									while (WaitTImc3 > GetTickCount64()) {
										if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
											// 对面振刀了直接出
											超时 = false;
											break;
										}
									}
								}
							}
						}
					}
				}

				WaitTIm3Temp++;
				Sleep(1);
			}

			if (超时) {
				MyLog("后闪");
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}

					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);

				int WaitTIm3Temp = 0;
				ULONGLONG WaitTIm3 = GetTickCount64() + 1500;
				while (WaitTIm3 > GetTickCount64()) {
					//MyLog("磐石状态循环次数:", WaitTIm3Temp);

					if (Function::侧键博弈::蓄力抓白刀 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
						MyLog(武器名, "柄击抓出刀");
						振刀逻辑::斩马柄击释放();
					}

					超时 = false;
					WaitTIm3Temp++;
					Sleep(1);
				}
			}


		}
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !斩马A2蓄住(LocalPlayer_Data.MyActionName) && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行斩马刀平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行斩马刀平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					//振刀逻辑::通用拳切百裂();
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SaberBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行斩马刀平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行斩马刀平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓振 && MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); }) && LocalPlayer_Data.MyBlueTime < 1.80) {
					if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
						if (LocalPlayer_Data.MyBlueTime < 1.01) {
							Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
							MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
							出蓄::通用出蓄(FatPlayerData);
						}

					}
					else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
						Sleep(Function::侧键博弈::蓄力抓振延迟);
						MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
						出蓄::通用出蓄(FatPlayerData);
					}

					return;
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::长棍博弈() {
	int 重置方式 = 1;
	string 武器名 = "长棍博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.3) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 600;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		if (可C切武器(WeaponType::rod) && (rand() % 6 < 2)) {
			if (!判断鼠标右键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入右蓄力小于10米");
				//Sleep(2);
				//root_kmNet_mouse_left(1);
				root_kmNet_mouse_right(1);
			}
		}
		else {
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力小于10米");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}


		}
		//if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
		//	MyLog("进入左蓄力小于10米");
		//	//Sleep(2);
		//	root_kmNet_mouse_left(1);
		//	//root_kmNet_mouse_right(1);
		//}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离 + 2) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}
				if (((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1)) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离 + 1) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行长棍平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 + 1 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行长棍平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(true);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (rodBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行长棍平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行长棍平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
					if (MatchEnemy([](const PlayerData& enemy) {
						return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
						})) {
						MyLog(武器名, "蓄力1.0后抓远程/空手");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				
				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::rod)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::长枪博弈() {
	int 重置方式 = 1;
	string 武器名 = "长枪博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 600;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下() && !活化闪避);
		if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入左蓄力小于10米");
			//Sleep(2);
			root_kmNet_mouse_left(1);
			//root_kmNet_mouse_right(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离 + 1) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行长枪平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 + 1 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行长枪平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(true);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SpearBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行长枪平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行长枪平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::Spear)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::拳刃博弈() {
	int 重置方式 = 1;
	string 武器名 = "拳刃博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 600;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下() && !活化闪避);
		if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入左蓄力小于10米");
			root_kmNet_mouse_left(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				//else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
				//	root_sendKeyPress(KEY_W);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入W闪 5-6米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(5);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_W);
				//	振刀逻辑::左右键直接松开();
				//}
				//else if (FatPlayerData.O_Dis <= 5) {
				//	root_sendKeyPress(KEY_S);
				//	root_sendKeyPress(KEY_LEFTSHIFT);
				//	ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
				//	while (WaitTIme3 > GetTickCount64()) {
				//		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				//		if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
				//		{
				//			MyLog("进入s闪 小于5米");
				//			break;
				//		}
				//		else if (振刀逻辑::是否特殊状态())
				//		{
				//			root_sendKeyUpAll();
				//			return;
				//		}
				//		Sleep(1);
				//	}
				//	root_sendKeyUp(KEY_LEFTSHIFT);
				//	root_sendKeyUp(KEY_S);
				//	振刀逻辑::左右键直接松开();

				//}
			}
			else {
				//通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离 + 1) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行拳刃平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 + 1 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行拳刃平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(true);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SpearBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行拳刃平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::punch)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}
void 振刀逻辑::飞刀博弈() {
	int 重置方式 = 1;
	string 武器名 = "飞刀博弈--";
	const float multiDistance = 7.0f;
	const auto multiEnemies = data_buffers.GetPlayersWithinDistance(multiDistance);
	const bool useMultiLogic = multiEnemies.size() >= 2;
	auto MatchEnemy = [&](const auto& predicate) -> bool {
		if (useMultiLogic) {
			for (const auto& enemy : multiEnemies) {
				if (!predicate(enemy)) {
					return false;
				}
			}
			return true;
		}
		return predicate(FatPlayerData);
	};
	if (FatPlayerData.O_Dis > Function::侧键博弈::博弈距离范围) {

		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			//root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int changshanTemp = 0;
			ULONGLONG WaitTIme3 = GetTickCount64() + 1000;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
				{
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				changshanTemp++;
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}

		/*起手方式*/
		if (LocalPlayer_Data.MyCurEnergy >= 5000) {
			// 左蓄力起手
			if (!判断鼠标左键是否按下() && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
				MyLog("进入左蓄力");
				//Sleep(2);
				root_kmNet_mouse_left(1);
				//root_kmNet_mouse_right(1);
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.2) {
				MyLog("起手闪避");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
				while (WaitTIme4 > GetTickCount64()) {
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
					{
						MyLog("起手式-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_LEFTSHIFT);
				振刀逻辑::左右键直接松开();
			}

		}
		else {
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme4 = GetTickCount64() + 500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (滑铲状态(LocalPlayer_Data.MyActionName) || JudgeSquat(LocalPlayer_Data.MyActionName))
				{
					MyLog("进入滑铲");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			Sleep(160);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeJump(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲起跳");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			Sleep(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme6 = GetTickCount64() + 600;
			while (WaitTIme6 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("滑铲落地");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

	if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围) {
		//如果真(跳跃状态(全_本人地址) 或 滞空状态(全_本人地址) 或 跳A状态(全_本人地址) 或 壁击状态(全_本人地址) 或 钩锁空A状态(全_本人地址))
		if (JudgeJump(LocalPlayer_Data.MyActionName) || JudgeJumpAndA(LocalPlayer_Data.MyActionName) || Judgefly(LocalPlayer_Data.MyActionName)
			|| 壁击状态(LocalPlayer_Data.MyActionName) || 钩锁空A状态(LocalPlayer_Data.MyActionName)) {
			ULONGLONG WaitTIme = GetTickCount64() + 1500;
			while (WaitTIme > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName) || 跳B状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		if (二段跳跃状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyPress(KEY_C);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
			while (WaitTIme2 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (落地状态(LocalPlayer_Data.MyActionName))
				{
					跳出方式 = 0;
					break;
				}
				else if (LocalPlayer_Data.MyEndureLevel == 6)
				{
					root_sendKeyUpAll();
					跳出方式 = 1;
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_sendKeyUp(KEY_C);
			Sleep(10);
			root_sendKeyPress(KEY_W);
			Sleep(10);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
			while (WaitTIme3 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName))
				{
					MyLog("下坠后进入闪避状态");
					break;
				}
				else if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(5);
			}
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			振刀逻辑::左右键直接松开();
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();
		}


		//MyLog("鼠标左键状态:", !判断鼠标左键是否按下() && !活化闪避);
		if (!判断鼠标左键是否按下() && !活化闪避 && !活化闪避 && LocalPlayer_Data.MyRangeReactionType != 12) {
			MyLog("进入左蓄力小于10米");
			root_kmNet_mouse_left(1);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		if (LocalPlayer_Data.MyBlueTime >= 1) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (FatPlayerData.O_Dis <= Function::侧键博弈::博弈距离范围 && FatPlayerData.O_Dis > Function::侧键博弈::博弈对蓄力距离) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W长闪 6-10");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				else if (FatPlayerData.O_Dis <= 6 && FatPlayerData.O_Dis >= 5) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme3 = GetTickCount64() + 500;
					int changshanTemp = 0;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入W闪 5-6米");
							break;
						}
						else if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_W);
					振刀逻辑::左右键直接松开();
				}
				else if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
					while (WaitTIme3 > GetTickCount64()) {
						//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180)
						{
							MyLog("进入s闪 小于5米");
							break;
						}
						else if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUp(KEY_S);
					振刀逻辑::左右键直接松开();

				}
			}
			else {
				通用切刀重置蓄力();
				//root_sendKeyPress(KEY_C);

			}

		}


	}

	if (FatPlayerData.O_Dis <= Function::LogicKnife::博弈对蓄力距离) {
		root_sendKeyUp(KEY_W);
		//MyLog("进入小于5距离判断");
		if (!useMultiLogic && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				//wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl;
				if (水娘水矛束缚状态(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel == 6) {
					MyLog(武器名, "蓄力1.0前抓 水矛");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
					// male_all_unarmed_be_tied
				}
				if (被武田夺刀状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 武田夺刀");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) && 抓闪判断(FatPlayerData.O_ActionName)) {
					if (Function::侧键博弈::白刀抓长闪 && JudgeLongFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓长闪最远距离 + 1) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 长闪");
							振刀逻辑::执行拳刃平A僵直抓取();
							return;
						}
					}
					else if (FatPlayerData.O_ActorWeapon != WeaponType::punch && Function::侧键博弈::白刀抓短闪 && JudgeDuanFlash(FatPlayerData.O_ActionName)) {
						if (FatPlayerData.O_Dis <= Function::侧键博弈::白刀抓短闪最远距离 && 抓短闪时间判断()) {
							wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
							MyLog(武器名, "蓄力1.0前抓 短闪");
							振刀逻辑::执行拳刃平A僵直抓取();
							return;
						}
					}

				}


				if (Function::LogicKnife::嫖刀开关 && (rand() % 2)) {
					if (FatPlayerData.O_BlueTime >= 0.2 && FatPlayerData.O_BlueTime < 0.34 && LocalPlayer_Data.MyCurEnergy >= 5000 && !AB2蓄力状态(LocalPlayer_Data.MyActionName)) {
						//if (rand() % 2) {
						通用嫖刀抓取();
						return;
						//}
					}
				}
				if (倒地状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前 跳A");
					处决后双跳A();
					return;
				}

				if (被壁击状态(FatPlayerData.O_ActionName) && FatPlayerData.O_ActionDuration > 0.8) {
					MyLog(武器名, "蓄力1.0前 抓壁击");
					振刀逻辑::通用拳切百裂(true);
					return;
				}

				if (JudgeYuanCheng()  || (FatPlayerData.O_WeaponID == WeaponType::Node && FatPlayerData.O_BlueTime <= 0)) {
					MyLog(武器名, "蓄力1.0前抓远程/空手");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if ((JudgeZhanLi(FatPlayerData.O_ActionName) || JudgeGouSuo(FatPlayerData.O_ActionName)) && FatPlayerData.O_BlueTime < 0.1 && FatPlayerData.O_EndureLevel != 20 && FatPlayerData.O_EndureLevel != 11) {
					//	wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-捏蓝时间=" << FatPlayerData.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
					MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (Function::侧键博弈::白刀抓蓄力后摇 && ShakeBack(FatPlayerData.O_ActionName) && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
					Sleep(Function::侧键博弈::白刀抓蓄力后摇延迟);
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (JudgeJump(FatPlayerData.O_ActionName) || JudgeCrouch(FatPlayerData.O_ActionName) || JudgeSquat(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓出跳蹲");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}


				if (Function::侧键博弈::白刀蓝顶 && FatPlayerData.O_RangeReactionType == 12 && FatPlayerData.O_EndureLevel != 20) {
					MyLog(武器名, "蓄力1.0前进入蓝顶判断");
					//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
					if (SpearBlueAttackWrite(FatPlayerData)) {
						MyLog(武器名, "蓄力1.0前蓝顶抓");
						if (LocalPlayer_Data.MyBlueTime >= 0.502) {
							出蓄::通用出蓄(FatPlayerData);
						}
						else {
							振刀逻辑::执行拳刃平A僵直抓取();
						}
						return;
					}
				}

				if (Function::侧键博弈::白刀抓振 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓振刀");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if ((FatPlayerData.O_EndureLevel == 5 || FatPlayerData.O_EndureLevel == 1) &&
					!JudgeShortFlash(FatPlayerData.O_ActionName)) {
					// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
					wcout << "敌人动作=" << FatPlayerData.O_ActionName << "-距离=" << FatPlayerData.O_Dis << endl;
					MyLog(武器名, "蓄力1.0前抓 状态5/1");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
				if (FatPlayerData.O_EndureLevel == 6 && 受击站立状态(FatPlayerData.O_ActionName)) {
					MyLog(武器名, "蓄力1.0前抓 受击站立状态");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}

				if (FatPlayerData.O_RangeReactionType == 10 && 出招完毕状态(FatPlayerData.O_ActionName)) {
					//wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
					MyLog(武器名, "蓄力1.0前抓 出招完毕状态");
					振刀逻辑::执行拳刃平A僵直抓取();
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
			if ((!英雄技能状态(FatPlayerData.O_ActionName))) {
				if (MatchEnemy([](const PlayerData& enemy) {
					return IsMeleeWeaponYC(static_cast<WeaponType>(enemy.O_ActorWeapon)) || enemy.O_ActorWeapon == WeaponType::Node;
					})) {
					MyLog(武器名, "蓄力1.0后抓远程/空手");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓白刀 && MatchEnemy([](const PlayerData& enemy) {
					return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20;
					})) {
					MyLog(武器名, "蓄力1.0后抓出刀");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return JudgeZhenDaoing(enemy.O_ActionName); })) {
					if (Function::侧键博弈::博弈切刀抓振刀 && !是否单刀武器() && MatchEnemy([](const PlayerData& enemy) {
						return !StrStrW(enemy.O_ActionName.data(), L"jump");
						}) &&
						可C切武器(WeaponType::Knife)) {
						振刀逻辑::C滚轮切刀重置蓄力();
						return;
					}
					else
						if (Function::侧键博弈::蓄力抓振 && LocalPlayer_Data.MyBlueTime < 1.15) {
							if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon == WeaponType::nunchucks; })) {
								if (LocalPlayer_Data.MyBlueTime < 1.01) {
									Sleep(Function::侧键博弈::蓄力抓振延迟 + 25);
									MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
									出蓄::通用出蓄(FatPlayerData);
								}

							}
							else if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_ActorWeapon != WeaponType::nunchucks; })) {
								Sleep(Function::侧键博弈::蓄力抓振延迟);
								MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
								出蓄::通用出蓄(FatPlayerData);
							}

							return;
						}
				}


				if (Function::侧键博弈::蓄力抓倒地 && MatchEnemy([](const PlayerData& enemy) { return 倒地状态(enemy.O_ActionName); })) {
					MyLog(武器名, "蓄力1.0后抓倒地");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (Function::侧键博弈::蓄力抓受击 && MatchEnemy([](const PlayerData& enemy) { return 受击状态(enemy.O_EndureLevel, enemy.O_ActionName); })) {
					MyLog(武器名, " 1.0后抓受击状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}

				if (MatchEnemy([](const PlayerData& enemy) { return enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20; })) {
					if (Function::侧键博弈::蓄力抓长闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeLongFlash(enemy.O_ActionName); })) {
						MyLog(武器名, "蓄力1.0后抓长闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓短闪 && MatchEnemy([](const PlayerData& enemy) { return JudgeDuanFlash(enemy.O_ActionName); }) && 蓄力抓短闪判断()) {
						MyLog(武器名, "蓄力1.0后抓短闪");
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓起跳 && MatchEnemy([](const PlayerData& enemy) { return JudgeJump(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓跳", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}

					if (Function::侧键博弈::蓄力抓下蹲 && MatchEnemy([](const PlayerData& enemy) { return JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName); })) {
						MyLog("蓄力1.0后抓蹲", 武器名);
						出蓄::通用出蓄(FatPlayerData);
						return;
					}
				}

				if (MatchEnemy([](const PlayerData& enemy) { return 被震刀僵直状态(enemy.O_ActionName) && enemy.ObjCurTime < enemy.O_ActionDuration; })) {
					MyLog(武器名, "蓄力1.0后抓 被震刀僵直状态");
					出蓄::通用出蓄(FatPlayerData);
					return;
				}
			}
		}
		if (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀最小时间) {
			if (重置方式 == 0) {
				// cc重置
				if (LocalPlayer_Data.MyEndureLevel == 20) {
					MyLog(武器名, "蓄力一段后CC重置蓄力");
					振刀逻辑::通用CC断蓄力();
					return;
				}
			}
			else {
				// 切刀重置
				
				if ((LocalPlayer_Data.MyEndureLevel == 20 && FatPlayerData.O_BlueTime > 0.5 && LocalPlayer_Data.MyBlueTime > Function::侧键博弈::动态断蓄时间点) || (LocalPlayer_Data.MyBlueTime > Function::侧键博弈::断续切刀时间)) {
					MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力--蓄力时间:", LocalPlayer_Data.MyBlueTime, "--断蓄时间点:", Function::侧键博弈::动态断蓄时间点);
					振刀逻辑::C滚轮切刀重置蓄力();
					return;
				}
			}
		}

	}

}

void 振刀逻辑::空手博弈() {
	string 标识 = "空手博弈--";
	if (LocalPlayer_Data.Weapon_1 != 0 || LocalPlayer_Data.Weapon_2 != 0) {
		// 切换武器1
		MyLog("直接切武器1");
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);  // 初始的按键按下
		Sleep(10);
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);

		int wpType = -1;
		ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
		ULONGLONG lastKeyPressTime = GetTickCount64();  // 记录上次按键的时间
		while (WaitTIme2 > GetTickCount64()) {
			// 每200毫秒执行一次root_sendKeyPress
			if (GetTickCount64() - lastKeyPressTime >= 200) {
				root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
				Sleep(10);
				root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
				lastKeyPressTime = GetTickCount64();  // 更新按键时间
			}

			// 你原来的逻辑
			if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && !IsMeleeWeaponYC(LocalPlayer_Data.MyWeaponType) && LocalPlayer_Data.MyWeaponType != 0) {
				wpType = 0;
				MyLog(标识, "切刀成功");
				break;
			}
			else if (IsMeleeWeaponYC(LocalPlayer_Data.MyWeaponType)) {
				wpType = 1;
				MyLog(标识, "当前远程武器");
				break;
			}

			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);  // 控制循环速度
		}

		if (wpType == 1) {
			root_sendKeyPress(KEY_2_AT);
			Sleep(10);
			root_sendKeyUp(KEY_2_AT);
			ULONGLONG WaitTImeb2 = GetTickCount64() + 1500;
			while (WaitTImeb2 > GetTickCount64()) {
				root_sendKeyPress(KEY_2_AT);
				Sleep(10);
				root_sendKeyUp(KEY_2_AT);
				if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && !IsMeleeWeaponYC(LocalPlayer_Data.MyWeaponType) && LocalPlayer_Data.MyWeaponType != 0) {
					wpType = 0;
					MyLog(标识, "切刀成功");
					break;
				}

				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		//root_sendKeyPress(KEY_2_AT);

	}
	else {
		// 巴掌博弈
	}
}
/**************武器博弈逻辑结束************/

void 振刀逻辑::双刀月影闪切横刀A僵直抓取() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	root_kmNet_mouse_right(1);
	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {
			MyLog("切横刀成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*root_kmNet_mouse_left(0);
	Sleep(200);
	if (FatPlayerData.O_EndureLevel != 20) {
		执行横刀平A僵直抓取();
	}*/
}

void 振刀逻辑::双刀月影闪切太刀A僵直抓取() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	root_kmNet_mouse_left(1);
	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
			MyLog("切太刀成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*root_kmNet_mouse_left(0);
	Sleep(100);
	if (FatPlayerData.O_EndureLevel != 20) {
		执行太刀平A僵直抓取();
	}*/
}

void 振刀逻辑::太刀月影闪切双刀A僵直抓取() {
	string 标识 = "太刀月影闪切双刀A僵直抓取--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	root_kmNet_mouse_right(1);
	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		//MyLog(标识, "武器id：", LocalPlayer_Data.MyWeaponType);
		if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
			MyLog("切双刀成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*root_kmNet_mouse_right(0);
	Sleep(100);
	执行双刀平A僵直抓取();*/

}
void 振刀逻辑::太刀月影闪切长剑A僵直抓取() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	root_kmNet_mouse_right(1);
	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
			MyLog("切长剑成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*root_kmNet_mouse_right(0);
	Sleep(170);
	执行双刀平A僵直抓取();*/

}
void 振刀逻辑::太刀月影闪切双戟A僵直抓取() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	root_kmNet_mouse_right(1);
	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
			MyLog("切双戟成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*root_kmNet_mouse_right(0);
	Sleep(100);
	执行双刀平A僵直抓取();*/

}

void 振刀逻辑::执行双戟平A僵直抓取() {
	string 标识 = "双戟平A僵直抓取--";
	int xunhuanTemp = 0;
	if (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade) {
		xunhuanTemp = 30;
	}
	root_sendKeyUpAll();
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	auto start = std::chrono::high_resolution_clock::now();


	//MyLog();
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		if (B1状态(LocalPlayer_Data.MyActionName) && !跑B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			跳出方式 = 0;
			break;
		}
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			跳出方式 = 3;
			break;
		}
		if (跑B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			跳出方式 = 2;

			break;
		}
		if ((A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName)) /*|| 判断出招()*/)
		{
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
	//std::cout << "双刀MyRangeReactionType时间: " << duration_ms1 << " 毫秒" << std::endl;
	if (跳出方式 == 0 || 跳出方式 == 2 || 跳出方式 == 3) {
		bool 未命中状态 = false;
		bool 敌人磐石 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (WaitTIme3Time > 60 + xunhuanTemp && FatPlayerData.O_ActionDuration > 0.35f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("双戟A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}

				else
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟太刀
						通用C切百裂();
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟横刀
						通用C切百裂();
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双截棍
						通用C切百裂();
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双刀
						通用C切百裂();
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Sw && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Sw && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟长剑
						通用C切百裂(1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟链剑
						通用C切百裂(1);
					}
					else if ((跳出方式 == 2 || 跳出方式 == 0) && LocalPlayer_Data.MyCurEnergy > 3100) {
						单刀拳双切A();
						break;
					}
					else if (跳出方式 == 3) {
						通用钩锁白裂(0, true);
					}
					else {
						双刀BB();
						root_kmNet_mouse_right(0);
						Sleep(2);
						root_kmNet_mouse_right(1);
						ULONGLONG WaitTImeB11 = GetTickCount64() + 600;
						while (WaitTImeB11 > GetTickCount64()) {

							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						break;
					}
			}
			else if ((FatPlayerData.O_ActionDuration > 0.15f && FatPlayerData.O_ActionDuration < 0.35f)) {
				双刀BB();
				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 600;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				break;
			}

			if (拼刀状态(FatPlayerData.O_ActionName)) {
				MyLog(标识, "拼刀");
				未命中状态 = true;
				break;
			}

			if (WaitTIme3Time > 100 && JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "对方磐石");
				未命中状态 = true;
				敌人磐石 = true;
				break;
			}

			if (WaitTIme3Time > 130 && FatPlayerData.O_EndureLevel != 6) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog("进入未命中逻辑");
			if (敌人磐石 && LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
				LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd ||
				LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
				LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd
				) {
				// 连续打三个c切
				双刀流打磐石();
				return;
			}
			if (可C切武器(WeaponType::twinblades)) {
				C滚轮切刀重置蓄力();
				return;
			}
			if (FatPlayerData.O_Dis < 3) {
				MyLog(标识, "敌人小于3米，直接捏住");
				root_sendKeyUpAll();

				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						MyLog(标识, "出刀完毕捏住");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				return;

			}
			else {
				通用切刀重置蓄力();

				return;
			}
		}
	}

	if (跳出方式 == 1) {
		bool 未命中状态 = true;
		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_EndureLevel == 6 && FatPlayerData.O_ActionDuration > 0.35f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "b2 a2命中，对面僵直:", FatPlayerData.O_ActionDuration);
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				未命中状态 = false;
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				{
					if (FatPlayerData.O_Dis < 3 && LocalPlayer_Data.MyCurEnergy < 6000) {
						MyLog(标识, "敌人小于3米，直接捏住");
						root_sendKeyUpAll();

						root_kmNet_mouse_right(0);
						Sleep(2);
						root_kmNet_mouse_right(1);
						ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
						while (WaitTImeB11 > GetTickCount64()) {

							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 4000) {
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						return;

					}
					else {
						通用切刀重置蓄力();

						return;
					}
					break;
				}
			}

			if (拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (LocalPlayer_Data.MyRangeReactionType == 10 && WaitTIme3Time > 150 && FatPlayerData.O_EndureLevel != 6) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog(标识, "进入未命中逻辑");
			if (FatPlayerData.O_Dis < 3) {
				MyLog(标识, "敌人小于3米，直接捏住");
				root_sendKeyUpAll();

				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						MyLog(标识, "出刀完毕捏住");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				return;

			}
			else {
				通用切刀重置蓄力();

				return;
			}
		}
		return;
	}

}

void 振刀逻辑::执行双刀平A僵直抓取() {
	string 标识 = "双刀平A僵直抓取--";
	int xunhuanTemp = 0;
	root_sendKeyUpAll();
	if (FatPlayerData.O_ActorWeapon == WeaponType::saber || FatPlayerData.O_ActorWeapon == WeaponType::Blade) {
		xunhuanTemp = 30;
	}
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	//MyLog();
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		if ((B1状态(LocalPlayer_Data.MyActionName)) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑B1状态(LocalPlayer_Data.MyActionName))
		{
			跳出方式 = 0;
			break;
		}
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			跳出方式 = 3;
			break;
		}
		if (跑B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			跳出方式 = 2;

			break;
		}
		if ((A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName)) /*|| 判断出招()*/)
		{
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (跳出方式 == 0 || 跳出方式 == 2 || 跳出方式 == 3) {
		MyLog("平A方式=", 跳出方式);
		bool 敌人磐石 = false;
		bool 未命中状态 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 800;
		int WaitTIme3Time = 0;
		MyLog("日志1");
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
			//MyLog(标识, "判断循环:", WaitTIme3Time);
			if (WaitTIme3Time > 60 + xunhuanTemp && FatPlayerData.O_EndureLevel == 6 && FatPlayerData.O_ActionDuration > 0.35f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "双刀命中，对面僵直:", FatPlayerData.O_ActionDuration);
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "双刀A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				if(JudgePanshi(FatPlayerData.O_ActionName)){
					MyLog(标识, "对方磐石");
					未命中状态 = true;
					敌人磐石 = true;
					break;
				}

				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 太双
					通用C切百裂();
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀横刀
					通用C切百裂();
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀双截棍
					通用C切百裂();
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟双刀
					通用C切百裂();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Sw && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Sw && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀长剑
					通用C切百裂(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀链剑
					通用C切百裂(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀长棍
					通用C切百裂();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 双刀长枪
					通用C切百裂();
					//长枪连招::拳切AA跳B();
				}else if ((跳出方式 == 2 || 跳出方式 == 0) && LocalPlayer_Data.MyCurEnergy > 3100) {
					单刀拳双切A();
					break;
				}
				else if (跳出方式 == 3) {
					通用钩锁白裂(0, true);
				}
				else {
					双刀BB();
					root_kmNet_mouse_right(0);
					Sleep(2);
					root_kmNet_mouse_right(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 650;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					break;
				}
				break;
			}
			else if ((FatPlayerData.O_ActionDuration > 0.15f && FatPlayerData.O_ActionDuration < 0.35f)) {
				双刀BB();
				
				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 740;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				break;
			}

			if (拼刀状态(FatPlayerData.O_ActionName)) {
				MyLog(标识, "拼刀");
				未命中状态 = true;
				break;
			}

			if (WaitTIme3Time > 120 && JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "对方磐石");
				未命中状态 = true;
				敌人磐石 = true;
				break;
			}

			if (WaitTIme3Time > 130 && FatPlayerData.O_EndureLevel != 6) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog(标识, "进入未命中逻辑");
			if (敌人磐石 && LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
				LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades ||
				LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
				LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades
				) {
				// 连续打三个c切
				双刀流打磐石();
				return;
			}
			if (可C切武器(WeaponType::twinblades)) {
				C滚轮切刀重置蓄力();
				return;
			}
			if (FatPlayerData.O_Dis < 3) {
				MyLog(标识, "敌人小于3米，直接捏住");
				root_sendKeyUpAll();

				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						MyLog(标识, "出刀完毕捏住");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				return;

			}
			else {
				通用切刀重置蓄力();

				return;
			}
		}
	}

	if (跳出方式 == 1) {
		bool 未命中状态 = true;
		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_EndureLevel == 6 && FatPlayerData.O_ActionDuration > 0.35f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "b2 a2命中，对面僵直:", FatPlayerData.O_ActionDuration);
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				未命中状态 = false;
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "双刀A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				{
					if (FatPlayerData.O_Dis < 3) {
						MyLog(标识, "敌人小于3米，直接捏住");
						root_sendKeyUpAll();

						root_kmNet_mouse_right(0);
						Sleep(2);
						root_kmNet_mouse_right(1);
						ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
						while (WaitTImeB11 > GetTickCount64()) {

							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						return;

					}
					else {
						通用切刀重置蓄力();

						return;
					}
					break;
				}
			}

			if (拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (LocalPlayer_Data.MyRangeReactionType == 10 && WaitTIme3Time > 150 && FatPlayerData.O_EndureLevel != 6) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog(标识, "进入未命中逻辑");
			if (FatPlayerData.O_Dis < 3) {
				MyLog(标识, "敌人小于3米，直接捏住");
				root_sendKeyUpAll();

				ULONGLONG WaitTIme2 = GetTickCount64() + 500;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						MyLog(标识, "出刀完毕捏住");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				return;

			}
			else {
				通用切刀重置蓄力();

				return;
			}
		}
		return;
	}
}

void 振刀逻辑::双刀BB右2() {
	string 标识 = "双刀BB右2--";
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_kmNet_mouse_right(0);
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (B1状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "B1状态");
			break;
		}
		Sleep(1);
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog(标识, "侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "攻击状态12");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog(标识, "侧键松开");
		root_sendKeyUpAll();
		return;
	}
	Sleep(100);
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// B2
	ULONGLONG WaitTIme4 = GetTickCount64() + 1500;
	while (WaitTIme4 > GetTickCount64()) {
		if (B2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyUpAll();

	bool 未命中状态 = true;
	ULONGLONG WaitTIme33 = GetTickCount64() + 500;
	int WaitTIme3Time = 0;
	while (WaitTIme33 > GetTickCount64()) {
		//MyLog(标识, "对面僵直", FatPlayerData.O_ActionDuration);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

		if (FatPlayerData.O_EndureLevel == 6 && FatPlayerData.O_ActionDuration > 0.35f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "命中，对面僵直:", FatPlayerData.O_ActionDuration);

			未命中状态 = false;
			if (!振刀逻辑::侧键按下或半自动()) {
				MyLog(标识, "侧键松开");
				root_sendKeyUpAll();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
				MyLog(标识, "双刀A浮空/倒地后跳A");
				处决后双跳A();
				break;
			}
			{
				if (FatPlayerData.O_Dis < 4.5) {
					MyLog(标识, "敌人小于4.5米，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_right(0);
					Sleep(2);
					root_kmNet_mouse_right(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					return;

				}
				else {
					通用切刀重置蓄力();

					return;
				}
				break;
			}
		}

		if (拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			未命中状态 = true;
			break;
		}

		if (LocalPlayer_Data.MyRangeReactionType == 10 && WaitTIme3Time > 150 && FatPlayerData.O_EndureLevel != 6) {
			未命中状态 = true;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme3Time++;
		Sleep(1);
	}

	if (未命中状态) {
		MyLog(标识, "进入未命中逻辑");
		if (FatPlayerData.O_Dis < 4.5) {
			MyLog(标识, "敌人小于4.5米，直接捏住");
			root_sendKeyUpAll();


			root_kmNet_mouse_right(0);
			Sleep(2);
			root_kmNet_mouse_right(1);
			ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
			while (WaitTImeB11 > GetTickCount64()) {

				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			return;
		}
		if (LocalPlayer_Data.MyCurEnergy > 5000) {
			root_sendKeyPress(KEY_S);
			root_sendKeyPress(KEY_LEFTSHIFT);
			ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
			while (WaitTIme11 > GetTickCount64()) {
				if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			root_sendKeyUp(KEY_S);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(10);
			return;

		}
		else {
			通用切刀重置蓄力();

			return;
		}
	}
	return;
}

void 振刀逻辑::匕首扇子BBCA() {
	// B2
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_kmNet_mouse_right(0);
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (B2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
	while (WaitTIme6 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
}
void 振刀逻辑::双刀BB() {
	root_kmNet_mouse_right(0);
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (B2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

}

void 振刀逻辑::双刀流打磐石() {
	string 标识 = "双刀流破磐石--";
	MyLog(标识, "开始");
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);

	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog(标识, "下蹲判断循环次数:", WaitTIme6Temp);
		if (WaitTIme6Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	int WaitTIme2Temp = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		//MyLog(标识, "切武器循环次数:", WaitTIme2Temp);
		if (WaitTIme2Temp > 40 || LocalPlayer_Data.MyWeaponType == 武器2) {
			MyLog(标识, "切武器2成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme2Temp++;
		Sleep(1);
	}


	root_sendKeyUp(KEY_C);
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIm3 = GetTickCount64() + 500;
	while (WaitTIm3 > GetTickCount64()) {
		if (A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_left(0);

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	ULONGLONG WaitTIm4 = GetTickCount64() + 500;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	//再次C切A
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTImeB1Temp = 0;
	ULONGLONG WaitTImeB1 = GetTickCount64() + 1500;
	while (WaitTImeB1 > GetTickCount64()) {
		//MyLog(标识, "再次C切下蹲判断循环次数:", WaitTImeB1Temp);
		//if (WaitTIme6Temp > 40 ||
		if (WaitTImeB1Temp > 30 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeB1Temp++;
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}
	int WaitTImeB2Temp = 0;
	ULONGLONG WaitTImeB2 = GetTickCount64() + 500;
	while (WaitTImeB2 > GetTickCount64()) {
		//MyLog(标识, "切武器循环次数:", WaitTImeB2Temp);
		if ((WaitTImeB2Temp > 30 || LocalPlayer_Data.MyWeaponType != 武器2) && LocalPlayer_Data.MyWeaponType != 0) {
			MyLog("切回武器1成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeB2Temp++;
		Sleep(1);
	}

	root_sendKeyUp(KEY_C);
	Sleep(23);
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_left(1);
	int WaitTImB3Temp = 0;
	ULONGLONG WaitTImB3 = GetTickCount64() + 500;
	while (WaitTImB3 > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTImB3Temp > 15 || A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImB3Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_left(0);

	ULONGLONG WaitTImB4 = GetTickCount64() + 500;
	while (WaitTImB4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTImeB5 = GetTickCount64() + 500;
	while (WaitTImeB5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyUp(KEY_C);
	C滚轮切刀重置蓄力();
	//root_sendKeyPress(KEY_C);
	//int WaitTIme6Tempb = 0;
	//ULONGLONG WaitTImeb1 = GetTickCount64() + 500;
	//while (WaitTImeb1 > GetTickCount64()) {
	//	//MyLog(标识, "下蹲判断循环次数:", WaitTIme6Temp);
	//	if (WaitTIme6Tempb > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	WaitTIme6Temp++;
	//	Sleep(1);
	//}

	//if (当前武器位置 == 1) {
	//	root_sendKeyPress(KEY_2_AT);
	//}
	//else {
	//	root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	//}
	//Sleep(5);
	//if (当前武器位置 == 1) {
	//	root_sendKeyUp(KEY_2_AT);
	//}
	//else {
	//	root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	//}
	//int WaitTIme2Tempb = 0;
	//ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	//while (WaitTImeb2 > GetTickCount64()) {
	//	//MyLog(标识, "切武器循环次数:", WaitTIme2Temp);
	//	if (WaitTIme2Tempb > 40 || LocalPlayer_Data.MyWeaponType == 武器2) {
	//		MyLog(标识, "切武器2成功");
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	WaitTIme2Temp++;
	//	Sleep(1);
	//}


	//root_sendKeyUp(KEY_C);
	//root_kmNet_mouse_left(1);
	//ULONGLONG WaitTIm3b = GetTickCount64() + 500;
	//while (WaitTIm3b > GetTickCount64()) {
	//	if (A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	Sleep(1);
	//}

	//root_kmNet_mouse_left(0);

	//if (!振刀逻辑::侧键按下或半自动()) {
	//	MyLog("侧键松开");
	//	root_sendKeyUpAll();
	//	return;
	//}
	//ULONGLONG WaitTIm4b = GetTickCount64() + 500;
	//while (WaitTIm4b > GetTickCount64()) {
	//	if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName)) {
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	Sleep(1);
	//}
	//ULONGLONG WaitTIme5b = GetTickCount64() + 500;
	//while (WaitTIme5b > GetTickCount64()) {
	//	if (LocalPlayer_Data.MyRangeReactionType == 10) {
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	Sleep(1);
	//}
}

// 平A类型0左 1右
void 振刀逻辑::通用C切百裂(int 平A类型) {
	string 标识 = "通用C切百裂--";
	auto start = std::chrono::high_resolution_clock::now();
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;

	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}

	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}

	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTIme1Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog(标识, "下蹲判断循环次数:", WaitTIme1Temp);
		if (/*WaitTIme1Temp > 70 || */JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态() || !判断敌方是否受击中())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTIme1Temp > 200) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}

	/*
	int WaitTIme1Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		MyLog(标识,"下蹲判断循环次数:", WaitTIme1Temp);
		// 强制读取一次本地动作名，避免依赖 LocalPlayer_Object_Data2 的刷新频率
		uintptr_t ActionNamePtr = 0;
		if (Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR)
		{
			const uintptr_t LayerPtr = mem.Read<uintptr_t>(Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR + 0x20);
			if (LayerPtr)
			{
				ActionNamePtr = mem.Read<uintptr_t>(LayerPtr + 0x18);
			}
		}
		if (ActionNamePtr)
		{
			wchar_t ActionName[80] = { 0 };
			mem.Read(ActionNamePtr + 0x14, ActionName, sizeof(ActionName));
			if (wcsstr(ActionName, L"squat"))
			{
				break;
			}
		}
		else if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态() || !判断敌方是否受击中())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}
	*/
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
	MyLog("CC执行时间: ", duration_ms1, " 毫秒");
	//std::cout << "CC执行时间: " << duration_ms1 << " 毫秒" << std::endl;

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	int WaitTIme2Temp = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	MyLog(标识, "切武器循环次数:", WaitTIme2Temp);
	while (WaitTIme2 > GetTickCount64()) {
		WeaponType weaponTypeFast = LocalPlayer_Data.MyWeaponType;
		if (Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR)
		{
			weaponTypeFast = mem.Read<WeaponType>(Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR + Offset::ActorModel.WeaponType);
		}
		if (weaponTypeFast == static_cast<WeaponType>(武器2)) {
			MyLog("切武器2成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme2Temp++;
		Sleep(1);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	//std::cout << "C切执行时间: " << duration_ms << " 毫秒" << std::endl;
	MyLog("CC执行时间: ", duration_ms, " 毫秒");
	root_sendKeyUp(KEY_C);

	if (平A类型 == 0) {
		root_kmNet_mouse_left(1);
	}
	else {
		root_kmNet_mouse_right(1);
	}

	ULONGLONG WaitTIm3 = GetTickCount64() + 500;
	while (WaitTIm3 > GetTickCount64()) {
		if (A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (平A类型 == 0) {
		root_kmNet_mouse_left(0);
	}
	else {
		root_kmNet_mouse_right(0);
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	ULONGLONG WaitTIm4 = GetTickCount64() + 500;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "出刀完毕");
	if (!判断敌方是否受击中() && !JudgePanshi(FatPlayerData.O_ActionName)) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}

	if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
		长枪连招::拳切AA跳B();
		return;
	}
	//Sleep(60);
	// 开始升龙
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme6 = GetTickCount64() + 1000;
	while (WaitTIme6 > GetTickCount64()) {
		//MyLog(标识, "下蹲判断循环次数:", WaitTIme6Temp);
		if (WaitTIme6Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {

		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 自动判断打什么连招
	if (LocalPlayer_Data.CameraPitch > 20 || LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd
		|| LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_kmNet_mouse_right(1);
		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(5);
		root_sendKeyUp(passKey);
		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		// 拳切985
		ULONGLONG WaitTIme15 = GetTickCount64() + 500;
		while (WaitTIme15 > GetTickCount64()) {
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 500;
		while (WaitTIme12 > GetTickCount64()) {
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);
		root_kmNet_mouse_right(1);
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
	}

	// 等待蓄力到1.0

	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme13 = GetTickCount64() + 1800;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if ((WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505) && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 出蓄力
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		WaitTIme8 = GetTickCount64() + 800;
	}
	else {
		WaitTIme8 = GetTickCount64() + 1500;
	}

	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	Sleep(30);
	if (是否半自动模式) {
		return;
	}

	bool cahngshan = false;
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
		cahngshan = true;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
		cahngshan = true;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						太刀月影闪切双刀A僵直抓取();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						太刀月影闪切双戟A僵直抓取();
						return;
					}
				}
				
				else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双戟
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟横刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟长棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双截棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双戟
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟扇子
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 双戟长剑
						切刀断闪蓄(1);
						return;
					}
				}

				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 100 + maxTemp && 首次循环到) {
							
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					return;
				}

				else {
					if (FatPlayerData.O_Dis < 4 && !cahngshan) {
						通用切刀重置蓄力();
						return;
					}
					MyLog("W闪-双刀太刀切拳百裂 命中后无合适武器");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 100 + maxTemp && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180 + maxTemp) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("S闪-百裂 未命中");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
		}

		Sleep(1);
	}
}
void 振刀逻辑::b1c切Ac切A(int 平A类型, int 魂玉类型) {
	string 标识 = "b1c切Ac切A--";
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}

	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog(标识, "下蹲判断循环次数:", WaitTIme6Temp);
		if (WaitTIme6Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	int WaitTIme2Temp = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		//MyLog(标识, "切武器循环次数:", WaitTIme2Temp);
		if (WaitTIme2Temp > 40 || LocalPlayer_Data.MyWeaponType == 武器2) {
			MyLog(标识, "切武器2成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme2Temp++;
		Sleep(1);
	}

	root_sendKeyUp(KEY_C);

	if (平A类型 == 0) {
		root_kmNet_mouse_left(1);
	}
	else {
		root_kmNet_mouse_right(1);
	}

	ULONGLONG WaitTIm3 = GetTickCount64() + 500;
	int WaitTIm3Temp = 0;
	while (WaitTIm3 > GetTickCount64()) {
		//MyLog(标识, "循环次数;", WaitTIm3Temp);
		if (WaitTIm3Temp > 15 || A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIm3Temp++;
		Sleep(1);
	}
	if (平A类型 == 0) {
		root_kmNet_mouse_left(0);
	}
	else {
		root_kmNet_mouse_right(0);
	}

	ULONGLONG WaitTIm4 = GetTickCount64() + 500;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	Sleep(10);
	// 再次C切
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTImeB1Temp = 0;
	ULONGLONG WaitTImeB1 = GetTickCount64() + 1500;
	while (WaitTImeB1 > GetTickCount64()) {
		//MyLog(标识, "再次C切下蹲判断循环次数:", WaitTImeB1Temp);
		//if (WaitTIme6Temp > 40 ||
		if (WaitTImeB1Temp > 15 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeB1Temp++;
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}
	int WaitTImeB2Temp = 0;
	ULONGLONG WaitTImeB2 = GetTickCount64() + 500;
	while (WaitTImeB2 > GetTickCount64()) {
		MyLog(标识, "切武器循环次数:", WaitTImeB2Temp);
		if ((WaitTImeB2Temp > 30 || LocalPlayer_Data.MyWeaponType != 武器2) && LocalPlayer_Data.MyWeaponType != 0) {
			MyLog("切回武器1成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeB2Temp++;
		Sleep(1);
	}

	root_sendKeyUp(KEY_C);
	Sleep(23);
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_left(1);
	int WaitTImB3Temp = 0;
	ULONGLONG WaitTImB3 = GetTickCount64() + 500;
	while (WaitTImB3 > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTImB3Temp > 15 || A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImB3Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_left(0);

	ULONGLONG WaitTImB4 = GetTickCount64() + 500;
	while (WaitTImB4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTImeB5 = GetTickCount64() + 500;
	while (WaitTImeB5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}

	//Sleep(60);
	// 开始升龙
	root_sendKeyPress(KEY_C);
	int WaitTIme65Temp = 0;
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		//MyLog(标识, "升龙下蹲循环次数:", WaitTIme65Temp);
		if (WaitTIme65Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme65Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {

		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 自动判断打什么连招
	if (LocalPlayer_Data.CameraPitch > 20 || LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd
		|| LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_kmNet_mouse_right(1);
		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(5);
		root_sendKeyUp(passKey);
		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		// 拳切985
		ULONGLONG WaitTIme15 = GetTickCount64() + 500;
		while (WaitTIme15 > GetTickCount64()) {
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 500;
		while (WaitTIme12 > GetTickCount64()) {
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);
		root_kmNet_mouse_right(1);
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
	}

	// 等待蓄力到1.0
	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme13 = GetTickCount64() + 1800;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 出蓄力
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8 = GetTickCount64() + 1000;
	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	Sleep(40);
	if (是否半自动模式) {
		return;
	}
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						太刀月影闪切双刀A僵直抓取();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						太刀月影闪切双戟A僵直抓取();
						return;
					}
				}

				else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双戟
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟横刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟长棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双截棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟双戟
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟扇子
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 双戟长剑
						切刀断闪蓄(1);
						return;
					}
				}

				else {
					MyLog("W闪-双刀太刀切拳百裂 命中后无合适武器");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 100 + maxTemp && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180 + maxTemp) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("S闪-百裂 未命中");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
		}

		Sleep(1);
	}
}
void 振刀逻辑::拳刃升龙bbca() {
	string 标识 = "拳刃升龙bbca--";
	if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {

		root_kmNet_mouse_left(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			MyLog("A1状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_C);
	int WaitTIme4Time = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 600;
	while (WaitTIme4 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTIme4Time);
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTIme4 > 10 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Time++;
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	MyLog(标识, "按下右键开始升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTImec6 = GetTickCount64() + 500;
	while (WaitTImec6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 右A
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTImeb1 = GetTickCount64() + 500;
	while (WaitTImeb1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (B2状态(LocalPlayer_Data.MyActionName)) {
			MyLog("B2状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTImebb2 = GetTickCount64() + 500;
	while (WaitTImebb2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTImebb3 = GetTickCount64() + 500;
	while (WaitTImebb3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			//MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 右B3
	root_kmNet_mouse_right(1);
	ULONGLONG WaitTImebc1 = GetTickCount64() + 500;
	while (WaitTImebc1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (B3状态(LocalPlayer_Data.MyActionName)) {
			MyLog("B3状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTImebbc2 = GetTickCount64() + 500;
	while (WaitTImebbc2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTImebbc3 = GetTickCount64() + 1400;
	while (WaitTImebbc3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("出刀状态", LocalPlayer_Data.MyRangeReactionType);
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog("按下C");
	root_sendKeyPress(KEY_C);
	int WaitTImae5Temp = 0;
	ULONGLONG WaitTImae5 = GetTickCount64() + 1500;
	while (WaitTImae5 > GetTickCount64()) {
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTImae5Temp > 300) {
			root_sendKeyUpAll();
			return;
		}
		WaitTImae5Temp++;
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTImae14 = GetTickCount64() + 500;
	while (WaitTImae14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CA状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CA状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTImeabbc2 = GetTickCount64() + 500;
	while (WaitTImeabbc2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTImeabbc3 = GetTickCount64() + 500;
	while (WaitTImeabbc3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
}
void 振刀逻辑::双刀太刀切拳百裂() {
	string 标识 = "双刀太刀切拳百裂--";
	auto start = std::chrono::high_resolution_clock::now();
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTIme1Time = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog(标识, "循环次数:", WaitTIme1Time);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTIme1Time > 200) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Time++;
		Sleep(1);
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
	//std::cout << "CC执行时间: " << duration_ms1 << " 毫秒" << std::endl;
	MyLog("CC执行时间: ", duration_ms1, " 毫秒");
	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
			MyLog(标识, "切太刀成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	//std::cout << "切太刀CC执行时间: " << duration_ms << " 毫秒" << std::endl;
	MyLog("切太刀CC执行时间: ", duration_ms, " 毫秒");
	root_sendKeyUp(KEY_C);
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIm3 = GetTickCount64() + 1000;
	while (WaitTIm3 > GetTickCount64()) {
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	ULONGLONG WaitTIm4 = GetTickCount64() + 1000;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme6 = GetTickCount64() + 1000;
	while (WaitTIme6 > GetTickCount64()) {
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTIme6Temp > 200) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme7 = GetTickCount64() + 1500;
	while (WaitTIme7 > GetTickCount64()) {

		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (LocalPlayer_Data.CameraPitch > 20) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_kmNet_mouse_right(1);
		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(5);
		root_sendKeyUp(passKey);
		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
		while (WaitTIme15 > GetTickCount64()) {
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 1500;
		while (WaitTIme12 > GetTickCount64()) {
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);
		root_kmNet_mouse_right(1);
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
	}

	// 等待蓄力到1.0
	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme13 = GetTickCount64() + 1800;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8 = GetTickCount64() + 1500;
	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "状态11，蓄力释放完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 1500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "蓄力命中");
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					太刀月影闪切双刀A僵直抓取();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					太刀月影闪切长剑A僵直抓取();
					return;
				}
				else {
					MyLog(标识, "S闪-双刀太刀切拳百裂 命中后无合适武器");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("S闪-双刀太刀切拳百裂 未命中");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
		}

		Sleep(1);
	}
}

void 振刀逻辑::抓壁击(int type) {
	if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_left(1);
		Sleep(10);
		root_kmNet_mouse_left(0);
	}

}

void 振刀逻辑::单刀拳双切A(bool 左A, int SleepTimp) {
	Sleep(SleepTimp);
	string 标识 = "单刀拳双切A--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);
	int WaitTIme1Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog("切拳次数:", WaitTIme1Temp);
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}
	int 按键判断 = KEY_1_EXCLAMATION_MARK;
	if (当前武器位置 != 1) {
		按键判断 = KEY_2_AT;
	}

	root_sendKeyPress(KEY_W);
	root_sendKeyPress(KEY_LEFTSHIFT);
	root_sendKeyPress(按键判断);
	if (左A) {
		root_kmNet_mouse_left(1);
	}
	else {
		root_kmNet_mouse_right(1);
	}
	Sleep(15);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(按键判断);
	root_sendKeyUp(KEY_W);


	//int WaitTIme2Temp = 0;
	//ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	//while (WaitTIme2 > GetTickCount64()) {
	//	MyLog("切回次数:", WaitTIme2Temp);
	//	if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && LocalPlayer_Data.MyWeaponType != 0) {
	//		break;
	//	}
	//	if (振刀逻辑::是否特殊状态())
	//	{
	//		root_sendKeyUpAll();
	//		return;
	//	}
	//	WaitTIme2Temp++;
	//	Sleep(1);
	//}

	//

	ULONGLONG WaitTIm3 = GetTickCount64() + 500;
	while (WaitTIm3 > GetTickCount64()) {
		if (A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (左A) {
		root_kmNet_mouse_left(0);
	}
	else {
		root_kmNet_mouse_right(0);
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	ULONGLONG WaitTIm4 = GetTickCount64() + 500;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "出刀完毕");
	int WaitTImeb1Temp = 0;
	ULONGLONG WaitTImeb1 = GetTickCount64() + 500;
	while (WaitTImeb1 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			break;
		}
		if (WaitTImeb1Temp > 15 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		WaitTImeb1Temp++;
		Sleep(1);
	}
	通用拳切百裂(true,1,true,1);
}

void 振刀逻辑::执行太刀平A僵直抓取() {
	string 标识 = "太刀抓僵直逻辑--";
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		/*if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}*/
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;

			break;
		}
		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 3;

			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;

			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;

			break;
		}

		if (LocalPlayer_Data.MyRangeReactionType == 12 && A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}



	ULONGLONG WaitTIme111 = GetTickCount64() + 1000;
	while (WaitTIme111 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0 || 平A方式 == 3) {
		MyLog(标识, "平A方式 == 3 / == 0");
		int tempI = 0;
		bool 超时跳出 = true;

		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				超时跳出 = false;
				MyLog(标识, "抓到僵直");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("太刀A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {

					if (平A方式 == 3) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}
					}

					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					MyLog(标识, "自动选择重置蓄力方式结束");
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						// 手持太双
						if (std::find(LocalPlayer_Data.mySoulItemTidList.begin(), LocalPlayer_Data.mySoulItemTidList.end(), HunYuID::TieMaCH) != LocalPlayer_Data.mySoulItemTidList.end()) {

							双刀铁马残红切刀蓄();
						}
						else {
							通用双刀流切拳蓄力();
						}
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 太刀双戟
						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::fan ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::fan) {
						// 太刀扇子
						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						通用双刀流切拳蓄力();
						return;
					}
					else {
						// 暂时只打拳切百裂
						if (mem.GetKeyboard()->IsKeyDown(VK_N)) {
							横刀切刀颠勺();
						}
						else {
							通用拳切百裂();
						}

						return;
					}

					break;
				}
			}

			if (tempI > 15 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				超时跳出 = false;
				MyLog(标识, "抓僵直失败");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if ((FatPlayerData.O_Dis < 4 && rand() % 4 < 1) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_left(0);
					Sleep(10);
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				else {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						if (rand() % 7 < 5) {
							if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
								LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
								// 链剑太刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀横刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀长棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双截棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双戟
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀扇子
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
								LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
								// 太刀长剑
								切刀断闪蓄(1);
								return;
							}
						}
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int WaitTIme11Temp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
							if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							WaitTIme11Temp++;
							Sleep(1);
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				超时跳出 = false;
				root_sendKeyUpAll();
				return;
			}

			this_thread::sleep_for(chrono::milliseconds(1));
			tempI++;
		}

		if (超时跳出) {
			MyLog(标识, "执行超时");
		}

	}

	if (平A方式 == 1) {
		MyLog(标识, "b1出刀结束");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "b1命中");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "双刀A浮空/倒地后跳A");
					处决后双跳A();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 太刀双刀
					if (std::find(LocalPlayer_Data.mySoulItemTidList.begin(), LocalPlayer_Data.mySoulItemTidList.end(), HunYuID::TieMaCH) != LocalPlayer_Data.mySoulItemTidList.end()) {

						双刀连招::b1c切Ac切A铁马();
					}
					else {
						振刀逻辑::b1c切Ac切A();
					}

					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 太刀双戟
					振刀逻辑::b1c切Ac切A();
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 太刀长剑
					振刀逻辑::通用C切百裂(1);
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks) {
					// 太刀双截棍
					振刀逻辑::通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 太刀长棍
					振刀逻辑::通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
					// 太刀链剑
					振刀逻辑::通用C切百裂(1);
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
					// 太刀横刀
					振刀逻辑::通用C切百裂();
					return;

				}

				if (LocalPlayer_Data.MyCurEnergy > 3100) {
					单刀拳双切A();
					break;
				}
				else {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_SPACEBAR);
					ULONGLONG WaitTIme13 = GetTickCount64() + 500;
					while (WaitTIme13 > GetTickCount64()) {
						if (JudgeJump(LocalPlayer_Data.MyActionName)) {
							MyLog("b1命中 起跳");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_kmNet_mouse_left(1);
					root_sendKeyUp(KEY_SPACEBAR);
					ULONGLONG WaitTIme14 = GetTickCount64() + 500;
					while (WaitTIme14 > GetTickCount64()) {
						if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
							MyLog("跳A");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					// 跳A结束
					root_sendKeyUp(KEY_W);
					root_kmNet_mouse_left(0);
					ULONGLONG WaitTIme15 = GetTickCount64() + 1000;
					while (WaitTIme15 > GetTickCount64()) {
						if (落地状态(LocalPlayer_Data.MyActionName)) {
							MyLog("落地");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
				}
				/*
				if (滑步B1状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_SPACEBAR);
					ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
					while (WaitTIme13 > GetTickCount64()) {
						if (JudgeJump(LocalPlayer_Data.MyActionName)) {
							MyLog("b1命中 起跳");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_kmNet_mouse_left(1);
					root_sendKeyUp(KEY_SPACEBAR);
					ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
					while (WaitTIme14 > GetTickCount64()) {
						if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
							MyLog("跳A");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					// 跳A结束
					root_sendKeyUp(KEY_W);
					root_kmNet_mouse_left(0);
					ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
					while (WaitTIme15 > GetTickCount64()) {
						if (落地状态(LocalPlayer_Data.MyActionName)) {
							MyLog("落地");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

				}
				else {
					if (LocalPlayer_Data.MyCurEnergy >= 5000) {
						root_sendKeyUp(KEY_S);
						root_sendKeyUpAll();
						root_sendKeyPress(KEY_S);
						Sleep(1);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180){
								MyLog("进入长闪");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
							Sleep(1);
						}
					}
					else {
						// 进行跳A

						root_sendKeyPress(KEY_W);
						root_sendKeyPress(KEY_SPACEBAR);
						ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
						while (WaitTIme13 > GetTickCount64()) {
							if (JudgeJump(LocalPlayer_Data.MyActionName)) {
								MyLog("起跳");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_kmNet_mouse_left(1);
						root_sendKeyUp(KEY_SPACEBAR);
						ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
						while (WaitTIme14 > GetTickCount64()) {
							if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
								MyLog("跳A");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						// 跳A结束
						root_sendKeyUp(KEY_W);
						root_kmNet_mouse_left(0);
						ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
						while (WaitTIme15 > GetTickCount64()) {
							if (落地状态(LocalPlayer_Data.MyActionName)) {
								MyLog("落地");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						root_sendKeyUpAll();
						return;
					}
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				root_kmNet_mouse_left(1);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUp(KEY_S);

				ULONGLONG WaitTIme16 = GetTickCount64() + 1500;
				while (WaitTIme16 > GetTickCount64()) {
					if (A1状态(LocalPlayer_Data.MyActionName)) {
						MyLog("A1状态");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				root_kmNet_mouse_left(0);
				ULONGLONG WaitTIme17 = GetTickCount64() + 1500;
				while (WaitTIme17 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 12) {
						break;
					}
					if (A1状态(LocalPlayer_Data.MyActionName) && FatPlayerData.O_EndureLevel != 6) {
						// 空A
						跳出方式 = 1;
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme18 = GetTickCount64() + 1500;
				while (WaitTIme18 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme19 = GetTickCount64() + 1500;
				int temp19 = 0;
				while (WaitTIme19 > GetTickCount64()) {
					// 平A 执行ACCA + 连招
					if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}

						// 暂时只打拳切百裂
						通用拳切百裂();
						break;

					}

					if (temp19 > 2 && !拼刀状态(FatPlayerData.O_ActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}

					temp19++;
					Sleep(1);
				}*/
			}

			if (tempI > 20 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "抓僵直失败");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (JudgePanshi(FatPlayerData.O_ActionName) && LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades
					) {
					// 连续打三个c切
					双刀流打磐石();
					return;
				}
				if ((FatPlayerData.O_Dis < 4 && rand() % 4 < 1) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_left(0);
					Sleep(10);
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				else {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						if (rand() % 7 < 5) {
							if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
								LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
								// 链剑太刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀横刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀长棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双截棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀双戟
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀扇子
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
								LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
								// 太刀长剑
								切刀断闪蓄(1);
								return;
							}
						}
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int WaitTIme11Temp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
							if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							WaitTIme11Temp++;
							Sleep(1);
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			this_thread::sleep_for(chrono::milliseconds(1));
			tempI++;
		}
	}

	if (平A方式 == 4) {
		MyLog(标识, "平A方式 == 4");
		ULONGLONG WaitTIme20 = GetTickCount64() + 1500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				//太刀A4拳切百裂();
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}

				通用拳切百裂(true);
				break;
			}
			if (temp20 > 20) {
				MyLog(标识, "A4未命中");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}
	}

	if (平A方式 == 2) {
		MyLog("平A方式 == 2");
		ULONGLONG WaitTIme20 = GetTickCount64() + 500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			//MyLog(标识, "a2循环次数：", temp20);
			if (temp20 > 5 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "a2命中");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (出招后血量判断()) {
					root_kmNet_mouse_left(1);
					Sleep(100);
				}
				return;
			}

			if (temp20 > 30) {
				MyLog(标识, "a2未命中");
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}

	}

}

bool 振刀逻辑::出招后血量判断() {
	// 血量判断
	if (FatPlayerData.O_MaxHp < 10) {
		处决后双跳A();
		return false;
	}
	return true;
}
void 振刀逻辑::执行长棍平A僵直抓取() {
	string 标识 = "长棍平A抓取逻辑--";
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		/*if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}*/
	}

	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << "出刀值=" << LocalPlayer_Data.MyRangeReactionType << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			MyLog(标识, "1A状态");
			平A方式 = 0;
			break;
		}
		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 3;
			break;
		}
		if (A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(5);
	}


	ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
	while (WaitTIme11 > GetTickCount64()) {
		//MyLog(标识, "出刀值=", LocalPlayer_Data.MyRangeReactionType);
		if (LocalPlayer_Data.MyRangeReactionType == 10 || 平A方式 == 3) {
			MyLog(标识, "出刀完毕10状态");
			break;
		}
		if (平A方式 == 1) {
			MyLog(标识, "出B1状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0) {
		MyLog(标识, "平A方式 == 0 ");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("长棍A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {

					/*if (!自动选择重置蓄力方式()) {
						root_sendKeyUpAll();
						return;
					}*/

					if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						// 长棍阔刀
						//通用拳切百裂();
						通用双刀流切拳蓄力(1, 0, true);

						return;
					}
					else {
						// 暂时只打拳切百裂
						通用拳切百裂(true);
						//长枪连招::拳切AA跳B();
						return;
					}

					break;
				}
			}

			if (tempI > 10 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("进入长棍未命中状态 tempI > 10");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);
			tempI++;
		}
	}
	if (平A方式 == 3) {
		MyLog(标识, "平A方式 = 跑A1");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
		while (WaitTIme1 > GetTickCount64()) {
			if (tempI > 60 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("长棍A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {

					/*if (!自动选择重置蓄力方式()) {
						root_sendKeyUpAll();
						return;
					}*/

					if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						// 长棍阔刀
						//通用拳切百裂();
						通用双刀流切拳蓄力(1, 0, true);

						return;
					}
					else {
						// 暂时只打拳切百裂
						通用拳切百裂(true);

						return;
					}

					break;
				}
			}

			if (tempI > 300 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("进入长棍未命中状态 tempI > 300");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);
			tempI++;
		}
	}
	if (平A方式 == 1 || 平A方式 == 2) {
		MyLog("平A方式 == 1");
		int WaitTIme1Temp = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		while (WaitTIme1 > GetTickCount64()) {
			if (WaitTIme1Temp > 60 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍太刀
					通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍横刀
					通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍双截棍
					通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 长棍双戟
					通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Sw && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Sw && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍长剑
					通用C切百裂(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍双刀
					通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
					// 长棍链剑
					通用C切百裂(1);
					return;
				}
				else {
					双刀BB();
					root_kmNet_mouse_right(0);
					Sleep(2);
					root_kmNet_mouse_right(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 600;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
				}
			}

			if (WaitTIme1Temp > 170 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("进入长棍未命中状态 WaitTIme1Temp > 170");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			WaitTIme1Temp++;
			Sleep(1);
		}
	}

}
void 振刀逻辑::执行长枪平A僵直抓取() {
	string 标识 = "执行长枪平A僵直抓取--";
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		/*if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}*/
	}
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 0;
			break;
		}

		if (A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}


	ULONGLONG WaitTIme11 = GetTickCount64() + 500;
	while (WaitTIme11 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0) {
		MyLog(标识, "平A方式 == 0");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 600;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("长枪A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					/*if (!自动选择重置蓄力方式()) {
						root_sendKeyUpAll();
						return;
					}*/

					if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::fan ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::fan) {
						// 长枪扇子
						通用双刀流切拳蓄力(1, 0, true);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						// 长枪阔刀
						通用双刀流切拳蓄力(1, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
						// 长枪太刀
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
						// 长枪横刀
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						// 长枪双刀
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 长枪双戟
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::rod ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::rod) {
						// 长枪棍子
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks) {
						// 长枪双截棍
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长枪长剑
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 长枪链剑
						通用双刀流切拳蓄力(0, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::punch ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::punch) {
						// 长枪拳刃
						通用双刀流切拳蓄力(1, 0, true);

						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::Knife ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::Knife) {
						// 长枪飞刀
						通用双刀流切拳蓄力(1, 0, true, true);

						return;
					}
					else {
						// 暂时只打拳切百裂
						//通用拳切百裂();
						//通用拳切百裂(true, 0);
						长枪连招::拳切AA跳B();
						return;
					}

					break;
				}
			}

			if (tempI > 10 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "未命中状态 tempI > 50");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);
			tempI++;
		}
	}

	if (平A方式 == 1 || 平A方式 == 2) {
		MyLog(标识, "平A方式 == 1");
		//ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		//while (WaitTIme1 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (!Function::半自动博弈::自动连招 && !博弈键按下) {
				return;
			}
			root_sendKeyPress(KEY_W);
			root_sendKeyPress(KEY_SPACEBAR);
			ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
			while (WaitTIme13 > GetTickCount64()) {
				if (JudgeJump(LocalPlayer_Data.MyActionName)) {
					MyLog("b1命中 起跳");
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			root_kmNet_mouse_left(1);
			root_sendKeyUp(KEY_SPACEBAR);
			ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
			while (WaitTIme14 > GetTickCount64()) {
				if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
					MyLog("跳A");
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			// 跳A结束
			root_sendKeyUp(KEY_W);
			root_kmNet_mouse_left(0);
			ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
			while (WaitTIme15 > GetTickCount64()) {
				if (落地状态(LocalPlayer_Data.MyActionName)) {
					MyLog("落地");
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
	}

}
void 振刀逻辑::执行斩马刀平A僵直抓取() {
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		if ((A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			跳出方式 = 0;
			break;
		}
		if ((A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName)) /*|| 判断出招()*/)
		{
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	int tempI = 0;
	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (!Function::半自动博弈::自动连招 && !博弈键按下) {
				return;
			}
			if (!振刀逻辑::侧键按下或半自动()) {
				MyLog("侧键松开");
				root_sendKeyUpAll();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
				MyLog("斩马A浮空/倒地后跳A");
				// 出B
				break;
			}
			else {
				root_kmNet_mouse_left(1);
				Sleep(500);
				break;
			}
		}

		if (tempI > 10 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("进入太刀未命中状态 tempI > 30");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 600;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				通用切刀重置蓄力();
				break;
			}
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}

		Sleep(1);
		tempI++;
	}
}
void 振刀逻辑::执行双截棍平A僵直抓取() {
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;
			break;
		}
		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 4;
			break;
		}
		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 2;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;
			break;
		}

		if (A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 3;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(5);
	}


	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0 || 平A方式 == 4) {
		// A1状态
		MyLog("平A方式 == 0");
		int tempI = 0;
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("双截棍A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {

					if (平A方式 == 4) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}
					}
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						// 手持双截棍双刀
						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
						// 双截棍太刀
						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::fan ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::fan) {
						// 太刀扇子
						通用双刀流切拳蓄力(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						// 太刀阔刀

						通用双刀流切拳蓄力(1);
						return;
					}
					else {
						// 暂时只打拳切百裂

						通用拳切百裂();
						return;
					}

					break;
				}
			}

			if (tempI > 10 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("未命中状态 tempI > 10");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					通用切刀重置蓄力();
					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);
			tempI++;
		}
	}

	if (平A方式 == 1) {
		MyLog("平A方式 == 1");
		//ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		//while (WaitTIme1 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (!Function::半自动博弈::自动连招 && !博弈键按下) {
				return;
			}
			if (!振刀逻辑::侧键按下或半自动()) {
				MyLog("侧键松开");
				root_sendKeyUpAll();
				return;
			}
			双刀BB();

			振刀逻辑::长闪蓄();
		}
		else {
			if (LocalPlayer_Data.MyCurEnergy >= 5000) {
				root_sendKeyUp(KEY_S);
				root_sendKeyUpAll();
				root_sendKeyPress(KEY_S);
				Sleep(1);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						MyLog("进入长闪");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
					Sleep(1);
				}
			}
			else {
				MyLog("双截棍B1未命中 开始CC重置");
				通用切刀重置蓄力();
			}
		}
	}

	if (平A方式 == 2) {
		MyLog("平A方式 == 2");
		ULONGLONG WaitTIme20 = GetTickCount64() + 600;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				太刀A4拳切百裂();
				break;
			}
			if (temp20 > 120) {
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();
					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}
	}

	if (平A方式 == 3) {
		MyLog("平A方式 == 3");
		ULONGLONG WaitTIme20 = GetTickCount64() + 600;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				处决后双跳A();
				break;
			}
			if (temp20 > 120) {
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}
	}


}
void 振刀逻辑::执行匕首扇子平A僵直抓取() {
	string 标识 = "匕首扇子平A僵直抓取--";
	int 跳出方式 = 0;
	root_sendKeyUpAll();
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	auto start = std::chrono::high_resolution_clock::now();


	//MyLog();
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		if ((A1状态(LocalPlayer_Data.MyActionName) || B1状态(LocalPlayer_Data.MyActionName)) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			跳出方式 = 0;
			break;
		}
		if ((A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName)) /*|| 判断出招()*/)
		{
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "匕首/扇子 状态10");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
	//std::cout << "MyRangeReactionType时间: " << duration_ms1 << " 毫秒" << std::endl;
	if (跳出方式 == 0) {
		MyLog(标识, "跳出方式 = 0");
		bool 未命中状态 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_ActionDuration > 0.10f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("匕首A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 太刀匕首
					通用C切百裂();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 横刀匕首
					通用C切百裂();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Sw && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Sw && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 匕首长剑
					通用C切百裂(1);
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 匕首双截棍
					通用C切百裂();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 匕首链剑
					通用C切百裂(1);
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 匕首双刀
					通用C切百裂();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Dagge ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Dagge) {
					// 匕首双戟
					通用C切百裂();
					//振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
					break;
				}
				else {
					匕首扇子BBCA();
					break;
				}
			}

			if (拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (WaitTIme3Time > 100 && FatPlayerData.O_EndureLevel != 6) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog("进入未命中逻辑");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				return;

			}
			else {
				通用切刀重置蓄力();
				return;
			}
		}
	}

	if (跳出方式 == 1) {
		MyLog(标识, "跳出方式 = 1");
		root_sendKeyUpAll();
		return;
	}

}
void 振刀逻辑::执行长剑平A僵直抓取() {
	string 标识 = "执行长剑平A僵直抓取--";
	int 平A方式 = 0;
	int 跳出方式 = 0;
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}

	/*if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_right(1);
		Sleep(10);
		root_kmNet_mouse_right(0);
	}*/

	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;
			跳出方式 = 1;
			break;
		}

		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 5;
			跳出方式 = 1;
			break;
		}

		if (跑B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "跑B1状态");
			平A方式 = 3;
			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;
			跳出方式 = 1;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑B1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 1;
			跳出方式 = 0;
			break;
		}

		if (A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			跳出方式 = 1;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme11 = GetTickCount64() + 500;
	while (WaitTIme11 > GetTickCount64()) {
		//MyLog(标识, "出刀状态==", LocalPlayer_Data.MyRangeReactionType);
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀完毕10状态");
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (跳出方式 == 0 || 平A方式 == 3) {
		bool 未命中状态 = false;
		bool 磐石状态 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_ActionDuration > 0.5f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("长剑A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {
					if (平A方式 == 3) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式(1)) {
							root_sendKeyUpAll();
							return;
						}
					}
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}

					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑太刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑横刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑长棍
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑双截棍
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑双刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑双戟
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑扇子
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑链剑
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Blade && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Blade && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑阔刀
						//长剑重刃s闪AB();
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::saber && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::saber && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑斩马
						长剑重刃s闪AB();
					}
					else {
						//振刀逻辑::长剑浮空3A(1);
						通用拳切百裂(false, 1, false, 1);
						break;
					}
				}
			}
			else if (WaitTIme3Time > 3 && FatPlayerData.O_ActionDuration > 0.1f && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "僵直:", FatPlayerData.O_ActionDuration);
				处决后双跳A();
				未命中状态 = false;
				break;
			}

			if (WaitTIme3Time > 10 || 拼刀状态(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				磐石状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog(标识, "进入未命中逻辑");
			/*if (可C切武器(WeaponType::Sw)) {
				MyLog(标识, "抓空C切");
				C滚轮切刀重置蓄力();
				return;
			}*/
			if (磐石状态) {
				MyLog(标识, "敌人磐石状态，直接捏住");
				root_sendKeyUpAll();

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑太刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑横刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑长棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双截棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双戟
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑扇子
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑链剑
					切刀断闪蓄(1);
				}
				else {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int WaitTIme11Temp = 0;
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						//MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
						if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						WaitTIme11Temp++;
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
				}
				return;
			}
			else {
				通用切刀重置蓄力();
				return;
			}
		}
	}

	if (跳出方式 == 1) {
		bool 未命中状态 = false;
		bool 磐石状态 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_ActionDuration > 0.3f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("长剑A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}

				if ((平A方式 == 5 || 平A方式 == 0) && 可C切武器(WeaponType::Sw)) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑太刀
						通用C切百裂();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑横刀
						通用C切百裂();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑双截棍
						通用C切百裂();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Sw && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Sw && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 双戟长剑
						通用C切百裂();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑链剑
						通用C切百裂(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑长棍
						通用C切百裂();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 长剑长枪
						通用C切百裂();
						return;
					}
					return;
				}
				else if (LocalPlayer_Data.MyCurEnergy > 3100) {
					单刀拳双切A(false);
					return;
				}
				处决后双跳A();
			}
			else if (WaitTIme3Time > 3 && FatPlayerData.O_ActionDuration > 0.1f && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "僵直:", FatPlayerData.O_ActionDuration);
				处决后双跳A();
				未命中状态 = false;
				break;
			}

			if (WaitTIme3Time > 10 || 拼刀状态(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				磐石状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}
		if (未命中状态) {
			MyLog(标识, "进入未命中逻辑");
			/*if (可C切武器(WeaponType::Sw)) {
				MyLog(标识, "抓空C切");
				C滚轮切刀重置蓄力();
				return;
			}*/
			if (磐石状态) {
				MyLog(标识, "敌人磐石状态，直接捏住");
				root_sendKeyUpAll();

				root_kmNet_mouse_right(0);
				Sleep(2);
				root_kmNet_mouse_right(1);
				ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
				while (WaitTImeB11 > GetTickCount64()) {

					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				return;
			}
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑太刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑横刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑长棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双截棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双戟
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑扇子
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑链剑
					切刀断闪蓄(1);
				}
				else {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int WaitTIme11Temp = 0;
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						//MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
						if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						WaitTIme11Temp++;
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
				}
				return;
			}
			else {
				通用切刀重置蓄力();
				return;
			}
		}
		return;
	}

}
void 振刀逻辑::执行链剑平A僵直抓取() {
	string 标识 = "执行链剑平A僵直抓取--";
	int 平A方式 = 0;
	int 跳出方式 = 0;
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	/*if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_right(1);
		Sleep(10);
		root_kmNet_mouse_right(0);
	}*/

	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;
			跳出方式 = 1;
			break;
		}

		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 5;
			跳出方式 = 1;
			break;
		}

		if (跑B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 3;
			break;
		}
		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;
			跳出方式 = 1;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑B1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 1;
			跳出方式 = 0;
			break;
		}

		if (A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			跳出方式 = 1;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme11 = GetTickCount64() + 500;
	while (WaitTIme11 > GetTickCount64()) {
		//MyLog(标识, "出刀状态==", LocalPlayer_Data.MyRangeReactionType);
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀完毕10状态");
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (跳出方式 == 0 || 平A方式 == 3) {
		bool 未命中状态 = false;
		ULONGLONG WaitTIme3 = GetTickCount64() + 1500;
		int WaitTIme3Time = 0;
		while (WaitTIme3 > GetTickCount64()) {
			//MyLog("对面僵直", FatPlayerData.O_ActionDuration);
			//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;

			if (FatPlayerData.O_ActionDuration > 0.5f && !StrStrW(FatPlayerData.O_ActionName.c_str(), L"parry") && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {
					if (平A方式 == 3) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}

					}
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}

					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑太刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑横刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑长棍
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双截棍
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双刀
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双戟
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑扇子
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 链剑长剑
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::Blade && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Blade && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑阔刀
						//链剑重刃s闪AB();
						通用双刀流切拳蓄力(1, 1);
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::saber && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::saber && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑斩马
						长剑重刃s闪AB();
					}
					else {
						通用拳切百裂(false, 1, false);
						break;
					}
				}
			}
			else if (WaitTIme3Time > 3 && FatPlayerData.O_ActionDuration > 0.1f && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "僵直:", FatPlayerData.O_ActionDuration);
				处决后双跳A();
				未命中状态 = false;
				break;
			}

			if (WaitTIme3Time > 10 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				未命中状态 = true;
				break;
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			WaitTIme3Time++;
			Sleep(1);
		}

		if (未命中状态) {
			MyLog("进入未命中逻辑");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				if (rand() % 7 < 5) {
					if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑太刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑横刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑长棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双截棍
						切刀断闪蓄();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双刀
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑双戟
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
						LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
						// 链剑扇子
						切刀断闪蓄(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
						LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
						// 链剑长剑
						切刀断闪蓄(1);
						return;
					}
				}
				
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int WaitTIme11Temp = 0;
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					//MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
					if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					WaitTIme11Temp++;
					Sleep(1);
				}
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				
				return;

			}
			else {
				通用切刀重置蓄力();
				return;
			}
		}
	}

	if (跳出方式 == 1) {
		if (!Function::半自动博弈::自动连招 && !博弈键按下) {
			return;
		}
		if (平A方式 == 5 && 可C切武器(WeaponType::Sw)) {
			if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑太刀
				通用C切百裂();
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑横刀
				通用C切百裂();
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑双截棍
				通用C切百裂();
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
				LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
				// 双戟链剑
				通用C切百裂();
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑链剑
				通用C切百裂(1);
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑长棍
				通用C切百裂();
				return;
			}
			else if (LocalPlayer_Data.Weapon_1 == WeaponType::Spear && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
				LocalPlayer_Data.Weapon_2 == WeaponType::Spear && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
				// 链剑长枪
				通用C切百裂();
				return;
			}
			return;
		}
		处决后双跳A();
		return;
	}

}

void 振刀逻辑::执行拳刃平A僵直抓取() {
	string 标识 = "拳刃抓僵直逻辑--";
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		/*if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}*/
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1000;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;

			break;
		}
		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 3;

			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;

			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;

			break;
		}

		if (LocalPlayer_Data.MyRangeReactionType == 12 && A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			平A方式 = 2;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}



	ULONGLONG WaitTIme111 = GetTickCount64() + 1000;
	while (WaitTIme111 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0 || 平A方式 == 3) {
		MyLog(标识, "平A方式 == 3 / == 0");
		int tempI = 0;
		bool 超时跳出 = true;

		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				超时跳出 = false;
				MyLog(标识, "抓到僵直");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("太刀A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {
					//if (LocalPlayer_Data.Weapon_1 == WeaponType::punch && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					//	LocalPlayer_Data.Weapon_2 == WeaponType::punch && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					//	// 拳刃长剑
					//	振刀逻辑::通用C切百裂(1);
					//	return;

					//}
					if (平A方式 == 3) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}
					}
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					MyLog(标识, "自动选择重置蓄力方式结束");

					拳刃升龙bbca();
					break;
				}
			}

			if (tempI > 15 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				超时跳出 = false;
				MyLog(标识, "抓僵直失败");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_left(0);
					Sleep(10);
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				else {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);

						int changshanTemp = 0;
						ULONGLONG WaitTIme10 = GetTickCount64() + 600;
						while (WaitTIme10 > GetTickCount64()) {
							Sleep(1);
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				超时跳出 = false;
				root_sendKeyUpAll();
				return;
			}

			this_thread::sleep_for(chrono::milliseconds(1));
			tempI++;
		}

		if (超时跳出) {
			MyLog(标识, "执行超时");
		}

	}

	if (平A方式 == 1) {
		MyLog(标识, "b1出刀结束");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "b1命中");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "双刀A浮空/倒地后跳A");
					处决后双跳A();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 太刀双刀
					振刀逻辑::b1c切Ac切A();
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 太刀双戟
					振刀逻辑::b1c切Ac切A();
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 太刀长剑
					振刀逻辑::通用C切百裂(1);
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks) {
					// 太刀双截棍
					振刀逻辑::通用C切百裂();
					return;
				}

				if (滑步B1状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_SPACEBAR);
					ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
					while (WaitTIme13 > GetTickCount64()) {
						if (JudgeJump(LocalPlayer_Data.MyActionName)) {
							MyLog("b1命中 起跳");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_kmNet_mouse_left(1);
					root_sendKeyUp(KEY_SPACEBAR);
					ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
					while (WaitTIme14 > GetTickCount64()) {
						if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
							MyLog("跳A");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					// 跳A结束
					root_sendKeyUp(KEY_W);
					root_kmNet_mouse_left(0);
					ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
					while (WaitTIme15 > GetTickCount64()) {
						if (落地状态(LocalPlayer_Data.MyActionName)) {
							MyLog("落地");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

				}
				else {
					if (LocalPlayer_Data.MyCurEnergy >= 5000) {
						root_sendKeyUp(KEY_S);
						root_sendKeyUpAll();
						root_sendKeyPress(KEY_S);
						Sleep(1);
						root_sendKeyPress(KEY_LEFTSHIFT);
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						int changshanTemp = 0;
						while (WaitTIme11 > GetTickCount64()) {
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								MyLog("进入长闪");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
							Sleep(1);
						}
					}
					else {
						// 进行跳A

						root_sendKeyPress(KEY_W);
						root_sendKeyPress(KEY_SPACEBAR);
						ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
						while (WaitTIme13 > GetTickCount64()) {
							if (JudgeJump(LocalPlayer_Data.MyActionName)) {
								MyLog("起跳");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_kmNet_mouse_left(1);
						root_sendKeyUp(KEY_SPACEBAR);
						ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
						while (WaitTIme14 > GetTickCount64()) {
							if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
								MyLog("跳A");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						// 跳A结束
						root_sendKeyUp(KEY_W);
						root_kmNet_mouse_left(0);
						ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
						while (WaitTIme15 > GetTickCount64()) {
							if (落地状态(LocalPlayer_Data.MyActionName)) {
								MyLog("落地");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						root_sendKeyUpAll();
						return;
					}
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				root_kmNet_mouse_left(1);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUp(KEY_S);

				ULONGLONG WaitTIme16 = GetTickCount64() + 1500;
				while (WaitTIme16 > GetTickCount64()) {
					if (A1状态(LocalPlayer_Data.MyActionName)) {
						MyLog("A1状态");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				root_kmNet_mouse_left(0);
				ULONGLONG WaitTIme17 = GetTickCount64() + 1500;
				while (WaitTIme17 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 12) {
						break;
					}
					if (A1状态(LocalPlayer_Data.MyActionName) && FatPlayerData.O_EndureLevel != 6) {
						// 空A
						跳出方式 = 1;
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme18 = GetTickCount64() + 1500;
				while (WaitTIme18 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme19 = GetTickCount64() + 1500;
				int temp19 = 0;
				while (WaitTIme19 > GetTickCount64()) {
					// 平A 执行ACCA + 连招
					if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}

						// 暂时只打拳切百裂
						通用拳切百裂();
						break;

					}

					if (temp19 > 2 && !拼刀状态(FatPlayerData.O_ActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}

					temp19++;
					Sleep(1);
				}
			}

			if (tempI > 20 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "抓僵直失败");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				else {
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);

						ULONGLONG WaitTIme10 = GetTickCount64() + 600;
						int changshanTemp = 0;
						while (WaitTIme10 > GetTickCount64()) {
							Sleep(1);
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			this_thread::sleep_for(chrono::milliseconds(1));
			tempI++;
		}
	}

	if (平A方式 == 4) {
		MyLog(标识, "平A方式 == 4");
		ULONGLONG WaitTIme20 = GetTickCount64() + 1500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				//太刀A4拳切百裂();
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				通用拳切百裂(true);
				break;
			}
			if (temp20 > 20) {
				MyLog(标识, "A4未命中");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}
	}

	if (平A方式 == 2) {
		MyLog("平A方式 == 2");
		ULONGLONG WaitTIme20 = GetTickCount64() + 500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			//MyLog(标识, "a2循环次数：", temp20);
			if (temp20 > 5 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "a2命中");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (出招后血量判断()) {
					root_kmNet_mouse_left(1);
					Sleep(100);
				}
				return;
			}

			if (temp20 > 30) {
				MyLog(标识, "a2未命中");
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}

	}

}
void 振刀逻辑::太刀A4拳切百裂() {
	string 标识 = "太刀A4拳切百裂--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyPress(KEY_W);

	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("下蹲");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTIme6Temp > 200) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 1500;
	while (WaitTIme5 > GetTickCount64()) {

		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyUp(KEY_W);
	ULONGLONG WaitTIme6 = GetTickCount64() + 1500;
	while (WaitTIme6 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);
	ULONGLONG WaitTIme12 = GetTickCount64() + 1500;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_LEFTSHIFT);
	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	root_kmNet_mouse_right(1);
	Sleep(15);
	root_sendKeyUp(KEY_LEFTSHIFT);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}

	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme7 = GetTickCount64() + 1500;
	while (WaitTIme7 > GetTickCount64()) {
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8 = GetTickCount64() + 1500;
	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 1500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("命中对方");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				//root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				int changshanTemp = 0;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("未命中/拼刀/打中磐石");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;

			}
			else {
				通用切刀重置蓄力();
				break;
			}
		}

		Sleep(1);
	}

}

/**************连招*******************/
void 振刀逻辑::判断断闪蓄() {
	string 标识 = "判断断闪蓄--";
	MyLog(标识, "开始");
	bool chanshan = false;
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
		chanshan = true;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
		chanshan = true;
	}

	if (!是否单刀武器() && (LocalPlayer_Data.Weapon_1 != WeaponType::Blade && LocalPlayer_Data.Weapon_2 != WeaponType::Blade &&
		LocalPlayer_Data.Weapon_1 != WeaponType::saber && LocalPlayer_Data.Weapon_2 != WeaponType::saber)
		&& rand() % 7 < 5) {
		int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;

		if (武器2 == WeaponType::Katana || 武器2 == WeaponType::hengdao || 
			武器2 == WeaponType::Sw || 武器2 == WeaponType::lj ||
			武器2 == WeaponType::twinblades || 武器2 == WeaponType::dualhalberd ||
			武器2 == WeaponType::Dagge || 武器2 == WeaponType::fan
			) {
			切刀断闪蓄(1);
		}
		else if (
			武器2 == WeaponType::punch || 武器2 == WeaponType::Knife ||
			武器2 == WeaponType::nunchucks || 武器2 == WeaponType::rod ||
			武器2 == WeaponType::Spear
			) {
			切刀断闪蓄();
		}
		else {
			int 自动按键 = KEY_W;
			if (rand() % 5 < 3) {
				自动按键 = KEY_S;
			}
			root_sendKeyPress(自动按键);
			root_sendKeyPress(KEY_LEFTSHIFT);
			int WaitTIme11Temp = 0;
			bool 首次循环到 = true;
			ULONGLONG WaitTIme10 = GetTickCount64() + 1000;
			while (WaitTIme10 > GetTickCount64()) {
				if (WaitTIme11Temp > 110 + maxTemp && 首次循环到) {
					root_kmNet_mouse_right(1);
					首次循环到 = false;
				}
				//MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
				if ((WaitTIme11Temp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180 + maxTemp) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				WaitTIme11Temp++;
				Sleep(1);
			}
			root_kmNet_mouse_right(1);
			root_sendKeyUp(自动按键);
			root_sendKeyUp(KEY_LEFTSHIFT);
			Sleep(1);
		}
	}
	else {
		int 自动按键 = KEY_W;
		if (rand() % 5 < 3) {
			自动按键 = KEY_S;
		}
		root_sendKeyPress(自动按键);
		root_sendKeyPress(KEY_LEFTSHIFT);
		int changshanTemp = 0;
		bool 首次循环到 = true;
		ULONGLONG WaitTIme10 = GetTickCount64() + 1000;
		while (WaitTIme10 > GetTickCount64()) {
			if (changshanTemp > 110 + maxTemp && 首次循环到) {
				root_kmNet_mouse_right(1);
				首次循环到 = false;
			}
			//MyLog(标识, "长闪判断次数:", changshanTemp);
			if ((changshanTemp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180 + maxTemp) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			changshanTemp++;
			Sleep(1);
		}
		root_kmNet_mouse_right(1);
		root_sendKeyUp(自动按键);
		root_sendKeyUp(KEY_LEFTSHIFT);
		Sleep(1);
	}
	
}
void 振刀逻辑::切刀断闪蓄(int 蓄力方式) {
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyUpAll();
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	ULONGLONG WaitTIme1 = GetTickCount64() + 700;
	while (WaitTIme1 > GetTickCount64()) {
		if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}

	if (蓄力方式 == 0) {
		root_kmNet_mouse_left(1);
	}
	else {
		root_kmNet_mouse_right(1);
	}

	Sleep(10);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 600;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyWeaponType != 当前武器 && LocalPlayer_Data.MyWeaponType != 0) {
			MyLog("闪切刀成功");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
}
void 振刀逻辑::横刀太刀切刀颠勺() {
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	/*if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {

	}*/
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_left(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(60);
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		if (WaitTIme6Temp > 200) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(2);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}

	ULONGLONG WaitTIme8 = GetTickCount64() + 600;
	while (WaitTIme8 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType != 当前武器 && LocalPlayer_Data.MyWeaponType != 0) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTIme9 = GetTickCount64() + 1500;
	while (WaitTIme9 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme30 = GetTickCount64() + 500;
	while (WaitTIme30 > GetTickCount64()) {
		Sleep(1);
		if (滑铲状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}
	root_sendKeyUp(KEY_S);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme10 = GetTickCount64() + 500;
	while (WaitTIme10 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme11 = GetTickCount64() + 500;
	while (WaitTIme11 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme12 = GetTickCount64() + 500;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}

	ULONGLONG WaitTIme13 = GetTickCount64() + 600;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == 当前武器) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	changshanTemp = 0;
	ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
	while (WaitTIme14 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}
	Sleep(20);
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme15 = GetTickCount64() + 500;
	while (WaitTIme15 > GetTickCount64()) {
		Sleep(1);
		if (滑铲状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}
	root_sendKeyUp(KEY_S);
	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme16 = GetTickCount64() + 500;
	while (WaitTIme16 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme17 = GetTickCount64() + 500;
	while (WaitTIme17 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme18 = GetTickCount64() + 500;
	while (WaitTIme18 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme19 = GetTickCount64() + 500;
	while (WaitTIme19 > GetTickCount64()) {
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme20 = GetTickCount64() + 500;
	while (WaitTIme20 > GetTickCount64()) {
		if (B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme21 = GetTickCount64() + 500;
	while (WaitTIme21 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme22 = GetTickCount64() + 500;
	while (WaitTIme22 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme23 = GetTickCount64() + 500;
	while (WaitTIme23 > GetTickCount64()) {
		if (A2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme24 = GetTickCount64() + 500;
	while (WaitTIme24 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme25 = GetTickCount64() + 500;
	while (WaitTIme25 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	bool 滞空判断 = false;
	root_sendKeyUp(KEY_W);
	ULONGLONG WaitTIme26 = GetTickCount64() + 500;
	while (WaitTIme26 > GetTickCount64()) {
		if (Judgefly(LocalPlayer_Data.MyActionName)) {
			滞空判断 = true;
			break;
		}
		else {
			滞空判断 = false;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (滞空判断) {
		// 直接蓄力
		root_kmNet_mouse_right(1);
		ULONGLONG WaitTIme27 = GetTickCount64() + 1800;
		while (WaitTIme27 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_kmNet_mouse_right(0);

		ULONGLONG WaitTIme28 = GetTickCount64() + 1500;
		while (WaitTIme28 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 11) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme29 = GetTickCount64() + 1500;
		while (WaitTIme29 > GetTickCount64()) {

			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力命中");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							MyLog("进入长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);

					break;

				}
				else {
					通用切刀重置蓄力();
					break;
				}
			}

			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("颠勺后 白裂未命中");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;

				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}
}

void 振刀逻辑::横刀切刀颠勺() {
	int 当前武器 = LocalPlayer_Data.MyWeaponType;
	/*if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {

	}*/
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	if (判断鼠标左键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_left(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(60);
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_W);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	Sleep(20);
	root_sendKeyUp(KEY_W);

	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);

	ULONGLONG WaitTIme8 = GetTickCount64() + 600;
	while (WaitTIme8 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_S);
	Sleep(5);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTIme9 = GetTickCount64() + 1500;
	while (WaitTIme9 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪避");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}


	/*ULONGLONG WaitTIme40 = GetTickCount64() + 1500;
	while (WaitTIme40 > GetTickCount64()) {
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (StrStrW(LocalPlayer_Data.MyActionName.data(), L"_sprint_01")) {
			MyLog("已落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			MyLog("特殊状态退出");
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}*/

	Sleep(85);
	MyLog("点按C");
	root_sendKeyPress(KEY_C);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(10);
	//root_sendKeyUp(KEY_S);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_W);
	ULONGLONG WaitTIme10 = GetTickCount64() + 500;
	while (WaitTIme10 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme11 = GetTickCount64() + 500;
	while (WaitTIme11 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme12 = GetTickCount64() + 500;
	while (WaitTIme12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyUp(KEY_W);
	Sleep(18);
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);

	ULONGLONG WaitTIme13 = GetTickCount64() + 600;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(40);
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	changshanTemp = 0;
	ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
	while (WaitTIme14 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪避");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}


	// 等待落地
	/*ULONGLONG WaitTIme40 = GetTickCount64() + 500;
	while (WaitTIme40 > GetTickCount64()) {
		Sleep(1);
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			MyLog("落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}*/

	Sleep(35);
	MyLog("点按C");
	root_sendKeyPress(KEY_C);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(10);
	//root_sendKeyUp(KEY_S);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme15 = GetTickCount64() + 500;
	while (WaitTIme15 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme16 = GetTickCount64() + 500;
	while (WaitTIme16 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme17 = GetTickCount64() + 500;
	while (WaitTIme17 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	/*88888888*/
	root_sendKeyUp(KEY_W);
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);

	ULONGLONG WaitTIme113 = GetTickCount64() + 600;
	while (WaitTIme113 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(40);
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	changshanTemp = 0;
	ULONGLONG WaitTIme114 = GetTickCount64() + 1500;
	while (WaitTIme114 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪避");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}


	// 等待落地
	/*ULONGLONG WaitTIme40 = GetTickCount64() + 500;
	while (WaitTIme40 > GetTickCount64()) {
		Sleep(1);
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			MyLog("落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}*/

	Sleep(35);
	MyLog("点按C");
	root_sendKeyPress(KEY_C);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(10);
	//root_sendKeyUp(KEY_S);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme115 = GetTickCount64() + 500;
	while (WaitTIme115 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme116 = GetTickCount64() + 500;
	while (WaitTIme116 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme117 = GetTickCount64() + 500;
	while (WaitTIme117 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*88888888*/

	/*88888**********************************************888*/
	root_sendKeyUp(KEY_W);
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);

	ULONGLONG WaitTIme1113 = GetTickCount64() + 600;
	while (WaitTIme1113 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(40);
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	changshanTemp = 0;
	ULONGLONG WaitTIme1114 = GetTickCount64() + 1500;
	while (WaitTIme1114 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪避");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}


	// 等待落地
	/*ULONGLONG WaitTIme40 = GetTickCount64() + 500;
	while (WaitTIme40 > GetTickCount64()) {
		Sleep(1);
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			MyLog("落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}*/

	Sleep(35);
	MyLog("点按C");
	root_sendKeyPress(KEY_C);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(10);
	//root_sendKeyUp(KEY_S);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme1115 = GetTickCount64() + 500;
	while (WaitTIme1115 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme1116 = GetTickCount64() + 500;
	while (WaitTIme1116 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme1117 = GetTickCount64() + 500;
	while (WaitTIme1117 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*88888**********************************************888*/

	/*9999999999**********************************************99999999999*/
	root_sendKeyUp(KEY_W);
	root_sendKeyPress(KEY_0_CPARENTHESIS);
	Sleep(10);
	root_sendKeyUp(KEY_0_CPARENTHESIS);

	ULONGLONG WaitTIme213 = GetTickCount64() + 600;
	while (WaitTIme213 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(40);
	root_sendKeyPress(KEY_S);
	Sleep(10);
	root_sendKeyPress(KEY_LEFTSHIFT);
	changshanTemp = 0;
	ULONGLONG WaitTIme214 = GetTickCount64() + 1500;
	while (WaitTIme214 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			MyLog("进入长闪避");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}

	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}


	// 等待落地
	/*ULONGLONG WaitTIme40 = GetTickCount64() + 500;
	while (WaitTIme40 > GetTickCount64()) {
		Sleep(1);
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			MyLog("落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}*/

	Sleep(35);
	MyLog("点按C");
	root_sendKeyPress(KEY_C);
	root_sendKeyUp(KEY_LEFTSHIFT);
	Sleep(10);
	//root_sendKeyUp(KEY_S);
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);

	ULONGLONG WaitTIme215 = GetTickCount64() + 500;
	while (WaitTIme215 > GetTickCount64()) {
		Sleep(1);
		if (滑铲CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("滑铲CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}

	ULONGLONG WaitTIme216 = GetTickCount64() + 500;
	while (WaitTIme216 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme217 = GetTickCount64() + 500;
	while (WaitTIme217 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	/*88888**********************************************888*/

	ULONGLONG WaitTIme19 = GetTickCount64() + 1500;
	while (WaitTIme19 > GetTickCount64()) {
		wcout << "等待落地=" << LocalPlayer_Data.MyActionName << endl;
		if (StrStrW(LocalPlayer_Data.MyActionName.data(), L"_start_")) {
			MyLog("已落地");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			MyLog("特殊状态退出");
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	ULONGLONG WaitTIme20 = GetTickCount64() + 500;
	while (WaitTIme20 > GetTickCount64()) {
		if (B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme21 = GetTickCount64() + 500;
	while (WaitTIme21 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme22 = GetTickCount64() + 500;
	while (WaitTIme22 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme23 = GetTickCount64() + 500;
	while (WaitTIme23 > GetTickCount64()) {
		if (A2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme24 = GetTickCount64() + 500;
	while (WaitTIme24 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme25 = GetTickCount64() + 500;
	while (WaitTIme25 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	bool 滞空判断 = false;
	root_sendKeyUp(KEY_W);
	ULONGLONG WaitTIme26 = GetTickCount64() + 500;
	while (WaitTIme26 > GetTickCount64()) {
		if (Judgefly(LocalPlayer_Data.MyActionName)) {
			滞空判断 = true;
			break;
		}
		else {
			滞空判断 = false;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (滞空判断) {
		// 直接蓄力
		root_kmNet_mouse_right(1);
		ULONGLONG WaitTIme27 = GetTickCount64() + 1800;
		while (WaitTIme27 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		出蓄::通用出蓄(FatPlayerData);
		return;
		root_kmNet_mouse_right(0);

		ULONGLONG WaitTIme28 = GetTickCount64() + 1500;
		while (WaitTIme28 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 11) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme29 = GetTickCount64() + 1500;
		while (WaitTIme29 > GetTickCount64()) {

			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力命中");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							MyLog("进入长闪");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);

					break;

				}
				else {
					通用切刀重置蓄力();

					break;
				}
			}

			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("颠勺后 白裂未命中");
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;

				}
				else {
					通用切刀重置蓄力();
					break;
				}
			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}
}
// type 0不切回 1切回原武器 左右刀0左 1右
void 振刀逻辑::通用双刀流切拳蓄力(int type, int 左右刀, bool 直接升龙, bool 左蓄) {
	//root_sendKeyUp(KEY_W);
	string 标识 = "通用双刀流切拳蓄力--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}

	if (!直接升龙) {
		if (左右刀 == 0) {
			if (判断鼠标左键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_left(1);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (A1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		else {
			if (判断鼠标右键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_right(1);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (B1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}


		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (左右刀 == 0) {
			root_kmNet_mouse_left(0);
		}
		else {
			root_kmNet_mouse_right(0);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "出刀过程中");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		while (WaitTIme3 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				MyLog(标识, "出刀结束-准备下蹲");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	//Sleep(80);
	MyLog(标识, "按下C键");
	root_sendKeyPress(KEY_C);
	int WaitTIme4Temp = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTIme4Temp);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTIme4Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Temp++;
		Sleep(1);
	}
	MyLog(标识, "右键按下准备升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "升龙中");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (LocalPlayer_Data.CameraPitch > 20) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);


		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (左蓄) {
			root_kmNet_mouse_left(1);
		}
		else {
			root_kmNet_mouse_right(1);
		}

		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_sendKeyPress(passKey);
		Sleep(10);
		root_sendKeyUp(passKey);

		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		ULONGLONG WaitTIme6 = GetTickCount64() + 500;
		while (WaitTIme6 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 600;
		while (WaitTIme12 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_S);
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);

		if (左蓄) {
			root_kmNet_mouse_left(1);
		}
		else {
			root_kmNet_mouse_right(1);
		}
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		root_sendKeyUp(KEY_S);
	}
	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme7 = GetTickCount64() + 1800;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	//root_sendKeyUp(KEY_C);
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		WaitTIme8 = GetTickCount64() + 800;
	}
	else {
		WaitTIme8 = GetTickCount64() + 1500;
	}

	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}

	bool chanshan = false;
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
		chanshan = true;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
		chanshan = true;
	}

	双刀上次出蓄力时间 = GetTickCount64();
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "切刀右蓄力命中");
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					太刀月影闪切双刀A僵直抓取();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					太刀月影闪切双戟A僵直抓取();
					return;
				}
			}

			else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
					// 链剑双戟
					切刀断闪蓄(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟横刀
					切刀断闪蓄(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟长棍
					切刀断闪蓄();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟双截棍
					切刀断闪蓄();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟双刀
					切刀断闪蓄(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟双戟
					切刀断闪蓄(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 双戟扇子
					切刀断闪蓄(1);
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 双戟长剑
					切刀断闪蓄(1);
					return;
				}
			}
			if ((FatPlayerData.O_Dis > 6 || chanshan) && LocalPlayer_Data.MyCurEnergy > 4000) {
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				int changshanTemp = 0;
				bool 首次循环到 = true;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if (changshanTemp > 100 + maxTemp && 首次循环到) {
						root_kmNet_mouse_right(1);
						首次循环到 = false;
					}
					if ((changshanTemp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180 + maxTemp) {
						MyLog(标识, "进入长闪");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				Sleep(30);
				if (type == 1) {
					// 切回原武器
					if (当前武器位置 == 1) {
						root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyPress(KEY_2_AT);
					}
					Sleep(1);
					if (当前武器位置 == 1) {
						root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyUp(KEY_2_AT);
					}
				}

				Sleep(60);
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);

				/*if ((LocalPlayer_Data.MyWeaponType == WeaponType::twinblades
					|| LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd)) {
					if (FatPlayerData.O_MaxHp < 10) {
						双刀BB右2();
					}
					else {
						root_kmNet_mouse_right(1);
						Sleep(1);
					}
				}*/


				break;

			}
			else {
				Sleep(maxTemp);
				通用切刀重置蓄力(type);

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "右蓄力未命中");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				Sleep(30);
				if (type == 1) {
					// 切回原武器
					if (当前武器位置 == 1) {
						root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyPress(KEY_2_AT);
					}
					Sleep(1);
					if (当前武器位置 == 1) {
						root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyUp(KEY_2_AT);
					}
				}
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;

			}
		}

		Sleep(1);
	}
}
// type 0不切回 1切回原武器 左右刀0左 1右
void 振刀逻辑::双刀铁马残红切刀蓄(int type, int 左右刀, bool 直接升龙, bool 左蓄) {
	//root_sendKeyUp(KEY_W);
	string 标识 = "双刀铁马残红切刀蓄--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}

	if (!直接升龙) {
		if (左右刀 == 0) {
			if (判断鼠标左键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_left(1);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (A1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		else {
			if (判断鼠标右键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_right(1);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (B1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}


		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (左右刀 == 0) {
			root_kmNet_mouse_left(0);
		}
		else {
			root_kmNet_mouse_right(0);
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				MyLog(标识, "出刀过程中");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		while (WaitTIme3 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				MyLog(标识, "出刀结束-准备下蹲");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	//Sleep(80);
	MyLog(标识, "按下C键");
	root_sendKeyPress(KEY_C);
	int WaitTIme4Temp = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTIme4Temp);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTIme4Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Temp++;
		Sleep(1);
	}
	MyLog(标识, "右键按下准备升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog(标识, "升龙中");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (LocalPlayer_Data.CameraPitch > 20) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);


		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}

		root_kmNet_mouse_left(1);


		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_sendKeyPress(passKey);
		Sleep(10);
		root_sendKeyUp(passKey);

		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		ULONGLONG WaitTIme6 = GetTickCount64() + 500;
		while (WaitTIme6 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 600;
		while (WaitTIme12 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_W);
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);


		root_kmNet_mouse_left(1);

		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		root_sendKeyUp(KEY_W);
	}
	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme7 = GetTickCount64() + 1800;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	//root_sendKeyUp(KEY_C);
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTIme8;
	WaitTIme8 = GetTickCount64() + 2000;


	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}
	双刀上次出蓄力时间 = GetTickCount64();
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "铁马残红命中");
			双刀BB();
		
			if (FatPlayerData.O_Dis > 2 && LocalPlayer_Data.MyCurEnergy > 6000) {
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						MyLog(标识, "进入长闪");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				Sleep(30);
				if (type == 1) {
					// 切回原武器
					if (当前武器位置 == 1) {
						root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyPress(KEY_2_AT);
					}
					Sleep(1);
					if (当前武器位置 == 1) {
						root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyUp(KEY_2_AT);
					}
				}

				Sleep(60);
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);

				/*if ((LocalPlayer_Data.MyWeaponType == WeaponType::twinblades
					|| LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd)) {
					if (FatPlayerData.O_MaxHp < 10) {
						双刀BB右2();
					}
					else {
						root_kmNet_mouse_right(1);
						Sleep(1);
					}
				}*/


				break;

			}
			else {
				通用切刀重置蓄力(type);

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "右蓄力未命中");
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				Sleep(30);
				if (type == 1) {
					// 切回原武器
					if (当前武器位置 == 1) {
						root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyPress(KEY_2_AT);
					}
					Sleep(1);
					if (当前武器位置 == 1) {
						root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
					}
					else {
						root_sendKeyUp(KEY_2_AT);
					}
				}
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;

			}
		}

		Sleep(1);
	}
}
void 振刀逻辑::长剑重刃s闪AB() {
	string 标识 = "长剑重刃s闪AB--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int 武器2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;

	if (判断鼠标右键是否按下()) {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_right(1);
	}
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出刀结束");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_C);
	int WaitTIme4Time = 0;
	ULONGLONG WaitTImeb4 = GetTickCount64() + 600;
	while (WaitTImeb4 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTIme4Time);
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTIme4Time > 10 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Time++;
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	MyLog(标识, "按下右键开始升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTImeb14 = GetTickCount64() + 500;
	while (WaitTImeb14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (当前武器位置 == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (当前武器位置 == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}

	ULONGLONG WaitTImeq12 = GetTickCount64() + 600;
	while (WaitTImeq12 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyWeaponType == 武器2) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_S);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTImeab10 = GetTickCount64() + 1500;
	while (WaitTImeab10 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}


	root_kmNet_mouse_left(1);

	root_sendKeyUp(KEY_LEFTSHIFT);
	root_sendKeyUp(KEY_S);
	ULONGLONG WaitTImeb1 = GetTickCount64() + 500;
	while (WaitTImeb1 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTImeb6 = GetTickCount64() + 500;
	while (WaitTImeb6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(50);
	root_kmNet_mouse_right(1);
	int WaitTImeab1Temp = 0;
	ULONGLONG WaitTImeab1 = GetTickCount64() + 1500;
	while (WaitTImeab1 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTImeab1Temp);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTImeab1Temp > 20 || 重刃AB状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeab1Temp++;
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	Sleep(1000);
	ULONGLONG WaitTImeba12 = GetTickCount64() + 1300;
	while (WaitTImeba12 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTImeba1 = GetTickCount64() + 1500;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 10) {
			MyLog(标识, "MyEndureLevel == 10");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog("ab释放完毕");
	通用切刀重置蓄力(true);
}

void 振刀逻辑::通用拳切百裂(bool 直接升龙, int 蓄力方式, bool 左A抓, int 结束闪避方式) {
	string 标识 = "通用拳切百裂--";
	MyLog(标识, "开始");
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	std::chrono::steady_clock::time_point start;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	if (!直接升龙) {
		if (左A抓) {
			if (判断鼠标左键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {

				root_kmNet_mouse_left(1);
			}
			if (!Function::半自动博弈::自动连招 && !博弈键按下) {
				return;
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (A1状态(LocalPlayer_Data.MyActionName)) {
					MyLog("A1状态");
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_kmNet_mouse_left(0);
			start = std::chrono::high_resolution_clock::now();
		}
		else {
			if (判断鼠标右键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_right(1);
			}
			if (!Function::半自动博弈::自动连招 && !博弈键按下) {
				return;
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (B1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
			root_kmNet_mouse_right(0);
		}

		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		while (WaitTIme3 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				MyLog(标识, "出刀结束");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}
	else {

	}
	//Sleep(60);
	MyLog(标识, "敌人僵直：", FatPlayerData.O_ActionDuration);
	MyLog(标识, "按下C键");

	//auto end1 = std::chrono::high_resolution_clock::now();
	//auto duration_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
	//std::cout << "平A到按下C时间: " << duration_ms1 << " 毫秒" << std::endl;
	root_sendKeyPress(KEY_C);
	int WaitTIme4Time = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 600;
	while (WaitTIme4 > GetTickCount64()) {
		//MyLog("循环次数:", WaitTIme4Time);
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTIme4 > 10 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲前摇");

			/*	auto end2 = std::chrono::high_resolution_clock::now();
				auto duration_ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();
				std::cout << "平A到已经下蹲时间: " << duration_ms2 << " 毫秒" << std::endl;*/
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Time++;
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	MyLog(标识, "按下右键开始升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (LocalPlayer_Data.CameraPitch > 20 || 振刀逻辑::不可拳切白裂武器(LocalPlayer_Data.MyWeaponType)) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		if (蓄力方式 == 1) {
			root_kmNet_mouse_right(1);
		}
		else {
			root_kmNet_mouse_left(1);
		}
		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(10);
		root_sendKeyUp(passKey);
		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		ULONGLONG WaitTIme6 = GetTickCount64() + 500;
		while (WaitTIme6 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(10);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 600;
		while (WaitTIme12 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		int key = rand() % 3;
		int keyBood = 0;
		if (key == 0) {
			keyBood = KEY_S;
		}
		else if (key == 1) {
			keyBood = KEY_A;
		}
		else if (key == 2) {
			keyBood = KEY_D;
		}
		root_sendKeyPress(keyBood);
		root_sendKeyPress(KEY_LEFTSHIFT);
		if (当前武器位置 == 1) {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyPress(KEY_2_AT);
		}
		if (蓄力方式 == 1) {
			root_kmNet_mouse_right(1);
		}
		else {
			root_kmNet_mouse_left(1);
		}
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		if (当前武器位置 == 1) {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyUp(KEY_2_AT);
		}
		root_sendKeyUp(keyBood);
	}

	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme7 = GetTickCount64() + 1800;
	while (WaitTIme7 > GetTickCount64()) {
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime > 0.505/* && LocalPlayer_Data.MyEndureLevel == 20*/) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme7Temp++;
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_sendKeyUp(KEY_C);
	if (蓄力方式 == 1) {
		root_kmNet_mouse_right(0);
	}
	else {
		root_kmNet_mouse_left(0);
	}
	ULONGLONG WaitTIme8 = GetTickCount64() + 2500;

	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		WaitTIme8 = GetTickCount64() + 800;
	}

	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "MyEndureLevel == 11");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	/*ULONGLONG WaitTIme81 = GetTickCount64() + 500;
	while (WaitTIme81 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}*/
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
	}
	
	MyLog(标识, "蓄力释放完毕");
	if (是否半自动模式) {
		return;
	}
	int temp1 = 0;
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "白裂命中");
			if ((FatPlayerData.O_Dis > 3 || 结束闪避方式 == 1) && LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog(标识, "进W长闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				bool 首次循环到 = true;;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if (changshanTemp > 100 + maxTemp && 首次循环到) {
						if (蓄力方式 == 1) {
							root_kmNet_mouse_right(1);
						}
						else {
							root_kmNet_mouse_left(1);
						}
						首次循环到 = false;
					}
					if ((changshanTemp > (130 + maxTemp) && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > (180 + maxTemp)) {
						MyLog(标识,"进长闪循环:", changshanTemp);
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (temp1 >= 10 && FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("白裂未命中");
			if (LocalPlayer_Data.MyCurEnergy > 3500) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑太刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑横刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑长棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双截棍
					切刀断闪蓄();
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双刀
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑双戟
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑扇子
					切刀断闪蓄(1);
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::lj && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::lj && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 长剑链剑
					切刀断闪蓄(1);
				}
				else {
					if (结束闪避方式 == 0) {
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(1);
					}
					else if (结束闪避方式 == 1) {
						// 长闪进蓄力
						int 自动按键 = KEY_S;
						if (FatPlayerData.O_Dis > 4) {
							自动按键 = KEY_W;
						}

						root_sendKeyPress(自动按键);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
							Sleep(1);
						}

						root_sendKeyUp(自动按键);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(1);
					}

				}
				break;

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}
		temp1++;
		Sleep(1);
	}

}

void 振刀逻辑::长枪基础跳A() {
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme114 = GetTickCount64() + 500;
	while (WaitTIme114 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	// 跳A
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme115 = GetTickCount64() + 300;
	while (WaitTIme115 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme16 = GetTickCount64() + 300;
	while (WaitTIme16 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme17 = GetTickCount64() + 300;
	while (WaitTIme17 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme117 = GetTickCount64() + 1000;
	while (WaitTIme117 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (Judgefly(FatPlayerData.O_ActionName)) {
		// 对面是击飞状态
	}
}

void 振刀逻辑::执行横刀平A僵直抓取() {
	string 标识 = "横刀平A僵直抓取--";
	if (是否半自动模式) {
		振刀逻辑::CollideKnife();
	}
	else {
		root_kmNet_mouse_left(0);
		root_kmNet_mouse_right(0);
		/*if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
		}*/
	}
	/*root_kmNet_mouse_left(0);
	root_kmNet_mouse_right(0);*/

	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12 && !跑A1状态(LocalPlayer_Data.MyActionName))
		{
			平A方式 = 0;
			break;
		}

		if (跑A1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			平A方式 = 3;
			break;
		}

		if (A4状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 4;
			break;
		}

		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12)
		{
			平A方式 = 1;
			break;
		}

		if (LocalPlayer_Data.MyRangeReactionType == 12 && A2状态(LocalPlayer_Data.MyActionName) || B2状态(LocalPlayer_Data.MyActionName) || B4状态(LocalPlayer_Data.MyActionName) /*|| 判断出招(读_攻击文本(全_本人地址), 全_本人手持, )*/)
		{
			跳出方式 = 2;
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	MyLog(标识, "跳出方式=", 跳出方式);

	ULONGLONG WaitTIme22 = GetTickCount64() + 500;
	while (WaitTIme22 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog("出刀结束");
			break;
		}

		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (平A方式 == 0 || 平A方式 == 3) {
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 600;
		while (WaitTIme1 > GetTickCount64()) {
			// 平A 执行ACCA + 连招
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "抓到僵直");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog("太刀A浮空/倒地后跳A");
					处决后双跳A();
					break;
				}
				else {
					if (平A方式 == 3) {
						if (!通用CC断蓄力()) {
							root_sendKeyUpAll();
							return;
						}
					}
					else {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}
					}

					MyLog(标识, "重置平A完毕");
					if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
						// 手持横刀双 后续自动判断视角打钩锁

						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
						// 横刀双戟 后续自动判断视角打钩锁

						通用双刀流切拳蓄力();
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::fan ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::fan) {
						// 横刀扇子

						通用双刀流切拳蓄力(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
						// 横刀阔刀

						通用双刀流切拳蓄力(1);
						return;
					}
					else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
						LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
						// 太刀横刀

						通用拳切百裂();

						return;
					}
					else {
						// 后续自动视角判断
						if (!振刀逻辑::侧键按下或半自动()) {
							MyLog("侧键松开");
							root_sendKeyUpAll();
							return;
						}
						if (mem.GetKeyboard()->IsKeyDown(VK_N)) {

							横刀切刀颠勺();
						}
						else {
							通用拳切百裂();
						}
						//横刀拳切百裂();
						//横刀切刀颠勺();
						return;
					}
				}
				break;
			}

			if (tempI > 15 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "未抓到僵直");
				if ((FatPlayerData.O_Dis < 4 && rand() % 4 < 1) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "敌人小于4米/磐石，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_left(0);
					Sleep(10);
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					if (rand() % 7 < 5) {
						if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
							LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
							// 链剑横刀
							切刀断闪蓄(1);
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 太刀横刀
							切刀断闪蓄(1);
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 横刀长棍
							切刀断闪蓄();
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 横刀双截棍
							切刀断闪蓄();
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 横刀双刀
							切刀断闪蓄(1);
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 横刀双戟
							切刀断闪蓄(1);
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
							LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
							// 横刀扇子
							切刀断闪蓄(1);
							return;
						}
						else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
							LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
							// 横刀长剑
							切刀断闪蓄(1);
							return;
						}
					}
					MyLog(标识, "S闪");
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int WaitTIme11Temp = 0;
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
						if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						WaitTIme11Temp++;
						Sleep(1);
					}
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					return;
				}
				if (FatPlayerData.O_Dis < 4 || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_right(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				{
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme10 = GetTickCount64() + 1000;
						while (WaitTIme10 > GetTickCount64()) {
							Sleep(1);
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			Sleep(1);
			tempI++;
		}
	}

	if (平A方式 == 1) {
		MyLog(标识, "b1出刀结束");
		int tempI = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "b1命中");
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (Judgefly(FatPlayerData.O_ActionName) || 落地状态(FatPlayerData.O_ActionName)) {
					MyLog(标识, "双刀A浮空/倒地后跳A");
					处决后双跳A();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					// 横刀双刀

					振刀逻辑::b1c切Ac切A();
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					// 横刀双戟

					振刀逻辑::b1c切Ac切A();
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
					// 横刀长剑

					振刀逻辑::通用C切百裂(1);
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
					// 横刀链剑

					振刀逻辑::通用C切百裂(1);
					return;

				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks) {
					// 横刀双截棍

					振刀逻辑::通用C切百裂();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
					// 横刀太刀

					振刀逻辑::通用C切百裂();
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 3100) {
					单刀拳双切A(true, 1);
					break;
				}
				else {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_SPACEBAR);
					ULONGLONG WaitTIme13 = GetTickCount64() + 500;
					while (WaitTIme13 > GetTickCount64()) {
						if (JudgeJump(LocalPlayer_Data.MyActionName)) {
							MyLog("b1命中 起跳");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_kmNet_mouse_left(1);
					root_sendKeyUp(KEY_SPACEBAR);
					ULONGLONG WaitTIme14 = GetTickCount64() + 500;
					while (WaitTIme14 > GetTickCount64()) {
						if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
							MyLog("跳A");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					// 跳A结束
					root_sendKeyUp(KEY_W);
					root_kmNet_mouse_left(0);
					ULONGLONG WaitTIme15 = GetTickCount64() + 1000;
					while (WaitTIme15 > GetTickCount64()) {
						if (落地状态(LocalPlayer_Data.MyActionName)) {
							MyLog("落地");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
				}
				/*if (滑步B1状态(LocalPlayer_Data.MyActionName)) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_SPACEBAR);
					ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
					while (WaitTIme13 > GetTickCount64()) {
						if (JudgeJump(LocalPlayer_Data.MyActionName)) {
							MyLog("b1命中 起跳");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					root_kmNet_mouse_left(1);
					root_sendKeyUp(KEY_SPACEBAR);
					ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
					while (WaitTIme14 > GetTickCount64()) {
						if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
							MyLog("跳A");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

					// 跳A结束
					root_sendKeyUp(KEY_W);
					root_kmNet_mouse_left(0);
					ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
					while (WaitTIme15 > GetTickCount64()) {
						if (落地状态(LocalPlayer_Data.MyActionName)) {
							MyLog("落地");
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}

				}
				else {
					if (LocalPlayer_Data.MyCurEnergy >= 5000) {
						root_sendKeyUp(KEY_S);
						root_sendKeyUpAll();
						root_sendKeyPress(KEY_S);
						Sleep(1);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int changshanTemp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
								MyLog("进入长闪");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							changshanTemp++;
							Sleep(1);
						}
					}
					else {
						// 进行跳A

						root_sendKeyPress(KEY_W);
						root_sendKeyPress(KEY_SPACEBAR);
						ULONGLONG WaitTIme13 = GetTickCount64() + 1500;
						while (WaitTIme13 > GetTickCount64()) {
							if (JudgeJump(LocalPlayer_Data.MyActionName)) {
								MyLog("起跳");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						root_kmNet_mouse_left(1);
						root_sendKeyUp(KEY_SPACEBAR);
						ULONGLONG WaitTIme14 = GetTickCount64() + 1500;
						while (WaitTIme14 > GetTickCount64()) {
							if (JudgeJumpAndA(LocalPlayer_Data.MyActionName)) {
								MyLog("跳A");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}

						// 跳A结束
						root_sendKeyUp(KEY_W);
						root_kmNet_mouse_left(0);
						ULONGLONG WaitTIme15 = GetTickCount64() + 1500;
						while (WaitTIme15 > GetTickCount64()) {
							if (落地状态(LocalPlayer_Data.MyActionName)) {
								MyLog("落地");
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							Sleep(1);
						}
						root_sendKeyUpAll();
						return;
					}
				}
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				root_kmNet_mouse_left(1);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUp(KEY_S);

				ULONGLONG WaitTIme16 = GetTickCount64() + 1500;
				while (WaitTIme16 > GetTickCount64()) {
					if (A1状态(LocalPlayer_Data.MyActionName)) {
						MyLog("A1状态");
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				root_kmNet_mouse_left(0);
				ULONGLONG WaitTIme17 = GetTickCount64() + 1500;
				while (WaitTIme17 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 12) {
						break;
					}
					if (A1状态(LocalPlayer_Data.MyActionName) && FatPlayerData.O_EndureLevel != 6) {
						// 空A
						跳出方式 = 1;
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme18 = GetTickCount64() + 1500;
				while (WaitTIme18 > GetTickCount64()) {
					if (LocalPlayer_Data.MyRangeReactionType == 10) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme19 = GetTickCount64() + 1500;
				int temp19 = 0;
				while (WaitTIme19 > GetTickCount64()) {
					// 平A 执行ACCA + 连招
					if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
						if (!自动选择重置蓄力方式()) {
							root_sendKeyUpAll();
							return;
						}

						// 暂时只打拳切百裂
						通用拳切百裂();
						break;

					}

					if (temp19 > 2 && !拼刀状态(FatPlayerData.O_ActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}

					temp19++;
					Sleep(1);
				}*/
			}

			if (tempI > 20 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "抓僵直失败");
				if (JudgePanshi(FatPlayerData.O_ActionName) && LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades
					) {
					// 连续打三个c切
					双刀流打磐石();
					return;
				}
				if ((FatPlayerData.O_Dis < 4 && rand() % 4 < 1 ) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();

					root_kmNet_mouse_right(0);
					Sleep(10);
					root_kmNet_mouse_right(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				else {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					if (LocalPlayer_Data.MyCurEnergy > 5000) {
						if (rand() % 7 < 5) {
							if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::lj ||
								LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::lj) {
								// 链剑横刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Katana ||
								LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Katana) {
								// 太刀横刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::rod && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
								LocalPlayer_Data.Weapon_2 == WeaponType::rod && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
								// 横刀长棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
								LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
								// 横刀双截棍
								切刀断闪蓄();
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
								LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
								// 横刀双刀
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
								LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
								// 横刀双戟
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::fan && LocalPlayer_Data.Weapon_2 == WeaponType::hengdao ||
								LocalPlayer_Data.Weapon_2 == WeaponType::fan && LocalPlayer_Data.Weapon_1 == WeaponType::hengdao) {
								// 横刀扇子
								切刀断闪蓄(1);
								return;
							}
							else if (LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && LocalPlayer_Data.Weapon_2 == WeaponType::Sw ||
								LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && LocalPlayer_Data.Weapon_1 == WeaponType::Sw) {
								// 横刀长剑
								切刀断闪蓄(1);
								return;
							}
						}
						MyLog(标识, "S闪");
						root_sendKeyPress(KEY_S);
						root_sendKeyPress(KEY_LEFTSHIFT);
						int WaitTIme11Temp = 0;
						ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
						while (WaitTIme11 > GetTickCount64()) {
							MyLog(标识, "长闪判断次数:", WaitTIme11Temp);
							if ((WaitTIme11Temp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || WaitTIme11Temp > 180) {
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								return;
							}
							WaitTIme11Temp++;
							Sleep(1);
						}
						root_sendKeyUp(KEY_S);
						root_sendKeyUp(KEY_LEFTSHIFT);
						Sleep(10);
						break;
					}
					else {
						MyLog(标识, "切刀重置蓄力");
						通用切刀重置蓄力();

						break;
					}
				}

			}

			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}

			this_thread::sleep_for(chrono::milliseconds(1));
			tempI++;
		}
	}

	if (平A方式 == 4) {
		MyLog(标识, "平A方式 == 4");
		ULONGLONG WaitTIme20 = GetTickCount64() + 1500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				//太刀A4拳切百裂();
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				通用拳切百裂(true);
				break;
			}
			if (temp20 > 40 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "A4未命中");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}
	}

	if (平A方式 == 2) {
		MyLog("平A方式 == 2");
		ULONGLONG WaitTIme20 = GetTickCount64() + 1500;
		int temp20 = 0;
		while (WaitTIme20 > GetTickCount64()) {
			if (temp20 > 5 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				if (!Function::半自动博弈::自动连招 && !博弈键按下) {
					return;
				}
				if (出招后血量判断()) {
					if (!振刀逻辑::侧键按下或半自动()) {
						MyLog("侧键松开");
						root_sendKeyUpAll();
						return;
					}
					root_kmNet_mouse_left(1);
					Sleep(100);
				}
				return;
			}

			if (temp20 > 20) {
				MyLog(标识, "未命中");
				if (!振刀逻辑::侧键按下或半自动()) {
					MyLog("侧键松开");
					root_sendKeyUpAll();
					return;
				}
				if (FatPlayerData.O_Dis < 4) {
					MyLog(标识, "敌人小于4米，直接捏住");
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					ULONGLONG WaitTImeB11 = GetTickCount64() + 800;
					while (WaitTImeB11 > GetTickCount64()) {

						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						Sleep(1);
					}
					return;
				}
				if (LocalPlayer_Data.MyCurEnergy > 5000) {
					// 暂时直接S闪
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
					while (WaitTIme11 > GetTickCount64()) {
						Sleep(1);
						if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}

					}

					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}
				else {
					MyLog("开始CC重置");
					通用切刀重置蓄力();

					break;
				}
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}

			temp20++;
			Sleep(1);
		}

	}
}

void 振刀逻辑::横刀拳切百裂() {
	string 标识 = "横刀拳切百裂--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}
	if (!振刀逻辑::侧键按下或半自动()) {
		MyLog("侧键松开");
		root_sendKeyUpAll();
		return;
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		if (A1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			MyLog("出刀中");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog("出刀完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(60);
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	int WaitTIme4Temp = 0;
	while (WaitTIme4 > GetTickCount64()) {
		if (WaitTIme4Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲前摇");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Temp++;
		Sleep(1);
	}
	MyLog("开始右键升龙");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	if (LocalPlayer_Data.CameraPitch > 20) {
		// 打钩锁白裂
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::侧键博弈::钩锁模式 == 1) {
			Sleep(250);
			MyLog(标识, "切换/长按钩锁模式");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// 判断钩锁出勾
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		root_kmNet_mouse_right(1);
		// 判断钩锁击中
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(10);
		root_sendKeyUp(passKey);
		// 敌人是否滞空
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

	}
	else {
		ULONGLONG WaitTIme6 = GetTickCount64() + 500;
		while (WaitTIme6 > GetTickCount64()) {
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_0_CPARENTHESIS);
		Sleep(1);
		root_sendKeyUp(KEY_0_CPARENTHESIS);
		ULONGLONG WaitTIme12 = GetTickCount64() + 1500;
		while (WaitTIme12 > GetTickCount64()) {
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(KEY_LEFTSHIFT);
		if (当前武器位置 == 1) {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyPress(KEY_2_AT);
		}
		root_kmNet_mouse_right(1);
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		if (当前武器位置 == 1) {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyUp(KEY_2_AT);
		}
	}

	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme7 = GetTickCount64() + 1500;
	while (WaitTIme7 > GetTickCount64()) {
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime > 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8 = GetTickCount64() + 1500;
	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 1500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_Dis > 6 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					changshanTemp++;
				}
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
			else {
				MyLog("开始CC重置");
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;

			}
			else {
				MyLog("开始CC重置");
				通用切刀重置蓄力();

				break;
			}
		}

		Sleep(1);
	}

}

bool 振刀逻辑::是否特殊状态() {
	bool cond_振刀未按下 = !振刀逻辑::侧键按下或半自动();
	bool cond_挂墙 = 挂墙状态(LocalPlayer_Data.MyActionName);
	bool cond_精力为6 = (LocalPlayer_Data.MyEndureLevel == 6);
	bool cond_拼刀 = 拼刀状态(LocalPlayer_Data.MyActionName);
	bool cond_正在振刀 = JudgeZhenDaoing(LocalPlayer_Data.MyActionName);
	bool cond_天海金钟罩 = 天海金钟罩(FatPlayerData.O_ActionName) || 天海金钟罩(FatPlayerData.O_ActionName);
	bool cond_活化闪避 = 活化闪避;

	// 分别打印日志
	if (cond_振刀未按下)
		MyLog("条件触发：振刀侧键未按下或半自动");

	if (cond_天海金钟罩)
		MyLog("条件触发：天海金钟罩");

	if (cond_挂墙)
		MyLog("条件触发：挂墙状态");

	if (cond_精力为6)
		MyLog("条件触发：精力等级为6");

	if (cond_拼刀)
		MyLog("条件触发：拼刀状态");

	if (cond_正在振刀)
		MyLog("条件触发：JudgeZhenDaoing = true");

	if (cond_活化闪避) {
		MyLog("条件触发：活化闪避");
		//root_kmNet_mouse_right(1);
		Sleep(300);
	}
		

	// 汇总判断（等价于你原来的 if）
	if (cond_振刀未按下 ||
		cond_挂墙 ||
		cond_精力为6 ||
		cond_拼刀 ||
		cond_正在振刀 ||
		cond_活化闪避 || 
		cond_天海金钟罩)
	{
		MyLog("特殊状态 解键返回");

		活化闪避 = false;
		return true;
	}

	return false;

}

bool 振刀逻辑::侧键按下或半自动() {
	if (是否半自动模式 || 博弈键按下) {
		return true;
	}
	return false;
}
void 振刀逻辑::长剑浮空3A(int 平A类型) {
	string 标识 = "长剑浮空3A--";
	if (平A类型 == 0) {
		if (判断鼠标左键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_left(1);
		}
		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (A1状态(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}
	else {
		if (判断鼠标右键是否按下()) {
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_right(0);
		}
		else {
			root_kmNet_mouse_right(1);

			//root_kmNet_mouse_right(0);
		}
		ULONGLONG WaitTIme1 = GetTickCount64() + 500;
		while (WaitTIme1 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (B1状态(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}


	if (平A类型 == 0) {
		root_kmNet_mouse_left(0);
	}
	else {
		root_kmNet_mouse_right(0);
	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	//Sleep(60);
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	while (WaitTIme4 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}

	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_Q);
	Sleep(1);
	if (Function::侧键博弈::钩锁模式 == 1) {
		Sleep(250);
		MyLog(标识, "切换/长按钩锁模式");
		root_kmNet_mouse_left(1);
		Sleep(5);
		root_kmNet_mouse_left(0);
		Sleep(10);
	}
	root_sendKeyUp(KEY_Q);
	// 判断钩锁出勾
	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}


	// 判断钩锁击中
	ULONGLONG WaitTIme8 = GetTickCount64() + 600;
	while (WaitTIme8 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_W);

	// 敌人是否滞空
	ULONGLONG WaitTIme16 = GetTickCount64() + 600;
	while (WaitTIme16 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (Judgefly(FatPlayerData.O_ActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// 判断自己是否落地
	ULONGLONG WaitTIme116 = GetTickCount64() + 600;
	while (WaitTIme116 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
		if (落地状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(1);

	ULONGLONG WaitTIme111 = GetTickCount64() + 200;
	while (WaitTIme111 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (B1状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme222 = GetTickCount64() + 500;
	while (WaitTIme222 > GetTickCount64()) {
		if (B1状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog("MyRangeReactionType == 10");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme121 = GetTickCount64() + 300;
	while (WaitTIme121 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A2状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme211 = GetTickCount64() + 1500;
	while (WaitTIme211 > GetTickCount64()) {
		if (A2状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog("MyRangeReactionType == 12");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTIme221 = GetTickCount64() + 500;
	while (WaitTIme221 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog("MyRangeReactionType == 10");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme131 = GetTickCount64() + 300;
	while (WaitTIme131 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (A3状态(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme15 = GetTickCount64() + 500;
	while (WaitTIme15 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					太刀月影闪切双刀A僵直抓取();
					return;
				}
				else {
					MyLog("W闪-通用白裂 命中后无合适武器");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme16 = GetTickCount64() + 1500;
					while (WaitTIme16 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("S闪-双刀太刀切拳百裂 未命中");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme18 = GetTickCount64() + 1500;
				while (WaitTIme18 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
			else {
				MyLog("开始CC重置 双刀太刀切拳百裂 未命中");
				通用切刀重置蓄力();

				break;
			}
		}

		Sleep(1);
	}
}
void 振刀逻辑::通用钩锁白裂(int 平A类型, bool 直接升龙) {
	string 标识 = "通用钩锁白裂--";

	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_2_quality >= LocalPlayer_Data.Weapon_1_quality) {
				MyLog("Weapon_2_quality >= Weapon_1_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality > LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}

	}
	else {
		MyLog("MyWeaponType == Weapon_2");
		if (当前武器位置 == 1) {
			MyLog("当前武器位置 == 1");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_2_AT;
			}
			else {
				MyLog("Weapon_1_quality < Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
		}
		else {
			MyLog("当前武器位置 == 2");
			if (LocalPlayer_Data.Weapon_1_quality >= LocalPlayer_Data.Weapon_2_quality) {
				MyLog("Weapon_1_quality >= Weapon_2_quality");
				passKey = KEY_1_EXCLAMATION_MARK;
			}
			else {
				MyLog("Weapon_2_quality < Weapon_1_quality");
				passKey = KEY_2_AT;
			}
		}
	}

	if (!直接升龙) {
		if (平A类型 == 0) {
			if (判断鼠标左键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_left(1);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (A1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		else {
			if (判断鼠标右键是否按下()) {
				root_kmNet_mouse_left(0);
				root_kmNet_mouse_right(0);
			}
			else {
				root_kmNet_mouse_right(1);

				//root_kmNet_mouse_right(0);
			}
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			while (WaitTIme1 > GetTickCount64()) {
				if (!判断敌方是否受击中()) {
					root_sendKeyUpAll();
					判断断闪蓄();
					return;
				}
				if (B1状态(LocalPlayer_Data.MyActionName)) {
					break;
				}
				if (振刀逻辑::是否特殊状态())
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}


		if (平A类型 == 0) {
			root_kmNet_mouse_left(0);
		}
		else {
			root_kmNet_mouse_right(0);
		}
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}

		ULONGLONG WaitTIme3 = GetTickCount64() + 500;
		while (WaitTIme3 > GetTickCount64()) {
			if (!判断敌方是否受击中()) {
				root_sendKeyUpAll();
				判断断闪蓄();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}

	//Sleep(60);
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme4 = GetTickCount64() + 500;
	int WaitTIme4Temp = 0;
	while (WaitTIme4 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (WaitTIme4Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Temp++;
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (CB状态(LocalPlayer_Data.MyActionName)) {
			MyLog("进入CB状态");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (!判断敌方是否受击中()) {
		root_sendKeyUpAll();
		判断断闪蓄();
		return;
	}

	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(KEY_Q);
	Sleep(1);
	if (Function::侧键博弈::钩锁模式 == 1) {
		Sleep(250);
		MyLog(标识, "切换/长按钩锁模式");
		root_kmNet_mouse_left(1);
		Sleep(5);
		root_kmNet_mouse_left(0);
		Sleep(10);
	}
	root_sendKeyUp(KEY_Q);
	// 判断钩锁出勾
	ULONGLONG WaitTIme7 = GetTickCount64() + 500;
	while (WaitTIme7 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (钩锁出钩(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_kmNet_mouse_right(1);
	// 判断钩锁击中
	ULONGLONG WaitTIme8 = GetTickCount64() + 600;
	while (WaitTIme8 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (钩锁撞击敌人(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyPress(passKey);
	Sleep(10);
	root_sendKeyUp(passKey);
	// 敌人是否滞空
	ULONGLONG WaitTIme16 = GetTickCount64() + 600;
	while (WaitTIme16 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		if (Judgefly(FatPlayerData.O_ActionName)) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme13 = GetTickCount64() + 1800;
	while (WaitTIme13 > GetTickCount64()) {
		if (!判断敌方是否受击中()) {
			root_sendKeyUpAll();
			判断断闪蓄();
			return;
		}
		//MyLog("蓄力判断循环次数:", WaitTIme7Temp, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("第二段循环次数:", WaitTIme7Temp2, "-蓄力时间:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 10 || LocalPlayer_Data.MyBlueTime >= 0.505 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(标识, "蓄力释放-蓄力时间:", LocalPlayer_Data.MyBlueTime, "--循环次数:", WaitTIme7Temp2);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	出蓄::通用出蓄(FatPlayerData);
	return;
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 2500;
	while (WaitTIme14 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			MyLog(标识, "蓄力释放完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	if (是否半自动模式) {
		return;
	}
	int maxTemp = 0;
	if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		双刀上次出蓄力时间 = GetTickCount64();
		maxTemp += 30;
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		长剑上次出蓄力时间 = GetTickCount64();
		maxTemp += 100;
	}
	ULONGLONG WaitTIme15 = GetTickCount64() + 500;
	while (WaitTIme15 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					太刀月影闪切双刀A僵直抓取();
					return;
				}
				else {
					MyLog("W闪-通用白裂 命中后无合适武器");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					int 首次循环到 = true;
					ULONGLONG WaitTIme16 = GetTickCount64() + 1500;
					while (WaitTIme16 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 100 + maxTemp && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 130 + maxTemp && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp + maxTemp > 180) {
							break;
						}
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							return;
						}
						changshanTemp++;
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}

			}
			else {
				通用切刀重置蓄力();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("S闪-双刀太刀切拳百裂 未命中");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme18 = GetTickCount64() + 1500;
				while (WaitTIme18 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(1);
				break;

			}
			else {
				MyLog("开始CC重置 双刀太刀切拳百裂 未命中");
				通用切刀重置蓄力();

				break;
			}
		}

		Sleep(1);
	}
}

void 振刀逻辑::通用嫖刀抓取() {
	if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana || LocalPlayer_Data.MyWeaponType == WeaponType::Dagge
		|| LocalPlayer_Data.MyWeaponType == WeaponType::fan || LocalPlayer_Data.MyWeaponType == WeaponType::twinblades
		|| LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd || LocalPlayer_Data.MyWeaponType == WeaponType::nunchucks
		|| LocalPlayer_Data.MyWeaponType == WeaponType::hengdao || LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {

		if (B1状态(LocalPlayer_Data.MyActionName) || A1状态(LocalPlayer_Data.MyActionName)) {
			左右键直接松开();
			ULONGLONG WaitTIme3 = GetTickCount64() + 500;
			while (WaitTIme3 > GetTickCount64()) {

				if (LocalPlayer_Data.MyRangeReactionType == 12 && !拼刀状态(LocalPlayer_Data.MyActionName))
				{
					MyLog("嫖刀出刀");
					break;
				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			ULONGLONG WaitTIme4 = GetTickCount64() + 500;
			while (WaitTIme4 > GetTickCount64()) {
				//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
				if (LocalPlayer_Data.MyRangeReactionType == 10)
				{
					MyLog("嫖刀出刀 结束");
					break;
				}
				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}

			ULONGLONG WaitTIme5 = GetTickCount64() + 500;
			while (WaitTIme5 > GetTickCount64()) {
				if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName))
				{
					MyLog("嫖刀僵直抓取成功");
					if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
						if (FatPlayerData.O_ActionDuration > 0.9) {
							执行太刀嫖刀僵直抓取();
						}
						else {
							处决后双跳A();
						}
						return;
					}
					if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
						if (FatPlayerData.O_ActionDuration > 0.35) {
							执行双刀嫖刀僵直抓取();
						}
						else {
							双刀BB();
						}

						return;
					}

				}

				if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog("嫖刀僵直抓取失败");
					break;
				}

				if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
				{
					root_sendKeyUpAll();
					return;
				}

			}

			// 嫖刀僵直抓失败 随机左右闪

			振刀逻辑::随机左右闪();

			if (跳出方式 == 1) {
				root_sendKeyUpAll();
			}

		}
	}
}

void 振刀逻辑::随机左右闪() {
	int keyDodge;
	if (rand() % 2) {
		keyDodge = KEY_A;
	}
	else {
		keyDodge = KEY_D;
	}

	root_sendKeyPress(KEY_W);
	root_sendKeyPress(keyDodge);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		Sleep(1);
		if (JudgeDuanFlash(LocalPlayer_Data.MyActionName)) {
			跳出方式 = 0;
			break;
		}
		if (LocalPlayer_Data.MyEndureLevel == 6) {
			跳出方式 = 1;
			root_sendKeyUpAll();
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}
	if (跳出方式 == 0) {
		root_sendKeyUpAll();
		root_sendKeyUp(KEY_W);
		root_sendKeyUp(KEY_LEFTSHIFT);
		Sleep(10);
		左右键直接松开();
	}

}
void 振刀逻辑::长闪蓄(bool 左蓄, int 方向) {
	string 标识 = "长闪蓄--";
	MyLog(标识, "开始,是否左蓄",左蓄,"-按键", 方向);
	int putKey = KEY_W;
	if (方向 == 0) {
		putKey = KEY_W;
	}
	else if (方向 == 1) {
		putKey = KEY_S;
	}
	else if (方向 == 2) {
		putKey = KEY_A;
	}
	else if (方向 == 3) {
		putKey = KEY_D;
	}

	root_sendKeyPress(putKey);
	root_sendKeyPress(KEY_LEFTSHIFT);
	int changshanTemp = 0;
	ULONGLONG WaitTIme16 = GetTickCount64() + 1000;
	while (WaitTIme16 > GetTickCount64()) {
		Sleep(1);
		if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
		changshanTemp++;
	}
	root_sendKeyUp(putKey);
	root_sendKeyUp(KEY_LEFTSHIFT);
}
bool 振刀逻辑::抓短闪时间判断() {
	return (GetTickCount64() - 敌人上次闪避时间 > Function::侧键博弈::白刀抓短闪延迟 && GetTickCount64() - 敌人上次闪避时间 < 500);
}

bool 振刀逻辑::蓄力抓短闪判断() {
	return (GetTickCount64() - 敌人上次闪避时间 < 600);
}
void 振刀逻辑::执行双刀嫖刀僵直抓取() {
	if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
		LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
		// 手持太双
		if (IsLookUp(LocalPlayer_Data.My_ActorRootPtr)) {

		}
		双刀太刀切拳百裂();
		return;
	}
	else {
		双刀BB();
		return;
	}
}

void 振刀逻辑::执行太刀嫖刀僵直抓取() {
	ULONGLONG WaitTIme3 = GetTickCount64() + 500;
	while (WaitTIme3 > GetTickCount64()) {

		if (B1状态(LocalPlayer_Data.MyActionName) || 滑步B1状态(LocalPlayer_Data.MyActionName))
		{
			//Sleep(1);
			break;
		}
		if (!B1状态(LocalPlayer_Data.MyActionName) && !滑步B1状态(LocalPlayer_Data.MyActionName))
		{
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	MyLog("开始嫖刀 CC重置2");
	root_sendKeyPress(KEY_C);
	ULONGLONG WaitTIme12 = GetTickCount64() + 1500;
	while (WaitTIme12 > GetTickCount64()) {
		if (JudgeCrouch(LocalPlayer_Data.MyActionName)) {
			MyLog("进入下蹲");
			break;
		}
		if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_sendKeyUp(KEY_C);
	Sleep(10);
	root_sendKeyPress(KEY_C);
	Sleep(1);
	root_sendKeyUp(KEY_C);
	Sleep(10);

	if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
		LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
		// 手持太双
		if (IsLookUp(LocalPlayer_Data.My_ActorRootPtr)) {

		}
		通用双刀流切拳蓄力();
		return;
	}
	else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
		LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
		// 太刀双戟
		通用双刀流切拳蓄力();
		return;
	}
	else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::fan ||
		LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::fan) {
		// 太刀扇子
		通用双刀流切拳蓄力();
		return;
	}
	else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
		LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::Blade) {
		// 太刀阔刀
		通用双刀流切拳蓄力();
		return;
	}
	else {
		// 暂时只打拳切百裂
		通用拳切百裂();
		return;
	}

}

void 振刀逻辑::释放受击技能() {

}

bool 振刀逻辑::判断敌方是否受击中() {
	if (FatPlayerData.O_EndureLevel != 6 && FatPlayerData.O_ActionDuration < 0.1) {
		MyLog("敌人不处于受击");
		return false;
	}
	else {
		//MyLog("敌人处于受击中");
		return true;
	}
}
void 振刀逻辑::通用切刀重置蓄力(bool 强制切刀) {
	root_sendKeyUpAll();
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	if (!强制切刀 && (是否单刀武器() ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Blade || LocalPlayer_Data.Weapon_2 == WeaponType::Blade ||
			LocalPlayer_Data.Weapon_1 == WeaponType::saber || LocalPlayer_Data.Weapon_2 == WeaponType::saber)
		)) {
		MyLog("cc断蓄力");
		通用CC断蓄力();
	}
	else {
		// 切刀蓄力
		int 当前武器 = LocalPlayer_Data.MyWeaponType;
		MyLog("c切刀");
		root_sendKeyPress(KEY_C);

		ULONGLONG WaitTIme = GetTickCount64() + 500;
		while (WaitTIme > GetTickCount64()) {
			if (JudgeCrouch(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			if (!侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		振刀逻辑::CollideKnife();
		if (当前武器位置 == 1) {
			root_sendKeyPress(KEY_2_AT);
		}
		else {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}

		Sleep(10);

		if (当前武器位置 == 1) {
			root_sendKeyUp(KEY_2_AT);
		}
		else {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}

		root_sendKeyUp(KEY_C);
		root_sendKeyUpAll();
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (LocalPlayer_Data.MyWeaponType != 当前武器 && LocalPlayer_Data.MyWeaponType != 0) {
				MyLog("切武器2成功");
				break;
			}
			if (振刀逻辑::是否特殊状态())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}
}

bool 振刀逻辑::抓闪判断(wstring O_ActionName) {
	if (StrStrW(O_ActionName.data(), L"_hero_spiritsman_")) {
		return false;
	}
	return true;
}

bool 振刀逻辑::不可拳切白裂武器(int 当前武器) {
	if (当前武器 == WeaponType::twinblades ||
		当前武器 == WeaponType::dualhalberd ||
		当前武器 == WeaponType::Knife) {
		return true;
	}
	return false;
}
bool 振刀逻辑::可C切武器(int 当前武器) {
	//MyLog("Weapon_1:", LocalPlayer_Data.Weapon_1, "Weapon_2:", LocalPlayer_Data.Weapon_2);

	if (
		(LocalPlayer_Data.Weapon_1 == WeaponType::twinblades && 当前武器 != WeaponType::twinblades) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::twinblades && 当前武器 != WeaponType::twinblades) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Sw && 当前武器 != WeaponType::Sw) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::Sw && 当前武器 != WeaponType::Sw) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::Katana && 当前武器 != WeaponType::Katana) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::Katana && 当前武器 != WeaponType::Katana) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd && 当前武器 != WeaponType::dualhalberd) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd && 当前武器 != WeaponType::dualhalberd) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::nunchucks && 当前武器 != WeaponType::nunchucks) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::nunchucks && 当前武器 != WeaponType::nunchucks) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::lj && 当前武器 != WeaponType::lj) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::lj && 当前武器 != WeaponType::lj) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::rod && 当前武器 != WeaponType::rod) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::rod && 当前武器 != WeaponType::rod) ||
		(LocalPlayer_Data.Weapon_1 == WeaponType::hengdao && 当前武器 != WeaponType::hengdao) ||
		(LocalPlayer_Data.Weapon_2 == WeaponType::hengdao && 当前武器 != WeaponType::hengdao)
		) {
		return true;
	}
	else {
		return false;
	}
}



bool 振刀逻辑::是否单刀武器() {
	MyLog("Weapon_1:", LocalPlayer_Data.Weapon_1, "Weapon_2:", LocalPlayer_Data.Weapon_2);
	if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0 && LocalPlayer_Data.Weapon_1 != LocalPlayer_Data.Weapon_2) {
		return false;
	}
	else {
		return true;
	}
}

bool 振刀逻辑::判断鼠标左键是否按下() {
	return 鼠标左键是否按下;
}

bool 振刀逻辑::判断鼠标右键是否按下() {
	return 鼠标右键是否按下;
}

void 振刀逻辑::判断鼠标左键状态改变(bool isEnd) {
	鼠标左键是否按下 = isEnd;
}

void 振刀逻辑::判断鼠标右键状态改变(bool isEnd) {
	鼠标右键是否按下 = isEnd;
}