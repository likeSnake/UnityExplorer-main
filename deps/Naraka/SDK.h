#pragma once
#include "Data.h"
#include "Kmbox.cpp"
inline _com myserial;
enum SDK_Hash:int {
	DINNER_DODGE_FRONT_DOUBLE_NAME_HASH = 264120126,
	DINNER_DODGE_LEFT_DOUBLE_NAME_HASH = -1024188518,
	DINNER_DODGE_RIGHT_DOUBLE_NAME_HASH = -306541748,
	DINNER_DODGE_BACK_DOUBLE_NAME_HASH = 1986392622,
	DINNER_ENHANCE_DODGE_FRONT_DOUBLE_BATTLE_NAME_HASH = 1877656274,
	DINNER_ENHANCE_DODGE_LEFT_DOUBLE_BATTLE_NAME_HASH = -1409224812,
	DINNER_ENHANCE_DODGE_RIGHT_DOUBLE_BATTLE_NAME_HASH = -214565578,
	DINNER_ENHANCE_DODGE_BACK_DOUBLE_BATTLE_NAME_HASH = -1399432426,
	//==========================================顾清寒相关
	ATTACK_1080_PREPARE_NAME_HASH = 489800962,
	ATTACK_1081_PREPARE_NAME_HASH = 172675393,
	ATTACK_1380_PREPARE_NAME_HASH = -2066504957,
	ATTACK_1381_PREPARE_NAME_HASH = -1817659584,
	ATTACK_1080_NAME_HASH = -1947866894, // size:0x4
	ATTACK_1081_NAME_HASH = -52242332, // size:0x4
	ATTACK_1380_NAME_HASH = -1985784149, // size:0x4
	ATTACK_1381_NAME_HASH = -22780355, // size:0x4
	DINNER_FROZEN_01_NAME_HASH = -1385389488,
	DINNER_FROZEN_02_NAME_HASH = 879096810,
	//==========================================迦南|火男相关
	ATTACK_1200_NAME_HASH = 1085847188,
	ATTACK_1201_NAME_HASH = 935298562,
	ATTACK_1500_NAME_HASH = 1173856273,
	ATTACK_1501_NAME_HASH = 854626439,
	ATTACK_1800_NAME_HASH = 1294938946,
	ATTACK_1801_NAME_HASH = 975700948,
	ATTACK_1601_PREPARE_NAME_HASH = -249013258,
	ATTACK_1600_PREPARE_NAME_HASH = -430747723,
	ATTACK_1611_PREPARE_NAME_HASH = 518664392,
	ATTACK_1600_NAME_HASH = 1202789960,
	ATTACK_1601_NAME_HASH = 817245918,
	ATTACK_1680_PREPARE_NAME_HASH = -175894719,
	ATTACK_1681_PREPARE_NAME_HASH = -486597886,
	ATTACK_1839_NAME_HASH = 534698021,
	//===========================================武田?
	ATTACK_1000_NAME_HASH = 1128030970,

	ATTACK_1999_NAME_HASH = -1289708981, //三娘开大招之后的F
	ATTACK_1998_NAME_HASH = 1563360103, //三娘开大招之后的F
	ATTACK_1997_NAME_HASH = -1004033315, //三娘开大招之后的F
	ATTACK_1996_NAME_HASH = 707382257, //三娘开大招之后的F
	ATTACK_1995_NAME_HASH = -1008160316, //三娘开大招之后的F
	ATTACK_1994_NAME_HASH = 876302956, //三娘开大招之后的F

	ATTACK_1993_NAME_HASH = 1512526779, //宁红叶F3

	ATTACK_18888_NAME_HASH = 1085847188, //玉玲珑大招
	ATTACK_18889_NAME_HASH = 1098570915, //玉玲珑F2

	ATTACK_1001_NAME_HASH = 876302956,
	ATTACK_1303_NAME_HASH = -663515879, // size:0x4
	ATTACK_1212_NAME_HASH = -1213339911,
	ATTACK_1216_NAME_HASH = -1329585440,
	ATTACK_1219_NAME_HASH = 545210225,
	ATTACK_1206_NAME_HASH = -1445260383,
	ATTACK_1226_NAME_HASH = -1678937821,
	ATTACK_1506_NAME_HASH = -1399585500,
	ATTACK_1516_NAME_HASH = -1248906139,
	ATTACK_1526_NAME_HASH = -1633523802,
	ATTACK_1806_NAME_HASH = -1538484617,
	ATTACK_1816_NAME_HASH = -1118329034,
	ATTACK_1826_NAME_HASH = -1770326795,
	ATTACK_1290_PREPARE_NAME_HASH = -1432414211,
	ATTACK_1590_PREPARE_NAME_HASH = -2086403458,

