#include "xulihou.h"


void 出蓄::通用出蓄(PlayerData FatPlayerData) {
	if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
		出蓄::太刀出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
		出蓄::双刀出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
		出蓄::双戟出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Dagge || LocalPlayer_Data.MyWeaponType == WeaponType::fan) {
		出蓄::扇子出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {
		出蓄::横刀出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
		出蓄::长剑出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::nunchucks) {
		出蓄::双截棍出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::rod) {
		出蓄::长棍出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::saber) {
		出蓄::斩马出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
		出蓄::长枪出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::lj) {
		出蓄::链剑出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
		出蓄::拳刃出蓄();
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Knife) {
		
	}
	else if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
		
	}
}

void 出蓄::双刀出蓄() {
	string 标识 = "双刀出蓄--";
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}
	Sleep(1);
	if (是否半自动模式) {
		return;
	}
	双刀上次出蓄力时间 = GetTickCount64();
	
	ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
	while (WaitTIme2 > GetTickCount64()) {
		int tempTime2 = 0;
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			root_sendKeyUpAll();
			if (LocalPlayer_Data.MyCurEnergy > 4000) {
				if (!Judgefly(FatPlayerData.O_ActionName) && (!振刀逻辑::是否单刀武器() && (LocalPlayer_Data.Weapon_1 != WeaponType::Blade && LocalPlayer_Data.Weapon_2 != WeaponType::Blade && LocalPlayer_Data.Weapon_1 != WeaponType::saber && LocalPlayer_Data.Weapon_2 != WeaponType::saber))) {
					振刀逻辑::判断断闪蓄();
					break;
				}
				else {
					int 当前按键 = KEY_W;
					if (rand() % 3 < 1) {
						当前按键 = KEY_S;
					}
					root_sendKeyPress(当前按键);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					root_sendKeyUp(当前按键);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}

			}
			else {
				振刀逻辑::通用切刀重置蓄力();
				break;
			}

		}
		if (tempTime2 > 10) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}
				else {
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
		}
		if (!振刀逻辑::侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::太刀出蓄() {
	string 标识 = "太刀出蓄--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 0;
	if (LocalPlayer_Data.MyBlueTime >= 1.0 || AB3蓄力状态(LocalPlayer_Data.MyActionName)) {
		MyLog(标识, "AB3/2.0 蓄力状态");
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
		if (振刀逻辑::是否特殊状态()) {
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
		if (振刀逻辑::是否特殊状态()) {
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
			root_sendKeyUpAll();

			//Sleep(witTime);
			if (LocalPlayer_Data.MyCurEnergy > 4000) {
				if ((!振刀逻辑::是否单刀武器() && (LocalPlayer_Data.Weapon_1 != WeaponType::Blade && LocalPlayer_Data.Weapon_2 != WeaponType::Blade && LocalPlayer_Data.Weapon_1 != WeaponType::saber && LocalPlayer_Data.Weapon_2 != WeaponType::saber))) {
					振刀逻辑::判断断闪蓄();
					break;
				}
				
				MyLog(标识, "精力足够 -进w闪-", LocalPlayer_Data.MyCurEnergy);
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				bool 首次循环到 = true;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1000;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if (changshanTemp > 110  && 首次循环到) {
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
				root_kmNet_mouse_right(1);
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog(标识, "精力不够 -CC重置");
				振刀逻辑::通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 10) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "太刀蓄力未命中 ");
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::横刀出蓄() {
	string 标识 = "横刀出蓄--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int xuType = 0; // 0左蓄力 1右蓄力
	// 判断当前是左蓄力还是右蓄力
	wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
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
		int WaitTIme1Tem = 0;
		ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
		while (WaitTIme1 > GetTickCount64()) {
			if (WaitTIme1Tem > 50 && LocalPlayer_Data.MyCurTime >= 0.2) {
				MyLog("横刀 --出左蓄力准备转曲步");
				左蓄力跳出方式 = 2;
				break;
			}
			if (LocalPlayer_Data.MyEndureLevel == 11) {
				MyLog("横刀 --左蓄力正常释放");
				左蓄力跳出方式 = 0;
				break;
			}
			if (WaitTIme1Tem > 50 && JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
				MyLog("横刀 --对面振刀，转抓");
				左蓄力跳出方式 = 1;
				break;
			}
			if (JudgeShortFlash(FatPlayerData.O_ActionName)) {
				MyLog("横刀 --对面闪了");
				//break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			WaitTIme1Tem++;
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

					if (FatPlayerData.O_Dis > 6 && LocalPlayer_Data.MyCurEnergy > 4000) {
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
						振刀逻辑::通用切刀重置蓄力();

						break;
					}

				}
				if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog("左蓄力未命中 --s闪");
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUpAll();
					if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						振刀逻辑::通用切刀重置蓄力();

						break;
					}

				}
				if (振刀逻辑::是否特殊状态()) {
					root_sendKeyUpAll();
					return;
				}
				Sleep(1);
			}
		}
		else if (左蓄力跳出方式 == 1) {
			// 对面振刀 a一下
			root_kmNet_mouse_left(0);
			root_kmNet_mouse_left(1);
			Sleep(10);
			root_kmNet_mouse_left(0);
			振刀逻辑::执行横刀平A僵直抓取();
		}
		else if (左蓄力跳出方式 == 2){
			MyLog(标识, "右键按下");
			root_kmNet_mouse_right(0);
			root_kmNet_mouse_right(1);
			ULONGLONG WaitTIme2 = GetTickCount64() + 1500;
			while (WaitTIme2 > GetTickCount64()) {
				Sleep(1);
				if (横刀曲步状态(LocalPlayer_Data.MyActionName)) {
					MyLog(标识,"曲步状态");
					break;
				}
				if (振刀逻辑::是否特殊状态()) {
					root_sendKeyUpAll();
					return;
				}
			}
		}

	}
	else {
		MyLog(标识, "右蓄力");
		// 右蓄力逻辑 
		ULONGLONG WaitTImea1 = GetTickCount64() + 500;
		while (WaitTImea1 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 20) {
				break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
		while (WaitTIme1 > GetTickCount64()) {
			if (LocalPlayer_Data.MyEndureLevel == 11 ) {
				break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		MyLog(标识, "蓄力释放完毕");
		int timeTemp2 = 0;
		ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
		while (WaitTIme2 > GetTickCount64()) {
			if (timeTemp2 > 10 && FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("横刀蓄力命中");
				root_sendKeyUpAll();

				if (LocalPlayer_Data.MyCurEnergy > 4000) {
					if (!Judgefly(FatPlayerData.O_ActionName) && (!振刀逻辑::是否单刀武器() && (LocalPlayer_Data.Weapon_1 != WeaponType::Blade && LocalPlayer_Data.Weapon_2 != WeaponType::Blade && LocalPlayer_Data.Weapon_1 != WeaponType::saber && LocalPlayer_Data.Weapon_2 != WeaponType::saber))) {
						振刀逻辑::判断断闪蓄();
						break;
					}
					MyLog("横刀蓄力命中 - 进w闪");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					ULONGLONG WaitTIme10 = GetTickCount64() + 1000;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
							MyLog(标识, "长闪判断次数:", changshanTemp);
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
					Sleep(10);
					break;
				}
				else {
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
			if (timeTemp2 > 35 || FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力未命中 --s闪");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
			if (!振刀逻辑::侧键按下或半自动() || 挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName)) {
				root_sendKeyUpAll();
				return;
			}
			timeTemp2++;
			Sleep(1);
		}
	}
}
void 出蓄::双戟出蓄(){
	string 标识 = "双戟出蓄--";
	bool 左蓄力状态 = false;
	if (双戟出左蓄状态(LocalPlayer_Data.MyActionName)) {
		左蓄力状态 = true;
	}
	
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	// 判断左右出蓄

	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	int WaitTIme1Temp = 0;
	if (左蓄力状态)WaitTIme1 += 1000;
	while (WaitTIme1 > GetTickCount64()) {
		if (WaitTIme1Temp > 300 && !振刀逻辑::判断敌方是否受击中()) {
			振刀逻辑::长闪蓄(false, 1);
			return;
		}
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}

	if (左蓄力状态) {
		root_kmNet_mouse_right(0);
		root_kmNet_mouse_right(1);
		Sleep(20);
		root_kmNet_mouse_right(0);

		ULONGLONG WaitTImez1 = GetTickCount64() + 500;
		while (WaitTImez1 > GetTickCount64()) {
			if (!振刀逻辑::判断敌方是否受击中()) {
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 12) {
				MyLog(标识, "追击蓄力中");
				break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		ULONGLONG WaitTImez2 = GetTickCount64() + 1500;
		while (WaitTImez2 > GetTickCount64()) {
			if (!振刀逻辑::判断敌方是否受击中()) {
				root_sendKeyUpAll();
				return;
			}
			if (LocalPlayer_Data.MyRangeReactionType == 10) {
				MyLog(标识, "追击蓄力完毕");
				break;
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			Sleep(1);
		}
		振刀逻辑::长闪蓄(false , 0);
		return;

	}
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		int tempTime2 = 0;
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			root_sendKeyUpAll();

			if (LocalPlayer_Data.MyCurEnergy > 4000) {
				
				if (LocalPlayer_Data.Weapon_1 == WeaponType::Katana && LocalPlayer_Data.Weapon_2 == WeaponType::dualhalberd ||
					LocalPlayer_Data.Weapon_2 == WeaponType::Katana && LocalPlayer_Data.Weapon_1 == WeaponType::dualhalberd) {
					振刀逻辑::双刀月影闪切太刀A僵直抓取();
					return;
				}
				if (!Judgefly(FatPlayerData.O_ActionName) && (!振刀逻辑::是否单刀武器() && (LocalPlayer_Data.Weapon_1 != WeaponType::Blade && LocalPlayer_Data.Weapon_2 != WeaponType::Blade && LocalPlayer_Data.Weapon_1 != WeaponType::saber && LocalPlayer_Data.Weapon_2 != WeaponType::saber))) {
					振刀逻辑::判断断闪蓄();
					break;
				}
				else {
					//双刀BB();
					//break;
					int 当前按键 = KEY_W;
					if (rand() % 3 < 1) {
						当前按键 = KEY_S;
					}
					root_sendKeyPress(当前按键);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					root_sendKeyUp(当前按键);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}

			}
			else {
				振刀逻辑::通用切刀重置蓄力();
				break;
			}

		}
		if (tempTime2 > 30) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
					root_sendKeyPress(KEY_S);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					Sleep(1);
					break;
				}
				else {
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::长剑出蓄() {
	string 标识 = "长剑出蓄--";
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife();

	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1000;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	
	长剑上次出蓄力时间 = GetTickCount64();
	if (跳出方式 == 1) {
		int timeTemp = 0;
		ULONGLONG WaitTIme = GetTickCount64() + 1000;
		while (WaitTIme > GetTickCount64()) {
			//wcout << "动作名=" << FatPlayerData.O_ActionName << "-人物状态=" << FatPlayerData.O_ActionName << endl;
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("命中");
				ULONGLONG WaitTIme11 = GetTickCount64() + 400;
				while (WaitTIme11 > GetTickCount64()) {
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						break;
					}
					Sleep(1);
				}

				/*while (GetTickCount64() - 长剑上次出蓄力时间 < 300 ) {
					if (是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
				}*/
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
					while (WaitTIme2 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}
			}
			if (timeTemp > 60 && FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("未命中/拼刀/磐石");
				ULONGLONG WaitTIme11 = GetTickCount64() + 300;
				while (WaitTIme11 > GetTickCount64()) {
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						break;
					}
					Sleep(1);
				}

				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						if (振刀逻辑::是否特殊状态()) {
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
			timeTemp++;
			Sleep(1);
		}
	}
	else {
		root_sendKeyUpAll();
		if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
				if (振刀逻辑::是否特殊状态()) {
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
void 出蓄::双截棍出蓄() {
	string 标识 = "双截棍出蓄--";
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	int WaitTIme1Temp = 0;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}
	MyLog("双截棍蓄力释放完毕");

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("双截棍蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			if (FatPlayerData.O_Dis > 3 && LocalPlayer_Data.MyCurEnergy > 4000) {
				MyLog("精力足够 -进w闪");
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
					if ((changshanTemp > 110 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 180) {
						MyLog(标识, "进入长闪:", changshanTemp);
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
			else {
				MyLog("精力不够 -CC重置");
				振刀逻辑::通用切刀重置蓄力();
				break;
			}

		}
		if (tempTime2 > 30) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("双截棍蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();
					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::链剑出蓄() {
	string 标识 = "链剑出蓄--";
	int 跳出方式 = 0;
	振刀逻辑::CollideKnife();

	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			break;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	int WaitTIme1Temp = 0;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}

	/*ULONGLONG WaitTIme = GetTickCount64() + 600;
	while (WaitTIme > GetTickCount64()) {
		Sleep(1);
		if (LocalPlayer_Data.MyEndureLevel == 11)
		{
			MyLog(标识, "出蓄力");
			跳出方式 = 1;
			break;
		}
		if (振刀逻辑::是否特殊状态())
		{
			root_sendKeyUpAll();
			return;
		}
	}*/
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
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
					MyLog(标识, "精力足够进入W长闪");
					root_sendKeyPress(KEY_W);
					root_sendKeyPress(KEY_LEFTSHIFT);
					int changshanTemp = 0;
					bool 首次循环到 = true;
					ULONGLONG WaitTIme10 = GetTickCount64() + 600;
					while (WaitTIme10 > GetTickCount64()) {
						Sleep(1);
						if (changshanTemp > 80 && 首次循环到) {
							root_kmNet_mouse_right(1);
							首次循环到 = false;
						}
						if ((changshanTemp > 100 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 160) {
							MyLog(标识, "进入长闪:", changshanTemp);
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
					Sleep(10);
					break;
				}
				else {
					振刀逻辑::通用切刀重置蓄力();

					break;
				}
			}
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "未命中/拼刀/磐石");
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						if (振刀逻辑::是否特殊状态()) {
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
		if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
				if (振刀逻辑::是否特殊状态()) {
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
void 出蓄::长棍出蓄() {
	string 标识 = "长棍出蓄--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	
	MouseReleaseNoCheck();
	root_sendKeyUpAll();

	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
	int WaitTIme1Temp = 0;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}
	MyLog("长棍蓄力释放完毕");
	if (长棍立棍(LocalPlayer_Data.MyActionName)) {
		int tempTime2 = 0;
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("立棍命中");
				root_sendKeyUpAll();
				root_kmNet_mouse_right(0);
				root_kmNet_mouse_right(1);
				Sleep(20);
				root_kmNet_mouse_right(0);
				ULONGLONG WaitTImelw1 = GetTickCount64() + 500;
				while (WaitTImelw1 > GetTickCount64()) {
					if (长棍乱点天宫一段(LocalPlayer_Data.MyActionName)) {
						MyLog(标识, "长棍乱点天宫一段");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				ULONGLONG WaitTImelw2 = GetTickCount64() + 500;
				while (WaitTImelw2 > GetTickCount64()) {
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				root_kmNet_mouse_right(0);
				root_kmNet_mouse_right(1);
				Sleep(20);
				root_kmNet_mouse_right(0);
				ULONGLONG WaitTImelw3 = GetTickCount64() + 1500;
				while (WaitTImelw3 > GetTickCount64()) {
					if (长棍乱点天宫二段(LocalPlayer_Data.MyActionName)) {
						MyLog(标识, "长棍乱点天宫二段");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				ULONGLONG WaitTImelw4 = GetTickCount64() + 600;
				while (WaitTImelw4 > GetTickCount64()) {
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				root_kmNet_mouse_right(0);
				root_kmNet_mouse_right(1);
				Sleep(20);
				root_kmNet_mouse_right(0);
				ULONGLONG WaitTImelw5 = GetTickCount64() + 500;
				while (WaitTImelw5 > GetTickCount64()) {
					if (长棍乱点天宫三段(LocalPlayer_Data.MyActionName)) {
						MyLog(标识, "长棍乱点天宫三段");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
				while (WaitTIme1 > GetTickCount64()) {
					if (LocalPlayer_Data.MyEndureLevel == 11) {
						MyLog(标识, "长棍乱点天宫完毕");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						break;
					}
					Sleep(1);
				}

				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();
					break;
				}
			}
			if (tempTime2 > 10) {
				if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog("蓄力未命中 ");
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUpAll();
					if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						振刀逻辑::通用切刀重置蓄力();
						break;
					}

				}
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			tempTime2++;
			Sleep(1);
		}
		return;
	}
	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog("长棍蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
				振刀逻辑::通用切刀重置蓄力();
				break;
			}
		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();
					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::长枪出蓄() {
	string 标识 = "长枪出蓄--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	
	MouseReleaseNoCheck();
	root_sendKeyUpAll();

	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 2500;
	int WaitTIme1Temp = 0;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
		Sleep(1);
	}
	MyLog(标识, "蓄力释放完毕");

	if (长枪龙王破(LocalPlayer_Data.MyActionName)) {
		// 进入龙王破
		MyLog(标识, "进入龙王破");
		int tempTime2 = 0;
		ULONGLONG WaitTIme2 = GetTickCount64() + 500;
		while (WaitTIme2 > GetTickCount64()) {
			if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "龙王破命中");
				root_sendKeyUpAll();

				root_kmNet_mouse_right(0);
				root_kmNet_mouse_right(1);
				Sleep(20);
				root_kmNet_mouse_right(0);
				ULONGLONG WaitTImelw1 = GetTickCount64() + 500;
				while (WaitTImelw1 > GetTickCount64()) {
					if (长枪龙王破一段(LocalPlayer_Data.MyActionName)) {
						MyLog(标识, "长枪龙王破一段");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}

				ULONGLONG WaitTImelw2 = GetTickCount64() + 600;
				while (WaitTImelw2 > GetTickCount64()) {
					if (StrStrW(LocalPlayer_Data.MyActionName.data(), L"recover")) {
						MyLog(标识, "长枪龙王破一段结束");
						break;
					}
					if (振刀逻辑::是否特殊状态()) {
						root_sendKeyUpAll();
						return;
					}
					Sleep(1);
				}
				Sleep(1);
				长枪连招::拳切AA跳B();

			}
			if (tempTime2 > 30) {
				if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
					MyLog(标识, "蓄力未命中 ");
					root_sendKeyUp(KEY_S);
					root_sendKeyUp(KEY_LEFTSHIFT);
					root_sendKeyUpAll();
					if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						振刀逻辑::通用切刀重置蓄力();

						break;
					}

				}
			}
			if (振刀逻辑::是否特殊状态()) {
				root_sendKeyUpAll();
				return;
			}
			tempTime2++;
			Sleep(1);
		}
		return;
	}

	int tempTime2 = 0;
	ULONGLONG WaitTIme2 = GetTickCount64() + 500;
	while (WaitTIme2 > GetTickCount64()) {
		if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
			MyLog(标识, "蓄力命中");
			root_sendKeyUp(KEY_C);
			root_sendKeyUp(KEY_W);
			root_sendKeyUp(KEY_LEFTSHIFT);
			root_sendKeyUpAll();

			if (LocalPlayer_Data.MyCurEnergy > 4000) {
				MyLog(标识, "精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 200) {
						MyLog(标识, "进入长闪:", changshanTemp);
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
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog(标识, "精力不够 -CC重置");
				振刀逻辑::通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::斩马出蓄() {
	string 标识 = "斩马出蓄--";
	int 当前武器位置 = LocalPlayer_Data.MyWeaponIndex == 0 ? 1 : 2;
	int witTime = 20;
	if (LocalPlayer_Data.MyBlueTime >= 1.0) {
		witTime += 50;
	}
	MouseReleaseNoCheck();
	root_sendKeyUpAll();
	ULONGLONG WaitTImeb2 = GetTickCount64() + 500;
	while (WaitTImeb2 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 12) {
			MyLog(标识, "出蓄力中");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		Sleep(1);
	}

	ULONGLONG WaitTIme1 = GetTickCount64() + 2500;
	int WaitTIme1Temp = 0;
	while (WaitTIme1 > GetTickCount64()) {
		if (LocalPlayer_Data.MyRangeReactionType == 10) {
			MyLog(标识, "出蓄力完毕");
			break;
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		WaitTIme1Temp++;
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

			if (LocalPlayer_Data.MyCurEnergy > 4000) {
				MyLog("精力足够 -进w闪");
				root_sendKeyPress(KEY_W);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int changshanTemp = 0;
				ULONGLONG WaitTIme10 = GetTickCount64() + 1500;
				while (WaitTIme10 > GetTickCount64()) {
					Sleep(1);
					if ((changshanTemp > 130 && JudgeLongFlash(LocalPlayer_Data.MyActionName)) || changshanTemp > 250) {
						MyLog(标识, "进入长闪:", changshanTemp);
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
				root_sendKeyUp(KEY_W);
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(10);
				break;
			}
			else {
				MyLog("精力不够 -CC重置");
				振刀逻辑::通用切刀重置蓄力();
				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("斩马蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();
					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::拳刃出蓄() {
	string 标识 = "拳刃出蓄--";
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
		if (振刀逻辑::是否特殊状态()) {
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
		if (振刀逻辑::是否特殊状态()) {
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

			if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
				振刀逻辑::通用切刀重置蓄力();

				break;
			}

		}
		if (tempTime2 > 20) {
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog(标识, "蓄力未命中 ");
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				root_sendKeyUpAll();
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}

			}
		}
		if (振刀逻辑::是否特殊状态()) {
			root_sendKeyUpAll();
			return;
		}
		tempTime2++;
		Sleep(1);
	}
}
void 出蓄::扇子出蓄() {
	string 标识 = "扇子出蓄--";
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
		if (振刀逻辑::是否特殊状态()) {
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
		if (振刀逻辑::是否特殊状态()) {
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
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						if (振刀逻辑::是否特殊状态()) {
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
					振刀逻辑::通用切刀重置蓄力();

					break;
				}
			}
			if (FatPlayerData.O_EndureLevel != 6 || 拼刀状态(FatPlayerData.O_ActionName) || JudgePanshi(FatPlayerData.O_ActionName)) {
				MyLog("未命中/拼刀/磐石");
				if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
						if (振刀逻辑::是否特殊状态()) {
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
		if (LocalPlayer_Data.MyCurEnergy > 4000) {
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
				if (振刀逻辑::是否特殊状态()) {
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