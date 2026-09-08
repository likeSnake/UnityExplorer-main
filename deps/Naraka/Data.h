#pragma once
#include "pak.h"
#include "SDK.h"
#include "ShockKnife.h"
#include<map>
#include <algorithm> // std::min_element
#include <optional>  // std::optional
#include <memory>
#include <vector>

namespace Global
{
inline struct
{
	std::unique_ptr<CharacterManager> Character = std::make_unique<CharacterManager>();
	std::unique_ptr<UserDataManager> UserData = std::make_unique<UserDataManager>();
	uintptr_t MatrixAddr = NULL;

	float ping = 0.f;
	double GlobalTime = 0.f;

	time_t t = time(nullptr);
	struct tm* now = localtime(&t);

	uintptr_t Knifevtable = NULL;
	uintptr_t FlushAddr = NULL;
} WorldPtr;

inline struct
{
	ActorModel* L_ActorModel = nullptr;
	ActorModelPropertyData* L_PropertyData = nullptr;
	ActorKit* L_ActorKit = nullptr;
	RuntimePropertyData* L_RuntimeProperty = nullptr;
	ObjectMessenger* L_ObjectMessenger = nullptr;
	ActionHitSimulateData* L_HitSimulate = nullptr;
	ActionHitSimulatePtr* L_HitSimulatePtr = nullptr;

	uintptr_t Cache_EventTracksForLayer_PTR = NULL;
	uintptr_t Cache_ActorWeapon_PTR = NULL;
	uintptr_t Cache_Pos_PTR = NULL;
	uintptr_t Cache_ActorRoot_PTR = NULL;
	uintptr_t Cache_CameraAgent_PTR = NULL;
	uintptr_t Cache_CameraController_PTR = NULL;
	uintptr_t Cache_AdventureRig_PTR = NULL;
	uintptr_t Cache_GetGlobalTime_PTR = NULL;
	uintptr_t Cache_GetCurTimeForLogicState_PTR = NULL;
	uintptr_t Cache_AvgFPSStatic_PTR = NULL;
	uintptr_t CharactorSyncManagerPtr = NULL;
	uintptr_t Cache_GetNameHash_PTR = NULL;
	uintptr_t Cache_ActorPhysics_PTR = NULL;
	uintptr_t Cache_Bluetime_PTR = NULL;
	uintptr_t Cache_PropertData_PTR = NULL;
	PropertyDataID Cache_PropertyDataID = { 0 };
	uintptr_t Cache_WeaponIndex_PTR = NULL;
	uintptr_t Cache_WeaponIndex_1PTR = NULL;
	uintptr_t Cache_WeaponIndex_2PTR = NULL;
	uintptr_t fpsFilterPtr = NULL;
} Cache_LocalPlayer;

inline struct
{
	AllSkillData* L_LocalSkillData = nullptr;
	uintptr_t Cache_Skill_PTR = NULL;
	uintptr_t Cache_SkillID = NULL;
	LocalSkillData MySkill;
} MySkillData;
}
struct ObjData
{
	ActorModel* O_ActorModel = nullptr;
	ActorModelPropertyData* O_PropertyData = nullptr;
	ActorKit* O_ActorKit = nullptr;
	RuntimePropertyData* O_RuntimeProperty = nullptr;
	ObjectMessenger* O_ObjectMessenger = nullptr;
	ActionHitSimulateData* O_HitSimulate = nullptr;
	ActionHitSimulatePtr* O_HitSimulatePtr = nullptr;

	uintptr_t Cache_EventTracksForLayer_PTR = NULL;
	uintptr_t Cache_ActorWeapon_PTR = NULL;
	uintptr_t Cache_Pos_PTR = NULL;
	uintptr_t Cache_ActorRoot_PTR = NULL;
	uintptr_t Cache_GetCurTimeForLogicState_PTR = NULL;
	uintptr_t Cache_GetNameHash_PTR = NULL;
	uintptr_t Cache_Bluetime_PTR = NULL;
	uintptr_t Cache_PropertData_PTR = NULL;
	PropertyDataID Cache_PropertyDataID = { 0 };

	bool ISRobot = false;
	float ping = 0;
	int Team = 0;
	int HeroId = 0;
	string PlayerName = "";
	LocalSkillData O_Skill{};
	int playerNowBag;

};

struct PlayerData
{
	bool IsHaveData = false;
	bool ISRobot = false;
	bool IsSoul = false;
	float ping = 0;
	int Team = 0;
	int HeroId = 0;
	string PlayerName = "";
	double ObjCurTime;
    double ObjCurTimeForLogic;
	string HeroName = "";
	
	wstring O_ActionName = L"";
	bool O_Visible = false;
	int O_WeaponID = 0;
	int O_RangeReactionType = 0;
	int O_NameHash;
	int O_SkillStatus;
	LocalSkillData O_Skill{};
	WeaponType O_ActorWeapon;
	ActionType O_EndureLevel;
	int O_XuListate;
	int O_ReactionType;
    int O_ReactionExtraType;
	float O_ActionDuration;
	float O_Direction;
	Vector4 O_Quat = { 0,0,0,0};
	Vector3 O_Pos = {0,0,0};
	float O_Dis = 0.f;
	int O_CurHp = 0;
	int O_MaxHp = 0;
	int O_Curshield = 0;
	int O_Maxshield = 0;
	int O_CurAnger =0;
	int O_MaxAnger = 0;
	float O_BlueTime = 0.f;
	Vector3 O_Velocity = { 0,0,0};
	uintptr_t Cache_PropertyData = 0;
	uintptr_t EventTracksForLayerPtr = 0;
	uintptr_t Cache_BonePtr = 0;
	uintptr_t Cache_GetCurTimeForLogicState_PTR = 0;
	uintptr_t Cache_ActorRootPtr = 0;
	uintptr_t Cache_HitSimulate = 0;
	uintptr_t Cache_ActionNamePtr = 0;
	uintptr_t Cache_NameHash = 0;
	uintptr_t O_ActorModel= 0;

	ActorModel* O_ActorModel1 = nullptr;
	uintptr_t O_HitSimulate = 0;
};



struct LocalPlayer_Data1
{
	ActionType MyEndureLevel;
	HeroType MyHero;
	wstring MyActionName;
	WeaponType MyWeaponType;
	int 敌人数量5M内;
	int MyWeaponIndex;
	int My_1weapon;
	int My_2weapon;
	int MyReactionType;
	int MyTeam;
	int MyCurEnergy;
	int MyCurshield;
	int MyCurkuorong;
	int MyRangeReactionType;
	int MyReactionExtraType;
	int MyReactionParryType;
	int MyCollisionFlags;
	int XuListate;
	int My_NameHash;
	int Weapon_1;
	int Weapon_2;

	int Weapon_1_quality;
	int Weapon_2_quality;
	uintptr_t Cache_ActionNamePtr;
	uintptr_t Cache_NameHash;
	uintptr_t My_ActorRootPtr;
	uintptr_t EventTracksForLayerPtr;
	int MySkillID;
	int MySkillState;
	float MyLeftContinueTime;
	float MyLeftCdTime;
	float MyActionDuration;
	float MyDirection;
	double MyCurTime;
	double MyCurTimeForLogic;
	float MyBlueTime;
	float MyPing;
	int MyFPS;
	Vector3 MyPos;
	Vector3 CameraEuler;
	float CameraPitch;
	int myNowBag;
	int myNowBagWeapon;
	int myNowBagsoul;
	std::vector<int> mySoulItemTidList;
	std::vector<int> myBagItemTidList;
	int _showExtraEvent;
	bool IsSpectator;
	double worldPtrGlobalTime;

	float 上次振刀时间;
};

extern PlayerData FatPlayerData;
extern int 跳出方式;
extern long 双刀上次出蓄力时间;
extern long 长剑上次出蓄力时间;
extern long 上次Shiftsh时间;
extern long 敌人上次闪避时间;
extern long 我上次闪避时间;
extern int 平A方式;
extern bool 鼠标左键是否按下;
extern bool 鼠标右键是否按下;
extern bool 是否半自动模式;
extern int 上次武器按键;
extern bool 博弈键按下;
extern bool 调试模式;
extern bool 活化闪避;
// 双缓存数据结构
class DoubleBuffer {
private:
	std::vector<PlayerData> buffers[2];         // 双缓冲存储
	std::atomic<int> read_index{ 0 };           // 当前读缓冲区索引
	std::mutex swap_mutex;                      // 保护交换操作
	std::condition_variable cv;                 // 数据更新通知
	std::optional<PlayerData> nearest_player;   // 最近的玩家

public:
	// 获取当前读缓冲区的索引（无锁）
	int GetCurrentIndex() const noexcept {
		return read_index.load(std::memory_order_acquire);
	}

	// 获取当前可读缓冲区（无锁）
	const std::vector<PlayerData>& GetReadBuffer() const noexcept {
		return buffers[GetCurrentIndex()];
	}

	// 获取当前可写缓冲区（无锁）
	std::vector<PlayerData>& GetWriteBuffer() noexcept {
		return buffers[1 - GetCurrentIndex()];
	}

	// 交换缓冲区并通知读者（线程安全）
	void SwapBuffers() {
		std::lock_guard<std::mutex> lock(swap_mutex);
		read_index.store(1 - GetCurrentIndex(), std::memory_order_release);
		cv.notify_all();

		// 每次交换时更新最近玩家
		UpdateNearestPlayer();
	}

	// 等待新数据（带超时避免死锁）
	template<typename Rep, typename Period>
	bool WaitForData(const std::chrono::duration<Rep, Period>& timeout) {
		std::unique_lock<std::mutex> lock(swap_mutex);
		return cv.wait_for(lock, timeout, [this] {
			return !buffers[GetCurrentIndex()].empty();
			});
	}

