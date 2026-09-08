#pragma once
#include "Data.h"


void ZhenDao(float ObjCurTime, float Shock_Timing = 0.f, bool special = false)
{
	if (LocalPlayer_Data.MyActionName.find(L"01") != wstring::npos && LocalPlayer_Data.MyEndureLevel == 5 || (LocalPlayer_Data.XuListate == 1 || LocalPlayer_Data.XuListate == 5) && LocalPlayer_Data.MyBlueTime <= 0.49)
	{
		if (ObjCurTime >= Shock_Timing)
		{
			Õñµ¶Âß¼­::BlueToRed(); Sleep(400);
		}
	}
	else if (LocalPlayer_Data.MyBlueTime > 0.5f && Function::Shock::Flag_CollideKnife)
	{
		float xuli_Timi = Shock_Timing - 0.15f;
		if (special == true && ObjCurTime >= Shock_Timing)
		{
			Õñµ¶Âß¼­::DodgeExBlueToRed(65); Sleep(500);
		}
		else if (xuli_Timi >= 0 && ObjCurTime >= xuli_Timi)
		{
			Õñµ¶Âß¼­::BlueToRed(); Sleep(400);
		}
		else
		{   
			if (ObjCurTime >= Shock_Timing)
			Õñµ¶Âß¼­::CollideKnife(); Sleep(300);
		}
	}
	else if (LocalPlayer_Data.MyEndureLevel < 30 && LocalPlayer_Data.MyEndureLevel != 21 && LocalPlayer_Data.MyEndureLevel != 22 && Function::Shock::Flag_ShortDodge)
	{
		if (ObjCurTime >= Shock_Timing)
		{
			Õñµ¶Âß¼­::ShortDodgeEx(); Sleep(480);
		}
	}
}



