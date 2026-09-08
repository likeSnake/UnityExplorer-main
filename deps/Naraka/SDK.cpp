#pragma once
#include "Data.h"
#include "log.h"
#include <unordered_set>


//世界坐标转屏幕坐标
bool WorldToScreen(float MATRI[4][4], Vector3 Pos, Vec2& PM, Vec4& RW)
{
	float SightX = Gui.Window.Size.x / 2, SightY = Gui.Window.Size.y / 2;

	float fViewW = MATRI[0][3] * Pos.x + MATRI[1][3] * Pos.y + MATRI[2][3] * Pos.z + MATRI[3][3];

	if (fViewW < 0.01f) { return false; }
	fViewW = 1 / fViewW;
	PM.x = SightX + (MATRI[0][0] * Pos.x + MATRI[1][0] * Pos.y + MATRI[2][0] * Pos.z + MATRI[3][0]) * fViewW * SightX;
	PM.y = SightY - (MATRI[0][1] * Pos.x + MATRI[1][1] * Pos.y + MATRI[2][1] * Pos.z + MATRI[3][1]) * fViewW * SightY;
	float ViewY = SightY - (MATRI[0][1] * Pos.x + MATRI[1][1] * Pos.y + MATRI[2][1] * Pos.z + MATRI[3][1]) * fViewW * SightY;

	RW.y = SightY - (MATRI[0][1] * Pos.x + MATRI[1][1] * (Pos.y + 1.8) + MATRI[2][1] * Pos.z + MATRI[3][1]) * fViewW * SightY;
	RW.z = ViewY - RW.y;
	RW.w = RW.z / 2;
	RW.x = PM.x - RW.z / 4;

	return true;
}

//AIM世界坐标转屏幕坐标
bool WorldToScreen_AIM(float MATRI[4][4], Vector3 Pos, Vec2& PM)
{
	float SightX = Gui.Window.Size.x / 2, SightY = Gui.Window.Size.y / 2;

	float fViewW = MATRI[0][3] * Pos.x + MATRI[1][3] * Pos.y + MATRI[2][3] * Pos.z + MATRI[3][3];

	if (fViewW < 0.01f) { return false; }
	fViewW = 1 / fViewW;
	PM.x = SightX + (MATRI[0][0] * Pos.x + MATRI[1][0] * Pos.y + MATRI[2][0] * Pos.z + MATRI[3][0]) * fViewW * SightX;
	PM.y = SightY - (MATRI[0][1] * Pos.x + MATRI[1][1] * Pos.y + MATRI[2][1] * Pos.z + MATRI[3][1]) * fViewW * SightY;


	return true;
}
struct WeaponParams
{
	uint32_t WeaponID;
	std::string WeaponName;
	ImColor WeaponColor;
};

const std::vector<WeaponParams> weaponParams = {
	// 近战
	{3209901, "拳头", ImColor{237, 221, 5}},	 // 金色
	{3201510, "双截棍", ImColor{202, 202, 202}}, // 银白
	{3201520, "双截棍", ImColor{82, 136, 255}},	 // 天蓝
	{3201530, "双截棍", ImColor{189, 66, 248}},	 // 暗紫
	{3201540, "双截棍", ImColor{237, 221, 5}},	 // 金色
	{3200001, "长剑", ImColor{202, 202, 202}},	 // 银白
	{3200011, "长剑", ImColor{82, 136, 255}},	 // 天蓝
	{3200021, "长剑", ImColor{189, 66, 248}},	 // 暗紫
	{3200031, "长剑", ImColor{237, 221, 5}},	 // 金色
	{3200071, "长剑", ImColor{255, 0, 0}},	 // 红色
	{3200300, "太刀", ImColor{202, 202, 202}},	 // 银白
	{3200310, "太刀", ImColor{82, 136, 255}},	 // 天蓝
	{3200020, "太刀", ImColor{189, 66, 248}},	 // 暗紫
	{3200030, "太刀", ImColor{237, 221, 5}},	 // 金色
	{3200400, "阔刀", ImColor{202, 202, 202}},	 // 银白
	{3200410, "阔刀", ImColor{82, 136, 255}},	 // 天蓝
	{3200420, "阔刀", ImColor{189, 66, 248}},	 // 暗紫
	{3200430, "阔刀", ImColor{237, 221, 5}},	 // 金色
	{3200200, "长枪", ImColor{202, 202, 202}},	 // 银白
	{3200210, "长枪", ImColor{82, 136, 255}},	 // 天蓝
	{3200220, "长枪", ImColor{189, 66, 248}},	 // 暗紫
	{3200230, "长枪", ImColor{237, 221, 5}},	 // 金色
	{3200101, "匕首", ImColor{202, 202, 202}},	 // 银白
	{3200111, "匕首", ImColor{82, 136, 255}},	 // 天蓝
	{3200121, "匕首", ImColor{189, 66, 248}},	 // 暗紫
	{3200131, "匕首", ImColor{237, 221, 5}},	 // 金色
	{3201610, "双刀", ImColor{202, 202, 202}},	 // 银白
	{3201620, "双刀", ImColor{82, 136, 255}},	 // 天蓝
	{3201630, "双刀", ImColor{189, 66, 248}},	 // 暗紫
	{3201640, "双刀", ImColor{237, 221, 5}},	 // 金色
	{3201810, "长棍", ImColor{202, 202, 202}},	 // 银白
	{3201820, "长棍", ImColor{82, 136, 255}},	 // 天蓝
	{3201830, "长棍", ImColor{189, 66, 248}},	 // 暗紫
	{3201840, "长棍", ImColor{237, 221, 5}},	 // 金色
	{3201710, "斩马刀", ImColor{202, 202, 202}}, // 银白
	{3201720, "斩马刀", ImColor{82, 136, 255}},	 // 天蓝
	{3201730, "斩马刀", ImColor{189, 66, 248}},	 // 暗紫
	{3201740, "斩马刀", ImColor{237, 221, 5}},	 // 金色
	{3201910, "双戟", ImColor{202, 202, 202}},	 // 银白
	{3201920, "双戟", ImColor{82, 136, 255}},	 // 天蓝
	{3201930, "双戟", ImColor{189, 66, 248}},	 // 暗紫
	{3201940, "双戟", ImColor{237, 221, 5}},	 // 金色
	{3202010, "扇子", ImColor{202, 202, 202}},	 // 银白
	{3202020, "扇子", ImColor{82, 136, 255}},	 // 天蓝
	{3202030, "扇子", ImColor{189, 66, 248}},	 // 暗紫
	{3202040, "扇子", ImColor{237, 221, 5}},	 // 金色
	{3202210, "横刀", ImColor{202, 202, 202}},	 // 银白
	{3202220, "横刀", ImColor{82, 136, 255}},	 // 天蓝
	{3202230, "横刀", ImColor{189, 66, 248}},	 // 暗紫
	{3202240, "横刀", ImColor{237, 221, 5}},	 // 金色
	{3202310, "拳刃", ImColor{202, 202, 202}},	 // 银白
	{3202320, "拳刃", ImColor{82, 136, 255}},	 // 天蓝
	{3202330, "拳刃", ImColor{189, 66, 248}},	 // 暗紫
	{3202340, "拳刃", ImColor{237, 221, 5}},	 // 金色
	{3202410, "飞刀", ImColor{202, 202, 202}},	 // 银白
	{3202420, "飞刀", ImColor{82, 136, 255}},	 // 天蓝
	{3202430, "飞刀", ImColor{189, 66, 248}},	 // 暗紫
	{3202440, "飞刀", ImColor{237, 221, 5}},	 // 金色

	{3202510, "链剑白", ImColor{202, 202, 202}}, // 银白
	{3202520, "链剑蓝", ImColor{82, 136, 255}},	 // 天蓝
	{3202530, "链剑紫", ImColor{189, 66, 248}},	 // 暗紫
	{3202540, "链剑金", ImColor{237, 221, 5}},	 // 金色
	// 远程
	{3201300, "万刃轮", ImColor{202, 202, 202}}, // 银白
	{3201310, "万刃轮", ImColor{82, 136, 255}},	 // 天蓝
	{3201320, "万刃轮", ImColor{189, 66, 248}},	 // 暗紫
	{3201330, "万刃轮", ImColor{237, 221, 5}},	 // 金色
	{3201000, "一窝蜂", ImColor{202, 202, 202}}, // 银白
	{3201010, "一窝蜂", ImColor{82, 136, 255}},	 // 天蓝
	{3201020, "一窝蜂", ImColor{189, 66, 248}},	 // 暗紫
	{3201030, "一窝蜂", ImColor{237, 221, 5}},	 // 金色
	{3200700, "连弩", ImColor{202, 202, 202}},	 // 银白
	{3200710, "连弩", ImColor{82, 136, 255}},	 // 天蓝
	{3200720, "连弩", ImColor{189, 66, 248}},	 // 暗紫
	{3200730, "连弩", ImColor{237, 221, 5}},	 // 金色
	{3201200, "鸟铳", ImColor{202, 202, 202}},	 // 银白
	{3201210, "鸟铳", ImColor{82, 136, 255}},	 // 天蓝
	{3201220, "鸟铳", ImColor{189, 66, 248}},	 // 暗紫
	{3201230, "鸟铳", ImColor{237, 221, 5}},	 // 金色
	{3201100, "五眼铳", ImColor{202, 202, 202}}, // 银白
	{3201110, "五眼铳", ImColor{82, 136, 255}},	 // 天蓝
	{3201120, "五眼铳", ImColor{189, 66, 248}},	 // 暗紫
	{3201130, "五眼铳", ImColor{237, 221, 5}},	 // 金色
	{3200900, "喷火筒", ImColor{202, 202, 202}}, // 银白
	{3200910, "喷火筒", ImColor{82, 136, 255}},	 // 天蓝
	{3200920, "喷火筒", ImColor{189, 66, 248}},	 // 暗紫
	{3200930, "喷火筒", ImColor{237, 221, 5}},	 // 金色
	{3200800, "火炮", ImColor{202, 202, 202}},	 // 银白
	{3200810, "火炮", ImColor{82, 136, 255}},	 // 天蓝
	{3200820, "火炮", ImColor{189, 66, 248}},	 // 暗紫
	{3200830, "火炮", ImColor{237, 221, 5}},	 // 金色
	{3200501, "弓箭", ImColor{202, 202, 202}},	 // 银白
	{3200511, "弓箭", ImColor{82, 136, 255}},	 // 天蓝
	{3200521, "弓箭", ImColor{189, 66, 248}},	 // 暗紫
	{3200531, "弓箭", ImColor{237, 221, 5}},	// 金色

};

WeaponData GetWeaponData(uint32_t WeaponID)
{
	WeaponData Temp;
	for (const auto& param : weaponParams)
	{
		if (param.WeaponID == WeaponID)
		{
			Temp.WeaponName = param.WeaponName;
			Temp.WeaponColor = param.WeaponColor;
			break;
		}
	}
	return Temp;
}
// 获取角色类型
string GetHeroType(int ID)
{
	if (ID == 1000001) { return  "胡桃"; }
	if (ID == 1000003) { return  "宁红夜"; }
	if (ID == 1000004) { return  "迦南"; }
	if (ID == 1000005) { return  "特木尔"; }
	if (ID == 1000006) { return  "季沧海"; }
	if (ID == 1000007) { return  "天海"; }
	if (ID == 1000008) { return  "大佛"; }
	if (ID == 1000009) { return  "妖刀姬"; }
	if (ID == 1000010) { return  "崔三娘"; }
	if (ID == 1000011) { return  "岳山"; }
	if (ID == 1000012) { return  "神将"; }
	if (ID == 1000013) { return  "无尘"; }
	if (ID == 1000015) { return  "顾清寒"; }
	if (ID == 1000016) { return  "武田信忠"; }
	if (ID == 1000017) { return  "殷紫萍"; }
	if (ID == 1000018) { return  "沈妙"; }
	if (ID == 1000019) { return  "铁卫"; }
	if (ID == 1000020) { return  "胡为"; }
	if (ID == 1000021) { return  "季莹莹"; }
	if (ID == 1000022) { return  "玉玲珑"; }
	if (ID == 1000023) { return  "哈迪"; }
	if (ID == 1000024) { return  "魏轻"; }
	if (ID == 1000025) { return  "刘炼"; }
	if (ID == 1000026) { return  "张启灵"; }
	if (ID == 1000027) { return  "席拉"; }
	if (ID == 1000028) { return  "彩戏师"; }
	if (ID == 1000031) { return  "李寻欢"; }
	return"";
}