	// 根据距离条件过滤数据
	std::vector<PlayerData> GetPlayersWithinDistance(float max_distance) const {
		const auto& current_buffer = GetReadBuffer();
		std::vector<PlayerData> filtered_players;

		for (const auto& player : current_buffer) {
			if (!player.IsSoul && player.O_Visible && player.O_Dis <= max_distance) {
				filtered_players.push_back(player);
			}
		}

		return filtered_players;
	}

	// 获取最近的玩家
	std::optional<PlayerData> GetNearestPlayer() const {
		return nearest_player;
	}

private:
	// 更新最近的玩家（最小距离的玩家）
	void UpdateNearestPlayer() {
		const auto& current_buffer = GetReadBuffer();

		// 如果缓存为空，则不更新
		if (current_buffer.empty()) {
			nearest_player.reset();
			FatPlayerData.IsHaveData = false;
			return;
		}

		// 查找最小距离的玩家（忽略魂体）
		bool found = false;
		PlayerData nearest{};
		for (const auto& player : current_buffer) {
			if (player.IsSoul) {
				continue;
			}
			if (!found || player.O_Dis < nearest.O_Dis) {
				nearest = player;
				found = true;
			}
		}

		if (!found) {
			nearest_player.reset();
			FatPlayerData.IsHaveData = false;
			return;
		}

		nearest_player = nearest; // 更新最近玩家
		FatPlayerData = nearest;
		FatPlayerData.IsHaveData = true;
	}
};




namespace Function
{
	namespace ESP
	{
		extern uint32_t 显示或隐藏界面按键;
		extern bool 菜单;
		extern bool 菜单_2;
		extern bool 方框;
		extern bool 骨骼;
		extern bool 信息;
		extern bool 血条;
		extern bool 预警;
		extern bool 射线;
		extern bool 手持;
		extern bool 名字;
		extern bool 灵魂;
		extern float daoJuTextSize;
		extern float userTextSize;

		extern bool 近战_白;
		extern bool 近战_蓝;
		extern bool 近战_紫;
		extern bool 近战_金;
		extern bool 远程_白;
		extern bool 远程_蓝;
		extern bool 远程_紫;
		extern bool 远程_金;
		extern bool 道具;
		extern bool 扩容;
		extern bool 智能扩容;
		extern bool 金魂;
		extern bool 夺魂;
		extern bool 属性;
		extern bool 盒子;

		extern bool 护甲Lv1;
		extern bool 护甲Lv2;
		extern bool 护甲Lv3;
		extern bool 护甲Lv4;
		extern bool 护甲Lv5;
		extern bool 换甲护甲;

		extern bool 果实;
		extern bool 萤火虫;
		extern bool 金堆; 
		extern bool 蓝堆;
		extern bool 绿堆;
		extern bool 任务;
		extern bool 秘籍;
		extern bool 所有对象;

		extern int 血条样式;
		extern int 预警样式;
		extern ImColor 人机颜色;
		extern ImColor 玩家颜色;
		extern ImColor 名字颜色;
		extern ImColor 信息颜色;
	}
	namespace Kmbox
	{
		namespace Net
		{
			extern char IP[0XFF], Port[0XFF], mac[0XFF];
			extern int NetState;
		}
		namespace B_Pro
		{
			extern bool BproState;
			extern char PorttextBuffer[0XFF];
			extern char SpeedBuffer[0XFF];
		}
		namespace DhzBox_Lite
		{
			extern bool DproState;
			extern char IP[0XFF];
			extern char Port[0XFF];
			extern char RANDOM[0XFF];
		}
	}
	namespace Shock
	{
		extern bool F_ShockKnife;		//振刀功能开关
		extern bool M_ShockKnife;				//预判振噬魂斩 龙虎乱舞等等
		extern bool Self_play;	//振刀判断到需要振刀时，是否执行振刀
		extern bool Flag_CollideKnife;	//振刀判断到需要拼刀时，是否执行拼刀
		extern bool Flag_ShortDodge;	//振刀判断到需要闪避时，是否执行闪避
		extern bool 扬鞭劲振刀;
		extern bool 横栏振刀;
		extern bool 横刀左蓄振刀;
		extern bool 只振视野范围内;
		extern int 振刀按键;
	}

	namespace 侧键博弈 {

		extern bool 自动连招;
		extern bool 蓄力抓振;
		extern bool 博弈切刀抓振刀;
		extern bool 蓄力抓长闪;
		extern bool 蓄力抓短闪;
		extern bool 蓄力抓白刀;
		extern bool 蓄力抓倒地;
		extern bool 蓄力抓受击;
		extern bool 蓄力抓起跳;
		extern bool 蓄力抓下蹲;
		extern bool 禁用CPU0;

		extern bool 白刀抓长闪;
		extern bool 白刀抓短闪;
		extern bool 白刀抓蓄力后摇;
		extern bool 白刀抓振;
		extern bool 白刀蓝顶;
		extern bool 切拳起身;

		extern int 动态线程;

		extern int 蓄力抓振延迟;
		extern int 白刀抓短闪延迟;
		extern float 白刀蓝顶延迟;
		extern float 白刀抓短闪最远距离;
		extern float 白刀抓长闪最远距离;
		extern int 白刀抓蓄力后摇延迟;

		extern int 钩锁模式;
		extern float 断续切刀时间;
		extern float 断续切刀最小时间;

		extern int 博弈热键;

		extern float 博弈距离范围;
		extern float 博弈对蓄力距离;
		extern float 动态断蓄时间点;
	}

	namespace 半自动博弈 {

		extern bool 总开关;
		extern bool 自动连招;
		extern bool 自动断蓄;
		extern bool 蓄力抓总开关;
		extern bool 蓄力抓振;
		extern bool 蓄力抓长闪;
		extern bool 蓄力抓短闪;
		extern bool 蓄力抓白刀;
		extern bool 蓄力抓倒地;
		extern bool 蓄力抓受击;
		extern bool 蓄力抓起跳;
		extern bool 蓄力抓下蹲;

		extern bool 白刀抓总开关;
		extern bool 白刀抓长闪;
		extern bool 白刀抓短闪;
		extern bool 白刀抓蓄力后摇;
		extern bool 白刀抓振;
		extern bool 白刀蓝顶;
		extern bool 白刀抓受击;

		extern int 动态线程;

		extern int 蓄力抓振延迟;
		extern float 白刀蓝顶延迟;
		extern float 白刀抓短闪最远距离;
		extern float 白刀抓长闪最远距离;
		extern int 白刀抓蓄力后摇延迟;

		extern int 钩锁模式;
		extern float 断续切刀时间;

		extern int 博弈热键;

	}

	namespace LogicKnife
	{
		extern bool BlueAttackShock;	//出蓄抓振刀功能开关
		extern bool BlueAttackWrite;	//出蓄抓白刀功能开关
		extern bool BlueAttackdodge;	//出蓄抓闪功能开关
		extern bool BlueAttacklanding;	//出蓄蓝顶功能开关
		extern bool BlueAttackDuanXu;	//自动断蓄功能开关
		extern bool AutoCombo;			//自动连招功能开关
		extern bool BlueAttackDuanXu_fast;	//快速切刀
		extern bool BlueZhuaJiangZhi;	//出蓄抓站立僵直
		extern bool auto_pickup_weapon;	//自动毛刀
		extern bool NO_Shock_3A;	//不振3A

		extern float 博弈距离范围外;
		extern float 博弈距离范围内;
		extern float 博弈对蓄力距离;
		extern bool 嫖刀开关;
	}
	namespace HeroActivation
	{
		extern bool IsDodge;		//闪避功能开关
		extern bool 躲避火男F;
		extern bool 躲避火男冲拳;
		extern bool 躲避妖刀大招;
		extern bool 躲避顾清寒V1V2;
		extern bool 躲避武田F;
		extern bool 躲避胡为;
		extern bool 躲避宁红叶F;
		extern bool 躲避狐狸大招;
		extern bool 躲避三娘V2;
		extern bool 躲避哈迪大招;
		extern bool 躲避岳山F;
		extern bool 躲避魏轻F1;
		extern bool 躲避魏轻V;
		extern bool 躲避迦南V2;
		extern bool 躲避迦南F2;
		extern bool 躲避刘炼V;
		extern bool 躲避张起灵;
		extern bool 躲避季盈盈V蓄力;
		extern bool 躲避无尘V2;


		extern bool 技能活化;
		extern bool 武田自动毛刀;
		extern bool 和尚自动F2;
		extern bool 季盈盈自动放蓄;
		extern bool 顾清寒自动放蓄;

		namespace 特殊处理
		{
			extern bool 永远躲避顾倾寒V1;
		}
	}
	namespace AiMBot
	{
		extern bool AiMBotState;
		extern char AimKey[0XFF];
		extern float smoothnessY;
		extern float smoothnessX;
		extern float AimRange;
		extern float maxSpeed;   // 大范围移动限制
		extern float smoothingFactor; // 小范围平滑系数
		extern float deadZone;    // 死区阈值
		extern float lockError;   // 锁定误差阈值
		extern float Dt;
		extern int 自瞄热键;
		extern int 总平滑度;
	}
	namespace Flag
	{
		extern bool IsMyManage;					//自身是否可以振刀的FLAG
		extern bool IsBlueToRed;					//自身是否可以续转
		extern bool dodgeBlueToRed;					//自身是否可以闪振
		extern bool ISAtoG;							//判断招式是否可以G震
		extern bool HeroTianHaiF2;				//天海是否可以使用F2
		extern bool 数据遍历;
		extern bool 全局数据;
		extern bool 人物数据;
		extern bool 对象数据;
		extern bool 堆堆数据;
		extern bool 物品数据;
		extern bool 显示未知物品ID;
		extern bool 显示未知堆ID;
		extern bool 开发日志;
		extern bool 性能测试开关;