auto PalmShockKnife(PlayerData obj)
{

	if (obj.O_ActionName == L"male_ka_attack_heavy_02_yuren")
	{
		if (obj.O_Dis <= »î»¯²ÎÊý::¾àÀë::ÎºÇáV2 ? obj.O_Direction <= »î»¯²ÎÊý::½Ç¶È::ÎºÇáV1 : FALSE)
		{
			if (obj.ObjCurTime > 0.f)
			{
				Õñµ¶Âß¼­::ShortDodge(KEY_S); Sleep(500);
				
			}

		}
	}
}
//³¤½£Õñµ¶
auto SwShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	// ÌøÕ¶
	if (obj.O_ActionName == L"male_sw_jump_attack_heavy_soul_01_pre" &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.ÌøÕ¶ && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.ÌøÕ¶) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.ÌøÕ¶;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}
	// ×óÐî
	if ((obj.O_ActionName == L"male_sw_attack_hold_light_01" || obj.O_ActionName == L"male_sw_attack_hold_light_02") &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.×ó½£Æø && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.×ó½£Æø /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.×ó½£Æø;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
			
		
	}

	// ÓÒÐî
	if ((obj.O_ActionName == L"male_sw_attack_hold_heavy_01" || obj.O_ActionName == L"male_sw_attack_hold_heavy_02")  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.ÓÒ½£Æø && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.ÓÒ½£Æø /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.ÓÒ½£Æø;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ·ï»ËÓð
	if ((StrStrW(obj.O_ActionName.c_str(), L"soul_01") || StrStrW(obj.O_ActionName.c_str(), L"soul_02"))  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.·ï»ËÓð && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.·ï»ËÓð && GetRangeReactionType(obj.Cache_HitSimulate) == 12) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.ÓÒ½£Æø;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ×ó3
	if (obj.O_ActionName == L"male_sw_attack_light_03"  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.×ó3 && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.×ó3 /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.×ó3;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ÓÒ3
	if (obj.O_ActionName == L"male_sw_attack_heavy_03"  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.ÓÒ3 && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.ÓÒ3 /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.ÓÒ3;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ²ÔÑÀ
	if (obj.O_ActionName == L"male_sw_attack_light_copy_03"  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.²ÔÑÀ && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.²ÔÑÀ /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.²ÔÑÀ;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ÆßÐÇ¶áÇÏ
	if (obj.O_ActionName == L"male_sw_attack_light_soul_04"  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.ÆßÐÇ¶áÇÏ && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.ÆßÐÇ¶áÇÏ && GetRangeReactionType(obj.Cache_HitSimulate) == 12) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.ÆßÐÇ¶áÇÏ;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}

	// ±Ú»÷
	if (obj.O_ActionName == L"male_sw_hang_wall_attack_light_01_2"  &&
		obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤½£.±Ú»÷ && obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤½£.±Ú»÷ /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/) {
		Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤½£.±Ú»÷;
		if (obj.O_Dis <= 4.5f) {
			Shock_Timing += 0.f;
		}
		else if (obj.O_Dis <= 7.5f) {
			Shock_Timing += obj.O_Dis / 100;
		}
		else if (obj.O_Dis <= 10.5f) {
			Shock_Timing += obj.O_Dis / 70;
		}
		else {
			Shock_Timing += obj.O_Dis / 40;
		}
		ZhenDao(obj.ObjCurTime, Shock_Timing);
		
	}
}
//Ì«µ¶Õñµ¶·ÖÖ§
auto KatanaShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	//ÊÉ»êÕ¶ 
	if (StrStrW(obj.O_ActionName.c_str(), L"charge_soul_01"))
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.É²ÄÇÕ¶ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.ÊÉ»êÕ¶ : FALSE)
		{
			
			if (obj.ObjCurTime > 0.85f)
			{
				Õñµ¶Âß¼­::BlueToRed(); Sleep(400);
			}
			
		}
	}
	
	//×óÐî  
	if ((obj.O_ActionName == L"male_katana_attack_hold_light_01" || obj.O_ActionName == L"male_katana_attack_hold_light_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction < Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.×óÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//É²ÄÇÕ¶  
	if (obj.O_ActionName == L"male_katana_attack_hold_light_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.É²ÄÇÕ¶ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.É²ÄÇÕ¶ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÓÒÐî   
	if ((obj.O_ActionName == L"male_katana_attack_hold_heavy_01" || obj.O_ActionName == L"male_katana_attack_hold_heavy_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}
	}


	//×ó3
	if (obj.O_ActionName == L"male_katana_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.×ó3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}
	}
	//ÓÒ3 3035595090
	if (obj.O_ActionName == L"male_katana_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.ÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}
	}
	//²ÔÑÀ 
	if (obj.O_ActionName == L"male_katana_attack_light_05" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.×ó3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.²ÔÑÀ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.²ÔÑÀ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
			
		}
	}
	//ÓÒ¾ªÀ×   561768067
	if ((obj.O_ActionName == L"male_katana_attack_heavy_soul_09") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.¾ªÀ×Ê®½Ù ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.¾ªÀ× : FALSE)
		{

			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.ÓÒ¾ªÀ×;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_katana_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ì«µ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}

	}
	//Í¬Ô´
	if (obj.O_ActionName == L"male_katana_origin_attack_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ì«µ¶.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ì«µ¶.ÓÒÐî : FALSE)
		{
			ZhenDao(obj.ObjCurTime, Shock_Timing);
		}
	}
}

//À«µ¶Õñµ¶·ÖÖ§
auto BladeShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	//×óÐîÒ»¶Î 
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_01") || StrStrW(obj.O_ActionName.c_str(), L"hold_light_soul_01")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÐîÒ»¶Î ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.×óÐîÒ»¶Î : FALSE)
		{	
			auto ObjCurTime = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×óÐîÒ»¶Î;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÁÒ»ðÕ¶2¶Î  
	if ((StrStrW(obj.O_ActionName.c_str(), L"hold_light_02") || StrStrW(obj.O_ActionName.c_str(), L"hold_light_copy_02")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÐîÒ»¶Î ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.×óÐî¶þ¶Î : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×óÐî¶þ¶Î;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÁÒ»ðÕ¶3¶Î  
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_03") || StrStrW(obj.O_ActionName.c_str(), L"hold_light_copy_03")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÐîÒ»¶Î ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.×óÐîÈý¶Î : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×óÐîÈý¶Î;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÓÒÐî 
	if ((StrStrW(obj.O_ActionName.c_str(), L"heavy_05") || StrStrW(obj.O_ActionName.c_str(), L"heavy_05_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_05_3") ||
		StrStrW(obj.O_ActionName.c_str(), L"heavy_06") || StrStrW(obj.O_ActionName.c_str(), L"heavy_06_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_06_3")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒÐî  ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing, Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::À«µ¶ÓÒÐîÊÇ·ñ¶ÌÉÁÕñ);
			
		}
	}
	//ÁÑ¿Õ
	if ((StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_05") || StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_05_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_05_3") ||
		StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_06") || StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_06_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_copy_06_3")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing, Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::À«µ¶ÓÒÐîÊÇ·ñ¶ÌÉÁÕñ);
			
			
		}
	}
	//ÓÒÓÒÐî 3635756764
	if ((StrStrW(obj.O_ActionName.c_str(), L"heavy_07") || StrStrW(obj.O_ActionName.c_str(), L"heavy_08") || StrStrW(obj.O_ActionName.c_str(), L"copy_07") || StrStrW(obj.O_ActionName.c_str(), L"copy_08")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×óÐîÈý¶Î;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}

	//À×µ¶ male_blade_attack_hold_light_soul_03_1
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_03_1") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_03_2") || 
		StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_03_3")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÐîÒ»¶Î ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.À×µ¶ : FALSE)
		{
			if (obj.ObjCurTime > 0.15f)
			{
				Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.À×µ¶;
				if (obj.O_Dis <= 4.5f) {
					Shock_Timing += 0.f;
				}
				else if (obj.O_Dis <= 7.5f) {
					Shock_Timing += obj.O_Dis / 100;
				}
				else if (obj.O_Dis <= 10.5f) {
					Shock_Timing += obj.O_Dis / 70;
				}
				else {
					Shock_Timing += obj.O_Dis / 40;
				}
				ZhenDao(obj.ObjCurTime, Shock_Timing);
				
			}
		}
	}


	//×ó×ó male_blade_attack_light_02
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_light_02") || StrStrW(obj.O_ActionName.c_str(), L"attack_light_05")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×ó×ó ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.×ó×ó : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×ó×ó;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}

	}
	//×óÓÒ
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_heavy_03") && GetRangeReactionType(obj.Cache_HitSimulate) == 12)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÓÒ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.×óÓÒ : FALSE)
		{
			
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.×óÓÒ;
			
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}

	}
	//ÓÒÓÒ 
	if ((StrStrW(obj.O_ActionName.c_str(), L"heavy_02") || StrStrW(obj.O_ActionName.c_str(), L"heavy_04") || StrStrW(obj.O_ActionName.c_str(), L"heavy_08") ||
		StrStrW(obj.O_ActionName.c_str(), L"heavy_08_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_08_3")) )
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒÓÒ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒÓÒ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.ÓÒÓÒ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}

	}
	//ÓÒ×ó male_blade_attack_light_06  heavy_copy_07
	if ((StrStrW(obj.O_ActionName.c_str(), L"light_06") || StrStrW(obj.O_ActionName.c_str(), L"light_07") || StrStrW(obj.O_ActionName.c_str(), L"heavy_07") ||
		StrStrW(obj.O_ActionName.c_str(), L"heavy_07_2") || StrStrW(obj.O_ActionName.c_str(), L"heavy_07_3")) )
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒ×ó ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒ×ó : FALSE)
		{
			
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.ÓÒ×ó;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}


	}

	//À×µ¶ÏÂÅü
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_03_4") )
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.ÓÒÓÒ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.À×µ¶ÏÂÅü : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.À×µ¶ÏÂÅü;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}
	}
	//Í¬Ô´ 
	if ((StrStrW(obj.O_ActionName.c_str(), L"origin_attack_hold_light_01")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.×óÐîÒ»¶Î ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.À×µ¶ÏÂÅü;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//±Ú»÷ 
	if (StrStrW(obj.O_ActionName.c_str(), L"hang_wall_attack_light_01_2") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::À«µ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::À«µ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::À«µ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}

	}

}

