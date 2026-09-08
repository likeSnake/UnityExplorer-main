#pragma once
#include "Data.h"

float ShockMaxTime = 0.73f;

//³¤½£
auto SwBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		
		//³¤½£½£Æø
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_(light|heavy)_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤½£½£Æø + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤½£½£Æø)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//³¤½£·ï»ËÓð×óÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_(light|heavy)_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤½£·ï»ËÓð + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤½£·ï»ËÓð)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//Ì«µ¶
auto KatanaBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//cout << "½øÈëÐîÁ¦1.0" << endl;
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_(light|heavy)_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ì«µ¶°ÙÁÑ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ì«µ¶°ÙÁÑ)
			{
				//cout << "°ÙÁÑ·´Õñµ¶" << endl;
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ì«µ¶ÊÉ»êÕ¶×óÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_(light|heavy)_charge_soul_(01|02|03)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ì«µ¶°ÙÁÑ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ì«µ¶ÊÉ»êÕ¶)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//À«µ¶
auto BladeBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Õ¾×ßÓÒÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade(_|_run_)attack_heavy_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶Õ¾×ßÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶Õ¾×ßÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//»¬²½ÓÒÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade_sprint_attack_heavy_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶»¬²½ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶»¬²½ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//×óÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade_attack_charge(_|_run_)(02|03|04)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			if (LocalPlayer_Data.MyBlueTime > 0.5f && LocalPlayer_Data.MyBlueTime < 0.99f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî1¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî1¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
			else if (LocalPlayer_Data.MyBlueTime > 0.99f && LocalPlayer_Data.MyBlueTime < 0.149f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî2¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî2¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
			else if (LocalPlayer_Data.MyBlueTime > 0.149f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî3¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî3¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
		}
		//Õ¾×ß(°üÀ¨·çÀ×²½)
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade_attack_charge_run_(02|03|04|start|)_(01|soul)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			if (LocalPlayer_Data.MyBlueTime > 0.5f && LocalPlayer_Data.MyBlueTime < 0.99f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî1¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî1¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
			else if (LocalPlayer_Data.MyBlueTime > 0.99f && LocalPlayer_Data.MyBlueTime < 0.149f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî2¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî2¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
			else if (LocalPlayer_Data.MyBlueTime > 0.149f)
			{
				ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
				if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÐî3¶Î + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÐî3¶Î)
				{
					Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
				}
			}
		}
	}
	//×óÓÒ
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade_attack_heavy_03_(pre|charge)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
	{
		if (LocalPlayer_Data.MyBlueTime < 0.49f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÓÒ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÓÒ)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		else
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶×óÓÒÄóÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶×óÓÒ)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
	//ÓÒÓÒ
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_blade_attack_heavy_02_(pre|charge)")))
	{
		ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::À«µ¶ÓÒÓÒ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::À«µ¶ÓÒÓÒ)
		{
			Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
		}
	}
}
//³¤Ç¹
auto SpearBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//³¤Ç¹×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 6.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤Ç¹×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤Ç¹×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//´©ÐÄ½Å×óÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_light_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 6.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤Ç¹×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤Ç¹×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}

		//³¤Ç¹ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 7.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤Ç¹ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤Ç¹ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//´©ÐÄ½ÅÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 7.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤Ç¹ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤Ç¹ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//Ø°Ê×