		extern float 蓝顶延迟;
	}
}	




struct Bone_D
{
	enum Bone : int {
		头部 = 10,
		脖子 = 9,
		胸部 = 8,
		盆骨 = 5,
		左肩 = 40,
		左肘 = 41,
		左手 = 42,
		右肩 = 12,
		右肘 = 13,
		右手 = 14,
		左大腿 = 69,
		左膝盖 = 70,
		左脚跟 = 71,
		右大腿 = 77,
		右膝盖 = 78,
		右脚跟 = 79,

	};
	list<INT> _上部 = { 脖子, 胸部 };
	list<INT> _右臂 = { 脖子, 右肩, 右肘, 右肘, 右手 };
	list<INT> _左臂 = { 脖子, 左肩, 左肘, 左肘, 左手 };
	list<INT> _脊柱 = { 胸部, 盆骨 };
	list<INT> _右腿 = { 盆骨, 右大腿 , 右膝盖, 右膝盖, 右脚跟 };
	list<INT> _左腿 = { 盆骨, 左大腿 , 左膝盖, 左膝盖, 左脚跟 };
	list<list<INT>> BoneList = { _上部, _右臂, _左臂, _脊柱, _右腿, _左腿 };
};


struct ItemInfoData
{
	uint32_t ID = 0;
	uint32_t  ItemType = 0;
	Vector3 Pos = { 0.f,0.f,0.f };
	string Name = {};
	ImColor Color = {};
};
struct InteractiveInfoData
{
	uint32_t ID = 0;
	uint32_t status = 0;//是否被打开
	Vector3 Pos = { 0.f,0.f,0.f };
	string Name = {};
	ImColor Color = {};
	uint32_t  ItemType = 0;
};


inline float MATRIX[4][4];
inline vector<ObjData> g_GameData;  // 原"全_游戏数据"
inline vector<PlayerData> g_PlayerData;// 原"全_玩家数据"
inline std::atomic<uint64_t> g_localplayer_reads_per_second{0};
inline vector<ItemInfoData> 全_物品数据;
inline vector<InteractiveInfoData> 全_金堆数据;
inline vector<Vec2> 全_骨骼数据;
extern LocalPlayer_Data1 LocalPlayer_Data;


extern DoubleBuffer data_buffers;
extern std::mutex g_InteractiveMutex;





namespace Color
{
	inline ImColor 大理石灰 = ImColor(196, 203, 207, 255);
	inline ImColor 润红 = ImColor(221, 68, 68, 255);
	inline ImColor 竹篁绿 = ImColor(119, 252, 119, 255);
	inline ImColor 星蓝 = ImColor(85, 117, 209, 255);
	inline ImColor 向日葵黄 = ImColor(239, 221, 53, 255);
	inline ImColor 美人焦橙 = ImColor(205, 149, 12, 255);
	inline ImColor 淡牵牛紫 = ImColor(193, 82, 254, 255);
	inline ImColor 白色 = ImColor(255, 255, 255, 255);
}
struct ItemData
{
	string Name;
	ImColor color;
	uint32_t itemType;
};

struct InteraData
{
	string Name;
	ImColor color;
	uint32_t itemType;
};

