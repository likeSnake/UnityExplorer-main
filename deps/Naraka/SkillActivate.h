#pragma once
#include "Data.h"



inline auto NingHongYeActivation(PlayerData obj)
{
	if (obj.O_Dis <= 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 90.f) : FALSE)
	{
		if (JudgeShortFlash(obj.O_ActionName) && obj.O_EndureLevel == 5 && (Global::MySkillData.MySkill.F_SkillID == 1500130 || Global::MySkillData.MySkill.F_SkillID == 1500131) && Global::MySkillData.MySkill.F_State == 0 )
		{
			if (obj.ObjCurTime > 0.f)
			{
				Õñµ¶Âß¼­::TianhaiF2(); Sleep(400);
			}
		}
	}

}
inline auto ShaNanActivation(PlayerData obj)
{
	if (obj.O_Dis <= 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 90.f) : FALSE)
	{
		if (obj.O_ReactionExtraType == 1 && (Global::MySkillData.MySkill.F_SkillID == 1500300 || Global::MySkillData.MySkill.F_SkillID == 1500301 || Global::MySkillData.MySkill.F_SkillID == 1500302) && Global::MySkillData.MySkill.F_State == 0)
		{
			if (obj.ObjCurTime > 0.f)
			{
				Õñµ¶Âß¼­::SkillF(); Sleep(400);
			}
		}
	}

}
//ÎäÌïËæ»ú×Ô¶¯F
inline auto WuTianActivation(PlayerData obj)
{
	float MySleep = Global::WorldPtr.ping / 1000;
	double ObjCurTime = 0;
	if (obj.O_Dis <= 5.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 90.f) : FALSE)
	{

		if ((Global::MySkillData.MySkill.F_SkillID == 1501401) && Global::MySkillData.MySkill.F_State == 0 && obj.O_EndureLevel == 20 && GetRangeReactionType(obj.Cache_HitSimulate) == 12)
		{
			if (obj.ObjCurTime > 0.f)
			{
				Õñµ¶Âß¼­::WuTianF(); Sleep(400);
			}


		}
		if ((Global::MySkillData.MySkill.F_SkillID == 1501402) && Global::MySkillData.MySkill.F_State == 0 && obj.O_XuListate == 5)
		{
			if (obj.ObjCurTime > 0.f)
			{
				Õñµ¶Âß¼­::WuTianF(); Sleep(400);
			}

		}
		if ((Global::MySkillData.MySkill.V_State == 1) && Global::MySkillData.MySkill.F_State == 0 && LocalPlayer_Data.XuListate == 5 && LocalPlayer_Data.MyBlueTime >= 0.5f)
		{

			Õñµ¶Âß¼­::SkillF();
			Õñµ¶Âß¼­::CollideKnife(); Sleep(300);


		}
	}
}


//Ììº£F2
inline auto TianHaiActivation(PlayerData obj)
{
	if (obj.O_Dis <= 5.5f ? CheckPlayerAngle(obj.Cache_ActorRootPtr, obj.O_Pos, LocalPlayer_Data.MyPos, 90.f) : FALSE)
	{

		if (Global::MySkillData.MySkill.F_SkillID == 1500501 && Global::MySkillData.MySkill.F_State == 0 && obj.O_EndureLevel == 20 && GetRangeReactionType(obj.Cache_HitSimulate) == 12)
		{

			auto ³öÕÐÖ´ÐÐ = (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
			if (³öÕÐÖ´ÐÐ > 0.f)
			{
				Õñµ¶Âß¼­::WuTianF(); Sleep(400);

			}

		}
	}
}

inline auto jiyingyingActivation(PlayerData obj)
{

	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"female_hero_jiyingying_bianzi(_|_sprint_)attack_light_01_90_charge_(1|2|3|4)")))
	{
		if (obj.O_Dis <= 9.5f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 90.f) : FALSE)
		{
			if (LocalPlayer_Data.MyBlueTime >= 0.5f)
			{
				Õñµ¶Âß¼­::CollideKnife(); Sleep(300);

			}
		}

	}
	if (LocalPlayer_Data.MyActionName == L"female_hero_jiyingying_bianzi_sprint_attack_light_01_90")
	{
		if (obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) < 10.f ? CheckPlayerAngle(LocalPlayer_Data.My_ActorRootPtr, LocalPlayer_Data.MyPos, obj.O_Pos, 90.f) : FALSE)
		{
			if (LocalPlayer_Data.µÐÈËÊýÁ¿5MÄÚ <= 1 && obj.O_Pos.DistTo(LocalPlayer_Data.MyPos) <= 5.5f)
			{
				if (obj.O_ActionDuration > 0.8f)
				{
					Õñµ¶Âß¼­::È­ÇÐ3A(100); Sleep(900);
				}

			}
		}

	}

}

inline auto GuQingHanActivation(PlayerData obj)
{
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"female_hero_guqinghan(_|_jump_)attack_(02|03)_90_pre")))
	{

	}
}



inline void SkillActivate(PlayerData obj)
{
	switch (LocalPlayer_Data.MyHero)
	{
	case 1000003: 
		NingHongYeActivation(obj);
		break;
	//case 1000005:
	//	ShaNanActivation(obj);
	//	break;
	case 1000007:
		TianHaiActivation(obj);
		break;
	case 1000015:
		GuQingHanActivation(obj);
		break;
	case 1000016:
		WuTianActivation(obj);
		break;
	case 1000021:
		jiyingyingActivation(obj);
		break;

	}
}