auto DaggeBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Ø°Ê××óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê××óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê××óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ø°Ê×ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê×ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê×ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ø°Ê×¿ºÁúÓÐ»Ú
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_light_(01|02|03)_charge_soul")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê×¿ºÁúÓÐ»Ú + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê×¿ºÁúÓÐ»Ú)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//Ë«½Ú¹÷
auto nunchucksBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Ë«½Ú¹÷×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«½Ú¹÷×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«½Ú¹÷×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ë«½Ú¹÷ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«½Ú¹÷ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«½Ú¹÷ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ñï±Þ¾¢
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_light_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«½Ú¹÷Ñï±Þ¾¢ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«½Ú¹÷Ñï±Þ¾¢)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//»¢Ð¥ÁúÅØ
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«½Ú¹÷»¢Ð¥ÁúÅØ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«½Ú¹÷»¢Ð¥ÁúÅØ)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//Ë«µ¶
auto twinbladesBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Ë«µ¶×óÐî male_twinblades_attack_charge_enter_idle_02
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_charge_enter_idle_02")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ë«µ¶ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ë«µ¶×ßÐî female_twinblades_attack_light_charge_run_02
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_light_charge_run_02")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ç¬À¤ÈÕÔÂÕ¶
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge_soul")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 6.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶Ç¬À¤ÈÕÔÂÕ¶ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶Ç¬À¤ÈÕÔÂÕ¶)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//¹÷
auto rodBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//¹÷×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_light_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤¹÷Í°¾¢ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤¹÷×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//¹÷ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_heavy_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤¹÷ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤¹÷ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
	//ÌÚÔÆÊ½
	if (LocalPlayer_Data.MyActionName.c_str() == L"male_rod_attack_hold_light_03")
	{
		ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::³¤¹÷ÌÚÔÆÊ½ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::³¤¹÷ÌÚÔÆÊ½)
		{
			Õñµ¶Âß¼­::ÌÚÔÆÊ½B();
			Sleep(100);
		}
	}
}
//Õ¶Âíµ¶
auto SaberBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Õ¾×ß×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber(_|_run_)attack_light_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Õ¶Âíµ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Õ¶Âíµ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//»¬²½×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_sprint_attack_light_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Õ¶Âíµ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Õ¶Âíµ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//ÓÒÐî(left|right|back|front)
		else if ((regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_enter_idle_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_run(_|_start_)(01|05)")) ||
			regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_turn_square_(left|right|back|front)_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_run_turn_back_(left|right|back|front)_05")))
			&& obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Õ¶Âíµ¶ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Õ¶Âíµ¶ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//³ãÑæÕ¶
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber(_|_sprint_|_run_)attack_light_soul_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 6.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Õ¶Âíµ¶³ãÑæÕ¶ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Õ¶Âíµ¶³ãÑæÕ¶)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
	//×óÓÒ
	if ((regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_heavy_03_pre")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_enter_idle_03")) ||
		regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_run_start_01")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_turn_square_(left|right|back|front)_05")) ||
		regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_saber_attack_charge_run_turn_back_(left|right|back|front)_05"))) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.5f)
	{
		ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Õ¶Âíµ¶×óÓÒ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Õ¶Âíµ¶×óÓÒ)
		{
			Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
		}
	}
}
//Ë«êª
auto dualhalberdBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//Ë«êª×óÐî   female_dualhalberd_attack_light_charge_run_02 male_dualhalberd_attack_charge_enter_idle_02 
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_charge_enter_idle_02")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ë«êªÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//Ë«êª×ßÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_light_charge_run_02")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}

		//ÐÎÒâ¹³ 
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_heavy_charge_soul_(01|02|03)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 6.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ë«µ¶Ç¬À¤ÈÕÔÂÕ¶ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ë«µ¶Ç¬À¤ÈÕÔÂÕ¶)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//ÉÈ×Ó
auto fanBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//ÉÈ×Ó×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê××óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê××óÐî)
			{
				Sleep(15);
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//ÉÈ×ÓÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê×ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê×ÓÒÐî)
			{
				Sleep(15);
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//ÉÈ×Ó²øÁú±¼Ò°
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::Ø°Ê×¿ºÁúÓÐ»Ú + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::Ø°Ê×¿ºÁúÓÐ»Ú)
			{
				Sleep(15);
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//ºáµ¶
auto hengdaoBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//cout << "½øÈëÐîÁ¦1.0" << endl;
		//ºáµ¶×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::ºáµ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::ºáµ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//ºáµ¶ÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::ºáµ¶ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::ºáµ¶ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}

		//ºáµ¶¼«¹âËéÔÆÉÁ
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_heavy_charge_soul_01")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime =(Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::ºáµ¶¼«¹âËéÔÆÉÁ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::ºáµ¶¼«¹âËéÔÆÉÁ)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//È­ÈÐ
auto punchBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{
	
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//cout << "½øÈëÐîÁ¦1.0" << endl;
		//È­ÈÐ×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::È­ÈÐ×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::È­ÈÐ×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//È­ÈÐÓÒÐî
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::È­ÈÐÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::È­ÈÐÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}

		//È­ÈÐÌ«¼«ÅªÔÆÊÖ 
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_light_charge_soul_01")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime -mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::È­ÈÐÌ«¼«ÅªÔÆÊÖ + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::È­ÈÐÌ«¼«ÅªÔÆÊÖ)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}
//·Éµ¶
auto knifeBlueAttackRed(PlayerData obj, float Õñµ¶Ê±»ú)
{

	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//cout << "½øÈëÐîÁ¦1.0" << endl;
		//·Éµ¶×óÐî
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_light_(01|02|03)_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 3.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::·Éµ¶×óÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::·Éµ¶×óÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//·Éµ¶ÓÒÐî male_knife_attack_charge_enter_idle_06
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife_attack_charge_enter_idle_(01|06)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::·Éµ¶ÓÒÐî + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::·Éµ¶ÓÒÐî)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//·Éµ¶ÓÒÐî 
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife_attack_charge_sprint_transform_(01|06)")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 4.5f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::·Éµ¶ÕÆÐÄÀ× + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::·Éµ¶ÕÆÐÄÀ×)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
		//·Éµ¶¶ÏÔÂÇ§ÈÐÎè male_knife_sprint_attack_light_soul_01_charge
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_light_soul_01_charge")) && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.f)
		{
			ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (ObjCurTime > Õñµ¶Ê±»ú - BlueAttackFront::·Éµ¶¶ÏÔÂÇ§ÈÐÎè + MySleep && ObjCurTime < ShockMaxTime - MySleep / 2 - BlueAttackFront::·Éµ¶¶ÏÔÂÇ§ÈÐÎè)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
			}
		}
	}
}

void BlueAttackRed(PlayerData obj, float Õñµ¶ºóÒ¡)
{
	//cout << Õñµ¶ºóÒ¡ << endl;
	
	switch (LocalPlayer_Data.MyWeaponType)
	{
	case 101:
		SwBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 102:
		KatanaBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 103:
		BladeBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 105:
		SpearBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 106:
		DaggeBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 116:
		nunchucksBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 118:
		twinbladesBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 119:
		SaberBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 120:
		rodBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 121:
		dualhalberdBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 122:
		fanBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 123:
		hengdaoBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 124:
		punchBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	case 127:
		knifeBlueAttackRed(obj, Õñµ¶ºóÒ¡);
		break;
	default:
		break;
	}
}