void CharacterManager::GetCharacterManagerPtr()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_CharacterManager);
	Manager = mem.Read<uintptr_t>(Manager + 0xB8);
	CharacterManagerPtr = mem.Read<uintptr_t>(Manager + 0x8);
}

// 获取游戏ping
float CharacterManager::GamePing()
{
	float shrtt = mem.Read<float>(CharactorSyncManagerPtr + Offset::ActorModel.shrtt);//Class: CharactorSync->Single shrtt
	//MyLog("游戏延迟:", shrtt);
	if (shrtt > 0.0f)
	{
		shrtt = (shrtt * 2.0f) * 1000.0f;
		return shrtt;
	}
	return 0.f;
}
uintptr_t CharacterManager::GetCharactorSyncManager()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_CharactorSync);
	Manager = mem.Read<uintptr_t>(Manager + 0xB8);
	CharactorSyncManagerPtr = mem.Read<uintptr_t>(Manager + 8);
	return CharactorSyncManagerPtr;
}
// 缓存角色数量和角色列表指针
void CharacterManager::Cache_PlayerCount_ActorModelList()
{
	auto Addr = mem.Read<uintptr_t>(CharacterManagerPtr + Offset::Character.AllAliveCharList);
	Cache_PlayerCount_ActorModelList_Ptr = mem.Read<uintptr_t>(Addr + 0x18);
}
// 获取角色数量
int CharacterManager::GetPlayerCount()
{
	return mem.Read<int>(Cache_PlayerCount_ActorModelList_Ptr + 0x18);
}
//获取本人角色指针
ActorModel* CharacterManager::GetLocalActorModel()
{
	return mem.Read<ActorModel*>(CharacterManagerPtr + 0x18);
}
//获取角色模型列表
ActorModel* CharacterManager::GetActorModelList(int index)
{
	return mem.Read<ActorModel*>(Cache_PlayerCount_ActorModelList_Ptr + Offset::Character.ArrayStart + index * 0x18ull);
}
//是否可见
bool ActorModel::IsVisible()
{
	return mem.Read<bool>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.cullingVisible); // Class: ActorModel->Boolean cullingVisible
}
// 是否存活
bool ActorModel::IsAlive()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + 0x38) == 0x101;
}
// 缓存事件轨道指针
uintptr_t ActorModel::Cache_GetEventTracksForLayer()
{
	auto temp = mem.Read<uint64_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ObjectMsg);
	return  mem.Read<uint64_t>(temp + Offset::ActorModel.EventTracksForLayer);


}

uintptr_t Cache_GetEventTracksForLayer(uintptr_t EventTracksForLayer)
{
	auto temp = mem.Read<uint64_t>(EventTracksForLayer + 0x20);
	return   temp = mem.Read<uint64_t>(temp + 0x10);
}

// 获取角色招式名称
//std::wstring GetActorName(uint64_t EventTracksForLayerPtr)
//{
//	
//	/*uint64_t temp = mem.Read<uint64_t>(EventTracksForLayerPtr + 0x10);*/
//	/*int NameLen = mem.Read<int>(temp + 0x10);*/
//	wchar_t Name[0xFF] = { '\0' };
//	mem.Read(EventTracksForLayerPtr + 0x14, &Name, /*NameLen * 2*/0x80);
//
//	return Name;
//}

// 获取角色坐标指针
uintptr_t ActorModel::Cache_GetPosPtr()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.Transform);
	temp = mem.Read<uintptr_t>(temp + 0x10);
	temp = mem.Read<uintptr_t>(temp + 0x30);
	temp = mem.Read<uintptr_t>(temp + 0x30);
	temp = mem.Read<uintptr_t>(temp + 0x08);
	return mem.Read<uintptr_t>(temp + 0x38);
}


// 获取角色坐标
Vector3 ActorModel::GetPos(uintptr_t PosPtr)
{

	return mem.Read<Vector3>(PosPtr + 0x90);
}

// 缓存行动根指针
uintptr_t ActorModel::Cache_GetActorRootPtr()
{
	auto BackingField = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.Transform);
	auto CachedPtr = mem.Read<uintptr_t>(BackingField + 0x10);
	return mem.Read<uintptr_t>(CachedPtr + 0x38);
}

uintptr_t ActorModel::Cache_GetActorNamePtr()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKit);
	temp = mem.Read<uintptr_t>(temp + 0x68);
	temp = mem.Read<uintptr_t>(temp + 0x68);
	temp = mem.Read<uintptr_t>(temp + 0x28);
	temp = mem.Read<uintptr_t>(temp + 0x30);
	temp = mem.Read<uintptr_t>(temp + 0x20);
	return mem.Read<uintptr_t>(temp + 0x10);

}

// 取招式名称
std::wstring GetActorName(uintptr_t Cache_GetActorNamePtr)
{
	wchar_t Name[80] = { NULL };

	mem.Read(Cache_GetActorNamePtr + 0x14, &Name, sizeof(Name));
	return std::wstring(Name);

}

// 获取角色武器类型
WeaponType ActorModel::GetActorWeapon()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorWeapon);
	return mem.Read<WeaponType>(temp + Offset::ActorModel.WeaponType);// Class: ActorWeapon->Int32 WeaponType
}

// 获取角色武器类型
uintptr_t ActorModel::Cache_GetActorWeaponPtr()
{
	return  mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorWeapon);
}
// 获取角色属性数据
ActorModelPropertyData* ActorModel::GetActorModelPropertyData()
{
	return mem.Read<ActorModelPropertyData*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.PropertyData);
}
//取行动
ActorKit* ActorModel::GetActorKit()
{
	return mem.Read<ActorKit*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKit);
}
// 获取角色运行时属性数据
RuntimePropertyData* ActorModel::GetRuntimePropertyData()
{
	return mem.Read<RuntimePropertyData*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.RuntimePropertyData);
}
// 获取角色消息
ObjectMessenger* ActorModel::GetObjectMessenger()
{
	return mem.Read<ObjectMessenger*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ObjectMsg);
}
// 获取角色是否为机器人
bool ActorModelPropertyData::GetISRobotId()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ProPerty.RobotId) != NULL;
}
// 获取角色队伍
int ActorModelPropertyData::GetTeam()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ProPerty.Group);
}
// 获取角色英雄ID
HeroType ActorModelPropertyData::GetHeroTID()
{
	return mem.Read<HeroType>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ProPerty.Heroid);
}
// 获取英雄状态灵魂
int ActorModelPropertyData::GetHeroSoul()
{
	return mem.Read<HeroType>(reinterpret_cast<uintptr_t>(this) + 0x14c);
}
// 获取角色武器ID
int ActorModelPropertyData::GetWeaponID()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ProPerty.WeaponId);
}
// 获取角色名字Ascii
std::string UnicodeToAscii(CONST std::wstring& wstr) {
	// 预算-缓冲区中多字节的长度    
	int ansiiLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// 给指向缓冲区的指针变量分配内存    
	char* pAssii = (char*)malloc(sizeof(char) * ansiiLen);
	// 开始向缓冲区转换字节    
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	std::string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}
// 获取角色同步管理器指针
void ActorModelPropertyData::GetCharactorSyncManager()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_CharactorSync);
	Manager = mem.Read<uintptr_t>(Manager + 0xB8);
	CharactorSyncManagerPtr = mem.Read<uintptr_t>(Manager);
}

// 获取游戏ping
float ActorModelPropertyData::GetPing()
{
	float shrtt = mem.Read<float>(CharactorSyncManagerPtr + Offset::ActorModel.shrtt);//Class: CharactorSync->Single shrtt
	//MyLog("游戏延迟:", shrtt);
	if (shrtt > 0.0f)
	{
		shrtt = (shrtt * 2.0f) * 1000.0f;
		return shrtt;
	}
	return 0.f;
}

// 获取角色名字
std::string ActorModelPropertyData::GetPlayerName()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ProPerty.PlayerName);
	//auto NameLen = mem.Read<int>(temp + 0x10);
	wchar_t Name[0xFF] = { '\0' };
	mem.Read(temp + 0x14, &Name, 0x20);
	return UnicodeToAscii(Name).c_str();
}
// 缓存角色蓄力时间
uintptr_t ActorKit::Cache_BlueTime()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.BackingField);
	temp = mem.Read<uintptr_t>(temp + 0x18);
	temp = mem.Read<uintptr_t>(temp + 0x20);
	return mem.Read<uintptr_t>(temp + 0x10);
}
// 缓存角色行动物理
uintptr_t ActorKit::Cache_ActorPhysics()
{
	return mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.ActorPhysics);
}

int ActorKit::GetCurrentCollisionFlags()
{
	auto Cache_ActorPhysics = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.ActorPhysics);
	return mem.Read<int>(Cache_ActorPhysics + 0xcc);
}
//获取角色蓄力时间
float ActorKit::BlueTime(uintptr_t Cache_Bluetime_PTR)
{
	return mem.Read<float>(Cache_Bluetime_PTR + 0x24 + 3 * 0xC);
}
// 获取角色状态指针
ActionHitSimulateData* ActorKit::GetActionHitSimulateData()
{
	return mem.Read<ActionHitSimulateData*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulateData);
}
//
ActionHitSimulatePtr* ActorKit::GetActionHitSimulatePtr()
{
	return mem.Read<ActionHitSimulatePtr*>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulatePtr);
}

uintptr_t ActorKit::GetCameraAgent()
{
	return mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.EntityKitS.CameraAgent);
}

uintptr_t GetCameraController(uintptr_t cameraAgentPtr)
{
	if (!cameraAgentPtr)
	{
		return NULL;
	}
	return mem.Read<uintptr_t>(cameraAgentPtr + Offset::ActorModel.CameraAgentS.CameraController);
}

uintptr_t GetAdventureRig(uintptr_t cameraControllerPtr)
{
	if (!cameraControllerPtr)
	{
		return NULL;
	}
	return mem.Read<uintptr_t>(cameraControllerPtr + Offset::ActorModel.LXCameraControllerS.AdventureRig);
}

// 获取角色当前出招状态
ActionType ActionHitSimulateData::GetEndureLevel()
{
	return mem.Read<ActionType>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel);
}
// 获取角色当前出刀动作 为2则是拼刀状态，1是磐石
int ActionHitSimulateData::GetReactionType()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType);
}
// 获取角色攻击状态
int ActionHitSimulateData::GetRangeReactionType()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType);
}


//人物僵直
float ActionHitSimulateData::GetActionDuration()
{
	return mem.Read<float>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration);
}

