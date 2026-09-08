#include "changqiang.h"
#include <stdint.h>
#include "Hook.h"
#include "Data.h"
#include "DhzBoxNet.h"
#include "ShockKnife.h"
#include "log.h"
#include "landing.h"
#include "Data.h"

void ³¤Ç¹Á¬ÕĞ::È­ÇĞAAÌøB(int Æ½AÀàĞÍ, int »êÓñÀàĞÍ) {
	string ±êÊ¶ = "³¤±øÆ÷È­ÇĞAAÌøB--";
	MyLog(±êÊ¶, "¿ªÊ¼");
	int µ±Ç°ÎäÆ÷Î»ÖÃ = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	root_sendKeyPress(KEY_C);
	int WaitTIme4Time = 0;
	ULONGLONG WaitTIme4 = GetTickCount64() + 600;
	while (WaitTIme4 > GetTickCount64()) {
		//MyLog("Ñ­»·´ÎÊı:", WaitTIme4Time);
		if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
			root_sendKeyUpAll();
			Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
			return;
		}
		//wcout << "¶¯×÷Ãû=" << LocalPlayer_Data.MyActionName << endl;
		if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
			MyLog("½øÈëÏÂ¶×Ç°Ò¡");

			/*	auto end2 = std::chrono::high_resolution_clock::now();
				auto duration_ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();
				std::cout << "Æ½Aµ½ÒÑ¾­ÏÂ¶×Ê±¼ä: " << duration_ms2 << " ºÁÃë" << std::endl;*/
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme4Time++;
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	MyLog(±êÊ¶, "°´ÏÂÓÒ¼ü¿ªÊ¼ÉıÁú");
	root_kmNet_mouse_right(1);
	root_sendKeyUp(KEY_C);
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
		if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
			root_sendKeyUpAll();
			Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
			return;
		}
		if (CB×´Ì¬(LocalPlayer_Data.MyActionName)) {
			MyLog("½øÈëCB×´Ì¬");
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
		if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
			root_sendKeyUpAll();
			Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	
	if (LocalPlayer_Data.CameraPitch > 20)
	{
		// ´ò¹³Ëø°×ÁÑ
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::²à¼ü²©ŞÄ::¹³ËøÄ£Ê½ == 1) {
			Sleep(250);
			MyLog(±êÊ¶, "ÇĞ»»/³¤°´¹³ËøÄ£Ê½");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// ÅĞ¶Ï¹³Ëø³ö¹´
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
				root_sendKeyUpAll();
				Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
				return;
			}
			if (¹³Ëø³ö¹³(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_kmNet_mouse_left(1);
		// ÅĞ¶Ï¹³Ëø»÷ÖĞ
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
				root_sendKeyUpAll();
				Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
				return;
			}
			if (¹³Ëø×²»÷µĞÈË(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyPress(KEY_2_AT);
		}
		
		Sleep(10);
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyUp(KEY_2_AT);
		}
		// µĞÈËÊÇ·ñÖÍ¿Õ
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
				root_sendKeyUpAll();
				Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
				return;
			}
			if (Judgefly(FatPlayerData.O_ActionName)) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
			if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
				root_sendKeyUpAll();
				Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
			if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
				root_sendKeyUpAll();
				Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
				return;
			}
			if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyPress(KEY_2_AT);
		}
		
		root_kmNet_mouse_left(1);
		
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
		}
		else {
			root_sendKeyUp(KEY_2_AT);
		}
		root_sendKeyUp(keyBood);

		ULONGLONG WaitTImeaas5 = GetTickCount64() + 170;
		while (WaitTImeaas5 > GetTickCount64()) {
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
	}

	ULONGLONG WaitTImea1 = GetTickCount64() + 500;
	while (WaitTImea1 > GetTickCount64()) {
		if (!Õñµ¶Âß¼­::ÅĞ¶ÏµĞ·½ÊÇ·ñÊÜ»÷ÖĞ()) {
			root_sendKeyUpAll();
			Õñµ¶Âß¼­::ÅĞ¶Ï¶ÏÉÁĞî();
			return;
		}
		if (A1×´Ì¬(LocalPlayer_Data.MyActionName)) {
			MyLog("A1×´Ì¬");
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIm4 = GetTickCount64() + 500;
	while (WaitTIm4 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !Æ´µ¶×´Ì¬(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTImeaa5 = GetTickCount64() + 500;
	while (WaitTImeaa5 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	ULONGLONG WaitTImeaas5 = GetTickCount64() + 130;
	while (WaitTImeaas5 > GetTickCount64()) {
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(1);
	ULONGLONG WaitTIme23 = GetTickCount64() + 500;
	while (WaitTIme23 > GetTickCount64()) {
		if (A2×´Ì¬(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_left(0);

	ULONGLONG WaitTIme24 = GetTickCount64() + 500;
	while (WaitTIme24 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12 && !Æ´µ¶×´Ì¬(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	root_sendKeyPress(KEY_SPACEBAR);
	ULONGLONG WaitTIme = GetTickCount64() + 500;
	while (WaitTIme > GetTickCount64()) {

		if (JudgeJump(LocalPlayer_Data.MyActionName))
		{
			MyLog("ÆğÌø");
			break;
		}
		else if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	root_kmNet_mouse_right(1);
	Sleep(5);
	root_kmNet_mouse_right(0);
}