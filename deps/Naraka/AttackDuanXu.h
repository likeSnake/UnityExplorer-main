#pragma once
#include "Data.h"
//³¤½£¶ÏÐî
auto SwBlueAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45) {
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					MyLog("³¤½£¿ìËÙÇÐµ¶");
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("³¤½£×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  && obj.O_BlueTime > LocalPlayer_Data.MyBlueTime ? LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
			
	}

}
//Ì«µ¶¶ÏÐî
auto KatanaAttackDuanXu(PlayerData obj)
{
	
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45) {
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					MyLog("Ì«µ¶ÇÐµ¶");
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
			else
			{
				// ×¥ÉÁ×¥Õ¾Á¢
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
					{
						MyLog("Ì«µ¶×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					// ¶Ô·½ÐîÁ¦Ê±¼äÐ¡ÓÚ1.0²¢ÇÒÎÒµÄÐîÁ¦Ê±¼ä´óÓÚ¶Ô·½ÐîÁ¦Ê±¼ä»òÕßÐîÁ¦µ½ÁÙ½çÖµ
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  && obj.O_BlueTime < 0.5f && LocalPlayer_Data.MyBlueTime > obj.O_BlueTime ? LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					// ×¥ÉÁ
					if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
			
		}
	}

}
//À«µ¶¶ÏÐî
auto BladeAttackDuanXu(PlayerData obj)
{
	
	
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade(_|_run_|_sprint_)attack_heavy_01_charge")) )
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("À«µ¶¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else
			{
				if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
				{
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
		}
		
	}

	//×óÐî
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_charge(_|_run_|_sprint_)(02|03|04)")))
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
	//×óÓÒ
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_heavy_03_(pre|charge)")) )
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
	//ÓÒÓÒ
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_heavy_02_(pre|charge)")))
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
}
//³¤Ç¹
auto SpearAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("³¤Ç¹¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.0)
					{
						MyLog("³¤Ç¹×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  && obj.O_BlueTime > LocalPlayer_Data.MyBlueTime ? LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
	
}
//Ø°Ê×
auto DaggeAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("Ø°Ê×¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else {
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.0)
					{
						MyLog("Ø°Ê××¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.20)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
}

//Ë«½Ú¹÷
auto nunchucksAttackDuanXu(PlayerData obj)
{
	
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("Ë«½Ø¹÷¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("Ë«½Ø¹÷×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
}
//Ë«µ¶
auto twinbladesAttackDuanXu(PlayerData obj)
{
	
	//Ë«µ¶×óÐî 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_charge_enter_idle_(01|02)")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE) {
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("Ë«µ¶×óÐî¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else {
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("Ë«µ¶×ó×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45) {
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					MyLog("Ë«µ¶ÇÐµ¶");
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("Ë«µ¶ÓÒ×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}		
	}
}

//Õ¶Âíµ¶
auto SaberAttackDuanXu(PlayerData obj)
{
	
	
	//Õ¾×ß×óÐî
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber(_|_run_|_sprint_)attack_light_01_charge")))
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
	
	//ÓÒÐî(left|right|back|front)
	else if ((regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_enter_idle_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run(_|_start_)(01|05)")) ||
		regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_turn_square_(left|right|back|front)_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run_turn_back_(left|right|back|front)_05")))
		)
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
	//³ãÑæÕ¶
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber(_|_sprint_|_run_)attack_light_soul_01_charge")))
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 75) : FALSE)
		{
			if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.90)
			{
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}

		}
	}
	
}
//³¤¹÷
auto rodAttackDuanXu(PlayerData obj)
{
	//¹÷ 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE) {
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
				MyLog("³¤¹÷¿ìËÙÇÐµ¶");
				Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
			}
			else {

				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.0)
					{
						MyLog("³¤¹÷×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  && obj.O_BlueTime > LocalPlayer_Data.MyBlueTime ? LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
}
//Ë«êª
auto dualhalberdAttackDuanXu(PlayerData obj)
{
	//Ë«êª×óÐî 
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_charge_enter_idle_(01|02)")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
			if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
			{
				if ( Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
				{
					MyLog("Ë«êª×ó×¥°×");
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

				}

			}
			else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
			{
				if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
				{
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

				}
				else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime &&  obj.O_BlueTime > 0.5)
				{
					if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
					{
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
					}
				}

			}
	}
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45) {
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					MyLog("Ë«êªÇÐµ¶");
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("Ë«êªÓÒ×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}
				}
			}	
		}	
	}
}

//ÉÈ×Ó
auto fanAttackDuanXu(PlayerData obj)
{
	
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45 && (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)) {
			MyLog("ÉÈ×Ó¿ìËÙÇÐµ¶");
			Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
		}
		else {
			if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE) {
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.0)
					{
						MyLog("ÉÈ×Ó×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.25)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
	}
}
//ºáµ¶
auto hengdaoAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
		{
			// Èç¹û¶Ô·½ÔÚÐîÁ¦²¢ÇÒ×Ô¼ºÐîÁ¦Ê±¼ä´óÓÚ0.49 Ö´ÐÐÇÐµ¶
			if (Function::LogicKnife::BlueAttackDuanXu_fast && (obj.O_BlueTime > 0 || obj.O_XuListate == 5) && LocalPlayer_Data.MyBlueTime >= 0.45) {
				if (LocalPlayer_Data.Weapon_1 != 0 && LocalPlayer_Data.Weapon_2 != 0)
				{
					MyLog("ºáµ¶ÇÐµ¶");
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
				}
			}
			else
			{
				if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
				{
					if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
					{
						MyLog("ºáµ¶×ó×¥°×");
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

					}

				}
				else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
				{
					if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
					{
						Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

					}
					else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && obj.O_BlueTime > 0.5)
					{
						if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
						{
							Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
						}
					}

				}
			}
		}
			
	}

	
}