inline map<uint32_t, ItemData> itemTable = {

	{3040002,{"魂冢",Color::向日葵黄,1}},
	//果实
	{3010008,{"[果实]-刺梨",Color::大理石灰,2}},
	{3010009,{"[果实]-沙叻",Color::润红,2}},
	{3010050,{"[果实]-沙梨",Color::美人焦橙,2}},
	{3010007,{"[状态]-蒲公英",Color::大理石灰,2}},
	//秘籍
	{ 3040066, {"[秘籍]-长剑秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040067, {"[秘籍]-阔刀秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040068, {"[秘籍]-匕首秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040069, {"[秘籍]-双刀秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040070, {"[秘籍]-太刀秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040084, {"[秘籍]-七星决秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040085, {"[秘籍]-天罡决秘籍",Color::美人焦橙,3} },	// 金色
	{ 3040086, {"[秘籍]-气经秘籍",Color::美人焦橙,3} },	// 金色
	//道具
	{3040001 , {"[道具]-暗潮币",Color::美人焦橙,4}},
	{3010004 , {"[道具]-凝血丸",Color::大理石灰,4}},
	{3010006 , {"[道具]-大包凝血丸",Color::润红,4}},
	{3010023 , {"[道具]-护甲粉末",Color::大理石灰,4}},
	{3010024 , {"[道具]-高级护甲粉末",Color::淡牵牛紫,4}},
	{3010018 , {"[道具]-武备匣",Color::大理石灰,4}},
	{3010020 , {"[道具]-飞索线轴",Color::星蓝,4}},

	{3010005,{"[怒气]-萤火虫",Color::美人焦橙,14}},
	//护甲
	{3005001 ,{"护甲Lv1",Color::大理石灰,10}},
	{3005002 ,{"护甲Lv2",Color::星蓝,11}},
	{3005003 ,{"护甲Lv3",Color::淡牵牛紫,12}},
	{3005004 ,{"护甲Lv4",Color::美人焦橙,13}},
	{3005005 ,{"护甲Lv5",Color::美人焦橙,15}},
	

	{3010001 , {"[扩容]-魂玉扩容",Color::星蓝,23}},
	{3010002 , {"[扩容]-武器扩容",Color::星蓝,24}},
	{3010003 , {"[扩容]-道具扩容",Color::星蓝,25}},
	//近战
	{3000100 , {"[近战]-匕首",Color::大理石灰,5}},
	{3000110 , {"[近战]-匕首",Color::星蓝,17}},
	{3000120 , {"[近战]-匕首",Color::淡牵牛紫,18}},
	{3000130 , {"[近战]-匕首",Color::美人焦橙,19}},
	{3001500 , {"[近战]-双截棍",Color::大理石灰,5}},
	{3001510 , {"[近战]-双截棍",Color::星蓝,17}},
	{3001520 , {"[近战]-双截棍",Color::淡牵牛紫,18}},
	{3001530 , {"[近战]-双截棍",Color::美人焦橙,19}},
	{3000001 , {"[近战]-长剑",Color::大理石灰,5}},
	{3000011 , {"[近战]-长剑",Color::星蓝,17}},
	{3000021 , {"[近战]-长剑",Color::淡牵牛紫,18}},
	{3000031 , {"[近战]-长剑",Color::美人焦橙,19}},
	{3000300 , {"[近战]-太刀",Color::大理石灰,5}},
	{3000310 , {"[近战]-太刀",Color::星蓝,17}},
	{3000320 , {"[近战]-太刀",Color::淡牵牛紫,18}},
	{3000330 , {"[近战]-太刀",Color::美人焦橙,19}},
	{3000400 , {"[近战]-阔刀",Color::大理石灰,5}},
	{3000410 , {"[近战]-阔刀",Color::星蓝,17}},
	{3000420 , {"[近战]-阔刀",Color::淡牵牛紫,18}},
	{3000430 , {"[近战]-阔刀",Color::美人焦橙,19}},
	{3000200 , {"[近战]-枪",Color::大理石灰,5}},
	{3000210 , {"[近战]-枪",Color::星蓝,17}},
	{3000220 , {"[近战]-枪",Color::淡牵牛紫,18}},
	{3000230 , {"[近战]-枪",Color::美人焦橙,19}},
	{3001600 , {"[近战]-双刀",Color::大理石灰,5}},
	{3001610 , {"[近战]-双刀",Color::星蓝,17}},
	{3001620 , {"[近战]-双刀",Color::淡牵牛紫,18}},
	{3001630 , {"[近战]-双刀",Color::美人焦橙,19}},
	{3001700 , {"[近战]-斩马刀",Color::大理石灰,5}},
	{3001710 , {"[近战]-斩马刀",Color::星蓝,17}},
	{3001720 , {"[近战]-斩马刀",Color::淡牵牛紫,18}},
	{3001730 , {"[近战]-斩马刀",Color::美人焦橙,19}},
	{3001900 , {"[近战]-双戟",Color::大理石灰,5}},
	{3001910 , {"[近战]-双戟",Color::星蓝,17}},
	{3001920 , {"[近战]-双戟",Color::淡牵牛紫,18}},
	{3001930 , {"[近战]-双戟",Color::美人焦橙,19}},
	{3002000 , {"[近战]-扇子",Color::大理石灰,5}},
	{3002010 , {"[近战]-扇子",Color::星蓝,17}},
	{3002020 , {"[近战]-扇子",Color::淡牵牛紫,18}},
	{3002030 , {"[近战]-扇子",Color::美人焦橙,19}},
	{3001800 , {"[近战]-长棍",Color::大理石灰,5}},
	{3001810 , {"[近战]-长棍",Color::星蓝,17}},
	{3001820 , {"[近战]-长棍",Color::淡牵牛紫,18}},
	{3001830 , {"[近战]-长棍",Color::美人焦橙,19}},
	{3202310 , {"[近战]-拳刃",Color::大理石灰,5}},
	{3202320 , {"[近战]-拳刃",Color::星蓝,17}},
	{3202330 , {"[近战]-拳刃",Color::淡牵牛紫,18}},
	{3202340 , {"[近战]-拳刃",Color::美人焦橙,19}},
	{3202410 , {"[近战]-飞刀",Color::大理石灰,5}},
	{3202420 , {"[近战]-飞刀",Color::星蓝,17}},
	{3202430 , {"[近战]-飞刀",Color::淡牵牛紫,18}},
	{3202440 , {"[近战]-飞刀",Color::美人焦橙,19}},
	//远程
	{ 3001200,{"[远程]-鸟铳",Color::大理石灰,6}},
	{ 3001210,{"[远程]-鸟铳",Color::星蓝,20}},
	{ 3001220,{"[远程]-鸟铳",Color::淡牵牛紫,21}},
	{ 3001230,{"[远程]-鸟铳",Color::美人焦橙,22}},
	{ 3000501,{"[远程]-弓箭",Color::大理石灰,6}},
	{ 3000511,{"[远程]-弓箭",Color::星蓝,20}},
	{ 3000521,{"[远程]-弓箭",Color::淡牵牛紫,21}},
	{ 3000531,{"[远程]-弓箭",Color::美人焦橙,22}},
	{ 3000800,{"[远程]-火炮",Color::大理石灰,6}},
	{ 3000810,{"[远程]-火炮",Color::星蓝,20}},
	{ 3000820,{"[远程]-火炮",Color::淡牵牛紫,21}},
	{ 3000830,{"[远程]-火炮",Color::美人焦橙,22}},
	{ 3001000,{"[远程]-一窝蜂",Color::大理石灰,6}},
	{ 3001010,{"[远程]-一窝蜂",Color::星蓝,20}},
	{ 3001020,{"[远程]-一窝蜂",Color::淡牵牛紫,21}},
	{ 3001030,{"[远程]-一窝蜂",Color::美人焦橙,22}},
	{ 3001100,{"[远程]-五眼铳",Color::大理石灰,6}},
	{ 3001110,{"[远程]-五眼铳",Color::星蓝,20}},
	{ 3001120,{"[远程]-五眼铳",Color::淡牵牛紫,21}},
	{ 3001130,{"[远程]-五眼铳",Color::美人焦橙,22}},
	{ 3000700,{"[远程]-连弩",Color::大理石灰,6}},
	{ 3000710,{"[远程]-连弩",Color::星蓝,20}},
	{ 3000720,{"[远程]-连弩",Color::淡牵牛紫,21}},
	{ 3000730,{"[远程]-连弩",Color::美人焦橙,22}},
	{ 3000900,{"[远程]-喷火筒",Color::大理石灰,6}},
	{ 3000910,{"[远程]-喷火筒",Color::星蓝,20}},
	{ 3000920,{"[远程]-喷火筒",Color::淡牵牛紫,21}},
	{ 3000930,{"[远程]-喷火筒",Color::美人焦橙,22}},
	//夺魂
	{ 3020243,{"[魂玉]-夺魂",Color::淡牵牛紫,7} },
	{ 3010126,{"[魂玉]-铜币",Color::淡牵牛紫,7} },
	//金魂玉
	{ 3020084,{"[魂玉]-噬魂斩",Color::美人焦橙,8} },
	{ 3020127,{"[魂玉]-惊雷十劫",Color::美人焦橙,8} },
	{ 3020132,{"[魂玉]-巽风震雷刀",Color::美人焦橙,8} },
	{ 3020193,{"[魂玉]-过关斩将",Color::美人焦橙,8} },
	{ 3020185,{"[魂玉]-七星夺窍",Color::美人焦橙,8} },
	{ 3020091,{"[魂玉]-凤凰羽",Color::美人焦橙,8} },
	{ 3020145,{"[魂玉]-风火穿心脚",Color::美人焦橙,8} },
	{ 3020220,{"[魂玉]-武道·六合枪",Color::美人焦橙,8} },
	{ 3020188,{"[魂玉]-五情七灭阵",Color::美人焦橙,8} },
	{ 3020138,{"[魂玉]-狂浪怒涛",Color::美人焦橙,8} },
	{ 3020195,{"[魂玉]-定海针·镇地撑天",Color::美人焦橙,8} },
	{ 3020187,{"[魂玉]-捅劲",Color::美人焦橙,8} },
	{ 3020142,{"[魂玉]-鬼哭神嚎",Color::美人焦橙,8} },
	{ 3020211,{"[魂玉]-缠龙奔野",Color::美人焦橙,8} },
	{ 3020158,{"[魂玉]-虎啸龙咆",Color::美人焦橙,8} },
	{ 3020146,{"[魂玉]-扬鞭劲",Color::美人焦橙,8} },
	{ 3020183,{"[魂玉]-乾坤日月斩",Color::美人焦橙,8} },
	{ 3020219,{"[魂玉]-武道·八斩刀",Color::美人焦橙,8} },
	{ 3020206,{"[魂玉]-战龙在天",Color::美人焦橙,8} },
	{ 3020177,{"[魂玉]-亢龙有悔",Color::美人焦橙,8} },
	{ 3020221,{"[魂玉]-极光碎云闪",Color::美人焦橙,8} },
	{ 3020074,{"[魂玉]-完璧",Color::美人焦橙,8} },
	{ 3020075,{"[魂玉]-光佑",Color::美人焦橙,8} },
	{ 3020078,{"[魂玉]-幻形淌&虚影步",Color::美人焦橙,8} },
	{ 3020150,{"[魂玉]-万夫莫敌",Color::美人焦橙,8} },
	{ 3020173,{"[魂玉]-强袭·离火式",Color::美人焦橙,8} },
	{ 3020101,{"[魂玉]-爆裂箭",Color::美人焦橙,8} },
	{ 3020122,{"[魂玉]-火龙炮",Color::美人焦橙,8} },
	{ 3020117,{"[魂玉]-爆风炮&燃烧",Color::美人焦橙,8} },
	{ 3020121,{"[魂玉]-御地雷",Color::美人焦橙,8} },
	{ 3020110,{"[魂玉]-穿甲弹",Color::美人焦橙,8} },
	{ 3020156,{"[魂玉]-续命术",Color::美人焦橙,8} },
	{ 3020116,{"[魂玉]-魂燃一线",Color::美人焦橙,8} },
	{ 3020124,{"[魂玉]-抽芯补天",Color::美人焦橙,8} },
	{ 3020151,{"[魂玉]-灵光一现",Color::美人焦橙,8} },
	{ 3020079,{"[魂玉]-毕工淌",Color::美人焦橙,8} },
	{ 3020164,{"[魂玉]-召雷术",Color::美人焦橙,8} },

	//属性
	{ 3020001,{"[魂玉]-大魂玉·体力",Color::美人焦橙,9}},
	{ 3020011,{"[魂玉]-大魂玉·攻击",Color::美人焦橙,9}},
	{ 3020051,{"[魂玉]-大魂玉·近抗",Color::美人焦橙,9}},
	{ 3020061,{"[魂玉]-大魂玉·远抗",Color::美人焦橙,9}},

	
};

inline ItemData GetItemInfo(uint32_t id) {
	auto it = itemTable.find(id);
	if (it != itemTable.end()) {
		return it->second;
	}
	static ItemData defaultData;
	defaultData = { std::to_string(id), Color::白色, 99 };
	return defaultData;
}
inline map<uint32_t, InteraData > InteractiveTable =
{
	{ (4004000),{"[落物堆]-绿1",Color::竹篁绿,1}},// 聚窟州  单
	{ (4004003),{"[落物堆]-绿2",Color::竹篁绿,1}},// 聚窟州  双
	{ (4004006),{"[落物堆]-绿3",Color::竹篁绿,1}},// 聚窟州  三
	{ (4004015),{"[落物堆]-绿4",Color::竹篁绿,1}},// 火罗国  单 三
	{ (4004021),{"[落物堆]-绿5",Color::竹篁绿,1}},// 火罗国  双
	{ (4004033),{"[落物堆]-绿6",Color::竹篁绿,1}},// 龙隐洞天  单
	{ (4004042),{"[落物堆]-绿7",Color::竹篁绿,1}},// 龙隐洞天  单
	{ (4004030),{"[落物堆]-绿8",Color::竹篁绿,1}},// 龙隐洞天  双
	{ (4004043),{"[落物堆]-绿9",Color::竹篁绿,1}},// 龙隐洞天  双 三
	{ 4000003,	{"[落物堆]-绿10",Color::竹篁绿,1}},
    { 4000007 , {"[落物堆]-绿11",Color::竹篁绿,1}},
    { 4000016 , {"[落物堆]-绿12",Color::竹篁绿,1}},
    { 4000019 , {"[落物堆]-绿13",Color::竹篁绿,1}},
    { 4000305 , {"[落物堆]-绿14",Color::竹篁绿,1}},
    { 4002004 , {"[落物堆]-绿15",Color::竹篁绿,1}},
    { 4002012 , {"[落物堆]-绿16",Color::竹篁绿,1}},
    { 4002105 , {"[落物堆]-绿17",Color::竹篁绿,1}},
    { 4002112 , {"[落物堆]-绿18",Color::竹篁绿,1}},
    { 4002200 , {"[落物堆]-绿19",Color::竹篁绿,1}},
    { 4002203 , {"[落物堆]-绿20",Color::竹篁绿,1}},
    { 4002208 , {"[落物堆]-绿21",Color::竹篁绿,1}},
    { 4002211 , {"[落物堆]-绿22",Color::竹篁绿,1}},
    { 4002900 , {"[落物堆]-绿23",Color::竹篁绿,1}},
    { 4002901 , {"[落物堆]-绿24",Color::竹篁绿,1}},
    { 4002920 , {"[落物堆]-绿25",Color::竹篁绿,1}},
    { 4002923 , {"[落物堆]-绿26",Color::竹篁绿,1}},
    { 4002928 , {"[落物堆]-绿27",Color::竹篁绿,1}},
    { 4002931 , {"[落物堆]-绿28",Color::竹篁绿,1}},
    { 4002932 , {"[落物堆]-绿29",Color::竹篁绿,1}},
    { 4002935 , {"[落物堆]-绿30",Color::竹篁绿,1}},
    { 4002937 , {"[落物堆]-绿31",Color::竹篁绿,1}},
    { 4002940 , {"[落物堆]-绿32",Color::竹篁绿,1}},
    { 4002942 , {"[落物堆]-绿33",Color::竹篁绿,1}},
    { 4002943 , {"[落物堆]-绿34",Color::竹篁绿,1}},
    { 4002946 , {"[落物堆]-绿35",Color::竹篁绿,1}},
    { 4002949 , {"[落物堆]-绿36",Color::竹篁绿,1}},
    { 4002954 , {"[落物堆]-绿37",Color::竹篁绿,1}},
    { 4002957 , {"[落物堆]-绿38",Color::竹篁绿,1}},
    { 4002958 , {"[落物堆]-绿39",Color::竹篁绿,1}},
    { 4002964 , {"[落物堆]-绿40",Color::竹篁绿,1}},
    { 4002961 , {"[落物堆]-绿41",Color::竹篁绿,1}},
    { 4002958 , {"[落物堆]-绿42",Color::竹篁绿,1}},
    { 4002979 , {"[落物堆]-绿43",Color::竹篁绿,1}},
    { 4002976 , {"[落物堆]-绿44",Color::竹篁绿,1}},
    { 4002973 , {"[落物堆]-绿45",Color::竹篁绿,1}},
	{ 4002803 , {"[落物堆]-绿46",Color::竹篁绿,1}},
	{ 4002300 , {"[落物堆]-绿47",Color::竹篁绿,1}},
	{ 4002313 , {"[落物堆]-绿48",Color::竹篁绿,1}},
    { 4002303 , {"[落物堆]-绿49",Color::竹篁绿,1}},
    { 4002317 , {"[落物堆]-绿40",Color::竹篁绿,1}},
	{ 4002800 , {"[落物堆]-绿50",Color::竹篁绿,1}},
	{ 4002812 , {"[落物堆]-绿51",Color::竹篁绿,1}},
	{ 4002816 , {"[落物堆]-绿52",Color::竹篁绿,1}},
	{ 4004033 , {"[落物堆]-绿53",Color::竹篁绿,1}},
	{ 4004218 , {"[落物堆]-绿54",Color::竹篁绿,1}},
	{ 4004203 , {"[落物堆]-绿55",Color::竹篁绿,1}},
	{ 4004233 , {"[落物堆]-绿56",Color::竹篁绿,1}},
	{ 4004230 , {"[落物堆]-绿57",Color::竹篁绿,1}},
	{ 4004200 , {"[落物堆]-绿58",Color::竹篁绿,1}},
	{ 4004215 , {"[落物堆]-绿59",Color::竹篁绿,1}},

	{ (4004001),{"[落物堆]-蓝1",Color::星蓝,2}},// 聚窟州 单
	{ (4004004),{"[落物堆]-蓝2",Color::星蓝,2}},// 聚窟州 双
	{ (4000101),{"[落物堆]-蓝3",Color::星蓝,2}},// 聚窟州 三
	{ (4004019),{"[落物堆]-蓝4",Color::星蓝,2}},// 火罗国  单 
	{ (4004016),{"[落物堆]-蓝5",Color::星蓝,2}},// 火罗国  双 三
	{ (4004034),{"[落物堆]-蓝6",Color::星蓝,2}},// 龙隐洞天  单
	{ (4004031),{"[落物堆]-蓝7",Color::星蓝,2}},// 龙隐洞天  双
	{ 4000005 , {"[落物堆]-蓝8",Color::星蓝,2}},
	{ 4000008 , {"[落物堆]-蓝9",Color::星蓝,2}},
    { 4000014 , {"[落物堆]-蓝10",Color::星蓝,2}},
    { 4000017 , {"[落物堆]-蓝11",Color::星蓝,2}},
    { 4002002 , {"[落物堆]-蓝12",Color::星蓝,2}},
    { 4002010 , {"[落物堆]-蓝13",Color::星蓝,2}},
    { 4002106 , {"[落物堆]-蓝14",Color::星蓝,2}},
    { 4002113 , {"[落物堆]-蓝15",Color::星蓝,2}},
    { 4002201 , {"[落物堆]-蓝16",Color::星蓝,2}},
    { 4002204 , {"[落物堆]-蓝17",Color::星蓝,2}},
    { 4002206 , {"[落物堆]-蓝18",Color::星蓝,2}},
    { 4002209 , {"[落物堆]-蓝19",Color::星蓝,2}},
    { 4002921 , {"[落物堆]-蓝20",Color::星蓝,2}},
    { 4002924 , {"[落物堆]-蓝21",Color::星蓝,2}},
    { 4002926 , {"[落物堆]-蓝22",Color::星蓝,2}},
    { 4002929 , {"[落物堆]-蓝23",Color::星蓝,2}},
    { 4002933 , {"[落物堆]-蓝24",Color::星蓝,2}},
    { 4002947 , {"[落物堆]-蓝25",Color::星蓝,2}},
    { 4002950 , {"[落物堆]-蓝26",Color::星蓝,2}},
    { 4002952 , {"[落物堆]-蓝27",Color::星蓝,2}},
    { 4002955 , {"[落物堆]-蓝28",Color::星蓝,2}},
    { 4002959 , {"[落物堆]-蓝29",Color::星蓝,2}},
    { 4002965 , {"[落物堆]-蓝30",Color::星蓝,2}},
    { 4002962 , {"[落物堆]-蓝31",Color::星蓝,2}},
    { 4002959 , {"[落物堆]-蓝32",Color::星蓝,2}},
    { 4002980 , {"[落物堆]-蓝33",Color::星蓝,2}},
    { 4002977 , {"[落物堆]-蓝34",Color::星蓝,2}},
    { 4002974 , {"[落物堆]-蓝35",Color::星蓝,2}},
	{ 4002804 , {"[落物堆]-蓝36",Color::星蓝,2}},
	{ 4002304 , {"[落物堆]-蓝37",Color::星蓝,2}},
	{ 4002801 , {"[落物堆]-蓝38",Color::星蓝,2}},
	{ 4002301 , {"[落物堆]-蓝39",Color::星蓝,2}},
	{ 4004231 , {"[落物堆]-蓝40",Color::星蓝,2}},
	{ 4004219 , {"[落物堆]-蓝41",Color::星蓝,2}},
	{ 4004204 , {"[落物堆]-蓝42",Color::星蓝,2} },
	{ 4004234 , {"[落物堆]-蓝43",Color::星蓝,2} },
	{ 4004201 , {"[落物堆]-蓝44",Color::星蓝,2} },
	{ 4004216 , {"[落物堆]-蓝45",Color::星蓝,2} },
	
	{ (4004002),{"[落物堆]-金1",Color::向日葵黄,3}},// 聚窟州 单
	{ (4004005),{"[落物堆]-金2",Color::向日葵黄,3}},// 聚窟州 双
	{ (4004008),{"[落物堆]-金3",Color::向日葵黄,3}},// 聚窟州 三
	{ (4004020),{"[落物堆]-金4",Color::向日葵黄,3} },// 火罗国 单
	{ (4004017),{"[落物堆]-金5",Color::向日葵黄,3} },// 火罗国 双 三
	{ (4004035),{"[落物堆]-金6",Color::向日葵黄,3} },// 龙隐洞天  单
	{ (4004047),{"[落物堆]-金7",Color::向日葵黄,3} },// 龙隐洞天  单
	{ (4004032),{"[落物堆]-金8",Color::向日葵黄,3} },// 龙隐洞天  双
	{ (4004046),{"[落物堆]-金9",Color::向日葵黄,3} },// 龙隐洞天  双  三
	{ 4000006 , {"[落物堆]-金10",Color::向日葵黄,3}},
    { 4000009 , {"[落物堆]-金11",Color::向日葵黄,3}},
    { 4000015 , {"[落物堆]-金12",Color::向日葵黄,3}},
    { 4000018 , {"[落物堆]-金13",Color::向日葵黄,3}},
    { 4000203 , {"[落物堆]-金14",Color::向日葵黄,3}},
    { 4000204 , {"[落物堆]-金15",Color::向日葵黄,3}},
    { 4002003 , {"[落物堆]-金16",Color::向日葵黄,3}},
    { 4002011 , {"[落物堆]-金17",Color::向日葵黄,3}},
    { 4002107 , {"[落物堆]-金18",Color::向日葵黄,3}},
    { 4002114 , {"[落物堆]-金19",Color::向日葵黄,3}},
    { 4002202 , {"[落物堆]-金20",Color::向日葵黄,3}},
    { 4002205 , {"[落物堆]-金21",Color::向日葵黄,3}},
    { 4002207 , {"[落物堆]-金22",Color::向日葵黄,3}},
    { 4002210 , {"[落物堆]-金23",Color::向日葵黄,3}},
    { 4002902 , {"[落物堆]-金24",Color::向日葵黄,3}},
    { 4002903 , {"[落物堆]-金25",Color::向日葵黄,3}},
    { 4002922 , {"[落物堆]-金26",Color::向日葵黄,3}},
    { 4002925 , {"[落物堆]-金27",Color::向日葵黄,3}},
    { 4002927 , {"[落物堆]-金28",Color::向日葵黄,3}},
    { 4002930 , {"[落物堆]-金29",Color::向日葵黄,3}},
    { 4002934 , {"[落物堆]-金30",Color::向日葵黄,3}},
    { 4002936 , {"[落物堆]-金31",Color::向日葵黄,3}},
    { 4002938 , {"[落物堆]-金32",Color::向日葵黄,3}},
    { 4002939 , {"[落物堆]-金33",Color::向日葵黄,3}},
    { 4002941 , {"[落物堆]-金34",Color::向日葵黄,3}},
    { 4002944 , {"[落物堆]-金35",Color::向日葵黄,3}},
    { 4002948 , {"[落物堆]-金36",Color::向日葵黄,3}},
    { 4002951 , {"[落物堆]-金37",Color::向日葵黄,3}},
    { 4002953 , {"[落物堆]-金38",Color::向日葵黄,3}},
    { 4002956 , {"[落物堆]-金39",Color::向日葵黄,3}},
    { 4002966 , {"[落物堆]-金40",Color::向日葵黄,3}},
    { 4002963 , {"[落物堆]-金41",Color::向日葵黄,3}},
    { 4002960 , {"[落物堆]-金42",Color::向日葵黄,3}},
    { 4002981 , {"[落物堆]-金43",Color::向日葵黄,3}},
    { 4002978 , {"[落物堆]-金44",Color::向日葵黄,3}},
    { 4002975 , {"[落物堆]-金45",Color::向日葵黄,3}},
	{ 4002817 , {"[落物堆]-金46",Color::向日葵黄,3}},
	{ 4002802 , {"[落物堆]-金47",Color::向日葵黄,3}},
	{ 4002805 , {"[落物堆]-金48",Color::向日葵黄,3}},
	{ 4002305 , {"[落物堆]-金49",Color::向日葵黄,3}},
	{ 4002316 , {"[落物堆]-金50",Color::向日葵黄,3}},
	{ 4002302 , {"[落物堆]-金51",Color::向日葵黄,3}},
	{ 4002813 , {"[落物堆]-金52",Color::向日葵黄,3}},
	{ 4004220 , {"[落物堆]-金53",Color::向日葵黄,3}},
	{ 4004205 , {"[落物堆]-金54",Color::向日葵黄,3} },
	{ 4004235 , {"[落物堆]-金55",Color::向日葵黄,3} },
	{ 4004232 , {"[落物堆]-金56",Color::向日葵黄,3} },
	{ 4004202 , {"[落物堆]-金57",Color::向日葵黄,3} },
	{ 4004217 , {"[落物堆]-金58",Color::向日葵黄,3} },
   


	{ 4000999 , {"[任务]-普通",Color::向日葵黄,4}},
	{ 4000998 , {"[任务]-追击",Color::淡牵牛紫,4}},
	{ 4000997 , {"[任务]-叫阵",Color::润红,4}},
};

namespace BlueAttackFront
{
	inline float 长剑剑气 = 0.232f;
	inline float 长剑凤凰羽 = 0.222f;
	inline float 太刀百裂 = 0.212f;
	inline float 太刀噬魂斩 = 0.f;
	inline float 阔刀站走右蓄 = 0.212f;
	inline float 阔刀滑步右蓄 = 0.303f;
	inline float 阔刀左蓄1段 = 0.492f;
	inline float 阔刀左蓄2段 = 0.522f;
	inline float 阔刀左蓄3段 = 0.463f;
	inline float 阔刀左右 = 0.243f;
	inline float 阔刀左右捏蓄 = 0.303f;
	inline float 阔刀右右 = 0.496f;
	inline float 长枪左蓄 = 0.402f;
	inline float 长枪右蓄 = 0.342f;
	inline float 长枪风火穿心脚 = 0.342f;
	inline float 匕首左蓄 = 0.204f;
	inline float 匕首右蓄 = 0.202f;
	inline float 匕首亢龙有悔 = 0.204f;
	inline float 双节棍左蓄 = 0.283f;
	inline float 双节棍右蓄 = 0.213f;
	inline float 双节棍扬鞭劲 = 0.148f;
	inline float 双节棍虎啸龙咆 = 0.123f;
	inline float 双刀乾坤日月斩 = 0.322f;
	inline float 双刀右蓄 = 0.203f;
	inline float 双刀左蓄 = 0.213f;
	inline float 长棍左蓄 = 0.254f;
	inline float 长棍右蓄 = 0.248f;
	inline float 长棍桶劲 = 0.201f;
	inline float 长棍腾云式 = 0.27f;
	inline float 斩马刀左蓄 = 0.232f;
	inline float 斩马刀左右 = 0.247f;
	inline float 斩马刀右右 = 0.497f;
	inline float 斩马刀右蓄 = 0.402f;
	inline float 斩马刀炽焰斩 = 0.252f;
	inline float 斩马刀过关斩将 = 0.452f;
	inline float 扇子左蓄 = 0.212f;
	inline float 扇子右蓄 = 0.202f;
	inline float 扇子缠龙奔野 = 0.222f;
	inline float 横刀左蓄 = 0.485f;
	inline float 横刀右蓄 = 0.225f;
	inline float 横刀极光碎云闪 = 0.248f;
	inline float 拳刃左蓄 = 0.217f;
	inline float 拳刃右蓄 = 0.237f;
	inline float 拳刃太极弄云手 = 0.252f;
	inline float 飞刀左蓄 = 0.257f;
	inline float 飞刀右蓄 = 0.237f;
	inline float 飞刀掌心雷 = 0.217f;
	inline float 飞刀断月千刃舞 = 0.212f;
}
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力1;
	int 左蓄力2;
	int 右蓄力1;
	int 右蓄力2;
	int 地龙滚堂刹;
	int 苍牙;
	int 跳斩;
	int 壁击;
	int 火龙卷云;
	int 断罪碎蜂;
	int 蓄力追击;
}链剑振刀方式;	

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力1;
	int 左蓄力2;
	int 右蓄力1;
	int 右蓄力2;
	int 地龙滚堂刹;
	int 苍牙;
	int 跳斩;
	int 壁击;
	int 火龙卷云;
	int 断罪碎蜂;
	int 蓄力追击;
}链剑闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 右蓄力;
	float 地龙滚堂刹;
	float 苍牙;
	float 跳斩;
	float 壁击;
	float 火龙卷云;
	float 断罪碎蜂;
	float 蓄力追击;
}链剑振刀方式2;
typedef struct
{
	int 左3;
	int 右3;
	int 左剑气1;
	int 左剑气2;
	int 右剑气1;
	int 右剑气2;
	int 凤凰羽1;
	int 凤凰羽2;
	int 苍牙;
	int 跳斩;
	int 壁击;
	int 七星夺窍;
	int 天外飞仙;
}长剑振刀方式;		//0-13米
typedef struct
{
	int 左3;
	int 右3;
	int 左剑气1;
	int 左剑气2;
	int 右剑气1;
	int 右剑气2;
	int 凤凰羽1;
	int 凤凰羽2;
	int 苍牙;
	int 跳斩;
	int 壁击;
	int 七星夺窍;
	int 天外飞仙;
}长剑闪避延迟;		//0-13米
typedef struct 
{
	float 左3;
	float 右3;
	float 左剑气;
	float 右剑气;
	float 凤凰羽;
	float 苍牙;
	float 跳斩;
	float 壁击;
	float 七星夺窍;
	float 天外飞仙;
}长剑1;		//0-13米
typedef struct
{
	float 左3;
	float 右3;
	float 左剑气;
	float 右剑气;
	float 凤凰羽;
	float 苍牙;
	float 跳斩;
	float 七星夺窍;
	float 天外飞仙;
	float 壁击;
}长剑3;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 壁击;
	float 刹那斩;
	float 惊雷;
	float 惊雷十劫;
	float 青鬼;
	float 同源;
	float 噬魂斩;
}太刀1;			//0-11M;
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄1;
	int 左蓄2;
	int 右蓄1;
	int 右蓄2;
	int 苍牙;
	int 壁击;
	int 刹那斩1;
	int 刹那斩2;
	int 惊雷;
	int 惊雷十劫;
	int 青鬼;
	int 同源;
	int 噬魂斩1;
	int 噬魂斩2;
}太刀振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄1;
	int 左蓄2;
	int 右蓄1;
	int 右蓄2;
	int 苍牙;
	int 壁击;
	int 刹那斩1;
	int 刹那斩2;
	int 惊雷;
	int 惊雷十劫;
	int 青鬼;
	int 同源;
	int 噬魂斩1;
	int 噬魂斩2;
}太刀闪避延迟;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 刹那斩;
	float 右惊雷;	//惊雷十劫和此参数一样
	float 青鬼;
	float 同源;
	float 噬魂斩;
	float 壁击;
}太刀3;
typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 左蓄一段;
	float 左蓄二段;
	float 左蓄三段;
	float 右蓄;
	float 右右蓄;
	float 雷刀;
	float 壁击;
	float 同源;
	float 雷刀下劈;
}阔刀1;			//0-9.5M
typedef struct
{
	int 左左;
	int 左右;
	int 右右;
	int 右左;
	int 左蓄一段;
	int 左蓄二段;
	int 左蓄三段;
	int 右蓄1;
	int 右蓄2;
	int 右右蓄;
	int 雷刀;
	int 壁击;
	int 同源;
	int 雷刀下劈;
	int 翻江倒海;
}阔刀振刀方式;