	ATTACK_1200_PREPARE_NAME_HASH = 1451899959,
	ATTACK_1201_PREPARE_NAME_HASH = 1106323572,
	ATTACK_1500_PREPARE_NAME_HASH = 2142316980,
	ATTACK_1501_PREPARE_NAME_HASH = 1758084599,
	ATTACK_1800_PREPARE_NAME_HASH = -1272617805,
	ATTACK_1801_PREPARE_NAME_HASH = -1554091792,

	ATTACK_1610_NAME_HASH = 1588211465,
	ATTACK_1611_NAME_HASH = 699211679,
	ATTACK_1310_NAME_HASH = 1482812898,
	ATTACK_1311_NAME_HASH = 795270516,
	ATTACK_1010_NAME_HASH = 1512526779,
	ATTACK_1011_NAME_HASH = 757097261,
	ATTACK_1300_NAME_HASH = 1098570915,
	ATTACK_1301_NAME_HASH = 914222133,
	ATTACK_1400_NAME_HASH = 1144375846,
	ATTACK_1401_NAME_HASH = 858978992,
	ATTACK_1100_NAME_HASH = 1123944653,
	ATTACK_1241_PREPARE_NAME_HASH = 1209698318,
	ATTACK_1242_PREPARE_NAME_HASH = 1905767627,
	//======================================
	COMMON_HURT_BACK_4_NAME_HASH = 2029290417, // size:0x4
	COMMON_HURT_DOWN_4_NAME_HASH = -936518418, // size:0x4
	COMMON_HURT_FRONT_4_NAME_HASH = 2124037849,
	COMMON_HURT_LEFT_4_NAME_HASH = -1747300226, // size:0x4
	COMMON_HURT_UP_5_NAME_HASH = -1182010033,
	//SQUAT && CROUCH
	DINNER_SQUAT_01_NAME_HASH = -320848798, // size:0x4
	DINNER_SQUAT_RUN_01_NAME_HASH = -1212535995, // size:0x4
	DINNER_SQUAT_01_BATTLE_NAME_HASH = 826692594, // size:0x4
	DINNER_SQUAT_RUN_01_BATTLE_NAME_HASH = 1260402954, // size:0x4
	DINNER_SQUAT_02_NAME_HASH = 1978232280, // size:0x4
	DINNER_SQUAT_RUN_02_NAME_HASH = 783514367, // size:0x4
	DINNER_SQUAT_02_BATTLE_NAME_HASH = -1077324783, // size:0x4
	DINNER_SQUAT_RUN_02_BATTLE_NAME_HASH = -978370839, // size:0x4

	DINNER_PARRY_01_NAME_HASH = -845241736, // size:0x4
	DINNER_PARRY_02_NAME_HASH = 1419244482, // size:0x4
	DINNER_PARRY_03_NAME_HASH = 596689748, // size:0x4
	DINNER_PARRY_04_NAME_HASH = -1108060425, // size:0x4
	DINNER_PARRY_05_NAME_HASH = -890018207, // size:0x4
	DINNER_PARRY_06_NAME_HASH = 1408907227, // size:0x4
	DINNER_PARRY_07_NAME_HASH = 620562253, // size:0x4
	DINNER_PARRY_08_NAME_HASH = -1270736164, // size:0x4


