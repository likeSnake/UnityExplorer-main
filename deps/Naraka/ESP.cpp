#pragma once
#include"ESP.h"
#include "aimbot.h"
#include "log.h"
#include <chrono>

std::string string_To_UTF8(const std::string& str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

void PlayerESP()
{	
	int last_read_index = -1;  // 跟踪已处理的数据版本
	Vec2 PM;
	Vec4 RW;
	int 信息Y轴 = 0, 预警Y轴 = 0;
	int 盲区外敌人数量 = 0, 盲区敌人怒气 = 0;
	float 盲区外最近敌人距离 = 101.f;
	string 盲区最近敌人职业 = "", 盲区最近敌人是否有大 = "";

	const uint64_t reads_per_second = g_localplayer_reads_per_second.load(std::memory_order_acquire);
	std::string reads_text = "缓存效率: " + std::to_string(reads_per_second);
	static auto last_fps_update = std::chrono::steady_clock::now();
	static double fps_sum = 0.0;
	static int fps_count = 0;
	static int fps_display = 0;

	if (LocalPlayer_Data.MyFPS > 0) {
		fps_sum += LocalPlayer_Data.MyFPS;
		++fps_count;
	}

	const auto now = std::chrono::steady_clock::now();
	if (now - last_fps_update >= std::chrono::seconds(1)) {
		if (fps_count > 0) {
			fps_display = static_cast<int>(fps_sum / fps_count + 0.5);
		}
		else {
			fps_display = 0;
		}
		fps_sum = 0.0;
		fps_count = 0;
		last_fps_update = now;
	}

	const float metrics_font_size = Function::ESP::userTextSize + 6;
	const float metrics_y = 10.0f;
	const float metrics_spacing = ImGui::GetStyle().ItemSpacing.x;
	const float metrics_text_max_width = 10000.0f;
	const float metrics_bold_offset = 0.0f;
	const ImColor metrics_color(255, 255, 255, 255);

	const std::string reads_text_utf8 = string_To_UTF8(reads_text);
	const float reads_width = ImGui::GetFont()->CalcTextSizeA(metrics_font_size, metrics_text_max_width, 0.0f, reads_text_utf8.c_str()).x;
	std::string fps_text;
	float fps_width = 0.0f;
	if (fps_display > 0) {
		fps_text = "游戏帧率: " + std::to_string(fps_display);
		const std::string fps_text_utf8 = string_To_UTF8(fps_text);
		fps_width = ImGui::GetFont()->CalcTextSizeA(metrics_font_size, metrics_text_max_width, 0.0f, fps_text_utf8.c_str()).x;
	}

	const float total_width = reads_width + (fps_width > 0.0f ? metrics_spacing + fps_width : 0.0f);
	const float start_x = (Gui.Window.Size.x - total_width) * 0.5f;

	//std::string reads_text_utf8 = string_To_UTF8(reads_text);
	//ImGui::GetForegroundDrawList()->AddText(ImVec2(100.0f, 10.0f), ImColor(255, 255, 255, 255), reads_text_utf8.c_str());
	
	Gui.StrokeText(reads_text, Vec2{ start_x, metrics_y }, metrics_color, metrics_font_size, false, metrics_bold_offset);
	if (fps_width > 0.0f) {
		Gui.StrokeText(fps_text, Vec2{ start_x + reads_width + metrics_spacing, metrics_y }, metrics_color, metrics_font_size, false, metrics_bold_offset);
	}
	

	if (data_buffers.WaitForData(std::chrono::milliseconds(10))) {
		const auto& readBuffer = data_buffers.GetReadBuffer();
		const int current_index = data_buffers.GetCurrentIndex();
		vector<Vec2> m_bonesData;
		{

			m_bonesData = 全_骨骼数据;
		}
		// 仅处理新数据
		if (current_index != last_read_index) {
			last_read_index = current_index;

			// 遍历玩家数据
			for (const auto& CurList : readBuffer)
			{
				const bool isSoul = CurList.IsSoul;
				if (isSoul && !Function::ESP::灵魂) {
					continue;
				}
				if (CurList.ISRobot && CurList.O_Dis > 100) {
					continue;
				}
				const bool canDrawHp = CurList.O_MaxHp > 0;

				if (WorldToScreen(MATRIX, CurList.O_Pos, PM, RW))
				{
					//MyLog("坐标：", PM.x, PM.y);
					if (Function::ESP::方框)
					{
						Gui.Rectangle({ RW.x,RW.y }, { RW.w,RW.z }, (CurList.ISRobot ? Function::ESP::人机颜色 : Function::ESP::玩家颜色), 1.f);
					}
					if (Function::ESP::手持)
					{
						string Name = GetWeaponData(CurList.O_WeaponID).WeaponName;
						ImColor Color = GetWeaponData(CurList.O_WeaponID).WeaponColor;
						float TextSize = ImGui::CalcTextSize(string_To_UTF8(Name).c_str()).x;
						Gui.StrokeText(Name.c_str(), { PM.x - TextSize / 2,PM.y }, Color, Function::ESP::userTextSize);

					}
					if (Function::ESP::骨骼 && !isSoul)
					{
						DrawBones(CurList);
					}
					if (Function::ESP::信息)
					{
						int 怒气 = ((float)CurList.O_CurAnger / 75000) * 100;
						std::string Iffo = "T:" + std::to_string(CurList.Team) + " | " + CurList.HeroName.c_str() + " | " + std::to_string(怒气) + "%%";
						float TextSize = ImGui::CalcTextSize(string_To_UTF8(Iffo).c_str()).x;
						float NameSize = ImGui::CalcTextSize(string_To_UTF8(CurList.PlayerName).c_str()).x;

						if (Function::ESP::血条样式 == 0) { 信息Y轴 = 30; }
						if (Function::ESP::血条样式 == 1) { 信息Y轴 = 25; }
						if (Function::ESP::血条样式 == 2) { 信息Y轴 = 25; }
						Gui.StrokeText(Iffo, { PM.x - TextSize / 2, RW.y - 信息Y轴 }, Function::ESP::信息颜色, Function::ESP::userTextSize+3);
						if (Function::ESP::名字) { Gui.StrokeText(CurList.PlayerName, { PM.x - NameSize / 2, RW.y - 信息Y轴 - 15 }, (CurList.ISRobot ? Function::ESP::人机颜色 : Function::ESP::信息颜色), Function::ESP::userTextSize); }
						
					}
					if (Function::ESP::血条 && canDrawHp)
					{
						//OB
						if (Function::ESP::血条样式 == 0)Gui.DrawArmorBar(RW, CurList.O_CurHp, CurList.O_MaxHp, CurList.O_Curshield, CurList.O_Maxshield);
						//自定义1
						if (Function::ESP::血条样式 == 1)Gui.PlayerHealth(RW, CurList.O_CurHp, CurList.O_MaxHp, CurList.O_Curshield, CurList.O_Maxshield);
						//自定义2
						if (Function::ESP::血条样式 == 2)Gui.PlayerHealth2(RW, CurList.O_CurHp, CurList.O_MaxHp, CurList.O_Curshield, CurList.O_Maxshield);


					}
					if (Function::ESP::射线)
					{
						Gui.Line({ Gui.Window.Size.x / 2,-Gui.Window.Size.y / 2 }, { RW.x ,RW.y }, (CurList.ISRobot ? Function::ESP::人机颜色 : Function::ESP::玩家颜色), 0.5f);

					}
				}

				if (Function::ESP::预警 && !isSoul)
				{
					if (!CurList.O_Visible)
					{
						盲区外敌人数量++;
						if (CurList.O_Dis < 盲区外最近敌人距离)
						{
							盲区外最近敌人距离 = CurList.O_Dis;
							盲区最近敌人职业 = CurList.HeroName.c_str();
							盲区敌人怒气 = CurList.O_CurAnger;
							if (盲区敌人怒气 == 75000)
							{
								盲区最近敌人是否有大 = "有大";
							}
							else 盲区最近敌人是否有大 = "没大";
						}


					}
				}

			}
			if (Function::ESP::预警)
			{
				if (Function::ESP::预警样式 == 0) { 预警Y轴 += 400; }
				if (Function::ESP::预警样式 == 1) { 预警Y轴 -= 400; }
				if (盲区外敌人数量 == 0) return;
				string Info = "盲区敌人[ " + to_string(盲区外敌人数量) + " ]" + " 最近距离[ " + to_string((int)盲区外最近敌人距离) + " ]" + " 玩家职业[ " + 盲区最近敌人职业 + " ]" + " 是否有大[ " + 盲区最近敌人是否有大 + " ]";
				float TextSize = ImGui::CalcTextSize(string_To_UTF8(Info).c_str()).x;
				Gui.StrokeText(Info, Vec2{ (Gui.Window.Size.x / 2) - TextSize / 1.6f ,Gui.Window.Size.y / 2 - 预警Y轴 }, ImColor{ 255,0,0 }, Function::ESP::userTextSize+3);
			}
		}
	}
	

} 




void DrawBones(PlayerData obj) {
	Bone_D boneData;
	Vector3 NeckPosition = GetBonePosition(obj.O_ActorModel, boneData.脖子);
	Vector3 WaistPosition = GetBonePosition(obj.O_ActorModel, boneData.盆骨);

	Vector3 previous, current;
	Vec2 p1, c1;
	for (auto& a : boneData.BoneList) {
		previous = Vector3{ 0.f, 0.f, 0.f };
		for (int bone : a) {
			current = bone == boneData.脖子 ? NeckPosition : (bone == boneData.盆骨 ? WaistPosition : GetBonePosition(obj.O_ActorModel, bone));
			if (previous.x == 0.f) {
				previous = current;
				continue;
			}
			WorldToScreen_AIM(MATRIX, previous, p1);
			WorldToScreen_AIM(MATRIX, current, c1);
			Gui.Line(Vec2(p1.x, p1.y), Vec2(c1.x, c1.y), (obj.ISRobot ? Function::ESP::人机颜色 : Function::ESP::玩家颜色), 2); 
			previous = current;
		}
	}
}

void ItemESP()
{
	std::vector<ItemInfoData> m_ListData = 全_物品数据;
	Vec2 PM;
	Vec4 RW;

	bool 全护甲显示 = false;
	/*
	if (Function::ESP::换甲护甲 && LocalPlayer_Data.MyCurshield < 300) {
		全护甲显示 = true;
	}
	*/

	// 建立 ItemType -> 显示开关 的映射（静态表，仅绑定布尔地址）
	static const std::unordered_map<int, bool*> itemTypeToSwitch = {
		{1,  &Function::ESP::盒子},
		{2,  &Function::ESP::果实},
		{3,  &Function::ESP::秘籍},
		{14, &Function::ESP::萤火虫},
		{10, &Function::ESP::护甲Lv1},
		{11, &Function::ESP::护甲Lv2},
		{12, &Function::ESP::护甲Lv3},
		{13, &Function::ESP::护甲Lv4},
		{15, &Function::ESP::护甲Lv5},
		{4,  &Function::ESP::道具},
		{23, &Function::ESP::扩容},
		{24, &Function::ESP::扩容},
		{25, &Function::ESP::扩容},
		{5,  &Function::ESP::近战_白},
		{17, &Function::ESP::近战_蓝},
		{18, &Function::ESP::近战_紫},
		{19, &Function::ESP::近战_金},
		{6,  &Function::ESP::远程_白},
		{20, &Function::ESP::远程_蓝},
		{21, &Function::ESP::远程_紫},
		{22, &Function::ESP::远程_金},
		{7,  &Function::ESP::夺魂},
		{8,  &Function::ESP::金魂},
		{99, &Function::ESP::所有对象},
	};

	for (auto& CurList : m_ListData)
	{
		if (!WorldToScreen(MATRIX, CurList.Pos, PM, RW))
			continue;
		if (Function::Flag::显示未知物品ID && CurList.ItemType == 0) {
			Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			continue;
		}

		// -------------------------------
		// 判断是否显示逻辑
		bool shouldDisplay = false;

		// 1?? 若全护甲显示并且类型在护甲范围内 (10~15)
		if (全护甲显示 && CurList.ItemType >= 10 && CurList.ItemType <= 15) {
			shouldDisplay = true;
		}
		else {
			// 2?? 否则根据配置开关判断
			auto it = itemTypeToSwitch.find(CurList.ItemType);
			if (it != itemTypeToSwitch.end() && *(it->second)) {
				shouldDisplay = true;
			}
		}

		if (Function::ESP::智能扩容 && CurList.ItemType >= 23 && CurList.ItemType <= 25) {
			if (CurList.ItemType == 23) {
				// 背包
				if(LocalPlayer_Data.myNowBag < 12){
					shouldDisplay = true;
				}
			}else if(CurList.ItemType == 24) {
				// 武器
				if (LocalPlayer_Data.myNowBagWeapon < 4) {
					shouldDisplay = true;
				}
			}
			else if (CurList.ItemType == 25) {
				// 魂玉
				if (LocalPlayer_Data.myNowBagsoul < 6) {
					shouldDisplay = true;
				}
			}

		}

		// -------------------------------
		if (shouldDisplay) {
			Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
		}
	}
}



void InteractiveESP()
{
	vector<InteractiveInfoData>m_ListData;
	{
		std::lock_guard<std::mutex> lock(g_InteractiveMutex);
		m_ListData = 全_金堆数据;
	}
	Vec2 PM;
	Vec4 RW;
	for (int i = 0; i < m_ListData.size(); i++)
	{
		auto& CurList = m_ListData.at(i);
		if (WorldToScreen(MATRIX, CurList.Pos, PM, RW))
		{
			if (Function::ESP::蓝堆 && CurList.ItemType == 2)
			{
				Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			}
			if (Function::ESP::绿堆 && CurList.ItemType == 1)
			{
				Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			}
			if (Function::ESP::金堆 && CurList.ItemType == 3)
			{
				Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			}
			if (Function::ESP::任务 && CurList.ItemType == 4)
			{
				Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			}
			if (Function::Flag::显示未知堆ID && CurList.ItemType == 0)
			{
				Gui.StrokeText(CurList.Name, PM, CurList.Color, Function::ESP::daoJuTextSize);
			}


		}

	}


}
