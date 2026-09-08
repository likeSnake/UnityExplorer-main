#include "ShuangDao.h"
#include <stdint.h>
#include "Hook.h"
#include "Data.h"
#include "DhzBoxNet.h"
#include "ShockKnife.h"
#include "log.h"
#include "landing.h"
#include "Data.h"

void Ë«µ¶Á¬ÕÐ::b1cÇÐAcÇÐAÌúÂí(int Æ½AÀàÐÍ, int »êÓñÀàÐÍ) {
	string ±êÊ¶ = "b1cÇÐAcÇÐA--";
	int µ±Ç°ÎäÆ÷ = LocalPlayer_Data.MyWeaponType;
	int ÎäÆ÷2 = LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1 ? LocalPlayer_Data.Weapon_2 : LocalPlayer_Data.Weapon_1;
	int µ±Ç°ÎäÆ÷Î»ÖÃ = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;

	int passKey;
	if (LocalPlayer_Data.MyWeaponType == LocalPlayer_Data.Weapon_1) {
		MyLog("MyWeaponType == Weapon_1");
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			MyLog("µ±Ç°ÎäÆ÷Î»ÖÃ == 1");
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
			MyLog("µ±Ç°ÎäÆ÷Î»ÖÃ == 2");
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
		if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
			MyLog("µ±Ç°ÎäÆ÷Î»ÖÃ == 1");
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
			MyLog("µ±Ç°ÎäÆ÷Î»ÖÃ == 2");
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

	if (!Õñµ¶Âß¼­::²à¼ü°´ÏÂ»ò°ë×Ô¶¯()) {
		MyLog("²à¼üËÉ¿ª");
		root_sendKeyUpAll();
		return;
	}

	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTIme6Temp = 0;
	ULONGLONG WaitTIme1 = GetTickCount64() + 500;
	while (WaitTIme1 > GetTickCount64()) {
		//MyLog(±êÊ¶, "ÏÂ¶×ÅÐ¶ÏÑ­»·´ÎÊý:", WaitTIme6Temp);
		if (WaitTIme6Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme6Temp++;
		Sleep(1);
	}

	if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
		root_sendKeyPress(KEY_2_AT);
	}
	else {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	Sleep(5);
	if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
		root_sendKeyUp(KEY_2_AT);
	}
	else {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	int WaitTIme2Temp = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		//MyLog(±êÊ¶, "ÇÐÎäÆ÷Ñ­»·´ÎÊý:", WaitTIme2Temp);
		if (WaitTIme2Temp > 40 || LocalPlayer_Data.MyWeaponType == ÎäÆ÷2) {
			MyLog(±êÊ¶, "ÇÐÎäÆ÷2³É¹¦");
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIme2Temp++;
		Sleep(1);
	}

	root_sendKeyUp(KEY_C);

	if (Æ½AÀàÐÍ == 0) {
		root_kmNet_mouse_left(1);
	}
	else {
		root_kmNet_mouse_right(1);
	}

	ULONGLONG WaitTIm3 = GetTickCount64() + 500;
	int WaitTIm3Temp = 0;
	while (WaitTIm3 > GetTickCount64()) {
		//MyLog(±êÊ¶, "Ñ­»·´ÎÊý;", WaitTIm3Temp);
		if (WaitTIm3Temp > 15 || A1×´Ì¬(LocalPlayer_Data.MyActionName) || B1×´Ì¬(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTIm3Temp++;
		Sleep(1);
	}
	if (Æ½AÀàÐÍ == 0) {
		root_kmNet_mouse_left(0);
	}
	else {
		root_kmNet_mouse_right(0);
	}

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
	ULONGLONG WaitTIme5 = GetTickCount64() + 500;
	while (WaitTIme5 > GetTickCount64()) {
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
	if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
		root_sendKeyUpAll();
		return;
	}
	Sleep(10);
	// ÔÙ´ÎCÇÐ
	root_sendKeyUp(KEY_C);
	root_sendKeyPress(KEY_C);
	int WaitTImeB1Temp = 0;
	ULONGLONG WaitTImeB1 = GetTickCount64() + 1500;
	while (WaitTImeB1 > GetTickCount64()) {
		//MyLog(±êÊ¶, "ÔÙ´ÎCÇÐÏÂ¶×ÅÐ¶ÏÑ­»·´ÎÊý:", WaitTImeB1Temp);
		//if (WaitTIme6Temp > 40 ||
		if (WaitTImeB1Temp > 15 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		WaitTImeB1Temp++;
		Sleep(1);
	}

	if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
		root_sendKeyPress(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyPress(KEY_2_AT);
	}
	Sleep(5);
	if (µ±Ç°ÎäÆ÷Î»ÖÃ == 1) {
		root_sendKeyUp(KEY_1_EXCLAMATION_MARK);
	}
	else {
		root_sendKeyUp(KEY_2_AT);
	}
	int WaitTImeB2Temp = 0;
	ULONGLONG WaitTImeB2 = GetTickCount64() + 500;
	while (WaitTImeB2 > GetTickCount64()) {
		MyLog(±êÊ¶, "ÇÐÎäÆ÷Ñ­»·´ÎÊý:", WaitTImeB2Temp);
		if ((WaitTImeB2Temp > 30 || LocalPlayer_Data.MyWeaponType != ÎäÆ÷2) && LocalPlayer_Data.MyWeaponType != 0) {
			MyLog("ÇÐ»ØÎäÆ÷1³É¹¦");
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
		//wcout << "¶¯×÷Ãû=" << LocalPlayer_Data.MyActionName << endl;
		if (WaitTImB3Temp > 15 || A1×´Ì¬(LocalPlayer_Data.MyActionName) || B1×´Ì¬(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
	ULONGLONG WaitTImeB5 = GetTickCount64() + 500;
	while (WaitTImeB5 > GetTickCount64()) {
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
	if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
		root_sendKeyUpAll();
		return;
	}

	//Sleep(60);
	// ¿ªÊ¼ÉýÁú
	root_sendKeyPress(KEY_C);
	int WaitTIme65Temp = 0;
	ULONGLONG WaitTIme6 = GetTickCount64() + 500;
	while (WaitTIme6 > GetTickCount64()) {
		//MyLog(±êÊ¶, "ÉýÁúÏÂ¶×Ñ­»·´ÎÊý:", WaitTIme65Temp);
		if (WaitTIme65Temp > 40 || JudgeSquat(LocalPlayer_Data.MyActionName)) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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
	if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
		root_sendKeyUpAll();
		return;
	}
	root_kmNet_mouse_right(0);

	ULONGLONG WaitTIme14 = GetTickCount64() + 500;
	while (WaitTIme14 > GetTickCount64()) {
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

	// ×Ô¶¯ÅÐ¶Ï´òÊ²Ã´Á¬ÕÐ
	if (LocalPlayer_Data.CameraPitch > 20 || LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd
		|| LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		// ´ò¹³Ëø°×ÁÑ
		root_sendKeyPress(KEY_Q);
		Sleep(1);
		if (Function::²à¼ü²©ÞÄ::¹³ËøÄ£Ê½ == 1) {
			Sleep(250);
			MyLog(±êÊ¶, "ÇÐ»»/³¤°´¹³ËøÄ£Ê½");
			root_kmNet_mouse_left(1);
			Sleep(5);
			root_kmNet_mouse_left(0);
			Sleep(10);
		}
		root_sendKeyUp(KEY_Q);
		// ÅÐ¶Ï¹³Ëø³ö¹´
		ULONGLONG WaitTIme7 = GetTickCount64() + 500;
		while (WaitTIme7 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
				root_sendKeyUpAll();
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

		root_kmNet_mouse_right(1);
		// ÅÐ¶Ï¹³Ëø»÷ÖÐ
		ULONGLONG WaitTIme8 = GetTickCount64() + 600;
		while (WaitTIme8 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
				root_sendKeyUpAll();
				return;
			}
			if (¹³Ëø×²»÷µÐÈË(LocalPlayer_Data.MyActionName)) {
				break;
			}
			if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
			{
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		root_sendKeyPress(passKey);
		Sleep(5);
		root_sendKeyUp(passKey);
		// µÐÈËÊÇ·ñÖÍ¿Õ
		ULONGLONG WaitTIme16 = GetTickCount64() + 600;
		while (WaitTIme16 > GetTickCount64()) {
			if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
				root_sendKeyUpAll();
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
		// È­ÇÐ985
		ULONGLONG WaitTIme15 = GetTickCount64() + 500;
		while (WaitTIme15 > GetTickCount64()) {
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
		ULONGLONG WaitTIme12 = GetTickCount64() + 500;
		while (WaitTIme12 > GetTickCount64()) {
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
		root_sendKeyPress(KEY_LEFTSHIFT);
		root_sendKeyPress(passKey);
		root_kmNet_mouse_right(1);
		Sleep(15);
		root_sendKeyUp(KEY_LEFTSHIFT);
		root_sendKeyUp(passKey);
	}

	// µÈ´ýÐîÁ¦µ½1.0
	int WaitTIme7Temp = 0;
	int WaitTIme7Temp2 = 0;
	ULONGLONG WaitTIme13 = GetTickCount64() + 1800;
	while (WaitTIme13 > GetTickCount64()) {
		if (!Õñµ¶Âß¼­::ÅÐ¶ÏµÐ·½ÊÇ·ñÊÜ»÷ÖÐ()) {
			root_sendKeyUpAll();
			return;
		}
		//MyLog("ÐîÁ¦ÅÐ¶ÏÑ­»·´ÎÊý:", WaitTIme7Temp, "-ÐîÁ¦Ê±¼ä:", LocalPlayer_Data.MyBlueTime);
		if (LocalPlayer_Data.MyBlueTime >= 0.49) {
			//	MyLog("µÚ¶þ¶ÎÑ­»·´ÎÊý:", WaitTIme7Temp2, "-ÐîÁ¦Ê±¼ä:", LocalPlayer_Data.MyBlueTime);
			if (WaitTIme7Temp2 > 9 || LocalPlayer_Data.MyBlueTime >= 0.5 && LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(±êÊ¶, "ÐîÁ¦ÊÍ·Å-ÐîÁ¦Ê±¼ä:", LocalPlayer_Data.MyBlueTime);
				break;
			}
			WaitTIme7Temp2++;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	// ³öÐîÁ¦
	root_kmNet_mouse_right(0);
	ULONGLONG WaitTIme8 = GetTickCount64() + 500;
	while (WaitTIme8 > GetTickCount64()) {
		if (LocalPlayer_Data.MyEndureLevel == 11) {
			break;
		}
		if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
		{
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}
	Sleep(40);
	if (ÊÇ·ñ°ë×Ô¶¯Ä£Ê½) {
		return;
	}
	ULONGLONG WaitTIme9 = GetTickCount64() + 500;
	while (WaitTIme9 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !Æ´µ¶×´Ì¬(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 3800) {
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::twinblades ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::twinblades) {
					Õñµ¶Âß¼­::Ì«µ¶ÔÂÓ°ÉÁÇÐË«µ¶A½©Ö±×¥È¡();
					return;
				}
				else if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					Õñµ¶Âß¼­::Ì«µ¶ÔÂÓ°ÉÁÇÐË«µ¶A½©Ö±×¥È¡();
					return;
				}

				else {
					MyLog("WÉÁ-Ë«µ¶Ì«µ¶ÇÐÈ­°ÙÁÑ ÃüÖÐºóÎÞºÏÊÊÎäÆ÷");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (JudgeLongFlash(LocalPlayer_Data.MyActionName)) {
							break;
						}
						if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
						{
							root_sendKeyUpAll();
							return;
						}
					}
					root_sendKeyUp(KEY_W);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(10);
					break;
				}

			}
			else {
				Õñµ¶Âß¼­::Í¨ÓÃÇÐµ¶ÖØÖÃÐîÁ¦();

				break;
			}
		}

		if (FatPlayerData.O_EndureLevel != 6 || Æ´µ¶×´Ì¬(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
			if (LocalPlayer_Data.MyCurEnergy > 5000) {
				MyLog("SÉÁ-°ÙÁÑ Î´ÃüÖÐ");

				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				ULONGLONG WaitTIme11 = GetTickCount64() + 1500;
				while (WaitTIme11 > GetTickCount64()) {
					if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
						break;
					}
					if (Õñµ¶Âß¼­::ÊÇ·ñÌØÊâ×´Ì¬())
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