//振刀状态
int ActionHitSimulateData::GetReactionExtraType()
{
	return mem.Read<float>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType);
}
//蓄转状态
int ActionHitSimulatePtr::GetXuLiState()
{
	return mem.Read<int>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel.ActorKitS.HitSimulate.XuListate);
}
// 获取角色属性数据ID
PropertyDataID RuntimePropertyData::GetPropertyDataID()
{
	auto Data = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x10);//Class: RuntimePlayerPropertyData->HashSet`1 NotInDieStatusSet
	Data = mem.Read<uintptr_t>(Data + 0x10);
	Data = mem.Read<uintptr_t>(Data + 0x18);
	auto DictCount = mem.Read<int>(Data + 0x18);
	int Index = 0;
	PropertyDataID temp = { 0 };
	for (size_t i = 0; i < DictCount; i++)
	{
		uintptr_t ID1 = Data + 0x28 + i * 0x10;

		uintptr_t ID2 = ID1 + 4;

		auto DataID = mem.Read<int>(ID1);

		auto TempID = mem.Read<int>(ID2);

		if (DataID == 11001) { temp.CurHpID = TempID; Index += 1; }
		if (DataID == 10001) { temp.HpMaxID = TempID; Index += 1; }
		if (DataID == 11024) { temp.CurshieldID = TempID; Index += 1; }
		if (DataID == 10107) { temp.shieldMaxID = TempID; Index += 1; }
		if (DataID == 11002) { temp.CurEnergyID = TempID; Index += 1; }
		if (DataID == 10006) { temp.EnergyMaxID = TempID; Index += 1; }

		if (DataID == 11025) { temp.bagMaxID = TempID; Index += 1; }
		if (DataID == 11026) { temp.weaponMaxID = TempID; Index += 1; }
		if (DataID == 11027) { temp.soulItemMaxID = TempID; Index += 1; }
		// 魂冢死亡进度
		if (DataID == 10075) { temp.Rt_be_dying_hp_max = TempID; Index += 1; }
		if (DataID == 11015) { temp.Rt_be_dying_hp = TempID; Index += 1; }
		// 被救援状态
		if (DataID == 11017) { temp.Rt_rescue_status = TempID; Index += 1; }
		// 死亡状态
		if (DataID == 11018) { temp.Rt_cur_die_status = TempID; Index += 1; }
		// 金币
		if (DataID == 11028) { temp.Rt_in_battle_coin = TempID; Index += 1; }
		// Rt_item_bag_max = 10111 // 背包可携带道具格数（物品栏最大容量）
		// Rt_weapon_bag_max = 10114 // 武器栏最大数量或武器背包上限
		// Rt_soul_item_bag_max = 10116 //魂玉物品栏最大数
		// Rt_in_battle_coin = 11028 // 当前战中携带的金币

		if (Index >= 14) { break; }
	}
	return temp;
}
// 缓存角色属性数据
uintptr_t RuntimePropertyData::Cache_PropertData()
{
	auto Data = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x10);
	Data = mem.Read<uintptr_t>(Data + 0x28);
	return mem.Read<uintptr_t>(Data + 0x10);
}
//获取角色当前HP
int RuntimePropertyData::GetCurHp(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.CurHpID * 4ull);
}
//获取角色最大HP
int RuntimePropertyData::GetHpMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.HpMaxID * 4ull);
}
// 获取角色当前护甲
int RuntimePropertyData::GetCurshield(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.CurshieldID * 4ull);
}
// 获取角色当前最大护甲
int RuntimePropertyData::GetshieldMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.shieldMaxID * 4ull);
}
// 获取角色当前精力
int RuntimePropertyData::GetCurEnergy(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.CurEnergyID * 4ull);
}
// 获取角色最大精力
int RuntimePropertyData::GetEnergyMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x20 + Cache_PropertyDataID.EnergyMaxID * 4ull);
}
// 获取角色当前怒气
int RuntimePropertyData::GetCurAnger(uintptr_t Cache_PropertData_PTR)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x30);
}
// 获取角色最大怒气
int RuntimePropertyData::GetAngerMax(uintptr_t Cache_PropertData_PTR)
{
	return mem.Read<int>(Cache_PropertData_PTR + 0x38);
}
// 缓存全局时间
uintptr_t ObjectMessenger::Cache_GetGlobalTime()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_GlobalTime);
	return mem.Read<uintptr_t>(Manager + 0xB8);
}
// 缓存当前时间
uintptr_t ObjectMessenger::Cache_GetCurTimeForLogicState()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + Offset::ActorModel._curAnimPlayableAgent); // Class: ObjectMessenger->AnimPlayable _curAnimPlayableAgent
	temp = mem.Read<uintptr_t>(temp + Offset::ActorModel._runtimeLogicLayers); // Class: AnimPlayable->RuntimeLogicLayer[] _runtimeLogicLayers
	return mem.Read<uintptr_t>(temp + 0x20);
}
//获取逻辑状态 A 的当前时间
float ObjectMessenger::GetCurTimeForLogicStateA(uintptr_t Cache_GetGlobalTime_PTR, uintptr_t Cache_GetCurTimeForLogicState_PTR)
{
	auto v2 = mem.Read<double>(Cache_GetGlobalTime_PTR + 0x28);
	return (float)(v2 - mem.Read<double>(Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
}
//动画后摇
float GetCurShareStateExitTime(uintptr_t Cache_GetCurTimeForLogicState_PTR)
{
	return  mem.Read<float>(Cache_GetCurTimeForLogicState_PTR + 0x20);
}
//动画前腰
float GetCurShareStateTransitionTime(uintptr_t Cache_GetCurTimeForLogicState_PTR)
{
	return  mem.Read<float>(Cache_GetCurTimeForLogicState_PTR + 0x24);
}
//动画总时间
float GetCurDuration(uintptr_t Cache_GetCurTimeForLogicState_PTR)
{
	return  mem.Read<float>(Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.length);//Class: BattleLineCurve->Single length
}
//当前动画执行时间
float GetCurTimeForLogicState(uintptr_t Cache_GetCurTimeForLogicState_PTR)
{
	return  mem.Read<float>(Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.nowTime);//Class: BattleLineCurve->Double nowTime
}

// 缓存角色名称哈希
uintptr_t ObjectMessenger::Cache_GetNameHash()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x68);
	temp = mem.Read<uintptr_t>(temp + 0x48);
	return mem.Read<uintptr_t>(temp + 0x20);
	//return mem.Read<uintptr_t>(temp + 0x48);
}
// 获取角色动作哈希
unsigned int ObjectMessenger::GetNameHash(uintptr_t Cache_GetNameHash_PTR)
{
	auto temp = mem.Read<uintptr_t>(Cache_GetNameHash_PTR + 0x48);
	return  mem.Read<unsigned int>(temp + 0x18);
}
// 获取人物技能指针
void UserDataManager::GetUserDataManager()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_UserDataManager);
	Manager = mem.Read<uintptr_t>(Manager + Offset::ActorModel.FestivalGiftBagData);
	UserDataManagerPtr = mem.Read<uintptr_t>(Manager);
}
// 获取技能列表
AllSkillData* UserDataManager::GetAllSkillDataList()
{
	auto UserBattleData = mem.Read<uintptr_t>(UserDataManagerPtr + Offset::ActorModel.battleData);
	auto skillData = mem.Read<uintptr_t>(UserBattleData + Offset::ActorModel.skillData);
	auto characterSkillDataDict = mem.Read<uintptr_t>(skillData + Offset::ActorModel.characterSkillDataDict);
	auto AllSkillDataList = mem.Read<uintptr_t>(characterSkillDataDict + 0x18);
	return  mem.Read<AllSkillData*>(AllSkillDataList + 0x30);
}
// Get skill list for a specific fid
AllSkillData* UserDataManager::GetAllSkillDataListByFid(int fid)
{
	if (fid == 0)
	{
		return nullptr;
	}
	auto UserBattleData = mem.Read<uintptr_t>(UserDataManagerPtr + Offset::ActorModel.battleData);
	if (!UserBattleData)
	{
		return nullptr;
	}
	auto skillData = mem.Read<uintptr_t>(UserBattleData + Offset::ActorModel.skillData);
	if (!skillData)
	{
		return nullptr;
	}
	auto characterSkillDataDict = mem.Read<uintptr_t>(skillData + Offset::ActorModel.characterSkillDataDict);
	if (!characterSkillDataDict)
	{
		return nullptr;
	}
	auto entries = mem.Read<uintptr_t>(characterSkillDataDict + 0x18);
	if (!entries)
	{
		return nullptr;
	}
	int entryCount = mem.Read<int>(entries + 0x18);
	for (int i = 0; i < entryCount; ++i)
	{
		auto entry = entries + 0x20 + static_cast<uintptr_t>(i) * 0x18ull;
		int hashCode = mem.Read<int>(entry);
		if (hashCode < 0)
		{
			continue;
		}
		int key = mem.Read<int>(entry + 0x8);
		if (key != fid)
		{
			continue;
		}
		return mem.Read<AllSkillData*>(entry + 0x10);
	}
	return nullptr;
}
// 判断是否为观战模式
bool UserDataManager::IsSpectator()
{
	auto SpectatorData = mem.Read<uintptr_t>(UserDataManagerPtr + Offset::UserData.SpectatorData);
	return  !mem.Read<bool>(SpectatorData + 24);// Boolean isSpectator
}
//是否打开背包
int UserDataManager::IsShowExtraEvent()
{
	auto uiData = mem.Read<uintptr_t>(UserDataManagerPtr + 0x40);// UserDataUI uiData
	auto battleStatsData = mem.Read<uintptr_t>(uiData + 0x38); // UIBattleStatsPanelData battleStatsData
	auto _showExtraEvent = mem.Read<int>(battleStatsData + 0x20);	// BattleStatsPanelShowExtraEvent _showExtraEvent
	return _showExtraEvent;
}
//取1 2 武器	id
uintptr_t UserDataManager::GetWeaponList(int index) {
	auto UserBattleEquipData = mem.Read<uintptr_t>(UserDataManagerPtr + Offset::ActorModel.battleData);
	if (!UserBattleEquipData) {
		return 0;
	}
	auto allPlayerEquipDict = mem.Read<uintptr_t>(UserBattleEquipData + Offset::ActorModel.fighterEquipData);
	allPlayerEquipDict = mem.Read<uintptr_t>(allPlayerEquipDict + 0x10);
	allPlayerEquipDict = mem.Read<uintptr_t>(allPlayerEquipDict + 0x18);
	allPlayerEquipDict = mem.Read<uintptr_t>(allPlayerEquipDict + 0x30);

	allPlayerEquipDict = mem.Read<uintptr_t>(allPlayerEquipDict + 0x28);
	allPlayerEquipDict = mem.Read<uintptr_t>(allPlayerEquipDict + 0x10);
	/*auto EncryptList = mem.Read<uintptr_t>(allPlayerEquipDict + 0x28 + (index * 8));
	mem.Read<int>(EncryptList + 0x14);*/
	return  mem.Read<uintptr_t>(allPlayerEquipDict + 0x28 + (index * 8));
}