	//ATTCKING
	ATTACK_101_NAME_HASH = 770101480, //站A
	ATTACK_110_NAME_HASH = 1140511039, //跑A
	ATTACK_227_NAME_HASH = -185227002, //走A
	ATTACK_201_NAME_HASH = 799043249, //站B
	ATTACK_210_NAME_HASH = 1102868326, //跑B
	ATTACK_228_NAME_HASH = 1682617495, //走B
	ATTACK_102_PREPARE_NAME_HASH = 733489033,//A2
	ATTACK_202_PREPARE_NAME_HASH = -1302707832, //B2
	ATTACK_120_NAME_HASH = 1758954236, //太刀 阔刀 马刀 长棍 长枪 双刀 匕首 跳A
	ATTACK_129_NAME_HASH = 285948504, //长剑跳A
	ATTACK_1224_NAME_HASH = 1977815055,
	ATTACK_142_NAME_HASH = -796703658,
	ATTACK_144_NAME_HASH = 971039075,
	//PARRY
	DINNER_EMPTYSTEP_DODGE_FRONT_NAME_HASH = 801000171, // size:0x4
	DINNER_EMPTYSTEP_DODGE_LEFT_NAME_HASH = -2077750324, // size:0x4
	DINER_EMPTYSTEP_DODGE_RIGHT_NAME_HASH = 1703894281, // size:0x4
	DINNER_EMPTYSTEP_DODGE_BACK_NAME_HASH = -1820250733, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_FRONT_NAME_HASH = -725672521, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_LEFT_NAME_HASH = 1362309779, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_RIGHT_NAME_HASH = -1634858411, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_BACK_NAME_HASH = 1184502988, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_NAME_HASH = 2071939033, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_LEFT_NAME_HASH = -1687023558, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_RIGHT_NAME_HASH = 827215931, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_BACK_NAME_HASH = -1931790747, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_FRONT_NAME_HASH = -1059491665, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_LEFT_NAME_HASH = 906126735, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_RIGHT_NAME_HASH = -1964480691, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_BACK_NAME_HASH = 564872144, // size:0x4
	//快捷振刀
	DINNER_EMPTYSTEP_DODGE_FRONT_PEACE_01_NAME_HASH = 1074308033, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_PEACE_02_NAME_HASH = -654183813, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_PEACE_01_NAME_HASH = 650084506, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_PEACE_02_NAME_HASH = -1078537952, // size:0x4
	DINNER_EMPTYSTEP_DODGE_FRONT_FAST_01_NAME_HASH = 741191779, // size:0x4
	DINNER_JUMP_EMPTYSTEP_DODGE_FRONT_FAST_01_NAME_HASH = -1657462343 // size:0x4

};
//存放武器类型
enum WeaponType :int
{
	Node = 100,				//空手
	Sw = 101,					//长剑
	//HSw = 0,					//红长剑
	lj = 128,		// 链剑
	Katana = 102,			//太刀
	Blade = 103,				//阔刀
	Spear = 105,				//长枪
	Dagge = 106,				//匕首
	喷火筒 = 110,
	nunchucks = 116,		//双节棍	
	twinblades = 118,		//双刀
	saber = 119,				//斩马刀
	rod = 120,					//棍
	dualhalberd = 121,	//双戟
	fan = 122,				//扇子
	hengdao = 123,		//横刀
	hadidazhao = 208,	//哈迪大招
	punch = 124,				//拳刃
	Knife = 127,				//飞刀

	//远程
	弓箭 = 104,
	连弩 = 108,
	火炮 = 109,
	喷火筒1 = 110,
	一窝蜂 = 111,
	五眼铳 = 112,
	鸟铳 = 113,
	三娘水矛 = 203,
	大佛手 = 204,
	神妙机甲 = 206,
	鞭子 = 207,			//季莹莹大招
	古刀 = 209,			//张启灵大招
	激光 = 211			//席拉大招
};
// 存放背包中的魂玉
enum HunYuID :int {
	TieMaCH = 3323705
};
//英雄ID
enum HeroType :int
{
	hutao = 1000001,
	ninghongye = 1000003,
	jianan = 1000004,
	shanan = 1000005,
	jicanghai = 1000006,
	tianhai = 1000007,
	dafo = 1000008,
	yaodaoji = 1000009,
	cuisanniang = 1000010,
	yueshan = 1000011,
	shenjiang = 1000012,
	wuchen = 1000013,
	guqinghan = 1000015,
	wutian = 1000016,
	yinziping = 1000017,
	shenmiao = 1000018,
	tiewei = 1000019,
	huwei = 1000020,
	jiyingying = 1000021,
	yulinglong = 1000022,
	hadi = 1000023,
	weiqing = 1000024,
	liulian = 1000025,
	zhangqiling = 1000026,
	xila = 1000027,
	caixishi = 1000028
};
//人物状态
enum ActionType :int
{
	Normal = 5,					//空手
	Hurt = 6,						//英雄挨打
	WhithAttack = 11,			//白
	ZhangqilingAttack = 18,		//张启灵
	BuleAttack = 20,			//蓝
	PurpleAttack = 21,			//紫霸体
	BuleAttack2 = 22,			//蓝
	ShockKnife = 30,			//金霸体出刀
	GoldState = 31,				//金霸体

