#pragma once
namespace Offset
{
	inline uint64_t UnityPlayer = 0;
	inline uint64_t GameAssembly = 0;
	inline uint64_t NarakaBladepoint = 0;
	inline uint64_t MatrixAddr = 0;
	inline uint64_t Base_WindowBase = 0x2795D38;  //游戏窗口大小
	inline uint64_t m_GlobalTime = 0xC62ADA8;            // 世界时间 GameBaseObject
	inline uint64_t m_CharacterManager = 0x11d9710;     // 世界地址   CharacterManager
	inline uint64_t m_UserDataManager = 0x8a4b20;		// 技能建筑数据基址 Class: UserData 
	inline uint64_t m_CharactorSync = 0x8a4b20;		// 人物同步 Class: CharactorSync
	inline uint64_t m_EntityManager = 0xC6298A0;        //物品基址 Class: GameEntity

	inline uint64_t InitActorKitRefer = 0x3a35840;        //内存振刀 
	inline uint64_t FlushState = 0x8a6010;					//刷新状态	

	inline uint64_t LocalGenRangeSingleData = 0x111;  //追踪
	inline uint64_t op_Subtraction = 0x1111;             //返回地址

	inline uint64_t Call_GetVelocity = 0x6C3C3D0;           //人物移动速c

	struct
	{
		DWORD lastSPLFramCount = 0xB8; //Class: CharacterManager -> Int32 lastSPLFramCount
		DWORD AllAliveCharList = 0x58;
		DWORD ArrayStart = 0x30;
	} Character;


	struct
	{
		DWORD ObjectMsg = 0xD0; //对象消息 Class: ActorModel -> ObjectMessenger objectMsg
		DWORD EventTracksForLayer = 0x30; //角色坐标 Class: ObjectMessenger -> EventTrackSet[] EventTracksForLayer
		DWORD Transform = 0x20; //角色坐标 Class: ActorModel -> Transform FastTransform					
		DWORD ActorWeapon = 0xF0; //武器结构 Class: ActorModel -> ActorWeapon actorWeapon
		DWORD ActorWeaponId = 0xF0; //武器id Class: ActorWeapon -> Int32 WeaponType
		DWORD RuntimePropertyData = 0xA0; // Class: ActorModel -> RuntimePlayerPropertyData RuntimePropertyData
		DWORD PropertyData = 0x340; // Class: ActorModel -> ActorModelPropertyData propertyData



		struct
		{
			DWORD Heroid = 0x108;   //英雄ID Class: ActorModelPropertyData -> Int32 _HeroTid
			DWORD Group = 0x24;    //团队 Class: ActorModelPropertyData -> Int32 _Group
			DWORD WeaponId = 0x128;  //武器id Class: ActorModelPropertyData -> Int32 _WeaponId
			DWORD RobotId = 0x44;   //机器人 Class: ActorModelPropertyData -> Int32 _RobotId
			DWORD PlayerName = 0x18; //人物名称 Class: ActorModelPropertyData -> String _Name
			DWORD SkillStatus = 0x198; //技能施放状态（0:未施放 1:前摇中 2:施放中） Class: ActorModelPropertyData -> Int32 _SkillCastStatus
		} ProPerty;

		DWORD ActorKit = 0xC0; //招式 Class: ActorModel -> ActorKit actorKit
		struct
		{
			DWORD BackingField = 0x38; //缓存行动根指针 Class: EntityKit -> ActorAnimatorParams <AniParams>k__BackingField
			DWORD ActorPhysics = 0xD0; //缓存角色行动物理 Class: EntityKit -> ActorKitPhysics actorPhysics
			DWORD HitSimulateData = 0xF0;   //状态 Class: EntityKit -> SkillComboData actorKitComboData
			DWORD HitSimulatePtr = 0xE0;   //状态 Class: EntityKit -> ActionHitSimulateData actorKitHitSimulateData
			DWORD actorKitBreakData = 0xF8; //Class: EntityKit -> ActionBreakData actorKitBreakData
			struct
			{
				DWORD ReactionType = 0x2c;				 //出刀状态 Class: ActionHitSimulateData -> EHitReactionType ReactionType
				DWORD RangeReactionType = 0x58;			//攻击状态 Class: ActionHitSimulateData -> EHitReactionType RangeReactionType
				DWORD ReactionExtraType = 0x5C;			//振刀状态 Class: ActionHitSimulateData -> EHitReactionExtraType ReactionExtraType
				DWORD endureLevel = 0x10;				 //当前状态 Class: ActionHitSimulateData -> Int32 EndureLevel
				DWORD ActionDuration = 0x2C;			//僵直 Class: ActionHitSimulateData -> Single ActionDuration
				DWORD ReactionParryType = 0xB4;			//磐石状态 Class: ActionHitSimulateData -> Int32 ParryGroup
				DWORD XuListate = 0x70;			//蓄力层级 Class: SkillComboData -> EComboCharge ChargeType
			} HitSimulate;
		} ActorKitS;
	} ActorModel;								//行动状态

	struct
	{
		DWORD InteractiveManager = 0x220; //落物堆数据 Class: UserBattleData -> InteractiveManager interactiveManager
		DWORD SpectatorData = 0x88; //判断观战 Class: UserData -> UserDataSpectatorData spectatorData
		DWORD Skill_ID = 0x10; // //判断技能id Class: SkillData -> Int32 fid
		DWORD skillState = 0x70;//判断技能状态 Class: SkillData -> SkillState skillState
		DWORD ClientLeftCdTime = 0x44;//技能冷却 Class: SkillData -> Single ClientLeftCdTime
		DWORD ClientLeftContinueTime = 0x4c;//技能持续时间 Class: SkillData -> Single ClientLeftContinueTime
	} UserData; //人物数据
}
