#pragma once
#include "Data.h"

//技能躲避
void DodgeSkill(PlayerData obj)
{
	auto CalcObjCurTime = [&]() {
		return (Global::WorldPtr.GlobalTime - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
		};
	auto IsYClose = [&](float range = 4.5f) {
		float dy = fabs(obj.O_Pos.y - LocalPlayer_Data.MyPos.y);
		return dy < range && dy > -range;
		};
	auto SkillState = [&]() {
		return mem.Read<int>((uintptr_t)obj.Cache_PropertyData + Offset::ActorModel.ProPerty.SkillStatus);

		};


	if (obj.HeroId == 1000006)
	{
		//躲避火男F1
		if (Function::HeroActivation::躲避火男F)
		{
			if ((obj.O_ActionName == L"male_hero_spiritsman_attack_01_counter_01" || obj.O_ActionName == L"male_hero_spiritsman_attack_02_counter_01" || obj.O_ActionName == L"male_hero_spiritsman_attack_02_counter_02") && obj.O_Dis <= 活化参数::距离::火男F)//判断动作，距离
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::火男F : FALSE)
				{
					if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return; }//判断自身等级
					if (CalcObjCurTime() > 0.f)
					{
						振刀逻辑::ShortDodge(KEY_S); 
						活化闪避 = true;
						Sleep(500);
					}
					
			
				}
			}
		}                                               
		//躲避火男冲拳  
		if (Function::HeroActivation::躲避火男冲拳 )
		{
			//if(StrStrW(O_ActionName.data(), L"_hero_spiritsman_dodge_front_double"))
			if ((StrStrW(obj.O_ActionName.data(), L"_hero_spiritsman_dodge_front_double") || 
				(StrStrW(obj.O_ActionName.data(), L"male_hero_spiritsman_attack_heavy_")) && !StrStrW(obj.O_ActionName.data(), L"_recover")) && 
				obj.O_Dis <= 活化参数::距离::火男冲拳)//判断动作，距离
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::火男冲拳 : FALSE)
				{
					//if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return ; }//判断自身等级
					if (CalcObjCurTime() > 0.f)
					{
						振刀逻辑::ShortDodge(KEY_S);
						活化闪避 = true;
						Sleep(600);
					}


				}
				
			}
			if (StrStrW(obj.O_ActionName.data(), L"male_hero_spiritsman_dodge_front_double") && obj.O_Dis <= 活化参数::距离::火男顶膝)//判断动作，距离 
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::火男冲拳 : FALSE)
				{
					if (CalcObjCurTime() > 0.f)
					{
						振刀逻辑::ShortDodge(KEY_S); 
						活化闪避 = true;
						Sleep(600);

					}

				}
			}
		} 
		
	}
	else if (obj.HeroId == 1000009 )
	{
		
		//躲避妖刀大招  
		if (Function::HeroActivation::躲避妖刀大招)
		{
			// V 
			if ((obj.O_ActionName == L"female_hero_yaodaoji_jump_uniqueskill_01_02_copy_02" || obj.O_ActionName == L"female_hero_yaodaoji_jump_uniqueskill_02_02_copy_02" ||
				obj.O_ActionName == L"female_hero_yaodaoji_jump_uniqueskill_03_02_copy_02" || obj.O_ActionName == L"female_hero_yaodaoji_jump_uniqueskill_extend_01_branch_01" ||
				obj.O_ActionName == L"female_hero_yaodaoji_jump_uniqueskill_extend_02_branch_01" || obj.O_ActionName == L"female_hero_yaodaoji_uniqueskill_extend_03_branch_01" ||
				obj.O_ActionName == L"female_hero_yaodaoji_uniqueskill_attack_02_hold_01_branch_03") && obj.O_Dis <= 活化参数::距离::妖刀大招)
			{
				if (obj.O_Dis <= 3.f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::妖刀大招3M : FALSE)
					{
						振刀逻辑::ShortDodge(KEY_W); 活化闪避 = true;Sleep(500);
						
					}
				}
				else//3-25M内
				{
					if (IsYClose(25.f) ? obj.O_Direction <= 活化参数::角度::妖刀大招3M外 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
							
						}
					}
				}
			}
			//v1  
			if ((obj.O_ActionName == L"female_hero_yaodaoji_uniqueskill_attack_02_hold_03_branch_03" || obj.O_ActionName == L"female_hero_yaodaoji_uniqueskill_attack_02_hold_02_branch_03_slide") && obj.O_Dis <= 活化参数::距离::妖刀大招 && obj.O_RangeReactionType == 12)
			{
				if (obj.O_Dis <= 4.f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::妖刀大招3M : FALSE)
					{
						振刀逻辑::ShortDodge(KEY_W); 活化闪避 = true;Sleep(500);
						
					}
				}
				else//3-25M内
				{
					if (IsYClose(25.f) ? obj.O_Direction <= 活化参数::角度::妖刀大招3M外 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
							
						}

					}
				}
			}
		}
	}
	else if (obj.HeroId == 1000015 )
	{
		//躲避顾清寒大招
		if (Function::HeroActivation::躲避顾清寒V1V2 )
		{
			//V1
			if ((obj.O_ActionName == L"female_hero_guqinghan_attack_02_90_pre" || obj.O_ActionName == L"female_hero_guqinghan_jump_attack_02_90_pre") && obj.O_Dis <= 活化参数::距离::顾倾寒大招)//判断动作
			{
				if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return; }//判断自身等级
				if (Function::HeroActivation::特殊处理::永远躲避顾倾寒V1)
				{
					if (obj.O_Dis <= 3.f)//3M
					{
						if (IsYClose() ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M : FALSE)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
							
						}
					}
					else//3-25M内
					{
						if (IsYClose(25.f) ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M外 : FALSE)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
							
						}
					}
				}
				else
				{
					if (LocalPlayer_Data.MyEndureLevel < 20)
					{
						if (obj.O_Dis <= 3.f)//3M
						{
							if (IsYClose() ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M : FALSE)
							{
								振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
								
							}
						}
						else//3-25M内
						{
							if (IsYClose(25.f) ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M外 : FALSE)
							{
								振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
								
							}
						}
					}
				}
			}
			//V2
			if (obj.O_ActionName == L"female_hero_guqinghan_attack_03_90_pre" || obj.O_ActionName == L"female_hero_guqinghan_jump_attack_03_90_pre")//判断动作
			{
				if (obj.O_Dis <= 3.f)//3M
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M : FALSE)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
						
					}
				}
				else//3-25M内
				{
					if (IsYClose(25.f) ? obj.O_Direction <= 活化参数::角度::顾倾寒大招3M外 : FALSE)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(500);
						
					}
				}
			}
		}
	}
	else if (obj.HeroId == 1000016 )
	{
		//武田F躲避
		if (Function::HeroActivation::躲避武田F)
		{
			if ( LocalPlayer_Data.MyEndureLevel == 20 || LocalPlayer_Data.MyEndureLevel == 22)
			{
				if ((obj.O_ActionName == L"male_hero_takeda_attack_01_branch_01" || obj.O_ActionName == L"male_hero_takeda_jump_attack_01_branch_01") && obj.O_Dis <= 4.5f)//F2
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::武田F21M : FALSE)
					{
						if (LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.MyBlueTime > 0.f)		//1.0之前
						{
							振刀逻辑::CollideKnife(); 活化闪避 = true;Sleep(400);
							
						}
						else
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(500);
							
						}
					}
				
				}
				if ((obj.O_ActionName == L"male_hero_takeda_attack_01_branch_02" || obj.O_ActionName == L"male_hero_takeda_jump_attack_01_branch_02") && obj.O_Dis <= 活化参数::距离::武田F3)//F3
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::武田F31M : FALSE)
					{
						if (LocalPlayer_Data.MyBlueTime <= 0.49 && LocalPlayer_Data.MyBlueTime > 0.f)		//1.0之前
						{
							振刀逻辑::CollideKnife(); 活化闪避 = true;Sleep(400);
							
						}
						else
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(500);
							
						}
					}
					
				}
			}
		}
	}
	else if (obj.HeroId == 1000020)
	{
		//胡为躲避 male_hero_huwei_jump_attack_01_branch_01
		if (Function::HeroActivation::躲避胡为)
		{
			if (LocalPlayer_Data.MyEndureLevel != 30 && LocalPlayer_Data.MyEndureLevel != 31 && LocalPlayer_Data.MyEndureLevel < 40 && LocalPlayer_Data.MyEndureLevel != 35)//判断自身等级
			{
				//躲避胡为F1
				if ((obj.O_ActionName == L"male_hero_huwei_attack_01" || obj.O_ActionName == L"male_hero_huwei_jump_attack_01") && obj.O_Dis <= 活化参数::距离::胡为F1)
				{
					振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(1000);

					
				}
				//胡为F2 
				if ((obj.O_ActionName == L"male_hero_huwei_attack_01_branch_01" || obj.O_ActionName == L"male_hero_huwei_jump_attack_01_branch_01") && obj.O_Dis <= 活化参数::距离::胡为F2)
				{
					if (IsYClose(7.5f) ? obj.O_Direction <= 活化参数::角度::胡为F2 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f)
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(600);
							
						}
					}

				}
				//wcout << "动作名=" << obj.O_ActionName << "攻击状态=" << obj.O_RangeReactionType << endl;
				//胡为F2 
				if (( obj.O_ActionName == L"male_hero_huwei_attack_02_branch_01" || obj.O_ActionName == L"male_hero_huwei_jump_attack_02_branch_01") && obj.O_Dis <= 活化参数::距离::胡为F2)
				{
					if (obj.O_Dis <= 15.f)
					{
						MyLog("闪避胡伟F");
						振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(600);
						
						/*if (CalcObjCurTime() > 0.05f)
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(600);
						}*/
					}

				}
				//躲避胡为V1 V2  
				if ((obj.O_ActionName == L"male_hero_huwei_uniqueskill_extend_05_pre" || obj.O_ActionName == L"male_hero_huwei_jump_uniqueskill_extend_05_pre") && obj.O_Dis <= 活化参数::距离::胡为V1V2)
				{
					if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::胡为V1V2 : FALSE)
					{
						if (CalcObjCurTime() > 0.03f)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(600);
							
						}
					}
				}
				//躲避胡为跳跃攻击   
				if (StrStrW(obj.O_ActionName.c_str(), L"tigerleap_attack_01_branch_01") || StrStrW(obj.O_ActionName.c_str(), L"tigerleap_attack_02_branch_01") || StrStrW(obj.O_ActionName.c_str(), L"tigerleap_attack_03_branch_01") && obj.O_Dis <= 活化参数::距离::胡为跃击)
				{
					if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::胡为跃击 : FALSE)
					{
						if (CalcObjCurTime() > 0.1f + obj.O_Dis /100 )
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(600);
							
						}

					}
				}

				// 胡为新版跳a
				// _tigerleap_attack_hold

				if(obj.O_ActionName.find(L"_tigerleap_attack_hold") != std::wstring::npos){
					if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::胡为跃击 : FALSE)
					{
						if (CalcObjCurTime() > 0.1f + obj.O_Dis / 100)
						{
							振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(600);
							
						}

					}
				}

				if ((obj.O_ActionName.find(L"_tigerleap_attack_") != std::wstring::npos) 
					&& obj.O_ActionName.find(L"_recover") == std::wstring::npos 
					&& obj.O_ActionName.find(L"_charge") == std::wstring::npos) {
					if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::胡为跃击 : FALSE)
					{
						振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}
		}
	}
	else if (obj.HeroId == 1000003 )
	{
		//躲避宁红叶F
		if (Function::HeroActivation::躲避宁红叶F)
		{
			if (LocalPlayer_Data.MyEndureLevel != 30 && LocalPlayer_Data.MyEndureLevel != 31 && LocalPlayer_Data.MyEndureLevel < 40 && LocalPlayer_Data.MyEndureLevel != 35)//判断自身等级
			{
				//躲避宁红叶F
				if ((obj.O_ActionName == L"female_hero_blindswordswoman_attack_04_copy_02" || obj.O_ActionName == L"female_hero_blindswordswoman_jump_attack_04_copy_02" || 
					obj.O_ActionName == L"female_hero_blindswordswoman_attack_04_copy_01" || obj.O_ActionName == L"female_hero_blindswordswoman_jump_attack_04_copy_01" ||
					obj.O_ActionName == L"female_hero_blindswordswoman_attack_04" || obj.O_ActionName == L"female_hero_blindswordswoman_jump_attack_04") && obj.O_Dis <= 活化参数::距离::宁红叶F1)
				{
					if (CalcObjCurTime() > 0.15f)
					{
						
						if (obj.O_Dis <= 3.5f)
						{
							if (IsYClose() ? obj.O_Direction <= 活化参数::角度::宁红叶F1 : FALSE)
							{
								振刀逻辑::ShortDodge(KEY_W); 
								活化闪避 = true;
								Sleep(1000);
							}
						}
						else
						{
							if (IsYClose() ? obj.O_Direction <= 活化参数::角度::宁红叶F1 : FALSE)
							{
								振刀逻辑::ShortDodge(KEY_S); 
								活化闪避 = true;
								Sleep(1000);

							}
						}
					}
				}

			}
		}
	}
	else if (obj.HeroId == 1000022 )
	{
		//躲避狐狸大招 
		if (Function::HeroActivation::躲避狐狸大招)
		{
			// v1
			if ((obj.O_ActionName == L"female_hero_yulinglong_uniqueskill_01" || obj.O_ActionName == L"female_hero_yulinglong_jump_uniqueskill_01" ||
				obj.O_ActionName == L"female_hero_yulinglong_uniqueskill_01_extend_01" || obj.O_ActionName == L"female_hero_yulinglong_jump_uniqueskill_01_extend_01") && obj.O_Dis <= 活化参数::距离::狐狸V1)
			{
				if (obj.O_Dis <= 4.f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::狐狸大招4M : FALSE)
					{
						if (CalcObjCurTime() > 0.03f)
						{
							MyLog("狐狸大招闪避小于4米");
							振刀逻辑::ShortDodge(KEY_W); 活化闪避 = true;Sleep(2500);
							
						}
					}
				}
				else if (obj.O_Dis <= 8.f)
				{
					if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::狐狸大招8M : FALSE)
					{
						if (CalcObjCurTime() > 0.08f)
						{
							MyLog("狐狸大招闪避小于8米");
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(2500);
							
						}
					}
				}
				else if (obj.O_Dis <= 13.f)
				{
					if (IsYClose(13.5f) ? obj.O_Direction <= 活化参数::角度::狐狸大招12M : FALSE)
					{
						if (CalcObjCurTime() > 0.105f + obj.O_Dis / 100)
						{
							MyLog("狐狸大招闪避小于13米");
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(2500);
							
						}
					}
				}
				else
				{
					if (obj.O_Direction <= 活化参数::角度::狐狸大招12M外)
					{
						if (CalcObjCurTime() > 0.1f + obj.O_Dis / 100)
						{
							MyLog("狐狸大招闪避大于13米");
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(2500);
							
						}
					}
				}
			}
			//v2
			if (obj.O_ActionName == L"female_hero_yulinglong_uniqueskill_02_extend_01" && obj.O_Dis <= 活化参数::距离::狐狸V2)
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::狐狸大招4M : FALSE)
				{
					if (CalcObjCurTime() > 0.1f)
					{
						振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(1500);
						
					}

				}
			}
		}
	}
	else if (obj.HeroId == 1000010 )
	{
		//躲避三娘V2 
		if (Function::HeroActivation::躲避三娘V2)
		{
			if ((obj.O_ActionName == L"female_hero_cuisanniang_uniqueskill_03_attack_01" || obj.O_ActionName == L"female_hero_cuisanniang_uniqueskill_03_jump_attack_01") && obj.O_Dis <= 活化参数::距离::三娘V2)
			{
				if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return; }
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::三娘V24M : FALSE)
				{
					if (CalcObjCurTime() > 0.f + obj.O_Dis / 100)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1500);
						
					}

				}
			}
			
		}
	}
	else if (obj.HeroId == 1000023)
	{
		//躲避哈迪大招
		if (Function::HeroActivation::躲避哈迪大招)
		{
			//一段大招  
			if ((obj.O_ActionName == L"male_hero_hadi_attack_01_branch_01" || obj.O_ActionName == L"male_hero_hadi_fly_attack_hold_01_90_branch_01") && obj.O_Dis <= 活化参数::距离::哈迪大招1段)
			{
				if (obj.O_Direction <= 活化参数::角度::哈迪大招)
				{
					if (CalcObjCurTime() > 0.05f)
					{

						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}

				}


			}
			//二段大招 
			if ((obj.O_ActionName == L"male_hero_hadi_attack_01_branch_02" || obj.O_ActionName == L"male_hero_hadi_fly_attack_hold_02_90_branch_01") && obj.O_Dis <= 活化参数::距离::哈迪大招2段)
			{
				if (obj.O_Direction <= 活化参数::角度::哈迪大招)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}

				}


			}

			//三段大招    
			if ((obj.O_ActionName == L"male_hero_hadi_attack_01_branch_03" || obj.O_ActionName == L"male_hero_hadi_fly_attack_hold_03_90_branch_01") && obj.O_Dis <= 活化参数::距离::哈迪大招3段)
			{
				if (obj.O_Direction <= 活化参数::角度::哈迪大招)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}

				}


			}
			
		}
	}
	else if (obj.HeroId == 1000024 )
	{
		//躲避魏轻F1
		if (Function::HeroActivation::躲避魏轻F1)
		{

			if ((obj.O_ActionName == L"female_hero_weiqing_attack_01" || obj.O_ActionName == L"female_hero_weiqing_jump_attack_01") && obj.O_Dis <= 活化参数::距离::魏轻F1)
			{
				if (IsYClose())
				{
					振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(800);
					
				}
			}
				
			
		}
		//躲避魏轻V
		if (Function::HeroActivation::躲避魏轻V)
		{
			//V1
			if (StrStrW(obj.O_ActionName.data(), L"_attack_03_90") && obj.O_Dis <= 活化参数::距离::魏轻V1)
			{
				if (obj.O_Direction <= 活化参数::角度::魏轻V1)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}
			//v2
			if (StrStrW(obj.O_ActionName.data(), L"_attack_03_copy_01") && obj.O_Dis <= 活化参数::距离::魏轻V2)
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::魏轻V2 : FALSE)
				{
					if (CalcObjCurTime() >= 0.25f)
					{
						振刀逻辑::ShortDodge(KEY_S); 活化闪避 = true;Sleep(1000);
						
					}

				}
			}
			
		}
	}
	else if (obj.HeroId == 1000004 )
	{
		//躲避迦南
		if (Function::HeroActivation::躲避迦南F2)
		{
			
			//F2  
			if ((obj.O_ActionName == L"female_hero_desertassassin_attack_01_90_copy_04_pre" || obj.O_ActionName == L"female_hero_desertassassin_jump_attack_01_90_copy_04_pre" ||
				obj.O_ActionName == L"female_hero_desertassassin_attack_02_90_copy_04_pre" || obj.O_ActionName == L"female_hero_desertassassin_jump_attack_02_90_copy_04_pre") && obj.O_Dis <= 活化参数::距离::迦南F2)
			{
				if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return; }
				if (IsYClose(7.5f) ? obj.O_Direction <= 活化参数::角度::迦南F2 : FALSE)
				{
					if (CalcObjCurTime() >= 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}

			}
			if ((obj.O_ActionName == L"female_hero_desertassassin_attack_hold_01_90_copy_04" || obj.O_ActionName == L"female_hero_desertassassin_jump_attack_01_90_copy_04" ||
				obj.O_ActionName == L"female_hero_desertassassin_attack_hold_02_90_copy_04" || obj.O_ActionName == L"female_hero_desertassassin_jump_attack_02_90_copy_04") && obj.O_Dis <= 活化参数::距离::迦南F2 || SkillState() == 2)
			{
				if (!JudgeStatus(LocalPlayer_Data.MyEndureLevel)) { return; }
				if (IsYClose(7.5f) ? obj.O_Direction <= 活化参数::角度::迦南F2 : FALSE)
				{
					if (CalcObjCurTime() >= 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}
				
		}
		
	}
	else if (obj.HeroId == 1000025)
	{
		//刘炼V1V2 
		if (Function::HeroActivation::躲避刘炼V)
		{
			//v1
			if ((obj.O_ActionName == L"male_hero_liulian_uniqueskill_01_branch_01_attack_01"|| obj.O_ActionName == L"male_hero_liulian_uniqueskill_01_branch_02_attack_1") && obj.O_Dis <= 活化参数::距离::刘炼V1)
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::刘炼V1 : FALSE)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeWx(); 活化闪避 = true;Sleep(500);
						
					}
				}
			}
		
		}
	}
	else if (obj.HeroId == 1000011 )
	{
		//躲避岳山
		if (Function::HeroActivation::躲避岳山F)
		{

			//f1male_hero_yueshan_uniqueskill_attack_05
			if ((obj.O_ActionName == L"male_hero_yueshan_attack_01" || obj.O_ActionName == L"male_hero_yueshan_jump_attack_01" || obj.O_ActionName == L"male_hero_yueshan_attack_01_branch_02"
				|| obj.O_ActionName == L"male_hero_yueshan_jump_attack_01_branch_02") && obj.O_Dis <= 活化参数::距离::岳山F)
			{
				if (obj.O_Dis <= 4.0f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::岳山F : FALSE)
					{
						if (CalcObjCurTime() > 0.f)
						{
							振刀逻辑::ShortDodge(KEY_W); 活化闪避 = true;Sleep(1000);
							
						}
					}
				}
				else
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::岳山F : FALSE)
					{
						if (CalcObjCurTime() > 0.05f + obj.O_Dis / 100)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
							
						}
					}
					
				}

			}
			//v1 
			if ((obj.O_ActionName == L"male_hero_yueshan_uniqueskill_attack_05" ) && obj.O_Dis <= 8.5f)
			{
				if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::岳山F : FALSE)
				{
					if (CalcObjCurTime() > 0.1f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}

			}

		}
	}
	else if (obj.HeroId == 1000021 )
	{
		//躲避季盈盈V蓄力
		if (Function::HeroActivation::躲避季盈盈V蓄力)
		{
			// v2
			if ((obj.O_ActionName == L"female_hero_jiyingying_bianzi_attack_hold_light_01_90" || obj.O_ActionName == L"female_hero_jiyingying_bianzi_attack_hold_light_02" )&& !StrStrW(obj.O_ActionName.data(), L"_recover")) {
				if (obj.O_Dis <= 2.0f) {
					if (CalcObjCurTime() > 0.05f) {
						振刀逻辑::ShortDodge(KEY_A); Sleep(1000);
					}
				}
				else {
					振刀逻辑::ShortDodge(KEY_S);
				}
			}
			//v1 蓄力  
			if ((obj.O_ActionName == L"female_hero_jiyingying_bianzi_attack_hold_light_04"
				|| obj.O_ActionName == L"female_hero_jiyingying_bianzi_attack_light_03_90") && !StrStrW(obj.O_ActionName.data(), L"_recover") && obj.O_Dis <= 活化参数::距离::季盈盈V1蓄力 && obj.O_RangeReactionType == 12)
			{
				
				if (obj.O_Dis <= 4.0f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::季盈盈V1蓄力 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f)
						{
							振刀逻辑::ShortDodgeWx(); 活化闪避 = true;Sleep(1000);
						}
					}
				}
				else if (obj.O_Dis <= 8.0f)
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::季盈盈V1蓄力 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f + obj.O_Dis/150)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						}
					}
				}
				else
				{
					if (IsYClose() ? obj.O_Direction <= 活化参数::角度::季盈盈V1蓄力 : FALSE)
					{
						if (CalcObjCurTime() > 0.05f + obj.O_Dis / 100)
						{
							振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						}
					}
				}
				
			}

			
		}
	}
	else if (obj.HeroId == 1000026)
	{
		//张起灵F2 V1V2 
		if (Function::HeroActivation::躲避张起灵)
		{

			//F2 
			if ((obj.O_ActionName == L"male_hero_zhangqiling_attack_01_branch_01" || obj.O_ActionName == L"male_hero_zhangqiling_attack_01") && obj.O_Dis <= 活化参数::距离::张起灵F2)
			{
				if (IsYClose() ? obj.O_Direction <= 活化参数::角度::张起灵F2 : FALSE)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}
			//v1  
			if ((obj.O_ActionName == L"male_hero_zhangqiling_gudao_hold_attack_01" || obj.O_ActionName == L"male_hero_zhangqiling_gudao_jump_hold_attack_01_1") && obj.O_Dis <= 活化参数::距离::张起灵V1)
			{
				if (obj.O_Direction <= 活化参数::角度::张起灵V1)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::短接长闪(); 活化闪避 = true;Sleep(1000);
						
					}
				}

				
			}
			//v2
			if ((obj.O_ActionName == L"male_hero_zhangqiling_gudao_hold_attack_02_90" || obj.O_ActionName == L"male_hero_zhangqiling_gudao_jump_hold_attack_02_90") && obj.O_RangeReactionType == 12 )
			{
				if (obj.O_Dis <= 活化参数::距离::张起灵V2 ? obj.O_Direction <= 活化参数::角度::张起灵V2 : FALSE)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}

				
			}

		}
	}
	else if (obj.HeroId == 1000013 )
	{
		//无尘V2
		if (Function::HeroActivation::躲避无尘V2)
		{
			//v2 
			if ((obj.O_ActionName == L"male_hero_wuchen_uniqueskill_02_attack_01" || obj.O_ActionName == L"male_hero_wuchen_uniqueskill_02_jump_attack_01") && obj.O_Dis <= 活化参数::距离::无尘V2)
			{
				if (obj.O_Direction <= 活化参数::角度::无尘V2)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}

		}
	}
	else if (obj.HeroId == 1000027)
	{
		//彩戏师 
		if ((obj.O_ActionName == L"female_hero_fuwenshi_uniqueskill_attack_03_branch_01_90") && obj.O_Dis <= 活化参数::距离::彩戏师F)
		{
			if (IsYClose(8.5f) ? obj.O_Direction <= 活化参数::角度::彩戏师F : FALSE)
			{
				if (CalcObjCurTime() > 0.1f)
				{
					振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
					
				}

			}
		}
	}
	else if (obj.HeroId == 1000028)
	{
		//彩戏师    
		if ((obj.O_ActionName == L"female_hero_caixishi_uniqueskill_attack_01_branch_01_hitball_success_01" || obj.O_ActionName == L"female_hero_caixishi_uniqueskill_jump_attack_01_branch_01_hitball_success_01" 
			|| obj.O_ActionName == L"female_hero_caixishi_uniqueskill_attack_01_branch_01_hitball_success_02" || obj.O_ActionName == L"female_hero_caixishi_uniqueskill_jump_attack_01_branch_01_hitball_success_02") && obj.O_Dis <= 活化参数::距离::彩戏师F)
		{
			if (IsYClose(20.f) ? obj.O_Direction <= 活化参数::角度::彩戏师F : FALSE)
			{
				if (CalcObjCurTime() > 0.1f)
				{
					振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
				}
				

			}
		}
	}
	else if (obj.HeroId == 1000031)
	{
		//李寻欢   
		//wcout << "动作名=" << obj.O_ActionName << "攻击状态=" << obj.O_RangeReactionType << endl;
		// 动作名=male_hero_lixunhuan_uniqueskill_kink_weight_attack_01_pre我的攻击状态=12
		if (obj.O_ActionName.find(L"_uniqueskill_kink_weight_attack_01_pre") != std::wstring::npos ||
			obj.O_ActionName.find(L"_uniqueskill_kink_weight_attack_02_pre") != std::wstring::npos ||
			obj.O_ActionName.find(L"uniqueskill_kink_weight_attack_01_branch_01_pre") != std::wstring::npos){
			if (obj.O_Dis <= 6.0f && CalcObjCurTime() > 0.1f) {
				LOG("李寻欢踩头快速振刀\n");
				振刀逻辑::BlueToRed(); 活化闪避 = true;Sleep(650);
			}
		}
		else if (obj.O_ActionName.find(L"_uniqueskill_attack_01_branch_01_90") != std::wstring::npos ||
			obj.O_ActionName.find(L"_uniqueskill_attack_02_branch_01_90") != std::wstring::npos ||
			obj.O_ActionName.find(L"_uniqueskill_attack_03_branch_01_90") != std::wstring::npos ||
			obj.O_ActionName.find(L"_uniqueskill_attack_04_branch_01_90") != std::wstring::npos) {
			
			if (obj.O_Dis <= 4.f)
			{
				
				if (CalcObjCurTime() > 0.15f)
				{
					LOG("李寻欢大招躲避小于4米\n");
					振刀逻辑::ShortDodge(KEY_W); 活化闪避 = true;Sleep(1000);

					
				}
			}
			else if (obj.O_Dis <= 8.f)
			{
				if (CalcObjCurTime() > 0.05f)
				{
					LOG("李寻欢大招躲避小于8米\n");
					振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
					
				}
			}
			else if (obj.O_Dis <= 13.f)
			{
				
				if (CalcObjCurTime() > 0.05f)
				{
					LOG("李寻欢大招躲避小于13米\n");
					振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
					
				}
				
			}
			else if (obj.O_Dis <= 18.f)
			{
				if (obj.O_Direction <= 180)
				{
					if (CalcObjCurTime() > 0.05f)
					{
						LOG("李寻欢大招躲避大于13米\n");
						振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
						
					}
				}
			}
			
			
		}
		else if (obj.O_ActionName == L"_uniqueskill_attack_05_branch_01_90") {
			// 最后一段

		}
		/*if ((obj.O_ActionName == L"male_hero_lixunhuan_attack_09" || obj.O_ActionName == L"female_hero_caixishi_uniqueskill_jump_attack_01_branch_01_hitball_success_01"
			|| obj.O_ActionName == L"female_hero_caixishi_uniqueskill_attack_01_branch_01_hitball_success_02" || obj.O_ActionName == L"female_hero_caixishi_uniqueskill_jump_attack_01_branch_01_hitball_success_02") && obj.O_Dis <= 活化参数::距离::彩戏师F)
		{
			if (IsYClose(20.f) ? obj.O_Direction <= 活化参数::角度::彩戏师F : FALSE)
			{
				if (CalcObjCurTime() > 0.1f)
				{
					振刀逻辑::ShortDodgeEx(); 活化闪避 = true;Sleep(1000);
				}

			}
		}*/
	}
	
}