    ShockKnife2 = 35,			//金霸体出刀

	TianhaiF = 50,				//和尚放F的霸体
	AwakenGole = 51,			//奥义金霸体(90减伤)
	NinghongyeAwaken = 52,			//宁红夜大招
	AwakenIng = 53,				//奥义金霸体后续(超高减伤)
	GuqinghanF = 59,			//顾清寒被自己F冻住时g
    ShenmiaoF = 80					//
};
enum SkillStatus :int //技能施放状态（0:未施放 1:前摇中 2:施放中）
{
	notreleased = 0,
	Forward = 1,
	released = 2
};

struct Rotation {
	float Pitch;
	float Yaw;
	float Roll;

	
	Rotation() : Pitch(0.0f), Yaw(0.0f), Roll(0.0f) {}
	Rotation(float inPitch, float inYaw, float inRoll) : Pitch(inPitch), Yaw(inYaw), Roll(inRoll) {}
};



//人物技能
struct LocalSkillData {
	int F_SkillID;
	int F_State;
	float F_CD;
	float F_LeftContinueTime;
	int V_SkillID;
	int V_State;
	float V_CD;
	float V_LeftContinueTime;
};
//血、甲、精等等
struct PropertyDataID
{
	int CurHpID;						//当前血量
	int HpMaxID;						//最大血量
	int CurshieldID;					//当前护甲
	int shieldMaxID;					//最大护甲
	int CurEnergyID;					//当前精力
	int EnergyMaxID;					//最大精力
	int bagMaxID;						//背包已开格子
	int weaponMaxID;					//武器已开格子
	int soulItemMaxID;					//魂玉已开格子

	int Rt_be_dying_hp_max;				//魂冢最大血量
	int Rt_be_dying_hp;					//魂冢当前血量
	int Rt_in_battle_coin;				//当前金币数量
	int Rt_rescue_status;				//被救援状态
	int Rt_cur_die_status;				//死亡状态
};
struct WeaponData
{
	ImColor WeaponColor = { 0,0,0 };
	string WeaponName = "";
};
//ActorModel->ActorModelPropertyData结构体数据 人物属性相关数据
class ActorModelPropertyData
{
public:
	uintptr_t CharactorSyncManagerPtr{ 0 };
	bool GetISRobotId();						//ActorModel->ActorModelPropertyData->ISRobotId判断数据 判断是否是人机
	int GetTeam();								// offset:0xf8  所属阵营分组（0:中立 1:红队 2:蓝队）
	HeroType GetHeroTID();					//ActorModel->ActorModelPropertyData->HeroTid整形数据 判断人物英雄ID
	int GetHeroSoul();					//ActorModel->ActorModelPropertyData->HeroTid整形数据 判断人物英雄ID
	int GetWeaponID();						//ActorModel->ActorModelPropertyData->WeaponID整形数据 判断人物手持武器具体ID
	std::string GetPlayerName();			//获取玩家名称
	int PlayerBag(uintptr_t Cache_PropertData_PTR);			//获取玩家背包容量
	float GetPing();
	void GetCharactorSyncManager();
	// offset:0x178 技能施放状态（0:未施放 1:前摇中 2:施放中）
	// offset:0x168 玩家在大厅中的状态标识
	
};

//ActorModel->ActorKit->GetActionHitSimulateData结构 人物出招时的状态
class ActionHitSimulateData
{
public:
	ActionType GetEndureLevel();						// 人物当前出招状态
	int GetReactionType();								// 为2则是可以拼刀状态，1是磐石
	int GetRangeReactionType();						//为12则是攻击状态
	float GetActionDuration();							//人物僵直					
	int GetReactionExtraType();                        //振刀状态
};


class ActionHitSimulatePtr
{
public:
	int GetXuLiState();
};

//ActorModel->ActorKit结构体数据 人物招式相关数据
class ActorKit
{
public:

	uintptr_t Cache_BlueTime();                                                          //缓存进蓝时间
	uintptr_t Cache_ActorPhysics();
	float BlueTime(uintptr_t Cache_Bluetime_PTR);									//ActorModel->ActorKit->BlueTime浮点型数据 人物进蓝的时间
	ActionHitSimulateData* GetActionHitSimulateData();	//ActorModel->ActorKit->GetActionHitSimulateData指针数据 指向ActionHitSimulateData结构
	ActionHitSimulatePtr* GetActionHitSimulatePtr();