typedef struct
{
	int 左左;
	int 左右;
	int 右右;
	int 右左;
	int 左蓄一段;
	int 左蓄二段;
	int 左蓄三段;
	int 右蓄1;
	int 右蓄2;
	int 右右蓄;
	int 雷刀;
	int 壁击;
	int 同源;
	int 雷刀下劈;
	int 翻江倒海;
}阔刀闪避延迟;

typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 左蓄一段;
	float 左蓄二段;
	float 左蓄三段;
	float 右蓄;
	float 右右蓄;
	float 雷刀;
	float 壁击;
	float 同源;
	float 雷刀下劈;
}阔刀3;			//0-9.5M
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 六合枪;
	float 右蓄;
	float 壁击;
	float 同源;
	float 大圣游;
	float 风卷云残;
	float 双环扫;
	float 龙王破;
	float 穿心脚;
}长枪1;			//0-11M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 六合枪;
	int 右蓄;
	int 壁击;
	int 同源;
	int 大圣游;
	int 风卷云残;
	int 双环扫;
	int 龙王破;
	int 穿心脚;
}长枪振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 六合枪;
	int 右蓄;
	int 壁击;
	int 同源;
	int 大圣游;
	int 风卷云残;
	int 双环扫;
	int 龙王破;
	int 穿心脚;
}长枪闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 六合枪;
	float 右蓄;
	float 壁击;
	float 同源;
	float 大圣游;
	float 风卷云残;
	float 双环扫;
	float 龙王破;
	float 穿心脚;
}长枪3;			//0-11M
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 鬼反;
	float 荆轲献匕;
	float 壁击;
	float 鬼刃暗扎;
	float 鬼哭神嚎;
	float 亢龙有悔;
}匕首1;			//0-10M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 鬼反;
	int 荆轲献匕;
	int 壁击;
	int 鬼刃暗扎;
	int 鬼哭神嚎;
	int 亢龙有悔;
}匕首振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 鬼反;
	int 荆轲献匕;
	int 壁击;
	int 鬼刃暗扎;
	int 鬼哭神嚎;
	int 亢龙有悔;
}匕首闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 鬼反;
	float 壁击;
	float 荆轲献匕;
	float 鬼刃暗扎;
	float 鬼哭神嚎;
	float 亢龙有悔;
}匕首3;			//0-10M
typedef struct
{
	int 左右3;
	int 左蓄;
	int 右蓄;
	int 飞踢;
	int 壁击;
	int 扬鞭劲;
	int 横栏;
	int 龙虎乱舞;
	int 三龙灭阳棍;
}双截棍振刀方式;