//È­Ì×
auto punchAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
			
			if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
			{
				if (Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
				{
					MyLog("È­ÈÐ×ó×¥°×");
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

				}

			}
			else if (LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
			{
				if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.30)
				{
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);
					
				}
				else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime &&  obj.O_BlueTime > 0.5)
				{
					if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
					{
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
					}
				}

			}
	}
}

//·Éµ¶
auto knifeAttackDuanXu(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_(light|heavy)_(|soul_)(01|02|03)_charge")))
	{
		if (obj.O_Dis < 7.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 60.f) : FALSE)
			if ((LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1)
			{
				if ( Function::LogicKnife::BlueZhuaJiangZhi && obj.O_EndureLevel == 5 && /*LocalPlayer_Data.MyCurEnergy > 3800 &&*/ obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 3.5)
				{
					MyLog("·Éµ¶×¥°×");
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

				}

			}
			else if(LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime > 0.5f)
			{
				if (Function::LogicKnife::BlueAttackDuanXu && obj.O_XuListate == 5  ? LocalPlayer_Data.MyBlueTime > obj.O_BlueTime && LocalPlayer_Data.MyBlueTime > 0.5 : LocalPlayer_Data.MyBlueTime > 1.3)
				{
					Õñµ¶Âß¼­::¶ÏÐî(); Sleep(300);

				}
				else if (Function::LogicKnife::BlueAttackdodge && JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 || obj.O_ActionDuration > 0.3f || LocalPlayer_Data.MyBlueTime > obj.O_BlueTime &&  obj.O_BlueTime > 0.5)
				{
					if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 4.5f)
					{
						Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
					}
				}

			}
	}
}

auto BlueAttackDuanXu(PlayerData obj)
{
	switch (LocalPlayer_Data.MyWeaponType)
	{
	case 101:
		SwBlueAttackDuanXu(obj);
		break;
	case 102:
		KatanaAttackDuanXu(obj);
		break;
	case 103:
		BladeAttackDuanXu(obj);
		break;
	case 105:
		SpearAttackDuanXu(obj);
		break;
	case 106:
		DaggeAttackDuanXu(obj);
		break;
	case 116:
		nunchucksAttackDuanXu(obj);
		break;
	case 118:
		twinbladesAttackDuanXu(obj);
		break;
	case 119:
		SaberAttackDuanXu(obj);
		break;
	case 120:
		rodAttackDuanXu(obj);
		break;
	case 121:
		dualhalberdAttackDuanXu(obj);
		break;
	case 122:
		fanAttackDuanXu(obj);
		break;
	case 123:
		hengdaoAttackDuanXu(obj);
		break;
	case 124:
		punchAttackDuanXu(obj);
		break;
	case 127:
		knifeAttackDuanXu(obj);
		break;
	default:
		break;
	}
}