	uintptr_t GetCameraAgent();

	int GetCurrentCollisionFlags();
};



//获取当前血甲精等等
class RuntimePropertyData
{
public:
	PropertyDataID GetPropertyDataID();
	uintptr_t Cache_PropertData();
	int GetCurHp(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);					//当前血量
	int GetHpMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);					//最大血量
	int GetCurshield(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);				//当前护甲
	int GetshieldMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);				//最大护甲
	int GetCurEnergy(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);				//当前精力
	int GetEnergyMax(uintptr_t Cache_PropertData_PTR, PropertyDataID Cache_PropertyDataID);				//最大精力
	int GetCurAnger(uintptr_t Cache_PropertData_PTR);				//当前怒气
	int GetAngerMax(uintptr_t Cache_PropertData_PTR);				//最大怒气
};

//ActorModel->ObjectMessenger结构体数据，人物动画相关
class ObjectMessenger
{
public:
	uintptr_t Cache_GetGlobalTime();                                                // 缓存全局时间
	uintptr_t Cache_GetCurTimeForLogicState();										// 缓存当前时间											
	float GetCurTimeForLogicStateA(uintptr_t Cache_GetGlobalTime_PTR, uintptr_t Cache_GetCurTimeForLogicState_PTR);				//当前动作执行时间
	uintptr_t Cache_GetNameHash();                                          //缓存动作hash
	unsigned int GetNameHash(uintptr_t Cache_GetNameHash_PTR);				//动作hash

};

//ActorModel结构体数据（是一个数组 周围有多名玩家）
class ActorModel
{
public:

	//int GetType();																												//ActorModel->Type整形数据 人物类型数据如果是35FD则为人物
	bool IsVisible();																												//ActorModel->IsVisible判断数据 判断自身视野内是否有敌人如果为1则有反之没有
	bool IsAlive();																												//ActorModel->IsVisibleAlive 判断人物是否存活，如果为101则存活
	//以下为获取动作名称缓存写法:在调用GetActorName之前一定要先调用过GetEventTracksForLayer获取到缓存
	uintptr_t Cache_GetEventTracksForLayer();			
	//以下为获取Pos缓存写法:先调用GetPosPtr之前一定要先调用过GetPosPtr确保获取到缓存
	uintptr_t Cache_GetPosPtr();																								//ActorModel->Vector指针数据 人物坐标数据

	Vector3 GetPos(uintptr_t PosPtr); 
	uintptr_t Cache_GetActorNamePtr();

	uintptr_t Cache_GetActorRootPtr();
	uintptr_t  Cache_GetActorWeaponPtr();
	WeaponType GetActorWeapon();																					//ActorModel->ActorWeapon指针数据	指向ActorWeapon武器结构
	ActorModelPropertyData* GetActorModelPropertyData();													//ActorModel->ActorModelPropertyData指针数据 指向ActorModelPropertyData结构
	ActorKit* GetActorKit();																									//ActorModel->ActorKit指针数据 指向ActorKit结构
	RuntimePropertyData* GetRuntimePropertyData();															//ActorModel->RuntimePropertyData指针数据	指向RuntimePropertyData结构
	ObjectMessenger* GetObjectMessenger();
};

class CharacterManager
{
public:
	uintptr_t CharacterManagerPtr{0};
	uintptr_t CharactorSyncManagerPtr{ 0 };

	uintptr_t Cache_PlayerCount_ActorModelList_Ptr{0};
	void GetCharacterManagerPtr();							//CharacterManager 人物管理器
	uintptr_t GetCharactorSyncManager();							//GetCharactorSyncManager ping管理器
	void Cache_PlayerCount_ActorModelList();				//Count与List的缓存
	int GetPlayerCount();										//CharacterCount整形数据 周围多少人数据
	float GamePing();										//获取游戏延迟
	ActorModel* GetLocalActorModel();					//LocalActorModel指针数据 本人数据	指针指向本人地址
	ActorModel* GetActorModelList(int index);			//ActorModel指针数据 指针指向周围人物地址（以上面数量形式进行遍历，包括本人，敌人）
};