typedef struct
{
	int 左右3;
	int 左蓄;
	int 右蓄;
	int 飞踢;
	int 壁击;
	int 扬鞭劲;
	int 横栏;
	int 龙虎乱舞;
	int 三龙灭阳棍;
}双截棍闪避延迟;
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 飞踢;
	float 壁击;
	float 扬鞭劲;
	float 横栏;
	float 龙虎乱舞;
	float 三龙灭阳棍;
}双截棍1;			//0-8M
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 飞踢;
	float 壁击;
	float 扬鞭劲;
	float 横栏;
	float 龙虎乱舞;
	float 三龙灭阳棍;
}双截棍3;			//0-8M
typedef struct
{
	int 左右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 铁马残红;
	int 乾坤日月斩;
	int 惊雷;
	int 壁击;
	int 八斩刀;
	int 分水斩;
}双刀振刀方式;

typedef struct
{
	int 左右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 铁马残红;
	int 乾坤日月斩;
	int 惊雷;
	int 壁击;
	int 八斩刀;
	int 分水斩;
}双刀闪避延迟;

typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 铁马残红;
	float 乾坤日月斩;
	float 惊雷;
	float 壁击;
	float 八斩刀;
	float 分水斩;
}双刀1;		//0-10M
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 铁马残红;
	float 乾坤日月斩;
	float 惊雷;
	float 壁击;
	float 八斩刀;
	float 分水斩;
}双刀3;		//0-10M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 壁击;
	int 同源;
	int 腾云式;
	int 乱点天宫;
	int 桶劲;
	int 五情七灭镇;
	int 双环扫;
	int 少林棍;
	int 镇地撑天;
}长棍振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 壁击;
	int 同源;
	int 腾云式;
	int 乱点天宫;
	int 桶劲;
	int 五情七灭镇;
	int 双环扫;
	int 少林棍;
	int 镇地撑天;
}长棍闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 壁击;
	float 同源;
	float 腾云式;
	float 乱点天宫;
	float 桶劲;
	float 五情七灭镇;
	float 双环扫;
	float 少林棍;
	float 镇地撑天;
}长棍1;		//0-14M
typedef struct
{
	float 左3;
	float 右3;
	float 少林棍;
	float 左蓄;
	float 右蓄;
	float 腾云式;
	float 乱点天宫;
	float 桶劲;
	float 壁击;
	float 同源;
	float 五情七灭镇;
	float 双环扫;
	float 倒海棍;
	float 镇地撑天;
}长棍3;		//0-14M
typedef struct
{
	int 左左;
	int 左右;
	int 右右;
	int 右左;
	int 左蓄1;
	int 左蓄2;
	int 左蓄3;
	int 右蓄1;
	int 右蓄2;
	int 右蓄3;
	int 柄击;
	int 壁击;
	int 同源;
	int 惊雷;
	int 炽焰斩;
	int 过关斩将;
	int 奔雷入阵;
}斩马刀振刀方式;