//取当前武器索引
uintptr_t UserDataManager::Cache_WeaponIndex()
{
	auto UserBattleEquipData = mem.Read<uintptr_t>(UserDataManagerPtr + Offset::ActorModel.battleData);
	auto equipDict1 = mem.Read<uintptr_t>(UserBattleEquipData + Offset::ActorModel.fighterEquipData);
	auto equipDict2 = mem.Read<uintptr_t>(equipDict1 + 0x10);
	auto equipDict3 = mem.Read<uintptr_t>(equipDict2 + 0x18);
	return mem.Read<uintptr_t>(equipDict3 + 0x30);
}
LocalSkillData UserDataManager::GetSkillID(int HeroID)
{
	return GetSkillID(HeroID, GetAllSkillDataList());
}
// 获取本人技能属性
LocalSkillData UserDataManager::GetSkillID(int HeroID, AllSkillData* AllSkillDataList)
{
	float F_CDTime = 0.f;
	float F_sustainTime = 0.f;
	int F_SkillID = 0;
	int F_SkillState = 0;
	float DZ_CDTime = 0.f;
	float DZ_sustainTime = 0.f;
	int DZ_SkillID = 0;
	int DZ_SkillState = 0;
	if (!AllSkillDataList)
	{
		return LocalSkillData({ 0, 0, 0.f, 0.f, 0, 0, 0.f, 0.f });
	}
	int m_cout = AllSkillDataList->GetCount();
	for (size_t i = 0; i < m_cout; i++)
	{

		auto SkillId = mem.Read<DWORD64>(reinterpret_cast<uintptr_t>(AllSkillDataList) + 0x20);
		SkillId = mem.Read<DWORD64>(SkillId + 0x10);
		SkillId = mem.Read<DWORD64>(SkillId + 0x20 + i * 0x08);

		float LeftCDTime = mem.Read<float>(SkillId + Offset::UserData.ClientLeftCdTime);
		float LeftContinueTime = mem.Read<float>(SkillId + Offset::UserData.ClientLeftContinueTime);
		int State = mem.Read<int>(SkillId + Offset::UserData.skillState);
		if (State == 3)
			continue;

		int m_SkillID = mem.Read<int>(mem.Read<DWORD64>(SkillId + Offset::UserData.logicData) + 0x10);
		//cout << "技能ID: " << m_SkillID << endl;

		switch (HeroID)
		{
		case 1000001: //胡桃
			if (m_SkillID == 1500000 || m_SkillID == 1500002) //F 技能
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500030 || m_SkillID == 1500032) // V技能
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000003: //宁红叶
			if (m_SkillID == 1500130 || m_SkillID == 1500131)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500120 || m_SkillID == 1500122)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000004://迦南
			if (m_SkillID == 1500201 || m_SkillID == 1500204)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500242 || m_SkillID == 1500245)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000005://特木尔
			if (m_SkillID == 1500300 || m_SkillID == 1500301 || m_SkillID == 1500302
				)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500320 || m_SkillID == 1500321 || m_SkillID == 1500322)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000006://季沧海
			if (m_SkillID == 1500404 || m_SkillID == 1500402)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;

			}
			if (m_SkillID == 1500415 || m_SkillID == 1500416)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;

			}
			break;
		case 1000007://天海
			if (m_SkillID == 1500500 || m_SkillID == 1500501)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500510 || m_SkillID == 1500512)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000009://妖刀姬
			if (m_SkillID == 1500700 || m_SkillID == 1500701 || m_SkillID == 1500702)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500720 || m_SkillID == 1500721 || m_SkillID == 1500722)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000010://崔三娘
			if (m_SkillID == 1500800 || m_SkillID == 1500802)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500840 || m_SkillID == 1500845)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000011://岳山
			if (m_SkillID == 1500900 || m_SkillID == 1500902)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1500910 || m_SkillID == 1500912)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000013://无尘
			if (m_SkillID == 1501102 || m_SkillID == 1501114)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501140 || m_SkillID == 1501143)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000015://顾清寒
			if (m_SkillID == 1501300 || m_SkillID == 1501301 || m_SkillID == 1501310)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501330 || m_SkillID == 1501332)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;

			}

			break;
		case 1000016://武田
			if (m_SkillID == 1501400 || m_SkillID == 1501401 || m_SkillID == 1501402 || m_SkillID == 1501423 || m_SkillID == 1501420 || m_SkillID == 1501421)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501410 || m_SkillID == 1501411 || m_SkillID == 1501412)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000017://殷紫萍
			if (m_SkillID == 1501500 || m_SkillID == 1501501)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501510 || m_SkillID == 1501511)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000018://沈妙
			if (m_SkillID == 1501600 || m_SkillID == 1501602)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501610 || m_SkillID == 1501622)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000020://胡为
			if (m_SkillID == 1501800 || m_SkillID == 1501801 || m_SkillID == 1501802 || m_SkillID == 1501830)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501810 || m_SkillID == 1501811 || m_SkillID == 1501812)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000021://季莹莹
			if (m_SkillID == 1501900 || m_SkillID == 1501902)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1501940 || m_SkillID == 1501941)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000022://玉玲珑
			if (m_SkillID == 1502000 || m_SkillID == 1502001)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502010 || m_SkillID == 1502012)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000023://哈迪
			if (m_SkillID == 1502100 || m_SkillID == 1502101 || m_SkillID == 1502121)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502111 || m_SkillID == 1502110)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000024://魏轻
			if (m_SkillID == 1502200 || m_SkillID == 1502201)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502220 || m_SkillID == 1502221)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000025://刘炼
			if (m_SkillID == 1502300 || m_SkillID == 1502302 || m_SkillID == 1502340 || m_SkillID == 1502330)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502311 || m_SkillID == 1502312)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000026://张起灵
			if (m_SkillID == 1502400 || m_SkillID == 1502401)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502410 || m_SkillID == 1502411)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000027://希拉
			if (m_SkillID == 1502500 || m_SkillID == 1502501)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502502 || m_SkillID == 1502503)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		case 1000028://彩戏师
			if (State == 1 && m_SkillID == 1502603 || m_SkillID == 1502605)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			else if (m_SkillID == 1502600 || m_SkillID == 1502601)
			{
				F_CDTime = LeftCDTime;
				F_sustainTime = LeftContinueTime;
				F_SkillID = m_SkillID;
				F_SkillState = State;
			}
			if (m_SkillID == 1502610 || m_SkillID == 1502611)
			{
				DZ_CDTime = LeftCDTime;
				DZ_sustainTime = LeftContinueTime;
				DZ_SkillID = m_SkillID;
				DZ_SkillState = State;
			}
			break;
		default:
			break;
		}
	}
	return LocalSkillData({ F_SkillID, F_SkillState,F_CDTime,F_sustainTime,DZ_SkillID, DZ_SkillState,DZ_CDTime,DZ_sustainTime });
}
LocalSkillData UserDataManager::GetSkillIDByFid(int HeroID, int fid)
{
	return GetSkillID(HeroID, GetAllSkillDataListByFid(fid));
}
// 获取技能列表长度
int AllSkillData::GetCount()
{
	auto temp = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x20);
	return mem.Read<int>(temp + 0x18);
}
// 缓存技能
uintptr_t AllSkillData::Cache_Skill(int index)
{
	auto SkillId = mem.Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x20);
	SkillId = mem.Read<uintptr_t>(SkillId + 0x10);
	return mem.Read<uintptr_t>(SkillId + 0x20 + index * 0x08);
}
// 获取技能ID
int AllSkillData::GetSkillID(uintptr_t Cache_Skill_PTR)
{
	auto SkillId = mem.Read<uintptr_t>(Cache_Skill_PTR + 0x60);
	return mem.Read<int>(SkillId + Offset::UserData.Skill_ID);
}
// 获取技能状态
int AllSkillData::GetSkillState(uintptr_t Cache_Skill_PTR)
{
	return mem.Read<int>(Cache_Skill_PTR + Offset::UserData.skillState);
}
// 获取技能冷却时间
float AllSkillData::Get_LeftCdTime(uintptr_t Cache_Skill_PTR)
{
	return mem.Read<float>(Cache_Skill_PTR + Offset::UserData.ClientLeftCdTime);
}
// 获取技能持续时间
float AllSkillData::Get_LeftContinueTime(uintptr_t Cache_Skill_PTR)
{
	return mem.Read<float>(Cache_Skill_PTR + Offset::UserData.ClientLeftContinueTime);
}
// 获取技能时间钟
float AllSkillData::Get_SkillTime(uintptr_t Cache_Skill_PTR)
{
	return mem.Read<float>(Cache_Skill_PTR + 0x58);
}
// 获取角色同步管理器指针
void CharactorSyncManager::GetCharactorSyncManager()
{
	auto Manager = mem.Read<uintptr_t>(Offset::GameAssembly + Offset::m_CharactorSync);
	Manager = mem.Read<uintptr_t>(Manager + 0xB8);
	CharactorSyncManagerPtr = mem.Read<uintptr_t>(Manager);
}
// 获取游戏ping
float CharactorSyncManager::GamePing()
{
	float shrtt = mem.Read<float>(CharactorSyncManagerPtr + Offset::ActorModel.shrtt);//Class: CharactorSync->Single shrtt
	//MyLog("游戏延迟:", shrtt);
	if (shrtt > 0.0f)
	{
		shrtt = (shrtt * 2.0f) * 1000.0f;
		return shrtt;
	}
	return 0.f;
}

// 获取玩家背包扩格数量
int ActorModelPropertyData::PlayerBag(uintptr_t Cache_PropertData_PTR)
{
	//cout << "调用指针:" << (Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID) << endl;
	int nowBag = mem.Read<int>(Cache_PropertData_PTR + 0x20 + (Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull));
	return nowBag;
}

// 获取玩家背包扩格数量
/*int CharactorSyncManager::PlayerBag(uintptr_t Cache_PropertData_PTR)
{
	int nowBag = mem.Read<int>(Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull);
	return nowBag;
}*/
// 获取游戏窗口大小
Vector2 GetWindowsize() {
	auto Base = mem.Read<ULONG64>(Offset::UnityPlayer + Offset::Base_WindowBase);
	Base = mem.Read<ULONG64>(Base + 0xB8);
	Base = mem.Read<ULONG64>(Base + 0x8);
	int v1 = mem.Read<int>(Base + 20);
	int v2 = mem.Read<int>(Base + 20 + 4);
	return Vector2(v1, v2);
}

BOOL IsLookUp(uintptr_t ActorRootPtr) {
	Vector3 dir = mem.Read<Vector3>(ActorRootPtr + 0xA4);
	float pitch = atan2f(-dir.y, sqrt(dir.x * dir.x + dir.z * dir.z)) * 180.0f / M_PI;
	bool lookUp = pitch > 0; 
	bool lookDown = pitch < 0;
	return lookUp;

	
	/*
	Vector4 quat = mem.Read<Vector4>(ActorRootPtr + 0xA0);
	float pitch360 = math::get_enityangle(quat); 
	bool lookUp = pitch360 < 180.f; bool lookDown = pitch360 > 180.f; 
	*/
}
BOOL CheckPlayerAngle(uintptr_t ActorRootPtr, Vector3 ObjPos, Vector3 MyPos, float NewAngle)
{
	Vector3 Direction = mem.Read<Vector3>(ActorRootPtr + 0xA4);
	float Yaw = atan2f(fabs(Direction.x), fabs(Direction.z)) * 360.0f / (float)M_PI;
	if (!(Direction.x > 0.0f && Direction.z > 0.0f || Direction.x < 0.0f && Direction.z < 0.0f))
	{
		Yaw = 360.0f - Yaw;
	}
	Yaw = Yaw + 180.0f;
	if (Yaw > 360.0f)
	{
		Yaw = Yaw - 360.0f;
	}
	float War = atan2f(ObjPos.x - MyPos.x, ObjPos.z - MyPos.z) * 180.0f / (float)M_PI;
	if (War <= 0.0f)
	{
		War = War + 360.0f;
	}
	float Phi = fmodf(fabs(War - Yaw), 360.0f);
	float Angle = Phi > 180.0f ? 360.0f - Phi : Phi;
	//cout << Angle << endl;
	调试::逻辑执行.判断角度 = Angle;
	if (Angle > 0.0f && Angle <= NewAngle)
	{
		return TRUE;
	}
	return FALSE;
}
float GetAngle(Vector3 Quat, Vector3 ObjPos, Vector3 MyPos)
{
	/*Vector3 Direction = mem.Read<Vector3>(ActorRootPtr + 0xA4);*/

	float Yaw = atan2f(fabs(Quat.x), fabs(Quat.z)) * 360.0f / (float)M_PI;
	if (!(Quat.x > 0.0f && Quat.z > 0.0f || Quat.x < 0.0f && Quat.z < 0.0f))
	{
		Yaw = 360.0f - Yaw;
	}
	Yaw = Yaw + 180.0f;
	if (Yaw > 360.0f)
	{
		Yaw = Yaw - 360.0f;
	}
	float War = atan2f(ObjPos.x - MyPos.x, ObjPos.z - MyPos.z) * 180.0f / (float)M_PI;
	if (War <= 0.0f)
	{
		War = War + 360.0f;
	}
	float Phi = fmodf(fabs(War - Yaw), 360.0f);
	float Angle = Phi > 180.0f ? 360.0f - Phi : Phi;
	return Angle;

}

//人物僵直
float Getactionduration(uintptr_t HitSimulateData)
{
	return mem.Read<float>(HitSimulateData + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration);
}
//拼刀
float GetactiondurationPD(uintptr_t HitSimulateData)
{
	return mem.Read<float>(HitSimulateData + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType);
}