//获取技能CD等结构
class AllSkillData
{
public:
	int GetCount();                                                         //技能列表长度
	uintptr_t SkillPtr{ 0 };
	uintptr_t Cache_Skill(int index);                                               //缓存技能
	int GetSkillID(uintptr_t Cache_Skill_PTR);								//获取本人技能ID
	int GetSkillState(uintptr_t Cache_Skill_PTR);							//获取本人技能状态
	float Get_LeftCdTime(uintptr_t Cache_Skill_PTR);					//当前技能CD
	float Get_LeftContinueTime(uintptr_t Cache_Skill_PTR);			//当前技能持续时间

	float Get_SkillTime(uintptr_t Cache_Skill_PTR);						 //当前技能时间钟

	
};

uintptr_t GetCameraController(uintptr_t cameraAgentPtr);
uintptr_t GetAdventureRig(uintptr_t cameraControllerPtr);

class UserDataManager
{
public:
	uintptr_t UserDataManagerPtr{0};
	void GetUserDataManager();                                        //取人物数据指针
	AllSkillData* GetAllSkillDataList();							//取技能列表

	LocalSkillData GetSkillID(int HeroID);
	LocalSkillData GetSkillID(int HeroID, AllSkillData* AllSkillDataList);
	AllSkillData* GetAllSkillDataListByFid(int fid);
	LocalSkillData GetSkillIDByFid(int HeroID, int fid);
	bool IsSpectator();												//是否是观战
	int IsShowExtraEvent();											//是否打开背包
	uintptr_t Cache_WeaponIndex();									//当前武器索引
	uintptr_t GetWeaponList(int index);                                  //当前武器ID
};

//获取ping结构
class CharactorSyncManager
{
public:
	uintptr_t CharactorSyncManagerPtr{0};
	void GetCharactorSyncManager();                               //人物同步
	float GamePing();											//游戏延迟 
	int PlayerBag(uintptr_t Cache_PropertData_PTR);											//获取玩家背包扩格数量 
};

// 获取游戏窗口大小
Vector2 GetWindowsize();

bool WorldToScreen(float MATRI[4][4], Vector3 Pos, Vec2& PM, Vec4& RW);

bool WorldToScreen_AIM(float MATRI[4][4], Vector3 Pos, Vec2& PM);

WeaponData GetWeaponData(uint32_t WeaponID);

string GetHeroType(int ID);

Vector3 GetItemPos(uint64_t CoordinatePointer);

BOOL CheckPlayerAngle(uintptr_t ActorRootPtr, Vector3 ObjPos, Vector3 MyPos, float NewAngle);		//ActorModel->Actor角度检查

float GetAngle(Vector3 Quat, Vector3 ObjPos, Vector3 MyPos);

int GetRangeReactionType(uintptr_t HitSimulateData_PTR);

int GetEndureLevel(uintptr_t HitSimulateData_PTR);

int GetReactionType(uintptr_t HitSimulateData_PTR);

float GetAimDis(float x, float y, float ObjeX, float ObjeY); //获取敌人到准星距离

float GetPlayerDis(Vector3 Obj, Vector3 Local);                    // 获取目标距离

float GetCurShareStateTransitionTime(uintptr_t Cache_GetCurTimeForLogicState_PTR); //获取前摇时间

std::wstring  GetActorName(uintptr_t Cache_GetActorNamePtr);
//动画总时间
float GetCurDuration(uintptr_t Cache_GetCurTimeForLogicState_PTR);
//当前动画执行时间
float GetCurTimeForLogicState(uintptr_t Cache_GetCurTimeForLogicState_PTR);
//动画后摇
float GetCurShareStateExitTime(uintptr_t Cache_GetCurTimeForLogicState_PTR);

//拼刀
float GetactiondurationPD(uintptr_t HitSimulateData);

float Getactionduration(uintptr_t HitSimulateData);

uintptr_t Cache_GetEventTracksForLayer(uintptr_t EventTracksForLayer);

uint64_t GetItemPosAddr(uint64_t Ptr);

bool JudgeStatus(uint32_t StatusId);

BOOL IsLookUp(uintptr_t ActorRootPtr);