typedef struct
{
	int 左左;
	int 左右;
	int 右右;
	int 右左;
	int 左蓄1;
	int 左蓄2;
	int 左蓄3;
	int 右蓄1;
	int 右蓄2;
	int 右蓄3;
	int 柄击;
	int 壁击;
	int 同源;
	int 惊雷;
	int 炽焰斩;
	int 过关斩将;
	int 奔雷入阵;
}斩马刀闪避延迟;

typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 左蓄;
	float 右蓄;
	float 柄击;
	float 壁击;
	float 同源;
	float 惊雷;
	float 炽焰斩;
	float 过关斩将;
	float 奔雷入阵;
}斩马刀1;		//0-9M
typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 左蓄;
	float 右蓄;
	float 柄击;
	float 壁击;
	float 同源;
	float 惊雷;
	float 炽焰斩;
	float 过关斩将;
	float 奔雷入阵;
}斩马刀3;		//0-9M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 惊雷;
	int 勾旋斩;
	int 探海蛟;
	int 钩挂;
	int 壁击;
	int 战龙在天;
	int 形意钩;
}双戟振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 惊雷;
	int 勾旋斩;
	int 探海蛟;
	int 钩挂;
	int 壁击;
	int 战龙在天;
	int 形意钩;
}双戟闪避延迟;

typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 勾旋斩;
	float 探海蛟;
	float 钩挂;
	float 壁击;
	float 战龙在天;
	float 形意钩;
}双戟1;
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 勾旋斩;
	float 探海蛟;
	float 钩挂;
	float 壁击;
	float 战龙在天;
	float 形意钩;
}双戟3;		//0-8M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 双开圆;
	int 三风摆;
	int 鬼反;
	int 惊雷;
	int 壁击;
	int 缠龙奔野;
	int 泽风上六;
}扇子振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄;
	int 双开圆;
	int 三风摆;
	int 鬼反;
	int 惊雷;
	int 壁击;
	int 缠龙奔野;
	int 泽风上六;
}扇子闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 双开圆;
	float 三风摆;
	float 鬼反;
	float 惊雷;
	float 壁击;
	float 缠龙奔野;
	float 泽风上六;
}扇子1;		//0-12.5M
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 双开圆;
	float 三风摆;
	float 鬼反;
	float 惊雷;
	float 壁击;
	float 缠龙奔野;
	float 泽风上六;
}扇子3;		//0-12.5M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 苍牙;
	int 壁击;
	int 同源;
	int 踏空闪;
	int 破千军;
	int 极光破云闪;
	int 乾坤一掷;
}横刀振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄;
	int 右蓄1;
	int 右蓄2;
	int 苍牙;
	int 壁击;
	int 同源;
	int 踏空闪;
	int 破千军;
	int 极光破云闪;
	int 乾坤一掷;
}横刀闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 壁击;
	float 同源;
	float 踏空闪;
	float 破千军;
	float 极光破云闪;
	float 乾坤一掷;
}横刀1;		//0-7M
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 壁击;
	float 同源;
	float 踏空闪;
	float 破千军;
	float 极光破云闪;
	float 乾坤一掷;
}横刀3;		//0-7M
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力1;
	int 左蓄力2;
	int 右蓄力1;
	int 右蓄力2;
	int 右蓄力2段;
	int 百裂腿;
	int 破空拳;
	int 苍牙;
	int 壁击;
	int 太极弄云手;
}拳刃振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力1;
	int 左蓄力2;
	int 右蓄力1;
	int 右蓄力2;
	int 右蓄力2段;
	int 百裂腿;
	int 破空拳;
	int 苍牙;
	int 壁击;
	int 太极弄云手;
}拳刃闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;	
	float 右蓄力;
    float 右蓄力2段;
	float 百裂腿;
	float 破空拳;
	float 苍牙;
	float 壁击;
	float 太极弄云手;
}拳刃1;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 右蓄力;
	float 右蓄力2段;
	float 百裂腿;
	float 破空拳;
	float 苍牙;
	float 壁击;
	float 太极弄云手;
}拳刃3;
typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力;
	int 左蓄力2段;
	int 右蓄力;
	int 掌心雷;
	int 苍牙;
	int 太极;
	int 壁击;
	int 断月千刃舞;
	int 万点寒梅;
}飞刀振刀方式;

typedef struct
{
	int 左3;
	int 右3;
	int 左蓄力;
	int 左蓄力2段;
	int 右蓄力;
	int 掌心雷;
	int 苍牙;
	int 太极;
	int 壁击;
	int 断月千刃舞;
	int 万点寒梅;
}飞刀闪避延迟;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 左蓄力2段;
	float 右蓄力;
	float 掌心雷;
	float 苍牙;
	float 太极;
	float 壁击;
	float 断月千刃舞;
	float 万点寒梅;
}飞刀1;

typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 右蓄力;
	float 苍牙;
	float 太极;
	float 掌心雷;
	float 壁击;
	float 左蓄力2段;
	float 断月千刃舞;
	float 万点寒梅;
}飞刀3;
typedef struct
{
	float 左3;
	float 右3;
	float 左剑气;
	float 右剑气;
	float 凤凰羽;
	float 苍牙;
	float 壁击;
	float 跳斩;
	float 七星夺窍;
	float 天外飞仙;
}长剑2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 壁击;
	float 刹那斩;
	float 惊雷十劫;
	float 青鬼;
	float 同源;
	float 噬魂斩;
}太刀2;
typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 壁击;
	float 左蓄一段;
	float 左蓄二段;
	float 左蓄三段;
	float 右蓄;
	float 同源;
	float 右右蓄;
}阔刀2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 六合枪;
	float 右蓄;
	float 壁击;
	float 同源;
	float 大圣游;
	float 双环扫;
	float 风卷云残;
	float 龙王破;
	float 穿心脚;
}长枪2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 鬼反;
	float 壁击;
	float 荆轲献匕;
	float 鬼刃暗扎;
	float 亢龙有悔;
	float 鬼哭神嚎;
}匕首2;
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 飞踢;
	float 壁击;
	float 扬鞭劲;
	float 横栏;
    float 龙虎乱舞;
    float 三龙灭阳棍;
}双截棍2;
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 铁马残红;
	float 乾坤日月斩;
	float 惊雷;
	float 壁击;
	float 八斩刀;
	float 分水斩;
}双刀2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 壁击;
	float 同源;
	float 腾云式;
	float 乱点天宫;
	float 桶劲;
	float 五情七灭镇;
	float 双环扫;
	float 少林棍;
	float 镇地撑天;
}长棍2;
typedef struct
{
	float 左左;
	float 左右;
	float 右右;
	float 右左;
	float 左蓄;
	float 右蓄;
	float 柄击;
	float 壁击;
	float 同源;
	float 惊雷;
	float 炽焰斩;
	float 过关斩将;
	float 奔雷入阵;
}斩马刀2;
typedef struct
{
	float 左右3;
	float 左蓄;
	float 右蓄;
	float 勾旋斩;
	float 探海蛟;
	float 钩挂;
	float 壁击;
	float 战龙在天;
	float 形意钩;
}双戟2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 双开圆;
	float 三风摆;
	float 鬼反;
	float 惊雷;
	float 壁击;
	float 缠龙奔野;
	float 泽风上六;
}扇子2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄;
	float 右蓄;
	float 苍牙;
	float 壁击;
	float 同源;
	float 踏空闪;
	float 破千军;
	float 极光破云闪;
	float 乾坤一掷;
}横刀2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 右蓄力;
	float 右蓄力2段;
	float 百裂腿; 
	float 破空拳;
	float 苍牙;
	float 壁击;
	float 太极弄云手;
}拳刃2;
typedef struct
{
	float 左3;
	float 右3;
	float 左蓄力;
	float 右蓄力;
	float 左蓄力2段;
	float 苍牙;
	float 掌心雷;
	float 太极;
	float 壁击;
	float 断月千刃舞;
	float 万点寒梅;
}飞刀2;

namespace 振刀参数
{
	namespace 振刀方式
	{
		extern 长剑振刀方式 长剑;
		extern 链剑振刀方式 链剑;
		extern 太刀振刀方式 太刀;
		extern 阔刀振刀方式 阔刀;
		extern 长枪振刀方式 长枪;
		extern 匕首振刀方式 匕首;
		extern 双截棍振刀方式 双截棍;
		extern 双刀振刀方式 双刀;
		extern 长棍振刀方式 长棍;
		extern 斩马刀振刀方式 斩马刀;
		extern 双戟振刀方式 双戟;
		extern 扇子振刀方式 扇子;
		extern 横刀振刀方式 横刀;
		extern 拳刃振刀方式 拳刃;
		extern 飞刀振刀方式 飞刀;
	}

	namespace 闪避
	{

		extern 长剑闪避延迟 长剑;
		extern 链剑闪避延迟 链剑;
		extern 太刀闪避延迟 太刀;
		extern 阔刀闪避延迟 阔刀;
		extern 长枪闪避延迟 长枪;
		extern 匕首闪避延迟 匕首;
		extern 双截棍闪避延迟 双截棍;
		extern 双刀闪避延迟 双刀;
		extern 长棍闪避延迟 长棍;
		extern 斩马刀闪避延迟 斩马刀;
		extern 双戟闪避延迟 双戟;
		extern 扇子闪避延迟 扇子;
		extern 横刀闪避延迟 横刀;
		extern 拳刃闪避延迟 拳刃;
		extern 飞刀闪避延迟 飞刀;
	}

	namespace 距离
	{
		extern 长剑1 长剑;
		extern 链剑振刀方式2 链剑;
		extern 太刀1 太刀;
		extern 阔刀1 阔刀;
		extern 长枪1 长枪;
		extern 匕首1 匕首;
		extern 双截棍1 双截棍;
		extern 双刀1 双刀;
		extern 长棍1 长棍;
		extern 斩马刀1 斩马刀;
		extern 双戟1 双戟;
		extern 扇子1 扇子;
		extern 横刀1 横刀;
		extern 拳刃1 拳刃;
		extern 飞刀1 飞刀;
	}

	namespace 角度
	{
		extern 长剑2 长剑;
		extern 链剑振刀方式2 链剑;
		extern 太刀2 太刀;
		extern 阔刀2 阔刀;
		extern 长枪2 长枪;
		extern 匕首2 匕首;
		extern 双截棍2 双截棍;
		extern 双刀2 双刀;
		extern 长棍2 长棍;
		extern 斩马刀2 斩马刀;
		extern 双戟2 双戟;
		extern 扇子2 扇子;
		extern 横刀2 横刀;
		extern 拳刃2 拳刃;
		extern 飞刀2 飞刀;
	}
	namespace 延迟
	{
		extern 长剑3 长剑;
		extern 链剑振刀方式2 链剑;
		extern 太刀3 太刀;
		extern 阔刀3 阔刀;
		extern 长枪3 长枪;
		extern 匕首3 匕首;
		extern 双截棍3 双截棍;
		extern 双刀3 双刀;
		extern 长棍3 长棍;
		extern 斩马刀3 斩马刀;
		extern 双戟3 双戟;
		extern 扇子3 扇子;
		extern 横刀3 横刀;
		extern 拳刃3 拳刃;
		extern 飞刀3 飞刀;
	}
	namespace 特殊处理
	{
		extern bool 阔刀右蓄是否短闪振;
		extern bool 双截棍左蓄是否短闪振;
		extern bool 斩马刀左蓄是否长闪振;
		extern bool 扇子左蓄是否中闪振;
		extern bool 扇子右蓄是否中闪振;
		extern bool 横刀左蓄是否只短闪;
		extern bool 拳刃右蓄是否短闪振;
        extern bool 长枪左蓄是否短闪振;
		extern bool 是否长闪振;
		extern bool 胡为开大振刀;
	}
}

namespace 活化参数
{
	namespace 距离
	{
		extern float 火男冲拳;
		extern float 火男顶膝;
		extern float 妖刀大招;
		extern float 顾倾寒大招;
		extern float 武田F2;
		extern float 武田F3;
		extern float 胡为F1;
		extern float 胡为F2;
		extern float 胡为跃击;
		extern float 胡为V1V2;
		extern float 宁红叶F1;
		extern float 宁红叶F2;
		extern float 宁红叶F3;
		extern float 狐狸V1;
		extern float 狐狸V2;
		extern float 火男F;
		extern float 岳山F;
		extern float 三娘V2;
		extern float 哈迪大招1段;
		extern float 哈迪大招2段;
		extern float 哈迪大招3段;
		extern float 魏轻F1;
		extern float 魏轻V1;
		extern float 魏轻V2;
		extern float 迦南V2;
		extern float 迦南F2;
		extern float 刘炼V1;
		extern float 刘炼V2;
		extern float 季盈盈V1蓄力;
		extern float 季盈盈V2蓄力;
		extern float 张起灵F2;
		extern float 张起灵V1;
		extern float 张起灵V12段;
		extern float 张起灵V2;
		extern float 无尘V2;
		extern float 希拉F;
		extern float 彩戏师F;
	}

	namespace 角度
	{
		extern float 火男冲拳;
		extern float 妖刀大招3M;
		extern float 妖刀大招3M外;
		extern float 顾倾寒大招3M;
		extern float 顾倾寒大招3M外;
		extern float 武田F21M;
		extern float 武田F21M外;
		extern float 武田F31M;
		extern float 武田F31M外;
		extern float 胡为F2;
		extern float 胡为跃击;
		extern float 胡为V1V2;
		extern float 宁红叶F1;
		extern float 宁红叶F2;
		extern float 宁红叶F3;
		extern float 狐狸大招4M;
		extern float 狐狸大招8M;
		extern float 狐狸大招12M;
		extern float 狐狸大招12M外;
		extern float 火男F;
		extern float 岳山F;
		extern float 三娘V24M;
		extern float 三娘V210M;
		extern float 三娘V210M外;
		extern float 哈迪大招;
		extern float 魏轻F1;
		extern float 魏轻V1;
		extern float 魏轻V2;
		extern float 迦南V2;
		extern float 迦南F2;
		extern float 刘炼V1;
		extern float 刘炼V2;
		extern float 季盈盈V1蓄力;
		extern float 季盈盈V2蓄力;
		extern float 张起灵F2;
		extern float 张起灵V1;
		extern float 张起灵V12段;
		extern float 张起灵V2;
		extern float 无尘V2;
		extern float 希拉F;
		extern float 彩戏师F;
	}

	extern int 出蓄抓振延迟;
	extern int 蓝顶延迟;
}

namespace 连招方式 {
	extern int 长剑;
	extern int 链剑;
	extern int 太刀;
	extern int 阔刀;
	extern int 长枪;
	extern int 匕首;
	extern int 双截棍;
	extern int 双刀;
	extern int 长棍;
	extern int 斩马刀;
	extern int 双戟;
	extern int 扇子;
	extern int 横刀;
	extern int 拳刃;
	extern int 飞刀;
}
typedef struct
{
	string 敌人类型;
	string 敌人动作;
	string 执行动作;
	float 振刀时间;
	float 判断角度;
	float 判断距离;
	float 执行角度;
	float 执行距离;
}逻辑执行1;		//这个结构体无需加在菜单，随动的

namespace 调试
{
	extern 逻辑执行1 逻辑执行;
	extern bool 输出逻辑执行;
	extern bool 输出逻辑执行Flag;		//这个不用加在菜单，随动的。
	extern bool 输出调试执行;
}