//----------------------------------------------------朝向--------------------------------------------------------
namespace math
{
	float VectorToRotationYaw(Vector3 pos)
	{
		float RADPI = (float)(180 / 3.1415926f);
		float Yaw = (float)atan2f(pos.x, pos.y) * RADPI;
		if (Yaw < 0)
		{
			Yaw += 180;
		}
		if (pos.x < 0)
		{
			Yaw += 180;
		}
		return Yaw;
	}
	float CalcYaw(Vector3 Loc, Vector3 Ent)
	{
		float yaw = atan2(Loc.x - Ent.x, Loc.y - Ent.y) * 180.f / 3.1415926f;
		if (yaw <= 0)
			yaw += 360;
		return yaw;
	}
	float RotateYaw(Vector3 Direction)
	{
		float Yaw = atan2(abs(Direction.x), abs(Direction.y)) * 360.f / 3.1415926f;
		if (!((Direction.x > 0 && Direction.y > 0) || (Direction.x < 0 && Direction.y < 0)))
		{
			Yaw = 360 - Yaw;
		}
		Yaw = Yaw + 180;
		if (Yaw > 360)
		{
			Yaw = Yaw - 360;
		}
		return Yaw;
	}
	float CheckForword(Vector3 Drt, Vector3 PtrPos, Vector3 MyPos)
	{
		float yaw = RotateYaw(Drt);
		float war = CalcYaw(PtrPos, MyPos);
		float phi = fmodf(abs(war - yaw), 360);
		float dis = phi > 180 ? 360 - phi : phi;
		return dis;
	}

	Vector3 QuatToRot(Vector4 Quaternion)
	{
		Vector3 rot;
		float qzsqr = 0;
		float T0 = 0;
		float T1 = 0;
		float t2 = 0;
		float t3 = 0;
		float t4 = 0;

		qzsqr = Quaternion.z * Quaternion.z;
		T0 = -2.f * (qzsqr + Quaternion.w * Quaternion.w) + 1.f;
		T1 = 2.f * (Quaternion.y * Quaternion.z + Quaternion.x * Quaternion.w);
		t2 = -2.f * (Quaternion.y * Quaternion.w - Quaternion.x * Quaternion.z);
		t3 = 2.f * (Quaternion.z * Quaternion.w + Quaternion.x * Quaternion.y);
		t4 = -2.f * (Quaternion.y * Quaternion.y + qzsqr) + 1.f;

		t2 = t2 > 1.f ? 1.f : t2;
		t2 = t2 < -1.f ? -1.f : t2;

		float RADPI = (float)(180 / 3.1415926f);
		float Yaw = (float)atan2f(T1, T0) * RADPI;
		float Roll = (float)atan2f(t3, t4) * RADPI;
		float Pitch = (float)asin(t2) * RADPI;

		return Vector3(Pitch, Yaw, Roll);
	}

	Vector3 FindLookAtVector(Vector3 pos1, Vector3 pos2)
	{
		Vector3 v1 = Vector3(pos2.x, pos2.z, pos2.y) - Vector3(pos1.x, pos1.z, pos1.y);
		float distance = v1.Length();
		return Vector3(v1.x / distance, v1.y / distance, v1.z / distance);
	}

	float ConverseAngle(float x)
	{
		return (float)(((int)(x + 180.f) % 360 + 180) % 360);
	}

	float AngleDifference(float a, float b)
	{
		int phi = ((int)fabsf((b - a)) % 360);
		int sign = 1;
		if (((a - b) >= 0 && (a - b) <= 180) || ((a - b) <= -180 && (a - b) >= -360))
		{
			sign = -1;
		}

		return ((phi > 180 ? 360 - phi : phi) * sign);
	}
	float AngleDifference360(float a, float b)
	{
		int phi = ((int)fabsf((b - a)) % 360);
		int sign = 1;
		if (((a - b) >= 0 && (a - b) <= 180) || ((a - b) <= -180 && (a - b) >= -360))
		{
			sign = -1;
		}

		return phi * sign;
	}
	float get_enityangle(Vector4 quat)
	{
		Vector3 rot;
		rot = QuatToRot(quat);
		rot.x = rot.x * -1.f;
		if (fabsf(quat.y) >= 0.7f)
		{
			rot.x = 180.f - rot.x;
		}
		return ConverseAngle(rot.x);
	}
}

//---------------------------------------------------------------------------------------------
// 获取范围反应类型
int GetRangeReactionType(uintptr_t HitSimulateData_PTR)
{
	return mem.Read<int>(HitSimulateData_PTR + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType);
}
// 获取反应类型
int GetReactionType(uintptr_t HitSimulateData_PTR)
{
	return mem.Read<int>(HitSimulateData_PTR + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType);
}

// 获取反应类型
int GetEndureLevel(uintptr_t HitSimulateData_PTR)
{
	return mem.Read<int>(HitSimulateData_PTR + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel);
}
//获取物品位置地址
uint64_t GetItemPosAddr(uint64_t Ptr)
{
	uint64_t TEMP;
	TEMP = mem.Read<uint64_t>(Ptr + 0x30);
	TEMP = mem.Read<uint64_t>(TEMP + 0x18);
	TEMP = mem.Read<uint64_t>(TEMP + 0x28);
	TEMP = mem.Read<uint64_t>(TEMP + 0x10);
	TEMP = mem.Read<uint64_t>(TEMP + 0x30);
	TEMP = mem.Read<uint64_t>(TEMP + 0x8);
	return TEMP;
}

//取物品坐标
Vector3 GetItemPos(uint64_t CoordinatePointer) {
	Vector3 Pos;

	mem.Read(CoordinatePointer, &Pos, sizeof(Pos));
	return Pos;
}
//判断武器
int JudgeWeapon(uint32_t WeaponId, int types)
{
	switch (WeaponId)
	{
	case 3200001:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 0;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 0;
		}
		return 101;

	case 3200011:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 101;

	case 3200021:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 101;

	case 3200031:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 101;

	case 3200041:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 101;

	case 3200071:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 5;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 5;
		}
		return 101;

	case 3202510:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 128;

	case 3202520:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 128;

	case 3202530:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 128;

	case 3202540:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 128;

	case 3200300:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 102;

	case 3200310:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 102;

	case 3200320:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 102;

	case 3200330:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 102;

	case 3200400:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 103;

	case 3200410:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 103;

	case 3200420:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 103;

	case 3200430:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 103;

	case 3200440:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 5;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 5;
		}
		return 103;

	case 3200200:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 105;

	case 3200210:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 105;

	case 3200220:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 105;

	case 3200230:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 105;

	case 3200101:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 106;

	case 3200111:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 106;

	case 3200121:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 106;

	case 3200131:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 106;

	case 3201510:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 116;

	case 3201520:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 116;

	case 3201530:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 116;

	case 3201540:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 116;

	case 3201610:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 118;

	case 3201620:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 118;

	case 3201630:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 118;

	case 3201640:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 118;
	case 3201650:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 5;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 5;
		}
		return 118;
	case 3201710:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 119;

	case 3201720:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 119;

	case 3201730:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 119;

	case 3201740:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 119;

	case 3202010:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 122;

	case 3202020:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 122;

	case 3202030:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 122;

	case 3202040:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 122;

	case 3202210:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 123;

	case 3202220:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 123;

	case 3202230:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 123;

	case 3202240:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 123;

	case 3202310:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 124;

	case 3202320:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 124;

	case 3202330:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 124;

	case 3202340:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 124;

	case 3202410:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 127;

	case 3202420:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 127;

	case 3202430:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 127;

	case 3202440:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 127;
	case 3201810:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 120;

	case 3201820:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 120;

	case 3201830:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 120;

	case 3201840:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 120;

	case 3201910:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 1;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 1;
		}
		return 121;

	case 3201920:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 2;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 2;
		}
		return 121;

	case 3201930:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 3;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 3;
		}
		return 121;

	case 3201940:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 4;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 4;
		}
		return 121;

	default:
		if (types == 1) {
			LocalPlayer_Data.Weapon_1_quality = 0;
		}
		else {
			LocalPlayer_Data.Weapon_2_quality = 0;
		}
		return 0;
		break;
	}
	return 0;
}

//是否手持远程
bool IsMeleeWeaponYC(WeaponType ObjWeaponType)
{
	if (ObjWeaponType == WeaponType::鸟铳 || ObjWeaponType == WeaponType::连弩 ||
		ObjWeaponType == WeaponType::弓箭 || ObjWeaponType == WeaponType::喷火筒1 ||
		ObjWeaponType == WeaponType::五眼铳 || ObjWeaponType == WeaponType::一窝蜂 ||
		ObjWeaponType == WeaponType::火炮)
	{
		return true;
	}
	return false;
}

//判断武器
int JudgeWeapon2(uint32_t WeaponId)
{
	switch (WeaponId)
	{
	case 3200001: case 3200011: case 3200071:
	case 3200021: case 3200031:  case 3200041://长剑
		return 101;
		break;
	case 3202510: case 3202520:
	case 3202530: case 3202540://链剑
		return 128;
		break;
	case 3200300: case 3200310:
	case 3200320: case 3200330: //太刀
		return 102;
		break;
	case 3200400: case 3200410: case 3200440:
	case 3200420: case 3200430: //阔刀
		return 103;
		break;
	case 3200200: case 3200210:
	case 3200220: case 3200230: //长枪
		return 105;
		break;
	case 3200101: case 3200111:
	case 3200121: case 3200131: //匕首
		return 106;
		break;
	case 3201510: case 3201520:
	case 3201530: case 3201540: //双节棍
		return 116;
		break;
	case 3201610: case 3201620: case 3201650:
	case 3201630: case 3201640: //双刀
		return 118;
		break;
	case 3201710: case 3201720:
	case 3201730: case 3201740: //斩马刀
		return 119;
		break;
	case 3201810: case 3201820:
	case 3201830: case 3201840: //棍
		return 120;
		break;
	case 3201910: case 3201920:
	case 3201930: case 3201940: //双戟
		return 121;
		break;
	case 3202010: case 3202020:
	case 3202030: case 3202040: //扇子
		return 122;
		break;
	case 3202210: case 3202220:
	case 3202230: case 3202240: //横刀
		return 123;
		break;
	case 3202310: case 3202320:
	case 3202330: case 3202340: //拳刃
		return 124;
		break;
	case 3202410: case 3202420:
	case 3202430: case 3202440: //飞刀
		return 127;
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

// 判断状态
bool JudgeStatus(uint32_t StatusId) {
	if (StatusId == 18) {
		return false;
	}
	if (StatusId == 21) {
		return false;
	}
	if (StatusId == 30) {
		return false;
	}
	if (StatusId == 31) {
		return false;
	}
	if (StatusId == 35) {
		return false;
	}
	if (StatusId == 50) {
		return false;
	}
	if (StatusId == 51) {
		return false;
	}
	if (StatusId == 52) {
		return false;
	}
	if (StatusId == 53) {
		return false;
	}
	if (StatusId == 55) {
		return false;
	}
	if (StatusId == 59) {
		return false;
	}
	if (StatusId == 80) {
		return false;
	}
	return true;
}

// 钩锁撞击敌人
bool 钩锁撞击敌人(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"down_to")) {
		return true;
	}
	return false;
}
// 钩锁出钩
bool 钩锁出钩(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"aim_hook_shoot_01")) {
		return true;
	}
	return false;
}

// CB升龙状态
bool CB状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"crouch_attack_heavy") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 重刃AB状态
bool 重刃AB状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_attack_heavy_03_pre")) {
		return true;
	}
	return false;
}


// A4状态
bool A4状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_light_04") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// A2状态
bool A2状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_light_02") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}
// A3状态
bool A3状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_light_03") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// B1状态
bool B1状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_heavy_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// B2状态
bool B2状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_heavy_02") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 拳刃B2状态
bool 拳刃B2状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_heavy_02_pre") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}


// B3状态
bool B3状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_heavy_03") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// B4状态
bool B4状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_heavy_04") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// A1状态
bool A1状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"attack_light_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 斩马A2蓄住
bool 斩马A2蓄住(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_saber_attack_light_02_charge")) {
		return true;
	}
	return false;
}

// 跑A1状态
bool 跑A1状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"sprint_attack_light_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}
// 跑B1状态
bool 跑B1状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"sprint_attack_heavy_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 滑步B1状态
bool 滑步B1状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"sprint_attack_heavy_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 横刀左蓄一段状态
bool 横刀左蓄一段状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_hengdao_attack_hold_light_01") || StrStrW(ActionName.data(), L"female_hengdao_attack_hold_light_01")) {
		return true;
	}
	return false;
}

// 横刀左蓄状态
bool 横刀左蓄状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hengdao_attack_light") || StrStrW(ActionName.data(), L"_hengdao_sprint_attack_light")
		|| StrStrW(ActionName.data(), L"_hengdao_run_attack_light")) {
		return true;
	}
	return false;
}