bool JudgeZhenDao(wstring ActionName);
bool JudgeJump(wstring ActionName); // 跳跃
bool JudgeCrouch(wstring ActionName); // 下蹲
bool JudgeSquat(wstring ActionName); // 下蹲前摇/过程中
bool 二段跳跃状态(wstring ActionName); // 二段跳跃状态
bool JudgeJumpAndA(wstring ActionName); // 跳A
bool 落地状态(wstring ActionName); // 落地状态
bool 跳B状态(wstring ActionName); // 跳B状态
bool 滑铲状态(wstring ActionName); // 滑铲状态
bool 滑铲CB状态(wstring ActionName); // 滑铲CB状态
bool Judgefly(wstring ActionName); // 滞空
bool 壁击状态(wstring ActionName); // 壁击
bool 被壁击状态(wstring ActionName); // 被壁击状态
bool 钩锁空A状态(wstring ActionName); // 钩锁空A状态
bool 拼刀状态(wstring ActionName); // 拼刀状态
bool 挂墙状态(wstring ActionName); // 挂墙状态
bool 受击站立状态(wstring ActionName); // 受击站立状态
bool 出招完毕状态(wstring ActionName); // 出招完毕状态
bool 被震刀僵直状态(wstring ActionName); // 被震刀僵直状态
bool 左蓄状态(wstring ActionName); // 左蓄状态
bool AB3蓄力状态(wstring ActionName); // AB3蓄力状态
bool AB2蓄力状态(wstring ActionName); // AB2蓄力状态
bool JudgePanshi(wstring ActionName); // 磐石状态
bool 磐石一段(wstring ActionName); // 磐石一段
bool 磐石二段(wstring ActionName); // 磐石二段
bool 磐石三段(wstring ActionName); // 磐石三段
bool 横栏状态(wstring ActionName); // 横栏状态
bool 武田F状态(wstring ActionName); //武田F状态
bool 横刀左蓄一段状态(wstring ActionName); // 横刀左蓄一段状态
bool 横刀右蓄状态(wstring ActionName); // 横刀右蓄状态
bool 横刀左蓄状态(wstring ActionName); // 横刀左蓄状态
bool A1状态(wstring ActionName); // A1状态
bool 跑A1状态(wstring ActionName); // 跑A1状态
bool 跑B1状态(wstring ActionName); // 跑B1状态
bool A4状态(wstring ActionName); // A4状态
bool B1状态(wstring ActionName); // B1状态
bool B2状态(wstring ActionName); // B2状态
bool B3状态(wstring ActionName); // B3状态
bool CA状态(wstring ActionName); // CA状态
bool 拳刃B2状态(wstring ActionName); // 拳刃B2状态
bool B4状态(wstring ActionName); // B4状态
bool A2状态(wstring ActionName); // A2状态
bool A3状态(wstring ActionName); // A2状态
bool CB状态(wstring ActionName); // CB状态
bool 重刃AB状态(wstring ActionName); // CB状态
bool 滑步B1状态(wstring ActionName); //滑步B1状态
bool 释放受击技能(wstring ActionName); //释放受击技能
bool 钩锁出钩(wstring ActionName); //钩锁出钩
bool 钩锁撞击敌人(wstring ActionName); //钩锁撞击敌人
bool 斩马A2蓄住(wstring ActionName); //斩马A2蓄住
bool 横刀曲步状态(wstring ActionName); //横刀曲步状态


/*英雄技能状态*/

bool 水娘水矛束缚状态(wstring ActionName); //水娘水矛束缚状态
bool 被武田夺刀状态(wstring ActionName); //被武田夺刀状态
bool 双戟出左蓄状态(wstring ActionName); //双戟出左蓄状态
bool 长枪龙王破(wstring ActionName); //长枪龙王破
bool 长枪龙王破一段(wstring ActionName); //长枪龙王破一段
bool 长棍立棍(wstring ActionName); //长棍立棍
bool 长棍乱点天宫一段(wstring ActionName); //长棍乱点天宫一段
bool 长棍乱点天宫二段(wstring ActionName); //长棍乱点天宫二段
bool 长棍乱点天宫三段(wstring ActionName); //长棍乱点天宫三段
bool 是否受击技能F(int fID); //是否受击技能F

bool JudgeShortFlash(wstring ActionName);
bool JudgeLongFlash(wstring ActionName);
bool 英雄技能状态(wstring ActionName);
bool JudgeDuanFlash(wstring ActionName);
bool JudgeZhanLi(wstring ActionName);
bool ShakeBack(wstring ActionName);
bool JudgeGouSuo(wstring ActionName);
bool JudgeZhenDaoing(wstring ActionName);
bool JudgeYuanCheng();
bool 判断是否手持远程(int O_WeaponID);
bool 倒地状态(wstring ActionName);
bool 受击状态(ActionType O_EndureLevel, wstring ActionName);
bool 英雄技能不振(wstring ActionName);
bool 胡为开大状态(wstring ActionName);
bool 天海金钟罩(wstring ActionName);
bool 天海振刀罩(wstring ActionName);
bool 倒地翻滚状态(wstring ActionName);
bool 判断振刀成功(wstring ActionName);

