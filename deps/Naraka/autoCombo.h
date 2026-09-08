#pragma once
#include "Data.h"
#include "log.h"

// 检查是否执行c切A  aType:1-左a 2-右a
void checkCCutter(int aType, const PlayerData& obj) {
	// 获取两个武器
	int weapon1 = LocalPlayer_Data.Weapon_1;
	int weapon2 = LocalPlayer_Data.Weapon_2;

	MyLog("武器1 ID:", weapon1);
	MyLog("武器2 ID:", weapon2);

	// 把需要处理的武器类型列表写在一起，方便判断
	auto handleWeapon = [&](int weaponType) {
		switch (weaponType)
		{
		case WeaponType::Katana:
		case WeaponType::hengdao:
		case WeaponType::nunchucks:
		{
			if (aType == 1) Sleep(50);
			振刀逻辑::月闪切(LocalPlayer_Data.MyPing);
			Sleep(450);
			return true;
		}
		case WeaponType::Sw:
		case WeaponType::lj:
		{
			if (aType == 1) Sleep(50);
			振刀逻辑::月闪切右A(LocalPlayer_Data.MyPing);
			Sleep(450);
			return true;
		}
		default:
			return false;
		}
		};

	// 依次尝试两个武器
	if (handleWeapon(weapon1) || handleWeapon(weapon2)) {
		return; // 若任意一个武器匹配到了就结束
	}

	// 默认逻辑（未匹配任何武器）
	if (aType == 1) {
		振刀逻辑::钩锁百裂自动钩锁();
		Sleep(800);
	}
	else if (obj.O_ActionDuration >= 0.1f) {
		MouseRightEx(1);
		Sleep(25);
		MouseRightEx(0);
	}
}


auto SwAutoCombo(PlayerData obj)
{
	static ULONG64 heavyTime = 0;
	//右键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_heavy_01")))
	{
		heavyTime = GetTickCount64() + 100;
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			//obj.O_ActionDuration = mem.Read<float>((uintptr_t)obj.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration);
			
			//ActorKit* O_ActorKit = mem.Read<ActorKit*>(reinterpret_cast<uintptr_t>(obj.O_ActorModel) + Offset::ActorModel.ActorKit);

			/*ActorKit* O_ActorKit = obj.O_ActorModel1->GetActorKit();
			MyLog("O_ActorKit地址：", O_ActorKit);
			ActionHitSimulateData* O_HitSimulate = O_ActorKit->GetActionHitSimulateData();
			MyLog("O_HitSimulate地址：", O_HitSimulate);

			float testTime = mem.Read<float>((uintptr_t)O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration);
			MyLog("僵直时间2:", testTime,"-敌人数量", LocalPlayer_Data.敌人数量5M内);
			//wcout << "动作名=" << obj.O_ActionName << endl;
			*/
			/*if (obj.O_ActionDuration > 1.0f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				LOG("执行ccA\n");
				振刀逻辑::cA或长闪A(1);// Sleep(200);
			}
			else*/ //MyLog("僵直时间2:", obj.O_ActionDuration);
			/*if (obj.O_ActionDuration > 1.0f && LocalPlayer_Data.MyCurEnergy >= 4000 && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				MyLog("执行长闪A");
				振刀逻辑::长闪A2(1, 400); Sleep(400);
				
			}
			else */
				
			// 初始化随机种子（建议放在程序启动处调用一次）
			//srand(static_cast<unsigned int>(time(nullptr)));
			/*MyLog("僵直时间2:", obj.O_ActionDuration);
			if (obj.O_ActionDuration > 0.8f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::cA或长闪A(1, 400);
				Sleep(400);
					
			}
			else*/ if (obj.O_ActionDuration > 0.8f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				MyLog("切刀985\n");
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(480); Sleep(800);
				}

			}
				
		}

	
	}

}

auto KatanaAutoCombo(PlayerData obj)
{

	//左键命中 male_katana_attack_light_02
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			// 初始化随机种子（建议放在程序启动处调用一次）
			srand(static_cast<unsigned int>(time(nullptr)));
			//MyLog("僵直时间2:", obj.O_ActionDuration);
			if (obj.O_ActionDuration > 1.0f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				if (LocalPlayer_Data.MyCurEnergy >= 4000) {
					
					int r = rand() % 100; // 0 ~ 99
					if (r < 40) { // 40% 概率执行长闪A
						振刀逻辑::长闪A2(0,350);
						Sleep(400);
					}
					else { // 60% 概率执行 cA或长闪A
						振刀逻辑::cA或长闪A(0, 350);
						//振刀逻辑::长闪A2(0);
						Sleep(400);
					}
				}
				else {
					MyLog("执行ccA");
					振刀逻辑::cA或长闪A(0, 350); Sleep(400);
				}
				
				
			}
			else
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				MyLog("连招");
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(450); Sleep(800);
				}
			}

		}
		
	}


}