//³¤Ç¹Õñµ¶·ÖÖ§
auto SpearShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	//×óÐî
	if (obj.O_ActionName == L"male_spear_attack_hold_light_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing, Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::³¤Ç¹×óÐîÊÇ·ñ¶ÌÉÁÕñ);
			
		}
	}
	//ÁùºÏÇ¹
	if (obj.O_ActionName == L"male_spear_attack_hold_light_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.ÁùºÏÇ¹ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.ÁùºÏÇ¹ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.ÁùºÏÇ¹;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÓÒÐî
	if (obj.O_ActionName == L"male_spear_attack_hold_heavy_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//´óÊ¥ÓÎ
	if (obj.O_ActionName == L"male_spear_attack_hold_heavy_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.´óÊ¥ÓÎ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.´óÊ¥ÓÎ : FALSE)
		{
			
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.´óÊ¥ÓÎ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//·ç¾íÔÆ²Ð 
	if (obj.O_ActionName == L"male_spear_attack_heavy_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=   Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.·ç¾íÔÆ²Ð ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.·ç¾íÔÆ²Ð : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.·ç¾íÔÆ²Ð;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
			
		}
	}
	//ÁúÍõÆÆ   
	if ((obj.O_ActionName == L"male_spear_attack_hold_light_03" || obj.O_ActionName == L"male_spear_attack_hold_heavy_03") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.ÁúÍõÆÆ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.ÁúÍõÆÆ : FALSE)
		{
			
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.ÁúÍõÆÆ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
			
		}
	}
	//´©ÐÄ½Å male_spear_attack_hold_light_03_soul_01 male_spear_attack_hold_light_03_soul_02
	if ((StrStrW(obj.O_ActionName.c_str(), L"light_03_soul_01") || StrStrW(obj.O_ActionName.c_str(), L"light_03_soul_02") || StrStrW(obj.O_ActionName.c_str(), L"heavy_03_soul_01") || StrStrW(obj.O_ActionName.c_str(), L"heavy_03_soul_02")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.´©ÐÄ½Å ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.´©ÐÄ½Å : FALSE)
		{
			
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.´©ÐÄ½Å;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}

	//×ó3
	if (obj.O_ActionName == L"male_spear_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//ÓÒ3
	if (obj.O_ActionName == L"male_spear_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.ÓÒ3 : FALSE)
		{

			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//Í¬Ô´
	if (obj.O_ActionName == L"male_spear_origin_attack_hold_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.×óÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.×óÐî : FALSE)
		{

			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.ÁùºÏÇ¹;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}
	}
	//Ë«»·É¨
	if (obj.O_ActionName == L"male_spear_attack_light_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.·ç¾íÔÆ²Ð ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.Ë«»·É¨ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.Ë«»·É¨;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_spear_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤Ç¹.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤Ç¹.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤Ç¹.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing);
			

		}

	}
}

//Ø°Ê×Õñµ¶·ÖÖ§
auto DaggeShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;

	//¹í·´ 
	if (obj.O_ActionName == L"male_dagger_flashstep_attack_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.¹í·´ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.¹í·´ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.¹í·´;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//×óÐî 
	if (obj.O_ActionName == L"male_dagger_attack_hold_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

			
		}
	}
	//ÓÒÐî 
	if (obj.O_ActionName == L"male_dagger_attack_hold_heavy_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.ÓÒÐî : FALSE)
		{
			
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.ÓÒÐî); 
		}
	}
	//¹íÈÐ°µÔú
	if (obj.O_ActionName == L"male_dagger_attack_hold_heavy_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.¹íÈÐ°µÔú ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.¹íÈÐ°µÔú : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.¹íÈÐ°µÔú;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//¹í¿ÞÉñº¿  3334720388
	if (obj.O_ActionName == L"male_dagger_flashjump_attack_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.¹í¿ÞÉñº¿)
		{
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.¹í¿ÞÉñº¿); 
		}
	}
	//¿ºÁúÓÐ»Ú 
	if (obj.O_ActionName == L"male_dagger_attack_hold_light_soul_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.¿ºÁúÓÐ»Ú ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.¿ºÁúÓÐ»Ú : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.¿ºÁúÓÐ»Ú;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_dagger_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_dagger_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	
	//¾£éðÏ×Ø°
	if (obj.O_ActionName == L"male_dagger_attack_hold_heavy_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.¾£éðÏ×Ø° ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.¾£éðÏ×Ø° : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.¾£éðÏ×Ø°;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_dagger_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ø°Ê×.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ø°Ê×.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ø°Ê×.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}

}

//Ë«½Ú¹÷Õñµ¶·ÖÖ§
auto nunchucksShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;

	//Ñï±Þ¾¢  male_nunchucks_attack_hold_light_soul_01
	if (StrStrW(obj.O_ActionName.c_str(), L"light_soul_01_charge") && obj.O_BlueTime > 0.5f)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.Ñï±Þ¾¢ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.Ñï±Þ¾¢ : FALSE)
		{
			if (LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49)
			{
				if (obj.ObjCurTime > 0.55f)
				{
					Õñµ¶Âß¼­::BlueToRed(); Sleep(400);
				}
			}
			else
			{
				Õñµ¶Âß¼­::LongDodge(KEY_S); Sleep(500);
			}
			

		}
	}
	//Áú»¢ÂÒÎè  
	if (StrStrW(obj.O_ActionName.c_str(), L"heavy_soul_01_charge"))
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.Áú»¢ÂÒÎè ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.Áú»¢ÂÒÎè : FALSE)
		{
			
			if (obj.ObjCurTime > 0.85f)
			{
				Õñµ¶Âß¼­::BlueToRed(); Sleep(400);
			}
			
		}
	}

	//×óÐî 
	if ((obj.O_ActionName == L"male_nunchucks_attack_hold_light_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.×óÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing, Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::Ë«½Ø¹÷×óÐîÊÇ·ñ¶ÌÉÁÕñ); 
		}

	}
	//ÓÒÐî 
	if ((obj.O_ActionName == L"male_nunchucks_attack_heavy_enhance_01" || obj.O_ActionName == L"male_nunchucks_attack_hold_heavy_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	// ºáÀ¸
	if ((obj.O_ActionName == L"male_nunchucks_attack_swing_light_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.ºáÀ¸ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.ºáÀ¸ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.ºáÀ¸;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//×ó3
	if (obj.O_ActionName == L"male_nunchucks_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.×óÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒ3
	if (obj.O_ActionName == L"male_nunchucks_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.×óÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//·ÉÌß
	if (obj.O_ActionName == L"male_nunchucks_attack_light_03_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.·ÉÌß ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.·ÉÌß : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.·ÉÌß;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_nunchucks_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«½Ø¹÷.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«½Ø¹÷.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«½Ø¹÷.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}

	

	
}

//Ë«µ¶Õñµ¶·ÖÖ§
auto twinbladesShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	//×óÐî male_twinblades_attack_light_03_soul
	if (obj.O_ActionName == L"male_twinblades_attack_hold_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒÐî 
	if ((obj.O_ActionName == L"male_twinblades_attack_hold_heavy_01" || obj.O_ActionName == L"male_twinblades_attack_hold_heavy_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÌúÂí²Ðºì
	if (obj.O_ActionName == L"male_twinblades_attack_hold_light_01_soul" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.ÌúÂí²Ðºì ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.ÌúÂí²Ðºì : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.ÌúÂí²Ðºì;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//Ç¬À¤ÈÕÔÂÕ¶
	if (obj.O_ActionName == L"male_twinblades_attack_hold_heavy_01_soul" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.Ç¬À¤ÈÕÔÂÕ¶ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.Ç¬À¤ÈÕÔÂÕ¶ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.Ç¬À¤ÈÕÔÂÕ¶;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//°ËÕ¶µ¶
	if (obj.O_ActionName == L"male_twinblades_attack_light_soul_09" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.°ËÕ¶µ¶ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.°ËÕ¶µ¶ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.°ËÕ¶µ¶;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}

	//×ó3
	if (obj.O_ActionName == L"male_twinblades_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.×óÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒ3
	if (obj.O_ActionName == L"male_twinblades_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.×óÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	//²ÔÑÀ 
	if (obj.O_ActionName == L"male_twinblades_attack_light_03_soul" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.·ÖË®Õ¶ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.·ÖË®Õ¶ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.·ÖË®Õ¶;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}

	//ÐîÁ¦ºó×·»÷   
	if (obj.O_ActionName == L"male_twinblades_attack_heavy_04" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.¾ªÀ× ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.¾ªÀ× : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.¾ªÀ×;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_twinblades_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«µ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«µ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«µ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
}

//³¤¹÷Õñµ¶·ÖÖ§
auto rodShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;

	//×óÐî 
	if (obj.O_ActionName == L"male_rod_attack_hold_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//ÓÒÐî 
	if (obj.O_ActionName == L"male_rod_attack_hold_heavy_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	
	//ÌÚÔÆÊ½ 
	if (obj.O_ActionName == L"male_rod_attack_hold_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÌÚÔÆÊ½ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÌÚÔÆÊ½ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÌÚÔÆÊ½;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÕòµØ³ÅÌì  
	if (obj.O_ActionName == L"male_rod_attack_hold_light_03_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÕòµØ³ÅÌì ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÕòµØ³ÅÌì : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÕòµØ³ÅÌì;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//Í°¾¢
	if (obj.O_ActionName == L"male_rod_attack_hold_light_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.Í°¾¢ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.Í°¾¢ : FALSE)
		{
			
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.Í°¾¢); 
		
		}
	}
	//Í¬Ô´
	if (obj.O_ActionName == L"male_rod_origin_attack_hold_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.Í°¾¢ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.Í°¾¢ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.Í°¾¢;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//ÎåÇéÆßÃðÕò  2947681866
	if (obj.O_ActionName == L"male_rod_attack_heavy_soul_05" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÎåÇéÆßÃðÕò ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÎåÇéÆßÃðÕò : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÎåÇéÆßÃðÕò;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_rod_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_rod_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÉÙÁÖ¹÷
	if (obj.O_ActionName == L"male_rod_attack_heavy_soul_03" )
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÉÙÁÖ¹÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÉÙÁÖ¹÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÉÙÁÖ¹÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÂÒµãÌì¹¬ 1451021845  
	if ((obj.O_ActionName == L"male_rod_attack_heavy_05_copy" || obj.O_ActionName == L"male_rod_attack_heavy_05_01_copy") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.ÂÒµãÌì¹¬ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.ÂÒµãÌì¹¬ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.ÂÒµãÌì¹¬;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}

	//Ë«»·É¨ 
	if (obj.O_ActionName == L"male_rod_attack_light_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.Ë«»·É¨ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.Ë«»·É¨ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.Ë«»·É¨;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

			
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_rod_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::³¤¹÷.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::³¤¹÷.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::³¤¹÷.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
}

//Õ¶Âíµ¶Õñµ¶·ÖÖ§g
auto SaberShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	//×óÐî  
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_01") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_02") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_03")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing, Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::Õ¶Âíµ¶×óÐîÊÇ·ñ³¤ÉÁÕñ); 

			
		}
	}
	//ÓÒÐî 
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_heavy_01") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_heavy_02") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_heavy_03")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	} 
	//³ãÑæÕ¶ 
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_01") || StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_02") ||
		StrStrW(obj.O_ActionName.c_str(), L"attack_hold_light_soul_03")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.³ãÑæÕ¶ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.³ãÑæÕ¶ : FALSE)
		{
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.³ãÑæÕ¶); 
			
		}

	}
	//¹ý¹ØÕ¶½« 
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_hold_heavy_soul_03") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.¹ý¹ØÕ¶½« ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.¹ý¹ØÕ¶½« : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.¹ý¹ØÕ¶½«;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
				
		}

	}

	//×ó×ó 3035595090
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_light_02") || StrStrW(obj.O_ActionName.c_str(), L"attack_light_05")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.×ó×ó ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.×ó×ó : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.×ó×ó;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//×óÓÒ 3249409949
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_heavy_03")) && GetRangeReactionType(obj.Cache_HitSimulate) == 12)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.×óÓÒ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.×óÓÒ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.×óÓÒ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	} 
	//ÓÒÓÒ 3064545035
	if ((StrStrW(obj.O_ActionName.c_str(), L"attack_heavy_02") || StrStrW(obj.O_ActionName.c_str(), L"attack_heavy_04")) /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.ÓÒÓÒ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.ÓÒÓÒ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.ÓÒÓÒ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//ÓÒ×ó 3011659083
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_light_06") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.ÓÒ×ó ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.ÓÒ×ó : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.ÓÒ×ó;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}

	//±ú»÷ 3286806980  
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_light_03") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.±ú»÷ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.±ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.±ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		
		}
	}
	//Í¬Ô´ 
	if (StrStrW(obj.O_ActionName.c_str(), L"origin_attack_hold_light_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.±ú»÷ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.±ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.±ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//±Ú»÷
	if (StrStrW(obj.O_ActionName.c_str(), L"hang_wall_attack_light_01_2") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//¾ªÀ×
	if (StrStrW(obj.O_ActionName.c_str(), L"attack_heavy_09") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Õ¶Âíµ¶.¾ªÀ× ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Õ¶Âíµ¶.¾ªÀ× : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Õ¶Âíµ¶.¾ªÀ×;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}

}	

//Ë«êªÕñµ¶·ÖÖ§g
auto dualhalberdKnife(PlayerData obj)
{ 
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	//¹³¹Ò 
	if ((obj.O_ActionName == L"male_dualhalberd_attack_light_09") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.¹³¹Ò ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.¹³¹Ò : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.¹³¹Ò;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//×óÐî 3905282567
	if (obj.O_ActionName == L"male_dualhalberd_attack_hold_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.×óÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒÐî  
	if ((obj.O_ActionName == L"male_dualhalberd_attack_hold_heavy_01" || obj.O_ActionName == L"male_dualhalberd_attack_hold_heavy_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	//Õ½ÁúÔÚÌì 
	if ((obj.O_ActionName == L"male_dualhalberd_attack_hold_light_01_soul_copy" || obj.O_ActionName == L"male_dualhalberd_attack_hold_light_02_soul_copy") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.Õ½ÁúÔÚÌì ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.Õ½ÁúÔÚÌì : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.Õ½ÁúÔÚÌì;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÐÎÒâ¹³ 1258511786
	if (obj.O_ActionName == L"male_dualhalberd_attack_hold_light_soul_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.ÐÎÒâ¹³ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.ÐÎÒâ¹³ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.ÐÎÒâ¹³;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//×ó3 3286806980
	if (obj.O_ActionName == L"male_dualhalberd_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.×óÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_dualhalberd_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.×óÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.×óÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.×óÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}

	//¹´ÐýÕ¶ 1487035907 
	if (obj.O_ActionName == L"male_dualhalberd_attack_light_08" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.¹´ÐýÕ¶ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.¹´ÐýÕ¶ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.¹´ÐýÕ¶;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//Ì½º£òÔ 2982476562
	if (obj.O_ActionName == L"male_dualhalberd_attack_heavy_09" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.Ì½º£òÔ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.Ì½º£òÔ : FALSE)
		{
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.Ì½º£òÔ); 

		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_dualhalberd_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::Ë«êª.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::Ë«êª.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::Ë«êª.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
	
}

//ÉÈ×ÓÕñµ¶·ÖÖ§
auto fanShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	//×óÐî  3905282567
	if (obj.O_ActionName == L"male_fan_attack_hold_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//ÓÒÐî 918249456
	if (obj.O_ActionName == L"male_fan_attack_hold_heavy_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//Èý·ç°Ú 3905282567
	if (obj.O_ActionName == L"male_fan_attack_hold_light_soul_01" )
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.Èý·ç°Ú ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.Èý·ç°Ú : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.Èý·ç°Ú;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//²øÁú±¼Ò° 918249456
	if (obj.O_ActionName == L"male_fan_attack_hold_heavy_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.²øÁú±¼Ò° ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.²øÁú±¼Ò° : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.²øÁú±¼Ò°;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_fan_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_fan_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}

	//¹í·´ 3297080797
	if ((obj.O_ActionName == L"male_fan_flashstep_attack_light_02" || obj.O_ActionName == L"male_fan_flashstep_attack_light_02_pre"))
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.¹í·´ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.¹í·´ : FALSE)
		{
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.¹í·´); 
			
		}
	}
	//¾ªÀ× 3286806980
	if (obj.O_ActionName == L"male_fan_attack_light_04" )
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.¾ªÀ× ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.¾ªÀ× : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.¾ªÀ×;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_fan_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::ÉÈ×Ó.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ÉÈ×Ó.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ÉÈ×Ó.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
}

//ºáµ¶Õñµ¶·ÖÖ§
auto hengdaoShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	//×óÐî 
	if ((obj.O_ActionName == L"male_hengdao_attack_hold_light_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.×óÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.×óÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.×óÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			if (Õñµ¶²ÎÊý::ÌØÊâ´¦Àí::ºáµ¶×óÐîÊÇ·ñÖ»¶ÌÉÁ)
			{
				if (obj.ObjCurTime > Shock_Timing)
				{
					Õñµ¶Âß¼­::ShortDodge(KEY_S); Sleep(500); 
				}
				
			}
			else
			{
				ZhenDao(obj.ObjCurTime, Shock_Timing); 
			}

		}
	}
	//ÓÒÐî
	if ((obj.O_ActionName == L"male_hengdao_attack_hold_heavy_01" || obj.O_ActionName == L"male_hengdao_attack_hold_heavy_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.ÓÒÐî ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
			
		}
	}
	//Ì¤¿ÕÉÁ
	if ((obj.O_ActionName == L"male_hengdao_crouch_attack_heavy_soul_01_1_pre" || obj.O_ActionName == L"male_hengdao_crouch_attack_heavy_soul_01_1") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.Ì¤¿ÕÉÁ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.Ì¤¿ÕÉÁ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.Ì¤¿ÕÉÁ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//²ÔÑÀ
	if ((obj.O_ActionName == L"male_hengdao_attack_light_soul_03") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.²ÔÑÀ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.²ÔÑÀ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.²ÔÑÀ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	
	//ÆÆÇ§¾ü  
	if (obj.O_ActionName == L"male_hengdao_attack_hold_light_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.ÆÆÇ§¾ü ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.ÆÆÇ§¾ü : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.ÆÆÇ§¾ü;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//¼«¹âÆÆÔÆÉÁ 
	if (obj.O_ActionName == L"male_hengdao_attack_hold_heavy_soul_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.¼«¹âÆÆÔÆÉÁ)
		{
			ZhenDao(obj.ObjCurTime, Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.¼«¹âÆÆÔÆÉÁ); 
		}

	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_hengdao_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_hengdao_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//Í¬Ô´
	if (obj.O_ActionName == L"male_hengdao_origin_attack_light_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.ÓÒÐî ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.ÓÒÐî : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.ÓÒÐî;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_hengdao_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::ºáµ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::ºáµ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::ºáµ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}

	}
}
//È­ÈÐ
auto punchShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	
	//×óÐî  
	if ((obj.O_ActionName == L"male_punch_attack_hold_light_02") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.×óÐîÁ¦ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.×óÐîÁ¦ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.×óÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//×óÐî 2¶Î 889053609
	if (obj.O_ActionName == L"male_punch_attack_hold_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.×óÐîÁ¦ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.×óÐîÁ¦ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.×óÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒÐîÁ¦ 
	if ((obj.O_ActionName == L"male_punch_attack_hold_heavy_01") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.ÓÒÐîÁ¦ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.ÓÒÐîÁ¦ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.ÓÒÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//ÓÒÐîÁ¦2¶Î  3635756764
	if (obj.O_ActionName == L"male_punch_attack_hold_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.ÓÒÐîÁ¦2¶Î ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.ÓÒÐîÁ¦2¶Î : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.ÓÒÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//°ÙÁÑÍÈ heavy_soul_02
	if (obj.O_ActionName == L"male_punch_attack_hold_heavy_soul_02" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.°ÙÁÑÍÈ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.°ÙÁÑÍÈ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.°ÙÁÑÍÈ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//ÆÆ¿ÕÈ­ 
	if (obj.O_ActionName == L"male_punch_crouch_attack_heavy_soul_01" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.ÆÆ¿ÕÈ­ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.ÆÆ¿ÕÈ­ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.ÆÆ¿ÕÈ­;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//Ì«¼«ÅªÔÆÊÖ male_punch_attack_hold_light_soul_03
	if (obj.O_ActionName == L"male_punch_attack_hold_light_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.Ì«¼«ÅªÔÆÊÖ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.Ì«¼«ÅªÔÆÊÖ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.Ì«¼«ÅªÔÆÊÖ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_punch_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.×ó3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}

	}
	//ÓÒ3 3249409949
	if (obj.O_ActionName == L"male_punch_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.ÓÒ3 ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}

	}

	//²ÔÑÀ male_punch_attack_light_soul_03
	if (obj.O_ActionName == L"male_punch_attack_light_soul_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <=  Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.²ÔÑÀ ?  obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.²ÔÑÀ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.²ÔÑÀ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}

	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_punch_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::È­ÈÐ.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::È­ÈÐ.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
}

//·Éµ¶
auto knifeShockKnife(PlayerData obj)
{
	float NewAngle = 0.f;
	float Shock_Timing = 0.f;
	//×óÐî  
	if ((obj.O_ActionName == L"male_knife_attack_hold_light_01_90") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.×óÐîÁ¦ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.×óÐîÁ¦ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::È­ÈÐ.×óÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) { 
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//×óÐî 2¶Î
	if ((obj.O_ActionName == L"male_knife_attack_hold_light_02_90" || obj.O_ActionName == L"male_knife_attack_hold_light_soul_02_01_90") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.×óÐîÁ¦2¶Î ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.×óÐîÁ¦2¶Î : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.×óÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	//ÓÒÐîÁ¦ 
	if ((obj.O_ActionName == L"male_knife_attack_hold_heavy_01_90") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.ÓÒÐîÁ¦ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.ÓÒÐîÁ¦ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.ÓÒÐîÁ¦;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 60;
			}
			else {
				Shock_Timing += obj.O_Dis / 30;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 


		}
	}
	//ÕÆÐÄÀ× 
	if ((obj.O_ActionName == L"male_knife_attack_hold_heavy_soul_01_90") /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.ÕÆÐÄÀ× ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.ÕÆÐÄÀ× : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.ÕÆÐÄÀ×;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}
	//Ì«¼«
	if (obj.O_ActionName == L"male_knife_attack_heavy_06" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.Ì«¼« ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.Ì«¼« : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.Ì«¼«;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}
	}
	//¶ÏÔÂÇ§ÈÐÎè
	if (obj.O_ActionName == L"male_knife_attack_hold_light_soul_02_02_90" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.¶ÏÔÂÇ§ÈÐÎè ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.¶ÏÔÂÇ§ÈÐÎè : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.¶ÏÔÂÇ§ÈÐÎè;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}
	}

	//×ó3 3286806980
	if (obj.O_ActionName == L"male_knife_attack_light_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.×ó3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.×ó3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.×ó3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 
		}

	}
	//ÓÒ3 3286806980
	if (obj.O_ActionName == L"male_knife_attack_heavy_03" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.ÓÒ3 ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.ÓÒ3 : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.ÓÒ3;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}

	//²ÔÑÀ 
	if (obj.O_ActionName == L"male_knife_attack_light_03_soul" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{

		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.²ÔÑÀ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.²ÔÑÀ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.²ÔÑÀ;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
	//±Ú»÷ 
	if (obj.O_ActionName == L"male_knife_hang_wall_attack_light_01_2" /*&& GetRangeReactionType(obj.Cache_HitSimulate) == 12*/)
	{
		
		if (obj.O_Direction <= Õñµ¶²ÎÊý::½Ç¶È::·Éµ¶.±Ú»÷ ? obj.O_Dis <= Õñµ¶²ÎÊý::¾àÀë::·Éµ¶.±Ú»÷ : FALSE)
		{
			Shock_Timing = Õñµ¶²ÎÊý::ÑÓ³Ù::·Éµ¶.±Ú»÷;
			if (obj.O_Dis <= 4.5f) {
				Shock_Timing += 0.f;
			}
			else if (obj.O_Dis <= 7.5f) {
				Shock_Timing += obj.O_Dis / 100;
			}
			else if (obj.O_Dis <= 10.5f) {
				Shock_Timing += obj.O_Dis / 70;
			}
			else {
				Shock_Timing += obj.O_Dis / 40;
			}
			ZhenDao(obj.ObjCurTime, Shock_Timing); 

		}

	}
}


//Õñµ¶
void Shock(PlayerData obj)
{
	
	switch (obj.O_ActorWeapon)
	{
	case 100:
		PalmShockKnife(obj);
		break;
	case 101:
		SwShockKnife(obj);
		break;
	case 102:
		KatanaShockKnife(obj);
		break;
	case 103:
		BladeShockKnife(obj);
		break;
	case 105:
		SpearShockKnife(obj);
		break;
	case 106:
		DaggeShockKnife(obj);
		break;
	case 116:
		nunchucksShockKnife(obj);
		break;
	case 118:
		twinbladesShockKnife(obj);
		break;
	case 119:
		SaberShockKnife(obj);
		break;
	case 120:
		rodShockKnife(obj);
		break;
	case 121:
		dualhalberdKnife(obj);
		break;
	case 122:
		fanShockKnife(obj);
		break;
	case 123:
		hengdaoShockKnife(obj);
		break;
	case 124:
		punchShockKnife(obj);
		break;
	case 127:
		knifeShockKnife(obj);
		break;
	default:
		break;
	}
}