bool JudgeFlash(wstring ActionName);
bool 判断短闪哈希(int hashName);

namespace math
{
	// # VectorToRotationYaw：将三维向量转换为旋转偏航角（Yaw轴角度）
	float VectorToRotationYaw(Vector3 pos);

	// # CalcYaw：计算目标位置相对于当前位置的偏航角
	float CalcYaw(Vector3 Loc, Vector3 Ent);
	// # QuatToRot：将四元数转换为欧拉角（三维旋转向量）
	Vector3 QuatToRot(Vector4 Quaternion);
	// # FindLookAtVector：计算从pos1到pos2的朝向向量
	Vector3 FindLookAtVector(Vector3 pos1, Vector3 pos2);

	// # ConverseAngle：转换角度（可能为弧度/角度转换或范围调整）
	float ConverseAngle(float x);
	// # AngleDifference：计算两个角度的差值（范围-180~180）
	float AngleDifference(float a, float b);

	// # get_enityangle：获取实体的角度（可能为朝向角）
	float get_enityangle(Vector4 quat);
	// # AngleDifference360：计算两个角度的差值（范围0~360）
	float AngleDifference360(float a, float b);
}

inline int 按键方式 = 0;

inline int 振刀按鍵 = 0;

inline int 自动断蓄 = 0;
//判断武器
int JudgeWeapon(uint32_t WeaponId, int types = 1);
int JudgeWeapon2(uint32_t WeaponId);
bool IsMeleeWeaponYC(WeaponType WeaponId);
//KmNet是否按下键
bool KmboxIsHokeyKey(short vk_key);

//VMM远程是否按下键
bool IsHokeyEx(int Key);
//模拟键盘按下(kmbox)
void SendKeyPressEx(uint32_t Key);

//模拟键盘弹起(kmbox)
void SendKeyUpEx(uint32_t Key);

//左键按下弹起(物理)
void MouseLeftEx(int key);

//右键按下弹起(物理)
void MouseRightEx(int key);

void MouseRightEx(int key);

//左键按弹起(kmbox)
void MouseLeftUp(int key);

//右键按下弹起(kmbox)
void MouseRightUp(int key);
//屏蔽左右键
void BlockLeftandRight();
//鼠标松开(kmbox)
void MouseRelease();
void MouseReleaseNoCheck();
//释放左右键
void ReleaseLeftandRight();
//鼠标移动(kmbox)
void Kmbox_Move(int x, int y);
//鼠标左右键松开
void MouseLeftorRightUp();
//鼠标移动(kmnet)
void Move_mouse(int x, int y);

void 按下键盘(int vk);

void 弹起键盘(int vk);

void 鼠标滚轮(int i);

void 鼠标側鍵2按下();

void 鼠标側鍵2弹起();

void 左键按下();

void 左键弹起();

void 右键按下();

void 右键弹起();

int 左键状态();

int 右键状态();
// 总方法控制
void root_sendKeyUp(int vk);
void root_sendKeyUpAll();
void root_sendKeyPress(int vk);
void root_kmNet_mouse_side(int vk);
void root_kmNet_mouse_side2(int vk);
void root_kmNet_mouse_right(int vk);
void root_kmNet_mouse_left(int vk);
void root_kmNet_mouse_middle(int vk);
void root_kmNet_middle(int vk);
void root_kmNet_unmask_all();
void root_BlockLeftandRight();
void root_ReleaseLeftandRight();

// dhz
void dhz_SendKeyPressEx(uint32_t vk);
void dhz_SendKeyUp(uint32_t vk);
void dhz_Move(int x, int y);
void dhz_鼠标左键(int Key);
void dhz_鼠标右键(int Key);
void dhz_鼠标侧键1(int Key);
void dhz_鼠标侧键2(int Key);
void dhz_鼠标中键(int Key);
void dhz_鼠标滚轮(int Key);
void dhz_解除所有键屏蔽();
void dhz_释放鼠标左右键();
void dhz_屏蔽鼠标左右键();