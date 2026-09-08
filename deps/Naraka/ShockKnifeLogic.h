#pragma once
#include "Data.h"

inline void JudgeActionName(PlayerData obj) {

	// 开始计时
	auto start = std::chrono::high_resolution_clock::now();

	auto in_y_range = [&](float range = 4.5f) {
		float dy = fabs(obj.O_Pos.y - LocalPlayer_Data.MyPos.y);
		return dy < range;
		};
	auto dir_check = [&](float yaw) { return obj.O_Direction <= yaw; };

	auto XZ_ShockKnife = []() { return  (LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49; };
	auto XZ_ShockKnife_3A = []() { return  (LocalPlayer_Data.XuListate == 0 || LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49; };
	auto XZ_ShockKnife2 = []() { return  (LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) ; };
	auto ZL_ShockKnife = []() { return LocalPlayer_Data.MyActionName.find(L"01") != wstring::npos && LocalPlayer_Data.MyEndureLevel == 5; };
	auto XL_CollideKnife = []() { return LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f; };

	// 闪避方式0左右 1前后 2左 3右 4前 5后
	auto check = [&](bool cond, float dis, float yaw, float sleep = 0.f, int levelV = 1/*, int 闪避方式 = 0*/, int 间隔 = 60) {
		间隔 *= 10;
		int 闪避方式 = 0;
		//MyLog("第一个:", cond, "距离,", dis, "敌人距离：", obj.O_Dis, "in_y_range值", in_y_range(), "dir_check值", dir_check(yaw), "对面角度值 = ", obj.O_Direction, "yaw值=", yaw);
		if (cond && obj.O_Dis <= dis && in_y_range() && dir_check(yaw) )
		{
			MyLog("振刀延迟:", sleep,"-振刀方式:",levelV, "-当前状态：", LocalPlayer_Data.XuListate);
			float XL_sleep = sleep - 0.15f;
			float ZL_sleep = sleep - 0.1f;

			if (Function::Flag::数据遍历 && false)
			{
				// 结束计时
				auto end = std::chrono::high_resolution_clock::now();

				// 计算并输出时间
				auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
				auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
				auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				auto duration_s = std::chrono::duration<double>(end - start).count();

				std::cout << "执行时间: " << duration_ns << " 纳秒" << std::endl;
				std::cout << "执行时间: " << duration_us << " 微秒" << std::endl;
				std::cout << "执行时间: " << duration_ms << " 毫秒" << std::endl;
				std::cout << "执行时间: " << duration_s << " 秒" << std::endl;
			}
			

			// static const char* 振刀方式列表[] = { 0"拼刀", 1"站震", 2"跳震", 3"后闪震", 4"闪避" 5"后闪修刀震" };
			switch (levelV)
			{
			case 0:
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				if (XZ_ShockKnife_3A() && obj.ObjCurTime >= sleep)
				{
					MyLog("快速振刀\n");
					振刀逻辑::BlueToRed(); Sleep(间隔);
				}
				else
				{
					MyLog("拼刀\n");
					振刀逻辑::CollideKnife(); Sleep(间隔);
				}
				break;
			case 1:
			{
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				//MyLog("站振\n");
				//振刀逻辑::BlueToRed(); Sleep(间隔);
				if (false) {
					MyLog("振\n");
					振刀逻辑::BlueToRed(); Sleep(间隔);
				}
				else {
					if (XZ_ShockKnife_3A() && obj.ObjCurTime >= sleep)
					{
						MyLog("站振\n");
						振刀逻辑::BlueToRed(); Sleep(间隔);
					}
					else
					{
						if (XL_CollideKnife() && XL_sleep > 0. && obj.ObjCurTime > XL_sleep)
						{
							MyLog("蓄转振\n");
							振刀逻辑::BlueToRed(); Sleep(间隔);
						}
						else if (XL_CollideKnife() && obj.ObjCurTime > sleep)
						{
							{
								MyLog("拼刀\n");
								振刀逻辑::CollideKnife(); Sleep(间隔);
							}
						}

					}
					if (ZL_ShockKnife() && obj.ObjCurTime > ZL_sleep)
					{
						MyLog("补振\n");
						振刀逻辑::BlueToRed(); Sleep(间隔);
					}
				}
				break;
			}

			case 2:
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				if (XZ_ShockKnife() && obj.ObjCurTime >= sleep)
				{
					MyLog("跳振\n");
					振刀逻辑::JumpShock(); Sleep(间隔);
				}
				else
				{
					if (XL_CollideKnife() && XL_sleep > 0. && obj.ObjCurTime > XL_sleep)
					{
						MyLog("蓄转跳振\n");
						振刀逻辑::JumpShock(); Sleep(间隔);
					}
					else if (XL_CollideKnife() && obj.ObjCurTime > sleep)
					{
						{
							MyLog("拼刀\n");
							振刀逻辑::CollideKnife(); Sleep(间隔);
						}
					}

				}
				if (ZL_ShockKnife() && obj.ObjCurTime > ZL_sleep)
				{
					MyLog("补振\n");
					振刀逻辑::JumpShock(); Sleep(间隔);
				}
				break;
			case 3:
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				MyLog("后闪振\n");
				振刀逻辑::强后闪振(); Sleep(间隔);
				break;
			case 4:
			{
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				MyLog("左右闪\n");
				// 随机选择向左或向右闪
				活化闪避 = true;
				int direction2 = rand() % 2;
				if (direction2 == 0) {
					振刀逻辑::ShortDodge(KEY_A);  // 左闪
				}
				else {
					振刀逻辑::ShortDodge(KEY_D);  // 右闪
				}
				Sleep(间隔);
				break;
			}
			case 5:
			{
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				// 后闪修刀震
				MyLog("后闪修刀震\n");
				振刀逻辑::DodgeExXiuBlueToRed(60); Sleep(间隔);
				break;
			}
			case 6:
			{
				// 强行站振
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				{
					if (XZ_ShockKnife_3A() && obj.ObjCurTime >= sleep)
					{
						MyLog("站立状态 强行站振\n");
						振刀逻辑::BlueToRed(); Sleep(间隔);
					}
					else
					{
						if (XL_CollideKnife() && XL_sleep > 0. && obj.ObjCurTime > XL_sleep)
						{
							MyLog("1.0前 强行站振\n");
							振刀逻辑::BlueToRed(); Sleep(间隔);
						}
						else if (XL_CollideKnife() && obj.ObjCurTime > sleep)
						{
							{
								MyLog("1.0后 强行站振\n");
								振刀逻辑::BlueToRed(); Sleep(间隔);
							}
						}

					}
					if (ZL_ShockKnife() && obj.ObjCurTime > ZL_sleep)
					{
						MyLog("强行站振 补振\n");
						振刀逻辑::BlueToRed(); Sleep(间隔);
					}
				}
				break;
			}
			case 99:
			{
				//后长闪
				MyLog("巴掌闪避\n");
				root_sendKeyPress(KEY_S);
				root_sendKeyPress(KEY_LEFTSHIFT);
				int WaitTIme4Temp = 0;
				bool 已经按下 = false;
				ULONGLONG WaitTIme4 = GetTickCount64() + 500;
				while (WaitTIme4 > GetTickCount64()) {
					//MyLog("循环次数:", WaitTIme4Temp);
					/*if (!已经按下 && WaitTIme4Temp > 50) {
						已经按下 = true;
						MyLogTrue("按下左键");
						root_kmNet_mouse_left(1);
					}*/
					//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
					if (JudgeLongFlash(LocalPlayer_Data.MyActionName))
					{
						MyLog("后-长闪");
						break;
					}
					else if (振刀逻辑::是否特殊状态())
					{
						root_sendKeyUpAll();
						break;
					}
					WaitTIme4Temp++;
					Sleep(1);
				}
				活化闪避 = true;
				root_sendKeyUp(KEY_S);
				root_sendKeyUp(KEY_LEFTSHIFT);
				Sleep(间隔);
				break;
			}	
			case 7:
			{
				string 标识 = "捏蓝振--";
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				// 捏蓝1.0前才振
				if (LocalPlayer_Data.MyBlueTime > 0.05 && LocalPlayer_Data.MyBlueTime < 0.5 /*|| ((振刀逻辑::判断鼠标左键是否按下 || 振刀逻辑::判断鼠标右键是否按下) && LocalPlayer_Data.MyBlueTime > 0)*/) {
					MyLog(标识 + "站振，捏蓝时间 = ", LocalPlayer_Data.MyBlueTime);
					振刀逻辑::BlueToRed();
				}
				else {
					if(LocalPlayer_Data.MyBlueTime > 0.5){
						MyLog(标识, "拼刀\n");
						振刀逻辑::CollideKnife(); 
					}
					else {
						活化闪避 = true;
						// 闪避方式0左右 1前后 2左 3右 4前 5后
						MyLog(标识 + "闪避方式 = ", 闪避方式);
						if (闪避方式 == 0) {
							// 随机选择向左或向右闪
							MyLog(标识, "左右闪\n");
							int direction2 = rand() % 2;
							if (direction2 == 0) {
								振刀逻辑::ShortDodge(KEY_A);  // 左闪

							}
							else {
								振刀逻辑::ShortDodge(KEY_D);  // 右闪
							}
						}else if(闪避方式 == 1){

							MyLog(标识, "前后闪\n");
							int direction2 = rand() % 2;
							if (direction2 == 0) {
								振刀逻辑::ShortDodge(KEY_W);  // 前

							}
							else {
								振刀逻辑::ShortDodge(KEY_S);  // 后
							}
						}
						else if (闪避方式 == 2) {
							MyLog(标识, "左闪\n");
							振刀逻辑::ShortDodge(KEY_S);  // 左闪
						}
						else if (闪避方式 == 3) {
							MyLog(标识, "右闪\n");
							振刀逻辑::ShortDodge(KEY_D);  // 右闪
						}
						else if (闪避方式 == 4) {
							MyLog(标识, "前闪\n");
							振刀逻辑::ShortDodge(KEY_W);  // 前
						}
						else if (闪避方式 == 5) {
							MyLog(标识, "后闪\n");
							振刀逻辑::ShortDodge(KEY_S);  // 后
						}
						else {
							if (obj.O_Dis < 1) {
								振刀逻辑::ShortDodge(KEY_S);  // 前闪
							}
							else if (obj.O_Dis > 1 && obj.O_Dis < 2) {
								// 随机选择向左或向右闪
								MyLog(标识, "左右闪\n");
								int direction2 = rand() % 2;
								if (direction2 == 0) {
									振刀逻辑::ShortDodge(KEY_A);  // 左闪

								}
								else {
									振刀逻辑::ShortDodge(KEY_D);  // 右闪
								}
							}

						}
						
					}
					
				}
				Sleep(间隔);
				break;
			}
			case 8:
			{
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				活化闪避 = true;
				// AS闪
				if (obj.ObjCurTime >= sleep)
				{
					MyLog("AS闪\n");
					振刀逻辑::双键闪(KEY_A, KEY_S); Sleep(间隔);
				}
				else
				{
					if ( XL_sleep > 0. && obj.ObjCurTime > XL_sleep)
					{
						MyLog("AS闪\n");
						振刀逻辑::双键闪(KEY_A, KEY_S); Sleep(间隔);
					}
					else if (obj.ObjCurTime > sleep)
					{
						{
							MyLog("AS闪\n");
							振刀逻辑::双键闪(KEY_A, KEY_S); Sleep(间隔);
						}
					}

				}
				if (obj.ObjCurTime > ZL_sleep)
				{
					MyLog("AS闪\n");
					振刀逻辑::双键闪(KEY_A, KEY_S); Sleep(间隔);
				}

				break;
			}
			case 9:
			{
				LocalPlayer_Data.上次振刀时间 = GetTickCount64();
				活化闪避 = true;
				// DS闪
				if (obj.ObjCurTime >= sleep)
				{
					MyLog("DS闪\n");
					振刀逻辑::双键闪(KEY_D, KEY_S); Sleep(间隔);
				}
				else
				{
					if (XL_sleep > 0. && obj.ObjCurTime > XL_sleep)
					{
						MyLog("DS闪\n");
						振刀逻辑::双键闪(KEY_D, KEY_S); Sleep(间隔);
					}
					else if (obj.ObjCurTime > sleep)
					{
						{
							MyLog("DS闪\n");
							振刀逻辑::双键闪(KEY_D, KEY_S); Sleep(间隔);
						}
					}

				}
				if (obj.ObjCurTime > ZL_sleep)
				{
					MyLog("DS闪\n");
					振刀逻辑::双键闪(KEY_D, KEY_S); Sleep(间隔);
				}

				break;
			}
			default:
				break;
			}
		}
		return false;
		};

#define STRW(str) StrStrW(obj.O_ActionName.c_str(), skCrypt(L##str))
	bool isShihunzhan = false;
	switch (obj.O_ActorWeapon) {
	case WeaponType::Node:
		// male_ka_attack_heavy_02_yuren
		if (check(STRW("male_ka_attack_heavy_02_yuren"), 5, 360.f, 0, 99, 1000)) return; /*巴掌*/
		break;
	case WeaponType::lj:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {

			if (check(STRW("male_chainsword_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::链剑.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.左3, 振刀参数::延迟::链剑.左3, 振刀参数::振刀方式::链剑.左3, 振刀参数::闪避::链剑.左3)) return; /*左键3连*/
			if (check(STRW("male_chainsword_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::链剑.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.右3, 振刀参数::延迟::链剑.右3, 振刀参数::振刀方式::链剑.右3, 振刀参数::闪避::链剑.右3)) return; /*右键3连*/
			if (check(STRW("male_chainsword_attack_light_soul_03"), 振刀参数::距离::链剑.苍牙, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.苍牙, 振刀参数::延迟::链剑.苍牙, 振刀参数::振刀方式::链剑.苍牙, 振刀参数::闪避::链剑.苍牙)) return; /*左键苍牙*/
			if (check(STRW("male_chainsword_attack_hold_light_01"), 振刀参数::距离::链剑.左蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.左蓄力, obj.O_Dis <= 4.5f ? 振刀参数::延迟::链剑.左蓄力 : 振刀参数::延迟::链剑.左蓄力 + obj.O_Dis / 50, 振刀参数::振刀方式::链剑.左蓄力1, 振刀参数::闪避::链剑.左蓄力1)) return; /*左键蓄力1*/
			if (check(STRW("male_chainsword_attack_hold_light_02"), 振刀参数::距离::链剑.左蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.左蓄力, obj.O_Dis <= 4.5f ? 振刀参数::延迟::链剑.左蓄力 : 振刀参数::延迟::链剑.左蓄力 + obj.O_Dis / 50, 振刀参数::振刀方式::链剑.左蓄力2, 振刀参数::闪避::链剑.左蓄力2)) return; /*左键蓄力2*/
			if (check(STRW("male_chainsword_attack_hold_heavy_01"), 振刀参数::距离::链剑.右蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.右蓄力, obj.O_Dis <= 4.5f ? 振刀参数::延迟::链剑.右蓄力 : 振刀参数::延迟::链剑.右蓄力 + obj.O_Dis / 500, 振刀参数::振刀方式::链剑.右蓄力1, 振刀参数::闪避::链剑.右蓄力1)) return; /*右键蓄力1*/
			if (check(STRW("male_chainsword_attack_hold_heavy_02"), 振刀参数::距离::链剑.右蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.右蓄力, obj.O_Dis <= 4.5f ? 振刀参数::延迟::链剑.右蓄力 : 振刀参数::延迟::链剑.右蓄力 + obj.O_Dis / 500, 振刀参数::振刀方式::链剑.右蓄力2, 振刀参数::闪避::链剑.右蓄力2)) return; /*右键蓄力2*/
			if (check(STRW("male_chainsword_attack_hold_light_soul_02") || STRW("male_chainsword_attack_hold_light_soul_01"), 振刀参数::距离::链剑.地龙滚堂刹, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.地龙滚堂刹, obj.O_Dis <= 4.5f ? 振刀参数::延迟::链剑.地龙滚堂刹 : 振刀参数::延迟::链剑.地龙滚堂刹 + obj.O_Dis / 80, 振刀参数::振刀方式::链剑.地龙滚堂刹, 振刀参数::闪避::链剑.地龙滚堂刹)) return; /*地龙滚堂刹*/
			if (check(STRW("male_chainsword_attack_hold_heavy_soul_01") || STRW("male_chainsword_attack_hold_heavy_soul_02"), 振刀参数::距离::链剑.火龙卷云, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.火龙卷云, 振刀参数::延迟::链剑.火龙卷云, 振刀参数::振刀方式::链剑.火龙卷云, 振刀参数::闪避::链剑.火龙卷云)) return; /*火龙卷云*/
			//if (check(STRW("male_chainsword_crouch_attack_light_soul_01") || STRW("male_chainsword_crouch_attack_light_soul_02"), 振刀参数::距离::链剑.断罪碎蜂, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.断罪碎蜂, 振刀参数::延迟::链剑.断罪碎蜂, 振刀参数::振刀方式::链剑.断罪碎蜂)) return; /*断罪碎蜂*/
			if (check(STRW("chainsword_crouch_attack_heavy_soul_09"), 振刀参数::距离::长剑.天外飞仙, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.天外飞仙, 振刀参数::延迟::长剑.天外飞仙, 振刀参数::振刀方式::长剑.天外飞仙, 振刀参数::闪避::长剑.天外飞仙)) return; /*天外飞仙*/
			if (check(STRW("male_chainsword_attack_light_09") , 振刀参数::距离::链剑.蓄力追击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::链剑.蓄力追击, 振刀参数::延迟::链剑.蓄力追击, 振刀参数::振刀方式::链剑.蓄力追击, 振刀参数::闪避::链剑.蓄力追击)) return; /*蓄力追击*/
		}
		//if (check(STRW("sw_jump_attack_heavy_soul_01_pre"), obj.O_Dis <= 2.f ? 360.f : 振刀参数::距离::长剑.跳斩, 振刀参数::角度::长剑.跳斩)) return; /*长剑跳斩*/
		break;
	case WeaponType::Sw:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("sw_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长剑.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.左3 , 振刀参数::延迟::长剑.左3, 振刀参数::振刀方式::长剑.左3, 振刀参数::闪避::长剑.左3)) return ; /*左键3连*/
			if (check(STRW("sw_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长剑.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.右3, 振刀参数::延迟::长剑.右3, 振刀参数::振刀方式::长剑.右3, 振刀参数::闪避::长剑.右3)) return ; /*右键3连*/
			if (check(STRW("sw_attack_light_copy"), 振刀参数::距离::长剑.苍牙, obj.O_Dis <= 3.f ? 360.f : 振刀参数::角度::长剑.苍牙, 振刀参数::延迟::长剑.苍牙, 振刀参数::振刀方式::长剑.苍牙, 振刀参数::闪避::长剑.苍牙)) return ; /*左键苍牙*/
			if (check(STRW("sw_attack_hold_light_01"), 振刀参数::距离::长剑.左剑气, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.左剑气, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.左剑气 : 振刀参数::延迟::长剑.左剑气 + obj.O_Dis / 50, 振刀参数::振刀方式::长剑.左剑气1, 振刀参数::闪避::长剑.左剑气1)) return ; /*左键蓄力1*/
			if (check(STRW("sw_attack_hold_light_02"), 振刀参数::距离::长剑.左剑气, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.左剑气, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.左剑气 : 振刀参数::延迟::长剑.左剑气 + obj.O_Dis / 50, 振刀参数::振刀方式::长剑.左剑气2, 振刀参数::闪避::长剑.左剑气2)) return ; /*左键蓄力2*/

			if (check(STRW("sw_attack_hold_heavy_01"), 振刀参数::距离::长剑.右剑气, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.右剑气, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.右剑气 : 振刀参数::延迟::长剑.右剑气 + obj.O_Dis / 50, 振刀参数::振刀方式::长剑.右剑气1, 振刀参数::闪避::长剑.右剑气1)) return ; /*右键蓄力1*/
			if (check(STRW("sw_attack_hold_heavy_02"), 振刀参数::距离::长剑.右剑气, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.右剑气, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.右剑气 : 振刀参数::延迟::长剑.右剑气 + obj.O_Dis / 50, 振刀参数::振刀方式::长剑.右剑气2, 振刀参数::闪避::长剑.右剑气2)) return ; /*右键蓄力2*/

			if (check(STRW("_sw_attack_hold_heavy_soul_01"), 振刀参数::距离::长剑.凤凰羽, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.凤凰羽, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.凤凰羽 : 振刀参数::延迟::长剑.凤凰羽 + obj.O_Dis / 80, 振刀参数::振刀方式::长剑.凤凰羽1, 振刀参数::闪避::长剑.凤凰羽1)) return ; /*凤凰羽*/
			if (check(STRW("_sw_attack_hold_heavy_soul_02"), 振刀参数::距离::长剑.凤凰羽, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.凤凰羽, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长剑.凤凰羽 : 振刀参数::延迟::长剑.凤凰羽 + obj.O_Dis / 80, 振刀参数::振刀方式::长剑.凤凰羽2, 振刀参数::闪避::长剑.凤凰羽2)) return ; /*凤凰羽*/

			if (check(STRW("sw_attack_light_soul_04"), 振刀参数::距离::长剑.七星夺窍, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.七星夺窍, 振刀参数::延迟::长剑.七星夺窍, 振刀参数::振刀方式::长剑.七星夺窍, 振刀参数::闪避::长剑.七星夺窍)) return ; /*七星夺窍*/
			if (check(STRW("sw_hang_wall_attack_light_01_2"), 振刀参数::距离::长剑.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长剑.壁击, 振刀参数::延迟::长剑.壁击, 振刀参数::振刀方式::长剑.壁击, 振刀参数::闪避::长剑.壁击)) return ; /*长剑壁击*/
			if (check(STRW("sw_crouch_attack_heavy_soul_09"), 振刀参数::距离::长剑.天外飞仙, 振刀参数::角度::长剑.天外飞仙, 振刀参数::延迟::长剑.天外飞仙 , 振刀参数::振刀方式::长剑.天外飞仙, 振刀参数::闪避::长剑.天外飞仙)) return; /*天外飞仙*/
		}
		if (check(STRW("sw_jump_attack_heavy_soul_01_pre") || STRW("sw_jump_attack_heavy_soul_01"), obj.O_Dis <= 3.f ? 360.f :振刀参数::距离::长剑.跳斩, 振刀参数::角度::长剑.跳斩, 振刀参数::延迟::长剑.跳斩,振刀参数::振刀方式::长剑.跳斩, 振刀参数::闪避::长剑.跳斩)) return ; /*长剑跳斩*/
		break;
	case WeaponType::Katana:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("katana_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::太刀.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.左3, 振刀参数::延迟::太刀.左3, 振刀参数::振刀方式::太刀.左3, 振刀参数::闪避::太刀.左3)) return ; /*左键3连*/
			if (check(STRW("katana_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::太刀.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.右3, 振刀参数::延迟::太刀.右3, 振刀参数::振刀方式::太刀.右3, 振刀参数::闪避::太刀.右3)) return ; /*右键3连*/
			if (check(STRW("katana_attack_hold_light_01"), 振刀参数::距离::太刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.左蓄, 振刀参数::延迟::太刀.左蓄, 振刀参数::振刀方式::太刀.左蓄1, 振刀参数::闪避::太刀.左蓄1)) return ; /*左键蓄力1*/
			if (check(STRW("katana_attack_hold_light_02"), 振刀参数::距离::太刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.左蓄, 振刀参数::延迟::太刀.左蓄, 振刀参数::振刀方式::太刀.左蓄2, 振刀参数::闪避::太刀.左蓄2)) return ; /*左键蓄力2 */
			if (check(STRW("katana_attack_hold_heavy_01"), 振刀参数::距离::太刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.右蓄, 振刀参数::延迟::太刀.右蓄, 振刀参数::振刀方式::太刀.右蓄1, 振刀参数::闪避::太刀.右蓄1)) return ; /*右键蓄力1*/
			if (check(STRW("katana_attack_hold_heavy_02"), 振刀参数::距离::太刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.右蓄, 振刀参数::延迟::太刀.右蓄, 振刀参数::振刀方式::太刀.右蓄2, 振刀参数::闪避::太刀.右蓄2)) return ; /*右键蓄力2*/
			if (check(STRW("katana_attack_hold_light_soul_03"), 振刀参数::距离::太刀.刹那斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.刹那斩, 振刀参数::延迟::太刀.刹那斩, 振刀参数::振刀方式::太刀.刹那斩1, 振刀参数::闪避::太刀.刹那斩1)) return ; /*左键刹那斩1*/
			if (check(STRW("katana_attack_heavy_soul_03"), 振刀参数::距离::太刀.青鬼, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.青鬼, 振刀参数::延迟::太刀.青鬼, 振刀参数::振刀方式::太刀.青鬼, 振刀参数::闪避::太刀.青鬼)) return ; /*青鬼*/
			if (check(STRW("katana_attack_heavy_09"), 振刀参数::距离::太刀.惊雷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.惊雷十劫, 振刀参数::延迟::太刀.右惊雷, 振刀参数::振刀方式::太刀.惊雷, 振刀参数::闪避::太刀.惊雷)) return ; /*右键惊雷*/
			if (check(STRW("katana_attack_heavy_soul_09"), 振刀参数::距离::太刀.惊雷十劫, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.惊雷十劫, 振刀参数::延迟::太刀.右惊雷, 振刀参数::振刀方式::太刀.惊雷, 振刀参数::闪避::太刀.惊雷)) return ; /*右键惊雷*/
			if (check(STRW("katana_hang_wall_attack_light_01_2"), 振刀参数::距离::太刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.壁击, 振刀参数::延迟::太刀.壁击, 振刀参数::振刀方式::太刀.壁击, 振刀参数::闪避::太刀.壁击)) return ; /*太刀壁击*/
			if (check(STRW("katana_origin_attack_light_01") || STRW("katana_origin_attack_hold_light_01"), 振刀参数::距离::太刀.同源, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.同源, 振刀参数::延迟::太刀.同源, 振刀参数::振刀方式::太刀.同源, 振刀参数::闪避::太刀.同源)) return ; /*太刀同源*/

			//if (check(STRW("charge_soul_01") || STRW("_soul_02"), 振刀参数::距离::太刀.噬魂斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.噬魂斩), 振刀参数::延迟::太刀.噬魂斩, 振刀参数::振刀方式::太刀.噬魂斩1)  return; /*噬魂斩*/

		}
		
		if ((STRW("charge_soul_01") || STRW("_soul_02")) && obj.O_BlueTime > 0.85) {
			isShihunzhan = true;
			//MyLog("满足条件");
		}
		if (check((isShihunzhan), 振刀参数::距离::太刀.噬魂斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::太刀.噬魂斩, 振刀参数::延迟::太刀.噬魂斩, 振刀参数::振刀方式::太刀.噬魂斩1, 振刀参数::闪避::太刀.噬魂斩1)) {
			return; /*噬魂斩*/
		}
		break;
	case WeaponType::Blade:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("attack_light_02") || STRW("attack_light_05"), 振刀参数::距离::阔刀.左左, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左左, 振刀参数::延迟::阔刀.左左, 振刀参数::振刀方式::阔刀.左左, 振刀参数::闪避::阔刀.左左)) return ; /*左键2连*/
			if (check(STRW("heavy_02") || STRW("heavy_04") || STRW("heavy_08"), 振刀参数::距离::阔刀.右右, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右右, 振刀参数::延迟::阔刀.右右, 振刀参数::振刀方式::阔刀.右右, 振刀参数::闪避::阔刀.右右)) return ; /*右键2连*/
			if (check(STRW("attack_heavy_03"), 振刀参数::距离::阔刀.左右, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左右, 振刀参数::延迟::阔刀.左右, 振刀参数::振刀方式::阔刀.左右, 振刀参数::闪避::阔刀.左右)) return ; /*左A接右*/
			if (check(STRW("male_blade_attack_heavy_soul_03"), 振刀参数::距离::阔刀.左右, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左右, 振刀参数::延迟::阔刀.左右, 振刀参数::振刀方式::阔刀.翻江倒海, 振刀参数::闪避::阔刀.翻江倒海)) return ; /*左A接右 `翻江倒海*/
			if (check(STRW("light_06") || STRW("light_07") || STRW("heavy_07"), 振刀参数::距离::阔刀.右左, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右左, 振刀参数::延迟::阔刀.右左, 振刀参数::振刀方式::阔刀.右左, 振刀参数::闪避::阔刀.右左)) return ; /*右A接左*/
			if (check(STRW("copy_07") || STRW("copy_08"), 振刀参数::距离::阔刀.右右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右右蓄, 振刀参数::延迟::阔刀.右右蓄, 1, 振刀参数::闪避::阔刀.右右蓄)) return ; /*右A接右右*/
			if (check(STRW("blade_attack_hold_light_01"), 振刀参数::距离::阔刀.左蓄一段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄一段, 振刀参数::延迟::阔刀.左蓄一段, 振刀参数::振刀方式::阔刀.左蓄一段, 振刀参数::闪避::阔刀.左蓄一段)) return ; /*左键1段蓄力*/
			if (check(STRW("blade_attack_hold_light_02"), 振刀参数::距离::阔刀.左蓄二段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄二段, 振刀参数::延迟::阔刀.左蓄三段, 振刀参数::振刀方式::阔刀.左蓄二段, 振刀参数::闪避::阔刀.左蓄二段)) return ; /*左键2段蓄力转圈*/
			if (check(STRW("blade_attack_hold_light_03"), 振刀参数::距离::阔刀.左蓄三段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄三段, obj.O_Dis <= 5.f ? 0.f : 振刀参数::延迟::阔刀.左蓄三段, 振刀参数::振刀方式::阔刀.左蓄三段, 振刀参数::闪避::阔刀.左蓄三段)) return ; /*左键3段蓄力转圈*/
			if (check(STRW("blade_attack_hold_light_soul_01"), 振刀参数::距离::阔刀.左蓄二段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄一段, 振刀参数::延迟::阔刀.左蓄一段, 振刀参数::振刀方式::阔刀.左蓄一段, 振刀参数::闪避::阔刀.左蓄一段)) return ; /*烈火斩*/
			if (check(STRW("blade_attack_hold_light_copy_01") || STRW("blade_attack_hold_light_copy_02"), 振刀参数::距离::阔刀.左蓄二段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄一段, 振刀参数::延迟::阔刀.左蓄三段, 振刀参数::振刀方式::阔刀.左蓄三段, 振刀参数::闪避::阔刀.左蓄三段)) return ; /*烈火斩转圈*/
			if (check(STRW("blade_attack_heavy_05"), 振刀参数::距离::阔刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右蓄, 振刀参数::延迟::阔刀.右蓄, 振刀参数::振刀方式::阔刀.右蓄1, 振刀参数::闪避::阔刀.右蓄1)) return ;/*右键 原地蓄力|滑步蓄力1*/
			if (check(STRW("blade_attack_heavy_06"), 振刀参数::距离::阔刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右蓄, 振刀参数::延迟::阔刀.右蓄, 振刀参数::振刀方式::阔刀.右蓄2, 振刀参数::闪避::阔刀.右蓄2)) return ;/*右键 原地蓄力|滑步蓄力2*/
			if (check(STRW("blade_attack_hold_light_soul_03"), 振刀参数::距离::阔刀.雷刀, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄一段, obj.O_Dis <= 3.f ? 0.f: 振刀参数::延迟::阔刀.雷刀, 振刀参数::振刀方式::阔刀.雷刀, 振刀参数::闪避::阔刀.雷刀)) return ;/*左键 巽风震雷刀*/
			if (check(STRW("heavy_copy_05"), 振刀参数::距离::阔刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右蓄, 振刀参数::延迟::阔刀.右蓄, 振刀参数::振刀方式::阔刀.右蓄1, 振刀参数::闪避::阔刀.右蓄1)) return ;/*右键 裂空1*/
			if (check(STRW("heavy_copy_06"), 振刀参数::距离::阔刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.右蓄, 振刀参数::延迟::阔刀.右蓄, 振刀参数::振刀方式::阔刀.右蓄2, 振刀参数::闪避::阔刀.右蓄2)) return ;/*右键 裂空2*/
			if (check(STRW("blade_origin_attack_light_01") || STRW("blade_origin_attack_hold_light_01"), 振刀参数::距离::阔刀.左左, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.左蓄一段, 振刀参数::延迟::阔刀.同源, 振刀参数::振刀方式::阔刀.同源, 振刀参数::闪避::阔刀.同源)) return ; /*重刃共鸣*/
			if (check(STRW("blade_hang_wall_attack_light_01_2"), 振刀参数::距离::阔刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::阔刀.壁击, 振刀参数::延迟::阔刀.壁击, 振刀参数::振刀方式::阔刀.壁击, 振刀参数::闪避::阔刀.壁击)) return ; /*阔刀壁击*/

		}
		break;
	
	case WeaponType::Spear:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("spear_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长枪.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.左3, 振刀参数::延迟::长枪.左3, 振刀参数::振刀方式::长枪.左3, 振刀参数::闪避::长枪.左3)) return ; /*左键3连*/
			if (check(STRW("spear_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长枪.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.右3, 振刀参数::延迟::长枪.右3, 振刀参数::振刀方式::长枪.右3, 振刀参数::闪避::长枪.右3)) return ; /*右键3连*/
			if (check(STRW("spear_attack_hold_light_02"), 振刀参数::距离::长枪.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.左蓄, 振刀参数::延迟::长枪.左蓄, 振刀参数::振刀方式::长枪.左蓄, 振刀参数::闪避::长枪.左蓄)) return ; /*左键蓄力*/
			if (check(STRW("spear_attack_hold_heavy_01"), 振刀参数::距离::长枪.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.右蓄, 振刀参数::延迟::长枪.右蓄, 振刀参数::振刀方式::长枪.右蓄, 振刀参数::闪避::长枪.右蓄)) return ; /*右键蓄力*/
			if (check(STRW("spear_attack_hold_heavy_soul_01"), 振刀参数::距离::长枪.大圣游, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.大圣游, 振刀参数::延迟::长枪.大圣游, 振刀参数::振刀方式::长枪.大圣游, 振刀参数::闪避::长枪.大圣游)) return ; /*右键大圣游*/
			if (check(STRW("spear_attack_heavy_soul_03"), 振刀参数::距离::长枪.风卷云残, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.风卷云残, 振刀参数::延迟::长枪.风卷云残, 振刀参数::振刀方式::长枪.风卷云残, 振刀参数::闪避::长枪.风卷云残)) return ; /*右键三连/风云残卷*/
			if (check(STRW("spear_attack_light_soul_03"), 振刀参数::距离::长枪.双环扫, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.双环扫, 振刀参数::延迟::长枪.双环扫, 振刀参数::振刀方式::长枪.双环扫, 振刀参数::闪避::长枪.双环扫)) return ; /*左键三连/双环扫*/
			if (check(STRW("spear_attack_hold_light_03") || STRW("spear_attack_hold_heavy_03"), 振刀参数::距离::长枪.龙王破, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.龙王破, obj.O_Dis <= 4.5f ? 振刀参数::延迟::长枪.龙王破 : 振刀参数::延迟::长枪.龙王破 + obj.O_Dis / 100, 振刀参数::振刀方式::长枪.龙王破, 振刀参数::闪避::长枪.龙王破)) return ; /*龙王破*/
			if (check(STRW("spear_attack_hold_light_soul_01"), 振刀参数::距离::长枪.六合枪, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.六合枪, 振刀参数::延迟::长枪.六合枪, 振刀参数::振刀方式::长枪.左蓄, 振刀参数::闪避::长枪.左蓄)) return ; /*武道·六合枪*/
			if (check(STRW("spear_attack_hold_light_03_soul") || STRW("spear_attack_hold_heavy_03_soul"), 振刀参数::距离::长枪.穿心脚, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.穿心脚, 振刀参数::延迟::长枪.穿心脚, 振刀参数::振刀方式::长枪.穿心脚, 振刀参数::闪避::长枪.穿心脚)) return ; /*风火穿心脚*/
			if (check(STRW("spear_origin_attack_light") || STRW("spear_attack_hold_heavy_03_soul"), 振刀参数::距离::长枪.同源, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.同源, 振刀参数::延迟::长枪.同源, 振刀参数::振刀方式::长枪.同源, 振刀参数::闪避::长枪.同源)) return ; /*长兵器共鸣*/
			if (check(STRW("spear_hang_wall_attack_light_01_2"), 振刀参数::距离::长枪.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长枪.壁击, 振刀参数::延迟::长枪.壁击, 振刀参数::振刀方式::长枪.壁击, 振刀参数::闪避::长枪.壁击)) return ; /*长枪壁击*/
		}
		break;
	case WeaponType::Dagge:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("dagger_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::匕首.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.左3, 振刀参数::延迟::匕首.左3, 振刀参数::振刀方式::匕首.左3, 振刀参数::闪避::匕首.左3)) return ; /*左键3连*/
			if (check(STRW("dagger_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::匕首.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.右3, 振刀参数::延迟::匕首.右3, 振刀参数::振刀方式::匕首.右3, 振刀参数::闪避::匕首.右3)) return ; /*右键3连*/
			if (check(STRW("dagger_attack_hold_light_01"), 振刀参数::距离::匕首.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.左蓄, 振刀参数::延迟::匕首.左蓄, 振刀参数::振刀方式::匕首.左蓄, 振刀参数::闪避::匕首.左蓄)) return ; /*左键蓄力*/
			if (check(STRW("dagger_attack_hold_heavy_02"), 振刀参数::距离::匕首.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.右蓄, 振刀参数::延迟::匕首.右蓄, 振刀参数::振刀方式::匕首.右蓄, 振刀参数::闪避::匕首.右蓄)) return ; /*右键蓄力*/
			if (check(STRW("dagger_flashstep_attack_light_01"), 振刀参数::距离::匕首.鬼反, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.鬼反, 振刀参数::延迟::匕首.鬼反, 振刀参数::振刀方式::匕首.鬼反, 振刀参数::闪避::匕首.鬼反)) return ; /*鬼反断*/
			if (check(STRW("dagger_flashjump_attack_soul_01"), 振刀参数::距离::匕首.鬼哭神嚎, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.鬼哭神嚎, 振刀参数::延迟::匕首.鬼哭神嚎, 振刀参数::振刀方式::匕首.鬼哭神嚎, 振刀参数::闪避::匕首.鬼哭神嚎)) return ; /*鬼哭神嚎*/
			if (check(STRW("dagger_attack_hold_light_soul_02"), 振刀参数::距离::匕首.亢龙有悔, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.亢龙有悔, 振刀参数::延迟::匕首.亢龙有悔, 振刀参数::振刀方式::匕首.亢龙有悔, 振刀参数::闪避::匕首.亢龙有悔)) return ; /*亢龙有悔*/
			if (check(STRW("dagger_attack_hold_heavy_soul_01"), 振刀参数::距离::匕首.鬼刃暗扎, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.鬼刃暗扎, 振刀参数::延迟::匕首.鬼刃暗扎, 振刀参数::振刀方式::匕首.鬼刃暗扎, 振刀参数::闪避::匕首.鬼刃暗扎)) return ; /*鬼刃暗扎*/
			if (check(STRW("dagger_attack_hold_heavy_01"), 振刀参数::距离::匕首.荆轲献匕, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.荆轲献匕, 振刀参数::延迟::匕首.荆轲献匕, 振刀参数::振刀方式::匕首.荆轲献匕, 振刀参数::闪避::匕首.荆轲献匕)) return ; /*荆轲献匕*/
			if (check(STRW("dagger_hang_wall_attack_light_01_2"), 振刀参数::距离::匕首.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.壁击, 振刀参数::延迟::匕首.壁击, 振刀参数::振刀方式::匕首.壁击, 振刀参数::闪避::匕首.壁击)) return ; /*匕首壁击*/
		}
		if (check(STRW("dagger_attack_hold_light_01_pre"), 振刀参数::距离::匕首.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.左蓄, 振刀参数::延迟::匕首.左蓄 + 0.15, 振刀参数::振刀方式::匕首.左蓄, 振刀参数::闪避::匕首.左蓄)) return ; /*左键蓄力*/
		if (check(STRW("dagger_attack_hold_heavy_02_pre"), 振刀参数::距离::匕首.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.右蓄, 振刀参数::延迟::匕首.右蓄 + 0.15, 振刀参数::振刀方式::匕首.右蓄, 振刀参数::闪避::匕首.右蓄)) return ; /*右键蓄力*/
		if (check(STRW("dagger_flashstep_attack_light_01_pre"), 振刀参数::距离::匕首.鬼反, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::匕首.鬼反, 振刀参数::延迟::匕首.鬼反 + 0.15, 振刀参数::振刀方式::匕首.鬼反, 振刀参数::闪避::匕首.鬼反)) return ; /*鬼反断*/
		break;
	case WeaponType::nunchucks:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("nunchucks_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::双截棍.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.左右3,振刀参数::延迟::双截棍.左右3, 振刀参数::振刀方式::双截棍.左右3, 振刀参数::闪避::双截棍.左右3)) return ; /*左键3连*/
			if (check(STRW("nunchucks_attack_hold_light_01"), 振刀参数::距离::双截棍.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.左蓄, 振刀参数::延迟::双截棍.左蓄, 振刀参数::振刀方式::双截棍.左蓄, 振刀参数::闪避::双截棍.左蓄)) return; /*左键蓄力*/
			if (check(STRW("male_nunchucks_attack_heavy_enhance"), 振刀参数::距离::双截棍.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.右蓄, 振刀参数::延迟::双截棍.右蓄, 振刀参数::振刀方式::双截棍.右蓄, 振刀参数::闪避::双截棍.右蓄)) return ; /*右键蓄力*/
			if (check(STRW("nunchucks_attack_hold_heavy_01"), 振刀参数::距离::双截棍.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.右蓄, 振刀参数::延迟::双截棍.右蓄, 振刀参数::振刀方式::双截棍.右蓄, 振刀参数::闪避::双截棍.右蓄)) return ; /*右键龙虎功*/
			if (check(STRW("male_nunchucks_attack_swing_light_01_soul"), 振刀参数::距离::双截棍.横栏, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.横栏, 振刀参数::延迟::双截棍.横栏, 振刀参数::振刀方式::双截棍.横栏, 振刀参数::闪避::双截棍.横栏)) return ; /*横栏左键机触反击*/
			if (check(STRW("nunchucks_attack_light_03_soul_01"), 振刀参数::距离::双截棍.飞踢, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.飞踢, 振刀参数::延迟::双截棍.飞踢, 振刀参数::振刀方式::双截棍.飞踢, 振刀参数::闪避::双截棍.飞踢)) return ; /*飞踢*/
			if (check(STRW("nunchucks_attack_hold_light_soul"), 振刀参数::距离::双截棍.扬鞭劲, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.扬鞭劲, 振刀参数::延迟::双截棍.扬鞭劲, 振刀参数::振刀方式::双截棍.扬鞭劲, 振刀参数::闪避::双截棍.扬鞭劲)) return ; /*扬鞭劲*/
			if (check(STRW("nunchucks_hang_wall_attack_light_01_2"), 振刀参数::距离::双截棍.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.壁击, 振刀参数::延迟::双截棍.壁击, 振刀参数::振刀方式::双截棍.壁击, 振刀参数::闪避::双截棍.壁击)) return ; /*双节棍壁击*/
			if (check(STRW("male_nunchucks_attack_hold_light_soul_02"), 振刀参数::距离::双截棍.三龙灭阳棍, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.三龙灭阳棍, 振刀参数::延迟::双截棍.三龙灭阳棍, 振刀参数::振刀方式::双截棍.三龙灭阳棍, 振刀参数::闪避::双截棍.三龙灭阳棍)) return ; /*双节棍壁击*/

		}
		if (check(STRW("heavy_soul_01_charge") && obj.O_BlueTime > 1.1, 振刀参数::距离::双截棍.龙虎乱舞, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双截棍.龙虎乱舞, 振刀参数::延迟::双截棍.龙虎乱舞, 振刀参数::振刀方式::双截棍.龙虎乱舞, 振刀参数::闪避::双截棍.龙虎乱舞)) return ; /*预判-右键虎啸龙咆*/
		break;
	case WeaponType::twinblades:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("twinblades_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::双刀.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.左右3, 振刀参数::延迟::双刀.左右3, 振刀参数::振刀方式::双刀.左右3, 振刀参数::闪避::双刀.左右3)) return ; /*左键3连*/
			if (check(STRW("twinblades_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::双刀.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.左右3, 振刀参数::延迟::双刀.左右3, 振刀参数::振刀方式::双刀.左右3, 振刀参数::闪避::双刀.左右3)) return ; /*右键3连*/
			if (check(STRW("twinblades_attack_hold_light_01"), 振刀参数::距离::双刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.左蓄, 振刀参数::延迟::双刀.左蓄, 振刀参数::振刀方式::双刀.左右3, 振刀参数::闪避::双刀.左右3)) return ; /*左键蓄力*/
			if (check(STRW("twinblades_attack_hold_light_01_soul"), 振刀参数::距离::双刀.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.铁马残红, 振刀参数::延迟::双刀.铁马残红, 振刀参数::振刀方式::双刀.左右3, 振刀参数::闪避::双刀.左右3)) return ; /*铁马残红*/
			if (check(STRW("twinblades_attack_hold_heavy_01"), 振刀参数::距离::双刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.右蓄, 振刀参数::延迟::双刀.右蓄, 振刀参数::振刀方式::双刀.右蓄1, 振刀参数::闪避::双刀.右蓄1)) return ; /*右键蓄力1*/
			if (check(STRW("twinblades_attack_hold_heavy_02"), 振刀参数::距离::双刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.右蓄, 振刀参数::延迟::双刀.右蓄, 振刀参数::振刀方式::双刀.右蓄2, 振刀参数::闪避::双刀.右蓄2)) return ; /*右键蓄力2*/
			if (check(STRW("twinblades_attack_heavy_04"), 振刀参数::距离::双刀.惊雷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.惊雷, 振刀参数::延迟::双刀.惊雷, 振刀参数::振刀方式::双刀.惊雷, 振刀参数::闪避::双刀.惊雷)) return ; /*右蓄力右A*/
			if (check(STRW("twinblades_attack_light_03_soul"), 振刀参数::距离::双刀.分水斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.分水斩, 振刀参数::延迟::双刀.分水斩, 振刀参数::振刀方式::双刀.分水斩, 振刀参数::闪避::双刀.分水斩)) return ; /*分水斩*/
			if (check(STRW("twinblades_attack_light_soul_09"), 振刀参数::距离::双刀.八斩刀, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.八斩刀, 振刀参数::延迟::双刀.八斩刀, 振刀参数::振刀方式::双刀.八斩刀, 振刀参数::闪避::双刀.八斩刀)) return ; /*武道·八斩刀*/
			if (check(STRW("twinblades_hang_wall_attack_light_01_2"), 振刀参数::距离::双刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.壁击, 振刀参数::延迟::双刀.壁击, 振刀参数::振刀方式::双刀.壁击, 振刀参数::闪避::双刀.壁击)) return ; /*双刀壁击*/
			//male_twinblades_attack_hold_heavy_01_soul
			if (check(STRW("twinblades_attack_hold_heavy_01_soul"), 振刀参数::距离::双刀.乾坤日月斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双刀.乾坤日月斩, 振刀参数::延迟::双刀.乾坤日月斩, 振刀参数::振刀方式::双刀.乾坤日月斩, 振刀参数::闪避::双刀.乾坤日月斩)) return; /*乾坤日月斩*/

		}

		break;
	case WeaponType::rod:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("rod_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长棍.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.左3, 振刀参数::延迟::长棍.左3, 振刀参数::振刀方式::长棍.左3, 振刀参数::闪避::长棍.左3)) return ; /*左键3连*/
			if (check(STRW("rod_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::长棍.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.右3, 振刀参数::延迟::长棍.右3, 振刀参数::振刀方式::长棍.右3, 振刀参数::闪避::长棍.右3)) return ; /*右键3连*/
			if (check(STRW("rod_attack_hold_light_03"), 振刀参数::距离::长棍.腾云式, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.腾云式, 振刀参数::延迟::长棍.腾云式, 振刀参数::振刀方式::长棍.腾云式, 振刀参数::闪避::长棍.腾云式)) return ; /*左键追风棍*/
			if (check(STRW("rod_attack_heavy_05"), 振刀参数::距离::长棍.乱点天宫, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.乱点天宫, 振刀参数::延迟::长棍.乱点天宫, 振刀参数::振刀方式::长棍.乱点天宫, 振刀参数::闪避::长棍.乱点天宫)) return ; /*乱点天宫*/
			if (check(STRW("rod_attack_hold_light_01"), 振刀参数::距离::长棍.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.左蓄, 振刀参数::延迟::长棍.左蓄, 振刀参数::振刀方式::长棍.左蓄, 振刀参数::闪避::长棍.左蓄)) return ; /*绞腿棍*/
			if (check(STRW("rod_attack_hold_heavy_01"), 振刀参数::距离::长棍.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.右蓄, 振刀参数::延迟::长棍.右蓄, 振刀参数::振刀方式::长棍.右蓄, 振刀参数::闪避::长棍.右蓄)) return ; /*翻摔*/
			if (check(STRW("rod_attack_light_soul_03"), 振刀参数::距离::长棍.双环扫, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.双环扫, 振刀参数::延迟::长棍.双环扫, 振刀参数::振刀方式::长棍.双环扫, 振刀参数::闪避::长棍.双环扫)) return ; /*双环扫*/
			if (check(STRW("rod_attack_hold_light_soul_01"), 振刀参数::距离::长棍.桶劲, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.桶劲, 振刀参数::延迟::长棍.桶劲, 振刀参数::振刀方式::长棍.桶劲, 振刀参数::闪避::长棍.桶劲)) return ; /*捅劲*/
			if (check(STRW("rod_attack_hold_light_03_soul_01"), 振刀参数::距离::长棍.镇地撑天, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.镇地撑天, 振刀参数::延迟::长棍.镇地撑天, 振刀参数::振刀方式::长棍.镇地撑天, 振刀参数::闪避::长棍.镇地撑天)) return ; /*定海神针·镇地撑天*/
			if (check(STRW("rod_attack_heavy_soul_05"), 振刀参数::距离::长棍.五情七灭镇, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.五情七灭镇, 振刀参数::延迟::长棍.五情七灭镇, 振刀参数::振刀方式::长棍.五情七灭镇, 振刀参数::闪避::长棍.五情七灭镇)) return ; /*五情七灭阵*/
			if (check(STRW("rod_attack_heavy_soul_03"), 振刀参数::距离::长棍.少林棍, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.少林棍, 振刀参数::延迟::长棍.少林棍, 振刀参数::振刀方式::长棍.少林棍, 振刀参数::闪避::长棍.少林棍)) return ; /*少林棍*/
			if (check(STRW("rod_origin_attack_light") || STRW("rod_origin_attack_hold_light"), 振刀参数::距离::长棍.同源, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.同源, 振刀参数::延迟::长棍.同源, 振刀参数::振刀方式::长棍.同源, 振刀参数::闪避::长棍.同源)) return ; /*长兵器共鸣*/
			if (check(STRW("rod_hang_wall_attack_light_01_2"), 振刀参数::距离::长棍.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::长棍.壁击, 振刀参数::延迟::长棍.壁击, 振刀参数::振刀方式::长棍.壁击, 振刀参数::闪避::长棍.壁击)) return ; /*长棍壁击*/

		}
		break;
	case WeaponType::saber:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("attack_light_02") || STRW("attack_light_05"), 振刀参数::距离::斩马刀.左左, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.左左, 振刀参数::延迟::斩马刀.左左, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*左左*/
			if (check(STRW("attack_heavy_03"), 振刀参数::距离::斩马刀.左右, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.左右, 振刀参数::延迟::斩马刀.左右, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*左右*/
			if (check(STRW("attack_heavy_02") || STRW("attack_heavy_04"), 振刀参数::距离::斩马刀.右右, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.右右, 振刀参数::延迟::斩马刀.右右, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*右右*/
			if (check(STRW("attack_light_06"), 振刀参数::距离::斩马刀.右左, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.右左, 振刀参数::延迟::斩马刀.右左, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*右左*/
			if (check(STRW("attack_light_03"), 振刀参数::距离::斩马刀.柄击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.柄击, 振刀参数::延迟::斩马刀.柄击, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*柄击*/
			if (check(STRW("attack_heavy_09"), 振刀参数::距离::斩马刀.惊雷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.惊雷, 振刀参数::延迟::斩马刀.惊雷, 振刀参数::振刀方式::斩马刀.左左, 振刀参数::闪避::斩马刀.左左)) return ; /*惊雷*/
			if (check(STRW("saber_attack_hold_light_01"), 振刀参数::距离::斩马刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.左蓄, 振刀参数::延迟::斩马刀.左蓄, 振刀参数::振刀方式::斩马刀.左蓄1, 振刀参数::闪避::斩马刀.左蓄1)) return ; /*左键蓄力1*/
			if (check(STRW("saber_attack_hold_light_02"), 振刀参数::距离::斩马刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.左蓄, 振刀参数::延迟::斩马刀.左蓄, 振刀参数::振刀方式::斩马刀.左蓄2, 振刀参数::闪避::斩马刀.左蓄2)) return ; /*左键蓄力2*/
			if (check(STRW("saber_attack_hold_light_03"), 振刀参数::距离::斩马刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.左蓄, 振刀参数::延迟::斩马刀.左蓄, 振刀参数::振刀方式::斩马刀.左蓄3, 振刀参数::闪避::斩马刀.左蓄3)) return ; /*左键蓄力3*/
			if (check(STRW("saber_attack_hold_heavy_01"), 振刀参数::距离::斩马刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.右蓄, 振刀参数::延迟::斩马刀.右蓄, 振刀参数::振刀方式::斩马刀.右蓄1, 振刀参数::闪避::斩马刀.右蓄1)) return ; /*右键蓄力1*/
			if (check(STRW("saber_attack_hold_heavy_02"), 振刀参数::距离::斩马刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.右蓄, 振刀参数::延迟::斩马刀.右蓄, 振刀参数::振刀方式::斩马刀.右蓄2, 振刀参数::闪避::斩马刀.右蓄2)) return ; /*右键蓄力2*/
			if (check(STRW("saber_attack_hold_heavy_03"), 振刀参数::距离::斩马刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.右蓄, 振刀参数::延迟::斩马刀.右蓄, 振刀参数::振刀方式::斩马刀.右蓄3, 振刀参数::闪避::斩马刀.右蓄3)) return ; /*右键蓄力3*/
			if (check(STRW("saber_attack_hold_light_soul"), 振刀参数::距离::斩马刀.炽焰斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.炽焰斩, 振刀参数::延迟::斩马刀.炽焰斩, 振刀参数::振刀方式::斩马刀.炽焰斩, 振刀参数::闪避::斩马刀.炽焰斩)) return ; /*炽焰斩*/
			if (check(STRW("saber_attack_hold_heavy_soul"), 振刀参数::距离::斩马刀.过关斩将, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.过关斩将, 振刀参数::延迟::斩马刀.过关斩将, 振刀参数::振刀方式::斩马刀.过关斩将, 振刀参数::闪避::斩马刀.过关斩将)) return ; /*过关斩将*/
			if (check(STRW("saber_origin_attack_light") || STRW("saber_origin_attack_hold_light"), 振刀参数::距离::斩马刀.同源, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.同源, 振刀参数::延迟::斩马刀.同源, 振刀参数::振刀方式::斩马刀.同源, 振刀参数::闪避::斩马刀.同源)) return ; /*重刃共鸣*/
			if (check(STRW("saber_hang_wall_attack_light_01_2"), 振刀参数::距离::斩马刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.壁击, 振刀参数::延迟::斩马刀.壁击, 振刀参数::振刀方式::斩马刀.壁击, 振刀参数::闪避::斩马刀.壁击)) return ; /*斩马刀壁击*/
			if (check(STRW("saber_attack_heavy_09_soul_01"), 振刀参数::距离::斩马刀.奔雷入阵, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::斩马刀.奔雷入阵, 振刀参数::延迟::斩马刀.奔雷入阵, 振刀参数::振刀方式::斩马刀.奔雷入阵, 振刀参数::闪避::斩马刀.奔雷入阵)) return ; /*奔雷入阵*/

		}
		break;
	case WeaponType::dualhalberd:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("dualhalberd_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::双戟.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.左右3, 振刀参数::延迟::双戟.左右3, 振刀参数::振刀方式::双戟.左3, 振刀参数::闪避::双戟.左3)) return ; /*左键3连*/
			if (check(STRW("dualhalberd_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::双戟.左右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.左右3, 振刀参数::延迟::双戟.左右3, 振刀参数::振刀方式::双戟.右3, 振刀参数::闪避::双戟.右3)) return ; /*右键3连*/
			if (check(STRW("dualhalberd_attack_hold_light_01"), 振刀参数::距离::双戟.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.左蓄, 振刀参数::延迟::双戟.左蓄, 振刀参数::振刀方式::双戟.左蓄, 振刀参数::闪避::双戟.左蓄)) return ; /*左键蓄力*/
			if (check(STRW("dualhalberd_attack_hold_heavy_01"), 振刀参数::距离::双戟.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.右蓄, 振刀参数::延迟::双戟.右蓄, 振刀参数::振刀方式::双戟.右蓄1, 振刀参数::闪避::双戟.右蓄1)) return ; /*右键蓄力1*/
			if (check(STRW("dualhalberd_attack_hold_heavy_02"), 振刀参数::距离::双戟.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.右蓄, 振刀参数::延迟::双戟.右蓄, 振刀参数::振刀方式::双戟.右蓄2, 振刀参数::闪避::双戟.右蓄2)) return ; /*右键蓄力2*/
			if (check(STRW("dualhalberd_attack_light_09"), 振刀参数::距离::双戟.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.右蓄, 振刀参数::延迟::双戟.右蓄, 振刀参数::振刀方式::双戟.惊雷, 振刀参数::闪避::双戟.惊雷)) return ; /*右蓄接左A*/
			if (check(STRW("dualhalberd_attack_light_08"), 振刀参数::距离::双戟.勾旋斩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.勾旋斩, 振刀参数::延迟::双戟.勾旋斩, 振刀参数::振刀方式::双戟.勾旋斩, 振刀参数::闪避::双戟.勾旋斩)) return ; /*勾旋斩*/
			if (check(STRW("dualhalberd_attack_heavy_soul_09"), 振刀参数::距离::双戟.探海蛟, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.探海蛟, 振刀参数::延迟::双戟.探海蛟, 振刀参数::振刀方式::双戟.探海蛟, 振刀参数::闪避::双戟.探海蛟)) return ; /*蛟龙入海*/
			if (check(STRW("dualhalberd_attack_hold_light_soul_02"), 振刀参数::距离::双戟.形意钩, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.形意钩, 振刀参数::延迟::双戟.形意钩, 振刀参数::振刀方式::双戟.形意钩, 振刀参数::闪避::双戟.形意钩)) return ; /*武道·形意钩*/
			if (check(STRW("dualhalberd_attack_hold_light_01_soul_copy") || STRW("dualhalberd_attack_hold_light_02_soul_copy"), 振刀参数::距离::双戟.战龙在天, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.战龙在天, 振刀参数::延迟::双戟.战龙在天, 振刀参数::振刀方式::双戟.战龙在天, 振刀参数::闪避::双戟.战龙在天)) return ; /*战龙再天*/
			if (check(STRW("dualhalberd_hang_wall_attack_light_01_2"), 振刀参数::距离::双戟.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::双戟.壁击, 振刀参数::延迟::双戟.壁击, 振刀参数::振刀方式::双戟.壁击, 振刀参数::闪避::双戟.壁击)) return ; /*双戢壁击*/
		}
		break;
	case WeaponType::fan:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("fan_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::扇子.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.左3, 振刀参数::延迟::扇子.左3, 振刀参数::振刀方式::扇子.左3, 振刀参数::闪避::扇子.左3)) return ; /*左键3连*/
			if (check(STRW("fan_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::扇子.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.右3, 振刀参数::延迟::扇子.右3, 振刀参数::振刀方式::扇子.右3, 振刀参数::闪避::扇子.右3)) return ; /*右键3连*/
			if (check(STRW("fan_attack_hold_light_01"), 振刀参数::距离::扇子.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.左蓄, 振刀参数::延迟::扇子.左蓄, 振刀参数::振刀方式::扇子.左蓄, 振刀参数::闪避::扇子.左蓄)) return ; /*左键蓄力*/
			if (check(STRW("fan_attack_hold_heavy_01"), 振刀参数::距离::扇子.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.右蓄, 振刀参数::延迟::扇子.右蓄, 振刀参数::振刀方式::扇子.右蓄, 振刀参数::闪避::扇子.右蓄)) return ; /*右键蓄力*/
			if (check(STRW("fan_attack_light_04"), 振刀参数::距离::扇子.惊雷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.惊雷, 振刀参数::延迟::扇子.惊雷, 振刀参数::振刀方式::扇子.惊雷, 振刀参数::闪避::扇子.惊雷)) return ; /*蓄力接左A*/
			if (check(STRW("fan_attack_hold_light_soul_01"), 振刀参数::距离::扇子.三风摆, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.三风摆, 振刀参数::延迟::扇子.三风摆, 振刀参数::振刀方式::扇子.三风摆, 振刀参数::闪避::扇子.三风摆)) return ; /*三风摆*/
			if (check(STRW("fan_attack_light_soul_03"), 振刀参数::距离::扇子.双开圆, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.双开圆, 振刀参数::延迟::扇子.双开圆, 振刀参数::振刀方式::扇子.左3, 振刀参数::闪避::扇子.左3)) return ; /*双开圆*/
			if (check(STRW("fan_attack_hold_heavy_soul_01"), 振刀参数::距离::扇子.缠龙奔野, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.缠龙奔野, obj.O_Dis <= 4.5f ? 振刀参数::延迟::扇子.缠龙奔野 : 振刀参数::延迟::扇子.缠龙奔野 + obj.O_Dis / 100, 振刀参数::振刀方式::扇子.缠龙奔野, 振刀参数::闪避::扇子.缠龙奔野)) return ; /*缠龙奔野*/
			if (check(STRW("fan_hang_wall_attack_light_01_2"), 振刀参数::距离::扇子.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.壁击, 振刀参数::延迟::扇子.壁击, 振刀参数::振刀方式::扇子.壁击, 振刀参数::闪避::扇子.壁击)) return ; /*扇子壁击*/
			if (check(STRW("fan_flashstep_attack_light_02"), 振刀参数::距离::扇子.鬼反, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.鬼反, 振刀参数::延迟::扇子.鬼反, 振刀参数::振刀方式::扇子.鬼反, 振刀参数::闪避::扇子.鬼反)) return ; /*鬼反*/
			if (check(STRW("fan_flashstep_attack_heavy_soul_01_2"), 振刀参数::距离::扇子.泽风上六, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::扇子.泽风上六, 振刀参数::延迟::扇子.泽风上六, 振刀参数::振刀方式::扇子.泽风上六, 振刀参数::闪避::扇子.泽风上六)) return ; /*泽风上六*/
		}
		break;
	case WeaponType::hengdao:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("hengdao_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::横刀.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.左3, 振刀参数::延迟::横刀.左3, 振刀参数::振刀方式::横刀.左3, 振刀参数::闪避::横刀.左3)) return ; /*左键3连*/
			if (check(STRW("hengdao_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::横刀.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.右3, 振刀参数::延迟::横刀.右3, 振刀参数::振刀方式::横刀.右3, 振刀参数::闪避::横刀.右3)) return ; /*右键3连*/
			if (check(STRW("hengdao_attack_hold_light_01") && Function::Shock::横刀左蓄振刀, 振刀参数::距离::横刀.左蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.左蓄, 振刀参数::延迟::横刀.左蓄, 振刀参数::振刀方式::横刀.左蓄, 振刀参数::闪避::横刀.左蓄) && obj.O_BlueTime > 0.31f) return ; /*左键蓄力*/
			if (check(STRW("hengdao_attack_hold_heavy_01"), 振刀参数::距离::横刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.右蓄, 振刀参数::延迟::横刀.右蓄, 振刀参数::振刀方式::横刀.右蓄1, 振刀参数::闪避::横刀.右蓄1)) return ; /*右键蓄力1*/
			if (check(STRW("hengdao_attack_hold_heavy_02"), 振刀参数::距离::横刀.右蓄, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.右蓄, 振刀参数::延迟::横刀.右蓄, 振刀参数::振刀方式::横刀.右蓄2, 振刀参数::闪避::横刀.右蓄2)) return ; /*右键蓄力2*/
			if (check(STRW("hengdao_attack_hold_light_soul_01") || STRW("hengdao_attack_light_soul_05"), 振刀参数::距离::横刀.破千军, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.破千军, 振刀参数::延迟::横刀.破千军, 振刀参数::振刀方式::横刀.破千军, 振刀参数::闪避::横刀.破千军)) return ; /*破千军*/
			if (check(STRW("hengdao_attack_light_soul_03"), 振刀参数::距离::横刀.苍牙, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.苍牙, 振刀参数::延迟::横刀.苍牙, 振刀参数::振刀方式::横刀.左3, 振刀参数::闪避::横刀.左3)) return ; /*苍牙*/
			if (check(STRW("hengdao_crouch_attack_heavy_soul_01_1"), 振刀参数::距离::横刀.踏空闪, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.踏空闪, 振刀参数::延迟::横刀.踏空闪, 振刀参数::振刀方式::横刀.踏空闪, 振刀参数::闪避::横刀.踏空闪)) return ; /*踏空闪*/
			if (check(STRW("hengdao_attack_hold_heavy_soul_02"), 振刀参数::距离::横刀.极光破云闪, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.极光破云闪, 振刀参数::延迟::横刀.极光破云闪, 振刀参数::振刀方式::横刀.极光破云闪, 振刀参数::闪避::横刀.极光破云闪)) return ; /*极光碎云闪*/
			if (check(STRW("hengdao_hang_wall_attack_light_01_2"), 振刀参数::距离::横刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.壁击, 振刀参数::延迟::横刀.壁击, 振刀参数::振刀方式::横刀.壁击, 振刀参数::闪避::横刀.壁击)) return ; /*横刀壁击*/
			if (check(STRW("hengdao_origin_attack_light_01") || STRW("hengdao_origin_attack_hold_light_01"), 振刀参数::距离::横刀.同源, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.同源, 振刀参数::延迟::横刀.同源, 振刀参数::振刀方式::横刀.同源, 振刀参数::闪避::横刀.同源)) return ; /*横刀同源*/
		
			if (check(STRW("hengdao_attack_heavy_soul_04"), 振刀参数::距离::横刀.乾坤一掷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::横刀.乾坤一掷, 振刀参数::延迟::横刀.乾坤一掷, 振刀参数::振刀方式::横刀.乾坤一掷, 振刀参数::闪避::横刀.乾坤一掷)) return; /*乾坤一掷*/
		}
		break;
	case WeaponType::punch:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("punch_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::拳刃.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.左3, 振刀参数::延迟::拳刃.左3, 振刀参数::振刀方式::拳刃.左3, 振刀参数::闪避::拳刃.左3)) return ; /*左键3连*/
			if (check(STRW("punch_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::拳刃.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.右3, 振刀参数::延迟::拳刃.右3, 振刀参数::振刀方式::拳刃.右3, 振刀参数::闪避::拳刃.右3)) return ; /*右键3连*/
			if (check(STRW("punch_attack_hold_light_02"), 振刀参数::距离::拳刃.左蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.左蓄力, 振刀参数::延迟::拳刃.左蓄力, 振刀参数::振刀方式::拳刃.左蓄力1, 振刀参数::闪避::拳刃.左蓄力1)) return ; /*左键蓄力1*/
			if (check(STRW("punch_attack_hold_light_03"), 振刀参数::距离::拳刃.左蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.左蓄力, 振刀参数::延迟::拳刃.左蓄力, 振刀参数::振刀方式::拳刃.左蓄力2, 振刀参数::闪避::拳刃.左蓄力2)) return ; /*左键蓄力2*/
			if (check(STRW("punch_attack_hold_heavy_01"), 振刀参数::距离::拳刃.右蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.右蓄力, 振刀参数::延迟::拳刃.右蓄力, 振刀参数::振刀方式::拳刃.右蓄力1, 振刀参数::闪避::拳刃.右蓄力1)) return ; /*右键蓄力1段*/
			if (check(STRW("punch_attack_hold_heavy_03"), 振刀参数::距离::拳刃.右蓄力2段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.右蓄力2段, 振刀参数::延迟::拳刃.右蓄力2段, 振刀参数::振刀方式::拳刃.右蓄力2, 振刀参数::闪避::拳刃.右蓄力2)) return ; /*右键蓄力2段*/
			if (check(STRW("punch_attack_hold_heavy_soul_02"), 振刀参数::距离::拳刃.百裂腿, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.百裂腿, 振刀参数::延迟::拳刃.百裂腿, 振刀参数::振刀方式::拳刃.百裂腿, 振刀参数::闪避::拳刃.百裂腿)) return ; /*烈风腿*/
			if (check(STRW("punch_attack_light_soul_03"), 振刀参数::距离::拳刃.苍牙, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.苍牙, 振刀参数::延迟::拳刃.苍牙, 振刀参数::振刀方式::拳刃.苍牙, 振刀参数::闪避::拳刃.苍牙)) return ; /*苍牙*/
			if (check(STRW("punch_crouch_attack_heavy_soul_01"), 振刀参数::距离::拳刃.破空拳, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.破空拳, 振刀参数::延迟::拳刃.破空拳, 振刀参数::振刀方式::拳刃.破空拳, 振刀参数::闪避::拳刃.破空拳)) return ; /*破空拳*/
			if (check(STRW("punch_attack_hold_light_soul_03"), 振刀参数::距离::拳刃.太极弄云手, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.太极弄云手, 振刀参数::延迟::拳刃.太极弄云手, 振刀参数::振刀方式::拳刃.太极弄云手, 振刀参数::闪避::拳刃.太极弄云手)) return ; /*太极弄云手*/
			if (check(STRW("punch_hang_wall_attack_light_01_2"), 振刀参数::距离::拳刃.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::拳刃.壁击, 振刀参数::延迟::拳刃.壁击, 振刀参数::振刀方式::拳刃.壁击, 振刀参数::闪避::拳刃.壁击)) return ; /*壁击*/

		}
		break;
	case WeaponType::Knife:
		if (obj.O_RangeReactionType == 12 || obj.O_ReactionType == 2) {
			if (check(STRW("knife_attack_light_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::飞刀.左3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.左3, 振刀参数::延迟::飞刀.左3, 振刀参数::振刀方式::飞刀.左3, 振刀参数::闪避::飞刀.左3)) return ; /*左键3连*/
			if (check(STRW("knife_attack_heavy_03") && !Function::LogicKnife::NO_Shock_3A, 振刀参数::距离::飞刀.右3, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.右3, 振刀参数::延迟::飞刀.右3, 振刀参数::振刀方式::飞刀.右3, 振刀参数::闪避::飞刀.右3)) return ; /*右键3连*/
			if (check(STRW("knife_attack_hold_light_01_90"), 振刀参数::距离::飞刀.左蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.左蓄力, 振刀参数::延迟::飞刀.左蓄力, 振刀参数::振刀方式::飞刀.左蓄力, 振刀参数::闪避::飞刀.左蓄力)) return ; /*左键蓄力1段*/
			if (check(STRW("knife_attack_hold_light_02_90"), 振刀参数::距离::飞刀.左蓄力2段, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.左蓄力2段, obj.O_Dis <= 4.5f ? 振刀参数::延迟::飞刀.左蓄力2段 : 振刀参数::延迟::飞刀.左蓄力2段 + obj.O_Dis / 150, 振刀参数::振刀方式::飞刀.左蓄力2段, 振刀参数::闪避::飞刀.左蓄力2段)) return ; /*左键蓄力2段*/
			if (check(STRW("knife_attack_hold_heavy_01_90"), 振刀参数::距离::飞刀.右蓄力, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.右蓄力, obj.O_Dis <= 4.5f ? 振刀参数::延迟::飞刀.右蓄力 : 振刀参数::延迟::飞刀.右蓄力 + obj.O_Dis / 80, 振刀参数::振刀方式::飞刀.右蓄力, 振刀参数::闪避::飞刀.右蓄力)) return ; /*右键蓄力*/
			if (check(STRW("knife_attack_hold_heavy_soul_01_90"), 振刀参数::距离::飞刀.掌心雷, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.掌心雷, obj.O_Dis <= 4.5f ? 振刀参数::延迟::飞刀.掌心雷 : 振刀参数::延迟::飞刀.掌心雷 + obj.O_Dis / 100, 振刀参数::振刀方式::飞刀.掌心雷, 振刀参数::闪避::飞刀.掌心雷)) return ; /*掌心雷*/
			if (check(STRW("knife_attack_light_03_soul"), 振刀参数::距离::飞刀.苍牙, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.苍牙, 振刀参数::延迟::飞刀.苍牙, 振刀参数::振刀方式::飞刀.苍牙, 振刀参数::闪避::飞刀.苍牙)) return ; /*苍牙*/
			if (check(STRW("knife_attack_heavy_06"), 振刀参数::距离::飞刀.太极, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.太极, 振刀参数::延迟::飞刀.太极, 振刀参数::振刀方式::飞刀.太极, 振刀参数::闪避::飞刀.太极)) return ; /*太极*/
			if (check(STRW("knife_hang_wall_attack_light_01_2"), 振刀参数::距离::飞刀.壁击, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.壁击, 振刀参数::延迟::飞刀.壁击, 振刀参数::振刀方式::飞刀.壁击, 振刀参数::闪避::飞刀.壁击)) return ; /*壁击*/
			if (check(STRW("knife_attack_hold_light_soul_02_01_90") || STRW("knife_attack_hold_light_soul_02_02_90"), 振刀参数::距离::飞刀.断月千刃舞, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.断月千刃舞, 振刀参数::延迟::飞刀.断月千刃舞, 振刀参数::振刀方式::飞刀.断月千刃舞, 振刀参数::闪避::飞刀.断月千刃舞)) return ; /*断月千刃舞*/
			if (check(STRW("knife_attack_hold_light_01_soul_01"), 振刀参数::距离::飞刀.万点寒梅, obj.O_Dis <= 2.f ? 360.f : 振刀参数::角度::飞刀.万点寒梅, 振刀参数::延迟::飞刀.万点寒梅, 振刀参数::振刀方式::飞刀.万点寒梅, 振刀参数::闪避::飞刀.万点寒梅)) return ; /*万点寒梅*/

		}
		break;
	default:
		break;
	}
	
}

void YpZhenDao(bool zhendao )
{
	if (zhendao)
	{
		if (LocalPlayer_Data.MyActionName.find(L"01") != wstring::npos && LocalPlayer_Data.MyEndureLevel == 5 || (LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5 ) && LocalPlayer_Data.MyBlueTime <= 0.49)
		{
			振刀逻辑::BlueToRed(); Sleep(650);
		}
		else
		{
			if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f && Function::Shock::Flag_CollideKnife)
			{
				振刀逻辑::CollideKnife(); Sleep(300);
			}
			else if (LocalPlayer_Data.MyEndureLevel < 30 && LocalPlayer_Data.MyEndureLevel != 21 && LocalPlayer_Data.MyEndureLevel != 22 && Function::Shock::Flag_ShortDodge)
			{
				振刀逻辑::ShortDodgeEx(); Sleep(500);
			}
		}
	}	
} 