auto SpearAutoCombo(PlayerData obj)
{

	//左键命中 male_sw_attack_light_01 male_sw_sprint_attack_light_01
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::升龙(); Sleep(450);
				}
				
			}

		}

	}

}
auto RodAutoCombo(PlayerData obj)
{

	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::拳切百裂(530); Sleep(800);
				/*if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(530); Sleep(800);
				}*/
			}
		
		}
	
	}

}

// 链剑
auto LianJIanAutoCombo(PlayerData obj)
{

	//右键命中  male_chainsword_attack_heavy_01
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_chainsword(_|_run_|_sprint_)attack_heavy_01")))
	{
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			//LOG(to_string(obj.O_ActionDuration).c_str());
			//LOG("\n");

			/*if (obj.O_ActionDuration > 1.0f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				LOG("执行ccA\n");
				振刀逻辑::cA或长闪A(1);// Sleep(200);
			}
			else*/ if (obj.O_ActionDuration > 0.6f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				//LOG("升龙白裂\n");
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(480); Sleep(800);
				}

			}

		}


	}
}

auto NunchucksAutoCombo(PlayerData obj)
{

	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			//MyLog("僵直时间2:", obj.O_ActionDuration);
			if (obj.O_ActionDuration > 0.9f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				MyLog("执行ccA");
				振刀逻辑::cA或长闪A(0, 350); Sleep(400);
			}
			else
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::拳切百裂(450); Sleep(800);
				/*if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(450); Sleep(800);
				}*/
			}
	
		}
	
		
	}
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_heavy_01")))
	{
		// 右键命中
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			//LOG("右A");
			MouseRightEx(1);
			Sleep(25);
			MouseRightEx(0);
		}

	}

}

auto TwinbladesAutoCombo(PlayerData obj)
{
	
	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				checkCCutter(1, obj);
			}
		
		}

	}
	auto start = std::chrono::high_resolution_clock::now();
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_heavy_01")))
	{

		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			MyLog("僵直:" , obj.O_ActionDuration, "-动作名:", obj.O_ActionName.c_str(),"-延迟：", LocalPlayer_Data.MyPing);
			//std::cout << "僵直:" << obj.O_ActionDuration  << "动作名:" << (obj.O_ActionName.c_str()) << endl;
			if (obj.O_ActionDuration > 0.35f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				auto end = std::chrono::high_resolution_clock::now();
				auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				std::cout << "双刀A中判断执行时间: " << duration_ms << " 毫秒" << std::endl;
				checkCCutter(2, obj);
			}
			else if(obj.O_ActionDuration >= 0.1f && !StrStrW(obj.O_ActionName.c_str(), L"parry")){
				//MyLog("双刀A僵直2:", obj.O_ActionDuration);
				MouseRightEx(1);
				Sleep(25);
				MouseRightEx(0);
			}
		
		}
		
	}

}

auto DualhalberdAutoCombo(PlayerData obj)
{

	
	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				checkCCutter(1, obj);
				/*if (LocalPlayer_Data.Weapon_2 == 102 || LocalPlayer_Data.Weapon_2 == 123 || LocalPlayer_Data.Weapon_1 == 102 || LocalPlayer_Data.Weapon_1 == 123 )
				{
					Sleep(40);
					振刀逻辑::月闪切(LocalPlayer_Data.MyPing); Sleep(450);
				}
				else {
					振刀逻辑::钩锁百裂自动钩锁(); Sleep(800);
				}*/
				
				//振刀逻辑::太双切A(); Sleep(450);
			}
			/*else if (obj.O_ActionDuration > 0.1f && !StrStrW(obj.O_ActionName.c_str(), L"parry"))
			{
				振刀逻辑::太双切A(); Sleep(450);
			}*/
		
		}

	}
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_heavy_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.35f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				checkCCutter(2, obj);
			}
			else if (obj.O_ActionDuration >= 0.1f) {
				MouseRightEx(1);
				Sleep(25);
				MouseRightEx(0);
			}
		
		}

	}
}