// 横刀右蓄状态
bool 横刀右蓄状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hengdao_attack_heavy")) {
		return true;
	}
	return false;
}

// 横栏状态
bool 横栏状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"nunchucks_parry")) {
		return true;
	}
	return false;
}

// 武田F状态
bool 武田F状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_hero_takeda_extend_attack_01") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 磐石状态
bool JudgePanshi(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"parry")) {
		return true;
	}
	return false;
}

// 磐石一段
bool 磐石一段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_parry_01")) {
		return true;
	}
	return false;
}
// 磐石二段
bool 磐石二段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_parry_02")) {
		return true;
	}
	return false;
}
// 磐石三段
bool 磐石三段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_parry_03")) {
		return true;
	}
	return false;
}


// 挂墙状态
bool 挂墙状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"walk_wall") || StrStrW(ActionName.data(), L"ceiling_idle")
		|| StrStrW(ActionName.data(), L"beam_above_idle") || StrStrW(ActionName.data(), L"climb_tree")) {
		return true;
	}
	return false;
}

// 受击站立状态
bool 受击站立状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hurt_stand_") && !StrStrW(ActionName.data(), L"parry")) {
		return true;
	}
	return false;
}

// 出招完毕状态
bool 出招完毕状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_recover")) {
		return true;
	}
	return false;
}


// 被震刀僵直状态
bool 被震刀僵直状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_shocked_")) {
		return true;
	}
	return false;
}

// 左蓄状态
bool 左蓄状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_attack_charge_enter_")) {
		return true;
	}
	return false;
}

// 跳跃状态
bool JudgeJump(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"jump") && 
		!StrStrW(ActionName.data(), L"jump_attack") && 
		!StrStrW(ActionName.data(), L"jump_down_to_land")&& 
		!StrStrW(ActionName.data(), L"jump_emptystep") && 
		!英雄技能状态(FatPlayerData.O_ActionName)) {
		return true;
	}
	return false;
}

// 下蹲状态
bool JudgeCrouch(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_crouch_idle_")) {
		return true;
	}
	return false;
}

// 下蹲前摇过程中状态
bool JudgeSquat(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"squat")) {
		return true;
	}
	return false;
}

// CA状态
bool CA状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"crouch_attack_light_01")) {
		return true;
	}
	return false;
}

// AB3蓄力状态
bool AB3蓄力状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"03_charge") && StrStrW(ActionName.data(), L"light_03") ) {
		return true;
	}
	return false;
}

// AB2蓄力状态
bool AB2蓄力状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"02_charge")) {
		return true;
	}
	return false;
}

// 拼刀状态
bool 拼刀状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"bounce")) {
		return true;
	}
	return false;
}

// 释放受击技能
bool 释放受击技能(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"受击技能")) {
		return true;
	}
	return false;
}

// 二段跳跃状态
bool 二段跳跃状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"airhike_up")) {
		return true;
	}
	return false;
}

// 跳B状态
bool 跳B状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"jump_attack_heavy") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 落地状态
bool 落地状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"to_land")) {
		return true;
	}
	return false;
}

// 跳A状态
bool JudgeJumpAndA(wstring ActionName) {
	if ((StrStrW(ActionName.data(), L"jump_attack_light") || StrStrW(ActionName.data(), L"jump_attack_03"))
		&& StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 滞空
bool Judgefly(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"fly")) {
		return true;
	}
	return false;
}

// 壁击状态
bool 壁击状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"wall_attack_light_01")) {
		return true;
	}
	return false;
}

// 被壁击状态
bool 被壁击状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_hurt_stand_heavy")) {
		return true;
	}
	return false;
}

// 钩锁空A状态
bool 钩锁空A状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"glide_attack_light") && StrStrW(ActionName.data(), L"recover") == nullptr) {
		return true;
	}
	return false;
}

// 判断振刀成功
bool JudgeZhenDao(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"shock") || StrStrW(ActionName.data(), L"hurt_fly")
		|| StrStrW(ActionName.data(), L"injured_lie")) {
		return true;
	}
	return false;
}

	
// 判断倒地状态
bool 倒地状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_injured_") && !StrStrW(ActionName.data(), L"dodge")) {
		return true;
	}
	return false;
}	

// 受击状态
bool 受击状态(ActionType O_EndureLevel, wstring ActionName) {
	if (O_EndureLevel == 6 && !StrStrW(ActionName.data(), L"parry")) {
		return true;
	}
	return false;
}	

// 英雄技能不振
bool 英雄技能不振(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hero_yaodaoji_") || StrStrW(ActionName.data(), L"_hero_spiritsman_")) {
		return true;
	}
	return false;
}

// 胡为开大状态
bool 胡为开大状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hero_huwei_")) {
		return true;
	}
	return false;
}

// 天海金钟罩
bool 天海金钟罩(wstring ActionName) {
	if (wcscmp(ActionName.c_str(), L"male_hero_monk_attack_01") == 0) {
		return true;
	}
	return false;
}
// 天海振刀罩
bool 天海振刀罩(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hero_monk_attack_01_copy_01")) {
		return true;
	}
	return false;
}

// 倒地翻滚状态
bool 倒地翻滚状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"injured_lie") && StrStrW(ActionName.data(), L"dodge")) {
		return true;
	}
	return false;
}

// 判断振刀成功
bool 判断振刀成功(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"shock")) {
		return true;
	}
	return false;
}

// 判断振刀状态
bool JudgeZhenDaoing(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"emptystep")) {
		return true;
	}
	return false;
}
// 横刀曲步状态
bool 横刀曲步状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hengdao_attack_flash")) {
		return true;
	}
	return false;
}

// 判断短闪
bool JudgeDuanFlash(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"dodge") && !StrStrW(ActionName.data(), L"sprint")
		&& !StrStrW(ActionName.data(), L"_injured_") && !英雄技能状态(ActionName)) {
		return true;
	}
	return false;
}

// 滑铲状态
bool 滑铲状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"sprint_downhill")) {
		return true;
	}
	return false;
}

// 滑铲CB状态
bool 滑铲CB状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"slide_attack_heavy")) {
		return true;
	}
	return false;
}

// 判断长闪
bool JudgeLongFlash(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"sprint") && !英雄技能状态(ActionName)) {
		return true;
	}
	return false;
}

// 英雄技能状态
bool 英雄技能状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_hero_")) {
		return true;
	}
	return false;
}

// 判断短闪哈希
bool 判断短闪哈希(int hashName) {
	// 定义一个包含所有短闪哈希值的集合
	std::unordered_set<int> validHashes = {
		-1269610686,   // 前闪
		233247244,     // 左闪
		-27111264,     // 右闪
		441332819,     // 后闪
		264120126,     // 二段前闪
		616853143,     // 持刀右闪
		193657409,     // 持刀左闪
		-1088743435,   // 持刀后闪
		-960445723     // 持刀前闪
	};

	// 如果 hashName 在集合中，则返回 true
	return validHashes.find(hashName) != validHashes.end() && !英雄技能状态(FatPlayerData.O_ActionName);
}

// 判断站立状态
bool JudgeZhanLi(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"idle") || StrStrW(ActionName.data(), L"_run_")) {
		return true;
	}
	return false;
}

// 判断出钩锁状态
bool JudgeGouSuo(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"aim_hook_shoot_01")) {
		return true;
	}
	return false;
}

// 判断是否手持远程
bool JudgeYuanCheng() {
	if (FatPlayerData.O_WeaponID == WeaponType::弓箭 || FatPlayerData.O_WeaponID == WeaponType::连弩 || 
		FatPlayerData.O_WeaponID == WeaponType::火炮 || FatPlayerData.O_WeaponID == WeaponType::喷火筒1 || 
		FatPlayerData.O_WeaponID == WeaponType::一窝蜂 || FatPlayerData.O_WeaponID == WeaponType::五眼铳 || 
		FatPlayerData.O_WeaponID == WeaponType::鸟铳 ) {
		return true;
	}
	return false;
}

// 判断是否手持远程
bool 判断是否手持远程(int O_WeaponID) {
	if (O_WeaponID == WeaponType::弓箭 || O_WeaponID == WeaponType::连弩 ||
		O_WeaponID == WeaponType::火炮 || O_WeaponID == WeaponType::喷火筒1 ||
		O_WeaponID == WeaponType::一窝蜂 || O_WeaponID == WeaponType::五眼铳 ||
		O_WeaponID == WeaponType::鸟铳) {
		return true;
	}
	return false;
}