auto HengdaoAutoCombo(PlayerData obj)
{

	//左键命中 male_hengdao_attack_hold_light_01
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_light_(01|04)")))
	{
		// 初始化随机种子（建议放在程序启动处调用一次）
		srand(static_cast<unsigned int>(time(nullptr)));
		//MyLog("僵直时间2:", obj.O_ActionDuration);
		if (obj.O_ActionDuration > 1.0f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
		{
			if (LocalPlayer_Data.MyCurEnergy >= 4000) {

				int r = rand() % 100; // 0 ~ 99
				if (r < 40) { // 40% 概率执行长闪A
					振刀逻辑::cA或长闪A(0, 350);
					Sleep(400);
				}
				else { // 60% 概率执行 cA或长闪A
					振刀逻辑::cA或长闪A(0, 350);
					//振刀逻辑::长闪A2(0);
					Sleep(400);
				}
			}
			else {
				MyLog("执行ccA");
				振刀逻辑::cA或长闪A(0, 350); Sleep(400);
			}


		}
		else
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				/*if (LocalPlayer_Data.Weapon_2 == 102 || LocalPlayer_Data.Weapon_2 == 123 || LocalPlayer_Data.Weapon_1 == 102 || LocalPlayer_Data.Weapon_1 == 123 && ObjCurTime > 0.2 && !StrStrW(obj.O_ActionName.c_str(), L"parry"))
				{
					振刀逻辑::太双切A太刀(); Sleep(450);
				}else */if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					振刀逻辑::背包切(); Sleep(800);
					//振刀逻辑::拳切百裂(450); Sleep(800);
				}
				else
				{
					振刀逻辑::拳切百裂(450); Sleep(800);
					//振刀逻辑::背包切(); Sleep(800);
					//振刀逻辑::钩锁百裂自动钩锁(); Sleep(800);
				}
			}
			/*else if (LocalPlayer_Data.Weapon_2 == 102 || LocalPlayer_Data.Weapon_2 == 123 || LocalPlayer_Data.Weapon_1 == 102 || LocalPlayer_Data.Weapon_1 == 123 && ObjCurTime > 0.2 && !StrStrW(obj.O_ActionName.c_str(), L"parry"))
			{
				振刀逻辑::太双切A太刀(); Sleep(450);
			}*/
	
		}

	}


}
auto FanAutoCombo(PlayerData obj)
{

	//左键命中  male_fan_sprint_attack_heavy_01_pre
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_(light|heavy)_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{

				振刀逻辑::升龙(); Sleep(850);
			}
		
		}
		

	}
	

}

auto DaggerAutoCombo(PlayerData obj)
{

	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_(light|heavy)_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::升龙(); Sleep(450);
			}

		}

	}


}
auto PunchAutoCombo(PlayerData obj)
{

	//左键命中  male_fan_attack_light_01
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::拳刃升龙右A(); Sleep(450);
			}

		}

	}



}
auto KnifeAutoCombo(PlayerData obj)
{

	//左键命中 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_light_01")))
	{
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (obj.O_Dis < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75.f) : FALSE)
		{
			if (obj.O_ActionDuration > 0.5f && !StrStrW(obj.O_ActionName.c_str(), L"parry") && LocalPlayer_Data.敌人数量5M内 <= 1)
			{
				振刀逻辑::升龙(); Sleep(450);
			}

		}

	}
}

//自动连招
void AutoCombo(PlayerData obj)
{
	switch (LocalPlayer_Data.MyWeaponType)
	{
	case WeaponType::Sw:
		// 长剑
		SwAutoCombo(obj);
		break;
	case WeaponType::Katana:
		// 太刀
		KatanaAutoCombo(obj);
		break;
	case WeaponType::Blade:
		// 阔刀
		break;
	case WeaponType::Spear:
		// 长枪
		SpearAutoCombo(obj);
		break;
	case WeaponType::Dagge:
		// 匕首
		//DaggerAutoCombo(obj);
		break;
	case WeaponType::nunchucks:
		// 双截棍
		NunchucksAutoCombo(obj);
		break;
	case WeaponType::twinblades:
		// 双刀
		TwinbladesAutoCombo(obj);
		break;
	case WeaponType::saber:
		// 斩马刀
		break;
	case WeaponType::rod:
		// 棍子
		RodAutoCombo(obj);
		break;
	case WeaponType::dualhalberd:
		// 双戟
		DualhalberdAutoCombo(obj);
		break;
	case WeaponType::fan:
		// 扇子
		//FanAutoCombo(obj);
		break;
	case WeaponType::hengdao:
		// 横刀
		HengdaoAutoCombo(obj);
		break;
	case WeaponType::punch:
		// 拳刃
		PunchAutoCombo(obj);
		break;
	case WeaponType::Knife:
		// 飞刀
		KnifeAutoCombo(obj);
		break;
	case WeaponType::lj:
		// 链剑
		LianJIanAutoCombo(obj);
		break;
	default:
		break;
	}
}