// 抓出蓄力后摇
bool ShakeBack(wstring ActionName) {
	// 长剑判断
	if (FatPlayerData.O_WeaponID == WeaponType::Sw) {
		if (ActionName == L"male_sw_attack_light_03") {  // 长剑左三连
			return true;
		}
		if (ActionName == L"male_sw_attack_heavy_03") {  // 长剑右三连
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_light_01") {  // 长剑左蓄力
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_heavy_01") {  // 长剑右蓄力
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_light_02") {  // 长剑左蓄力2段
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_heavy_02") {  // 长剑右蓄力2段
			return true;
		}
		if (ActionName == L"male_sw_attack_light_copy_03") {  // 长剑蓝月左1
			return true;
		}
		if (ActionName == L"male_sw_attack_light_copy_03_02") {  // 长剑蓝月左2
			return true;
		}
		if (ActionName == L"male_sw_attack_heavy_copy_03") {  // 长剑蓝月右1
			return true;
		}
		if (ActionName == L"male_sw_attack_heavy_copy_03_02") {  // 长剑蓝月右2
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_light_soul_01") {  // 长剑左凤凰羽
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_heavy_soul_01") {  // 长剑右凤凰羽
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_light_soul_02") {  // 长剑左凤凰羽2段
			return true;
		}
		if (ActionName == L"male_sw_attack_hold_heavy_soul_02") {  // 长剑右凤凰羽2段
			return true;
		}
		if (ActionName == L"male_sw_attack_light_soul_04") {  // 长剑七星夺窍
			return true;
		}
	}

	// 长枪判断
	if (FatPlayerData.O_WeaponID == WeaponType::Spear) {
		if (ActionName == L"male_spear_attack_light_03") {  // 长枪左三连
			return true;
		}
		if (ActionName == L"male_spear_attack_heavy_03") {  // 长枪右三连
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_light_01") {  // 长枪左蓄力
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_01") {  // 长枪右蓄力
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_light_03") {  // 长枪左龙王破
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_03") {  // 长枪右龙王破
			return true;
		}
		if (ActionName == L"male_spear_attack_heavy_soul_03") {  // 长枪风云
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_soul_01") {  // 长枪大圣
			return true;
		}
		if (ActionName == L"male_spear_attack_light_soul_03") {  // 长枪双环
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_03_soul") {  // 长枪风火
			return true;
		}
		if (ActionName == L"male_spear_attack_light_soul_05") {  // 长枪狂狼
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_light_03_soul_01") {  // 左风火一段
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_light_03_soul_02") {  // 左风火二段
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_03_soul_01") {  // 右风火一段
			return true;
		}
		if (ActionName == L"male_spear_attack_hold_heavy_03_soul_02") {  // 右风火二段
			return true;
		}
	}

	// 阔刀判断
	if (FatPlayerData.O_WeaponID == WeaponType::Blade) {
		if (ActionName == L"male_blade_attack_hold_light_soul_03_1" || ActionName == L"female_blade_attack_hold_light_soul_03_1") {  // 雷刀1段
			return true;
		}
		if (ActionName == L"male_blade_attack_hold_light_soul_03_2" || ActionName == L"female_blade_attack_hold_light_soul_03_2") {  // 雷刀2段
			return true;
		}
		if (ActionName == L"male_blade_attack_hold_light_soul_03_3" || ActionName == L"female_blade_attack_hold_light_soul_03_3") {  // 雷刀3段
			return true;
		}
		if (ActionName == L"male_blade_attack_hold_light_soul_03_4" || ActionName == L"female_blade_attack_hold_light_soul_03_4") {  // 雷刀4段
			return true;
		}
		if (ActionName == L"male_blade_attack_heavy_08_2" || ActionName == L"female_blade_attack_heavy_08_2") {  // 左平A 接右平A 接右平A（捏住二段）
			return true;
		}
		if (ActionName == L"male_blade_attack_heavy_08_3" || ActionName == L"female_blade_attack_heavy_08_3") {  // 左平A 接右平A 接右平A（捏住三段）
			return true;
		}
		if (ActionName == L"male_blade_attack_light_07" || ActionName == L"female_blade_attack_light_07") {  // 左平A 接右平A 接左平A
			return true;
		}
		if (ActionName == L"male_blade_attack_heavy_07" || ActionName == L"female_blade_attack_heavy_07") {  // 左平A 接右平A 接左平A 接右平A（捏住一段）
			return true;
		}
		if (ActionName == L"male_blade_attack_heavy_07_2" || ActionName == L"female_blade_attack_heavy_07_2") {  // 左平A 接右平A 接左平A 接右平A（捏住二段）
			return true;
		}
		if (ActionName == L"male_blade_attack_heavy_07_3" || ActionName == L"female_blade_attack_heavy_07_3") {  // 左平A 接右平A 接左平A 接右平A（捏住三段）
			return true;
		}
	}

	// 匕首判断
	if (FatPlayerData.O_WeaponID == WeaponType::Dagge) {
		if (ActionName == L"male_dagger_attack_light_03") {  // 匕首左三连
			return true;
		}
		if (ActionName == L"male_dagger_attack_heavy_03") {  // 匕首右三连
			return true;
		}
		if (ActionName == L"male_dagger_flashstep_attack_light_01") {  // 匕首闪避左三连
			return true;
		}
		if (ActionName == L"male_dagger_attack_hold_light_01") {  // 匕首左蓄力
			return true;
		}
		if (ActionName == L"male_dagger_attack_hold_heavy_02") {  // 匕首右蓄力
			return true;
		}
		if (ActionName == L"male_dagger_attack_hold_heavy_01") {  // 匕首闪避左荆轲献匕
			return true;
		}
		if (ActionName == L"male_dagger_attack_hold_heavy_soul_01") {  // 匕首右鬼刃暗扎
			return true;
		}
		if (ActionName == L"male_dagger_flashjump_attack_soul_01") {  // 匕首鬼哭狼嚎
			return true;
		}
		if (ActionName == L"male_dagger_attack_hold_light_soul_02") {  // 匕首表莲华
			return true;
		}
	}

	// 双截棍判断
	if (FatPlayerData.O_WeaponID == WeaponType::nunchucks) {
		if (ActionName == L"male_nunchucks_attack_light_03") {  // 双截棍左三连
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_light_01") {  // 双截棍左蓄力
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_heavy_enhance_01") {  // 双截棍右蓄力
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_light_03_soul_01") {  // 双截棍飞踢
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_01") {  // 双截棍龙虎功
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_02") {  // 双截棍第二段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_03") {  // 双截棍第三段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_04") {  // 双截棍第四段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_05") {  // 双截棍第五段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_06") {  // 双截棍第六段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_01_soul_0") {  // 双截棍魂技1段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_02_soul_0") {  // 双截棍魂技2段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_04_soul_0") {  // 双截棍魂技4段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_05_soul_0") {  // 双截棍魂技5段
			return true;
		}
		if (ActionName == L"male_nunchucks_attack_hold_heavy_06_soul_01") {  // 双截棍魂技6段
			return true;
		}
	}

	// 太刀判断
	if (FatPlayerData.O_WeaponID == WeaponType::Katana) {
		if (ActionName == L"male_katana_attack_light_03") {  // 太刀左三连
			return true;
		}
		if (ActionName == L"male_katana_attack_heavy_03") {  // 太刀右三连
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_light_01") {  // 太刀左蓄力1
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_light_02") {  // 太刀左蓄力2
			return true;
		}
		if (ActionName == L"male_katana_attack_light_05") {  // 太刀左蓄力后一刀
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_heavy_01") {  // 太刀右蓄力1
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_heavy_02") {  // 太刀右蓄力2
			return true;
		}
		if (ActionName == L"male_katana_attack_heavy_09") {  // 太刀右蓄力后一刀
			return true;
		}
		if (ActionName == L"male_katana_attack_light_soul_03") {  // 太刀左青鬼
			return true;
		}
		if (ActionName == L"male_katana_attack_heavy_soul_03") {  // 太刀右青鬼
			return true;
		}
		if (ActionName == L"male_katana_attack_heavy_soul_09") {  // 太刀十方雷劫
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_light_soul_02") {  // 太刀左噬魂
			return true;
		}
		if (ActionName == L"male_katana_attack_hold_heavy_soul_02") {  // 太刀右噬魂
			return true;
		}
	}

	// 双刀判断
	if (FatPlayerData.O_WeaponID == WeaponType::twinblades) {
		if (ActionName == L"male_twinblades_attack_light_03") {  // 双刀左三连
			return true;
		}
		if (ActionName == L"male_twinblades_attack_heavy_03") {  // 双刀右三连
			return true;
		}
		if (ActionName == L"male_twinblades_attack_hold_light_01") {  // 双刀左蓄力
			return true;
		}
		if (ActionName == L"male_twinblades_attack_hold_heavy_01") {  // 双刀右蓄力
			return true;
		}
		if (ActionName == L"male_twinblades_attack_hold_heavy_02") {  // 双刀右蓄力二段
			return true;
		}
		if (ActionName == L"male_twinblades_attack_light_03_soul") {  // 左键三连分水斩
			return true;
		}
		if (ActionName == L"male_twinblades_attack_hold_light_01_soul") {  // 铁马左键蓄力
			return true;
		}
		if (ActionName == L"male_twinblades_attack_hold_heavy_01_soul") {  // 双刀乾坤日月斩
			return true;
		}
		if (ActionName == L"male_twinblades_attack_heavy_04") {  // 双刀蓄力后右键追击蓄力
			return true;
		}
	}

	// 长棍判断
	if (FatPlayerData.O_WeaponID == WeaponType::rod) {
		if (ActionName == L"male_rod_attack_hold_light_01") {  // 棍子左键蓄力
			return true;
		}
		if (ActionName == L"male_rod_attack_light_03") {  // 棍子左键三连
			return true;
		}
		if (ActionName == L"male_rod_attack_hold_heavy_01") {  // 棍子右键蓄力
			return true;
		}
		if (ActionName == L"male_rod_attack_heavy_03") {  // 棍子右键三连
			return true;
		}
		if (ActionName == L"male_rod_attack_heavy_05_copy") {  // 点乱天宫
			return true;
		}
		if (ActionName == L"male_rod_attack_hold_light_03") {  // 定海针 腾云式
			return true;
		}
		if (ActionName == L"male_rod_attack_heavy_soul_05_copy") {  // 无情七灭阵
			return true;
		}
		if (ActionName == L"male_rod_attack_hold_light_soul_01") {  // 捅劲
			return true;
		}
		if (ActionName == L"male_rod_attack_light_soul_03") {  // 双环扫
			return true;
		}
		if (ActionName == L"male_rod_attack_hold_light_03_soul_01") {  // 镇地撑天
			return true;
		}
		if (ActionName == L"male_rod_attack_light_soul_05") {  // 捣海棍
			return true;
		}
	}

	// 斩马刀判断
	if (FatPlayerData.O_WeaponID == WeaponType::saber) {
		if (ActionName == L"male_saber_attack_hold_light_soul_01" || ActionName == L"female_saber_attack_hold_light_soul_01") {  // 左键炽焰斩蓄力一段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_light_soul_02" || ActionName == L"female_saber_attack_hold_light_soul_02") {  // 左键炽焰斩蓄力二段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_light_soul_03" || ActionName == L"female_saber_attack_hold_light_soul_03") {  // 左键炽焰斩蓄力三段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_light_01" || ActionName == L"female_saber_attack_hold_light_01") {  // 左蓄一段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_light_02" || ActionName == L"female_saber_attack_hold_light_02") {  // 左蓄二段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_light_03" || ActionName == L"female_saber_attack_hold_light_03") {  // 左蓄三段
			return true;
		}
		if (ActionName == L"male_saber_attack_light_02" || ActionName == L"female_saber_attack_light_02") {  // 左键接左键
			return true;
		}
		if (ActionName == L"male_saber_attack_light_05" || ActionName == L"female_saber_attack_light_05") {  // 右键接左键接左键
			return true;
		}
		if (ActionName == L"male_saber_attack_light_06" || ActionName == L"female_saber_attack_light_06") {  // 右键接左键
			return true;
		}
		if (ActionName == L"male_saber_attack_light_07" || ActionName == L"female_saber_attack_light_07") {  // 左键接右键接左键
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_heavy_01" || ActionName == L"female_saber_attack_hold_heavy_01") {  // 右键蓄力一段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_heavy_02" || ActionName == L"female_saber_attack_hold_heavy_02") {  // 右键蓄力二段
			return true;
		}
		if (ActionName == L"male_saber_attack_hold_heavy_03" || ActionName == L"female_saber_attack_hold_heavy_03") {  // 右键蓄力三段
			return true;
		}
	}

	// 双戟判断
	if (FatPlayerData.O_WeaponID == WeaponType::dualhalberd) {
		if (ActionName == L"male_dualhalberd_attack_hold_light_01") {  // 左蓄一段
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_hold_heavy_01") {  // 右蓄一段
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_hold_heavy_02") {  // 右蓄二段
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_heavy_09") {  // 左蓄接右蓄
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_light_09" || ActionName == L"male_dualhalberd_attack_light_09_pre") {  // 右蓄接左蓄
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_light_03") {  // 左3A蓄
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_heavy_03") {  // 右3A蓄
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_hold_light_01_soul") {  // 战龙在天
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_light_08") {  // 勾旋斩
			return true;
		}
		if (ActionName == L"male_dualhalberd_attack_heavy_soul_09") {  // 蛟龙入海
			return true;
		}
	}

	return false;  // 默认返回false
}


// 判断闪避
bool JudgeShortFlash(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"dodge")) {
		return true;
	}
	return false;
}



bool JudgeFlash(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"dodge")) {
		return true;
	}
	if (StrStrW(ActionName.data(), L"emptystep_back")) {
		return true;
	}
	if (StrStrW(ActionName.data(), L"crouch_idle")) {

		return true;
	}
	if (StrStrW(ActionName.data(), L"jump")) {

		return true;
	}
	return false;
}
// 获取准星目标距离
float GetAimDis(float x, float y, float ObjeX, float ObjeY)
{

	float _x = x - ObjeX;
	float _Y = y - ObjeY;
	return sqrt(_x * _x + _Y * _Y);
}
// 获取目标距离
float GetPlayerDis(Vector3 Obj, Vector3 Local)
{

	float _x = Obj.x - Local.x;
	float _Y = Obj.y - Local.y;
	float _Z = Obj.z - Local.z;
	return sqrt(_x * _x + _Y * _Y + _Z * _Z);
}

/*总调用，盒子控制*/

// 总控制，键盘按下
void root_sendKeyPress(int Key) {
	
	if (按键方式 == 0)
	{
		SendKeyPressEx(Key);
	}
	else if (按键方式 == 1) {
		按下键盘(Key);
	}
	else if (按键方式 == 2) {
		dhz_SendKeyPressEx(Key);
	}
}

// 总控制，连招用到的所有键盘弹起
void root_sendKeyUpAll() {
	// 需要弹起的所有按键
	int keys[] = {
		KEY_A, KEY_S, KEY_W, KEY_D, KEY_Q, KEY_R, KEY_T, KEY_F,
		KEY_G, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_LEFTSHIFT,KEY_SPACEBAR
	};

	// 遍历所有按键，依次调用 root_sendKeyUp
	for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		root_sendKeyUp(keys[i]);
	}
	root_kmNet_mouse_left(0);
	root_kmNet_mouse_right(0);
}


// 总控制，键盘弹起
void root_sendKeyUp(int Key) {
	if (Key == KEY_LEFTSHIFT) {
		上次Shiftsh时间 = GetTickCount64();
	}
	if (按键方式 == 0)
	{
		SendKeyUpEx(Key);
	}
	else if (按键方式 == 1) {
		弹起键盘(Key);
	}
	else if (按键方式 == 2) {
		dhz_SendKeyUp(Key);
	}
}

// 总-侧键按下弹起
void root_kmNet_mouse_side(int key) {
	if (按键方式 == 0)
	{
		kmNet_mouse_side1(key);
	}
	else if (按键方式 == 1) {
		if (key == 0) {
			鼠标側鍵2弹起();
		}
		else {
			鼠标側鍵2按下();
		}
		
	}
	else if (按键方式 == 2) {
		dhz_鼠标侧键1(key);
	}
}

void root_kmNet_mouse_side2(int key) {
	if (按键方式 == 0)
	{
		kmNet_mouse_side2(key);
	}
	else if(按键方式 == 1){
		root_kmNet_mouse_side(key);
	}
	else if (按键方式 == 2) {
		dhz_鼠标侧键2(key);
	}
}

void root_kmNet_mouse_middle(int key) {
	if (按键方式 == 0)
	{
		kmNet_mouse_middle(key);
	}
	else if (按键方式 == 1) {
		
	}
	else if (按键方式 == 2) {
		dhz_鼠标中键(key);
	}
}


// 总-鼠标左键按下弹起
void root_kmNet_mouse_left(int key) {

	振刀逻辑::判断鼠标左键状态改变(key == 1);
	if (按键方式 == 0)
	{
		kmNet_mouse_left(key);
	}
	else if (按键方式 == 1) {
		if (key == 0) {
			左键弹起();
		}
		else {
			左键按下();
		}

	}
	else if (按键方式 == 2) {
		dhz_鼠标左键(key);
	}
}

// 总-鼠标右键按下弹起
void root_kmNet_mouse_right(int key) {
	//MyLog("右键状态改变",key);
	振刀逻辑::判断鼠标右键状态改变(key == 1);
	if (按键方式 == 0)
	{
		kmNet_mouse_right(key);
	}
	else if (按键方式 == 1) {
		if (key == 0) {
			右键弹起();
		}
		else {
			右键按下();
		}

	}
	else if (按键方式 == 2) {
		dhz_鼠标右键(key);
	}
}

// 总鼠标滚轮控制
void root_kmNet_middle(int vk) {
	if (按键方式 == 0)
	{
		kmNet_mouse_wheel(vk);
	}
	else if (按键方式 == 1) {
		鼠标滚轮(vk);

	}
	else if (按键方式 == 2) {
		dhz_鼠标滚轮(vk);
	}
}


// 总-解除所有按键屏蔽
void root_kmNet_unmask_all() {
	if (按键方式 == 0)
	{
		kmNet_unmask_all();
	}
	else if (按键方式 == 1) {

	}
	else if (按键方式 == 2) {
		dhz_解除所有键屏蔽();
	}
}

// 总-屏蔽左右键
void root_BlockLeftandRight() {
	if (按键方式 == 0)
	{
		BlockLeftandRight();
	}
	else if (按键方式 == 1) {

	}
	else if (按键方式 == 2) {
		dhz_屏蔽鼠标左右键();
	}
}

// 总-释放左右键
void root_ReleaseLeftandRight() {
	if (按键方式 == 0)
	{
		ReleaseLeftandRight();
	}
	else if (按键方式 == 1) {

	}
	else if (按键方式 == 2) {
		dhz_释放鼠标左右键();
	}
}

/*总调用结束*/

// DHZ
void dhz_SendKeyPressEx(uint32_t Key)
{
	
	DHZBOX::getInstance().KeyDown(Key);
}

void dhz_SendKeyUp(uint32_t Key)
{
	DHZBOX::getInstance().KeyUp(Key);
}

void dhz_Move(int x, int y)
{
	DHZBOX::getInstance().move(x, y);
}

void dhz_鼠标左键(int Key)
{
	DHZBOX::getInstance().left(Key);
}

void dhz_鼠标右键(int Key)
{
	DHZBOX::getInstance().right(Key);
}

void dhz_鼠标侧键1(int Key)
{
	DHZBOX::getInstance().side1(Key);
}

void dhz_鼠标侧键2(int Key)
{
	DHZBOX::getInstance().side2(Key);
}

void dhz_鼠标中键(int Key)
{
	DHZBOX::getInstance().middle(Key);
}

void dhz_鼠标滚轮(int Key)
{
	DHZBOX::getInstance().wheel(Key);
}

void dhz_解除所有键屏蔽()
{
	DHZBOX::getInstance().dismask_keyboard_all();
}

void dhz_屏蔽鼠标左右键()
{
	DHZBOX::getInstance().mask_left(1);
	DHZBOX::getInstance().mask_right(1);
}

void dhz_释放鼠标左右键()
{
	DHZBOX::getInstance().mask_left(0);
	DHZBOX::getInstance().mask_right(0);
}

//KmNet是否按下键
bool KmboxIsHokeyKey(short vk_key)
{
	return kmNet_monitor_keyboard(vk_key);
}

//VMM远程是否按下键
bool IsHokeyEx(int Key)
{
	return mem.GetKeyboard()->IsKeyDown(Key);
}
//模拟键盘按下(kmbox)
void SendKeyPressEx(uint32_t Key)
{
	kmNet_keyup(Key);
	kmNet_keydown(Key);
}
//模拟键盘弹起(kmbox)
void SendKeyUpEx(uint32_t Key)
{
	kmNet_keyup(Key);
}
//左键按下弹起(物理)
void MouseLeftEx(int key)
{
	root_kmNet_mouse_left(key);
}
//右键按下弹起(物理)
void MouseRightEx(int key)
{
	root_kmNet_mouse_right(key);
}
//左键按下弹起(kmbox)
void MouseLeftUp(int key)
{
	kmNet_mask_mouse_left(true);
	kmNet_mouse_move(1, 1);
	kmNet_mask_mouse_left(false);
}
//右键按下弹起(kmbox)
void MouseRightUp(int key)
{
	kmNet_mask_mouse_right(true);
	kmNet_mouse_move(1, 1);
	kmNet_mask_mouse_right(false);
}

void BlockLeftandRight()
{
	kmNet_mask_mouse_left(true);
	kmNet_mask_mouse_right(true);

}
void ReleaseLeftandRight()
{
	kmNet_mask_mouse_left(false);
	kmNet_mask_mouse_right(false);

}

void MouseReleaseNoCheck()
{
	root_kmNet_mouse_left(0);
	Sleep(1);
	root_kmNet_mouse_right(0);
}
void MouseRelease()
{
	if (IsHokeyEx(VK_LBUTTON))
	{
		root_kmNet_mouse_left(1);
		Sleep(1);
		root_kmNet_mouse_left(0);

	}
	if (IsHokeyEx(VK_RBUTTON))
	{
		root_kmNet_mouse_right(1);
		Sleep(1);
		root_kmNet_mouse_right(0);

	}
}
//鼠标移动(kmbox)
void Kmbox_Move(int x, int y)
{
	char buff[1024];
	sprintf_s(buff, "km.move(%d,%d)\r\n", x, y);
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
//鼠标移动(kmnet)
void Move_mouse(int x, int y)
{
	if (按键方式 == 0)
	{
		kmNet_mouse_move(x, y);
	}
	else if (按键方式 == 1)
	{
		Kmbox_Move(x, y);
	}
	else if (按键方式 == 2)
	{
		dhz_Move(x, y);
	}

}
//鼠标左右键松开
void MouseLeftorRightUp()
{
	if (按键方式 == 0)
	{
		MouseRelease();
	}
	else if (按键方式 == 1)
	{
		左键弹起();
		Sleep(1);
		右键弹起();
	}
	else if (按键方式 == 2)
	{
		dhz_鼠标左键(0);
		Sleep(1);
		dhz_鼠标右键(0);
	}
}
void 按下键盘(int vk)
{
	char buff[1024];
	sprintf_s(buff, "km.down(%d)\r\n", vk);
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 弹起键盘(int vk)
{
	char buff[1024];
	sprintf(buff, "km.up(%d)\r\n", vk);
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 左键按下()
{
	char buff[1024];
	sprintf(buff, "%s", "km.left(1)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 左键弹起()
{
	char buff[1024];
	sprintf(buff, "%s", "km.left(0)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 右键按下()
{
	char buff[1024];
	sprintf(buff, "%s", "km.right(1)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 右键弹起()
{
	char buff[1024];
	sprintf(buff, "%s", "km.right(0)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 鼠标側鍵2按下()
{
	char buff[1024];
	sprintf(buff, "%s", "km.side1(1)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 鼠标側鍵2弹起()
{
	char buff[1024];
	sprintf(buff, "%s", "km.side1(0)\r\n");
	myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
void 鼠标滚轮(int i)
{
	char buff[1024];
	if(i = 0 ? sprintf(buff, "%s", "km.wheel(2)\r\n") : sprintf(buff, "%s", "km.wheel(-2)\r\n"))	
		myserial.write(buff);
	Sleep(10);//等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	//myserial.read(buff, 20);    //接收串口数据，超时时间为20ms（有的函数不需要回码，可以不需要接收。如果想精确收发请自定义串口协议）
}
int 左键状态()
{
	char buff[1024];
	sprintf(buff, "%s", "km.left()\r\n");
	myserial.write(buff);
	//Sleep(1);			          //等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	myserial.read(buff, 20);
	if (strstr(buff, "1") != NULL)return 1;

}
int 右键状态()
{
	char buff[1024];
	sprintf(buff, "%s", "km.right()\r\n");
	myserial.write(buff);
	//Sleep(1);			          //等待10ms(串口传输是需要时间，kmbox内部代码执行也需要时间。)
	myserial.read(buff, 20);
	if (strstr(buff, "1") != NULL)return 1;

}

/*英雄技能*/

// 水娘水矛束缚状态
bool 水娘水矛束缚状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_all_unarmed_be_tied")) {
		return true;
	}
	return false;
}

// 被武田夺刀状态
bool 被武田夺刀状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"_skill_hurt_02")) {
		return true;
	}
	return false;
}


// 双戟出左蓄状态
bool 双戟出左蓄状态(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"dualhalberd")) {
		if (StrStrW(ActionName.data(), L"_attack_charge")) {
			return true;
		}
	}
	return false;
}

// 长枪龙王破
bool 长枪龙王破(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_spear_attack_hold_light_03")) {
		return true;
	}
	return false;
}
// 长枪龙王破一段
bool 长枪龙王破一段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_spear_attack_light_05")) {
		return true;
	}
	return false;
}

// 长棍立棍
bool 长棍立棍(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_rod_attack_hold_light_03")) {
		return true;
	}
	return false;
}

// 长棍乱点天宫一段
bool 长棍乱点天宫一段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_rod_attack_heavy_05")) {
		return true;
	}
	return false;
}

// 长棍乱点天宫二段
bool 长棍乱点天宫二段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_rod_attack_heavy_05_01")) {
		return true;
	}
	return false;
}

// 长棍乱点天宫三段
bool 长棍乱点天宫三段(wstring ActionName) {
	if (StrStrW(ActionName.data(), L"male_rod_attack_heavy_05_02")) {
		return true;
	}
	return false;
}
// 是否受击技能
bool 是否受击技能F(int fID) {
	// 定义受击技能F的ID列表
	int 受击技能F[] = {
		1500500, // 天海F1
		1501600, // 沈妙F1
		1500300, // 沙男F1
		1500301, // 沙男F2
		1500702, // 妖刀F2
		1500802, // 水娘F2
		1500900, // 岳山F1
		1501102, // 无尘F1
		1501114, // 无尘F2
		1501301, // 冰女F2
		1501900, // 纪莹莹F1
		1502000, // 狐狸F1
		1502101, // 哈迪F2
		1502200, // 魏青F1
		1502300, // 刘炼F1
		1502400, // 张起灵F1
		1502500, // 希拉F1
		1502605  // 蓝梦F1
	};

	// 遍历ID列表，检查传入的fID是否匹配
	for (int i = 0; i < sizeof(受击技能F) / sizeof(受击技能F[0]); i++) {
		if (fID == 受击技能F[i]) {
			return true;  // 如果匹配，则返回true
		}
	}

	return false;  // 如果没有匹配，返回false
}
