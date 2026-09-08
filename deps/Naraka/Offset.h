#pragma once
namespace Offset
{
	inline uint64_t UnityPlayer = 0;
	inline uint64_t GameAssembly = 0;
	inline uint64_t NarakaBladepoint = 0;
	inline uint64_t MatrixAddr = 0;
	inline uint64_t Base_WindowBase = 0x2795D38;  //游戏窗口大小
	inline uint64_t m_GlobalTime = 0xd22e0e8;            // 世界时间 Class: GameBaseObject
	inline uint64_t m_CharacterManager = 0xd22be70;     // 世界地址  Class: CharacterManager
	inline uint64_t m_UserDataManager = 0xd22d9a8;		// 技能建筑数据基址 Class: UserData 
	inline uint64_t m_CharactorSync = 0xd563880;		// 人物同步 Class: CharactorSync
	inline uint64_t m_EntityManager = 0xd22cac0;        //物品基址 Class: GameEntity

	inline uint64_t Base_WindowBase2 = 0x33E44B8;  //游戏窗口大小
	inline uint64_t m_GlobalTime2 = 0x33D7968;            // 世界时间 Class: GameBaseObject
	inline uint64_t m_CharacterManager2 = 0x33CEEF0;     // 世界地址  Class: CharacterManager
	inline uint64_t m_UserDataManager2 = 0x33F1068;		// 技能建筑数据基址 Class: UserData 
	inline uint64_t m_CharactorSync2 = 0x33CEF58;		// PING人物同步 Class: CharactorSync
	inline uint64_t m_EntityManager2 = 0x33D7A20;        //物品基址 Class: GameEntity



	inline void ApplyGameAssemblyOffsets(bool useSuper)
	{
		static const uint64_t kBase_WindowBase = Base_WindowBase;
		static const uint64_t kGlobalTime = m_GlobalTime;
		static const uint64_t kCharacterManager = m_CharacterManager;
		static const uint64_t kUserDataManager = m_UserDataManager;
		static const uint64_t kCharactorSync = m_CharactorSync;
		static const uint64_t kEntityManager = m_EntityManager;

		if (useSuper) {
			Base_WindowBase = Base_WindowBase2;
			m_GlobalTime = m_GlobalTime2;
			m_CharacterManager = m_CharacterManager2;
			m_UserDataManager = m_UserDataManager2;
			m_CharactorSync = m_CharactorSync2;
			m_EntityManager = m_EntityManager2;
		}
		else {
			Base_WindowBase = kBase_WindowBase;
			m_GlobalTime = kGlobalTime;
			m_CharacterManager = kCharacterManager;
			m_UserDataManager = kUserDataManager;
			m_CharactorSync = kCharactorSync;
			m_EntityManager = kEntityManager;
		}
	}

	inline uint64_t InitActorKitRefer = 0x111;        //内存振刀 
	inline uint64_t FlushState = 0x11111;					//刷新状态	

	inline uint64_t LocalGenRangeSingleData = 0x111;  //追踪
	inline uint64_t op_Subtraction = 0x1111;             //返回地址

	inline uint64_t Call_GetVelocity = 0x6C3C3D0;           //人物移动速c

	struct
	{
		DWORD lastSPLFramCount = 0xB8; //Class: CharacterManager->Int32 lastSPLFramCount
		DWORD AllAliveCharList = 0x58; //Class: CharacterManager->fid2AliveCharacter
		DWORD ArrayStart = 0x30;
		DWORD fpsFilter = 0x48; // FPS过滤器 Class: CharacterManager->FpsFilter fpsFilter
	} Character;
	struct
	{
		DWORD fps = 0x2C; // FPS值 Class: FpsFilter->Int32 fps
	} FpsFilter;

	struct
	{
		DWORD ObjectMsg = 0xd8; //对象消息 Class: ActorModel->ObjectMessenger objectMsg
		DWORD EventTracksForLayer = 0x30; //角色坐标 Class: ObjectMessenger->EventTrackSet[] EventTracksForLayer
		DWORD _curAnimPlayableAgent = 0x68; //当前时间 Class: ObjectMessenger->AnimPlayable _curAnimPlayableAgent
		DWORD _runtimeLogicLayers = 0x48; //当前时间 Class: AnimPlayable->RuntimeLogicLayer[] _runtimeLogicLayers

		DWORD Transform = 0x118; //角色坐标 Class: ActorModel->Transform FastTransform					
		DWORD ActorWeapon = 0xf8; //武器结构 Class: ActorModel->ActorWeapon actorWeapon
		DWORD WeaponType = 0x60; //武器类型 Class: ActorWeapon->Int32 WeaponType
		DWORD RuntimePropertyData = 0xa8; // Class: ActorModel->RuntimePlayerPropertyData RuntimePropertyData
		DWORD PropertyData = 0x348; // Class: ActorModel->ActorModelPropertyData propertyData
		// 技能偏移
		DWORD FestivalGiftBagData = 0xb8; // Class: UserData->UserDataFestivalGiftBagActData FestivalGiftBagData
		DWORD battleData = 0x30; // Class: UserData->UserBattleData battleData
		DWORD skillData = 0x108; // Class: UserBattleData->UserBattleSkillData skillData
		DWORD characterSkillDataDict = 0x10; // Class: UserBattleSkillData->Dictionary`2 characterSkillDataDict
		// 武器索引
		DWORD fighterEquipData = 0x68; // Class: UserBattleData->UserBattleEquipData fighterEquipData

		DWORD cur_weapon_slot = 0x20; //手持武器索引 Class: PlayerEquipInfoData->Int32 cur_weapon_slot
		// 是否可见
		DWORD cullingVisible = 0x6C; //是否可见 Class: ActorModel->Boolean cullingVisible
		// 游戏延迟
		DWORD shrtt = 0x68; //游戏延迟 Class: CharactorSync->Single shrtt
		// 游戏时间
		DWORD currentTime = 0x58; //当前动作时间 Class: BattleLineCurve->Single currentTime
		DWORD length = 0x54; //动画总时间 Class: BattleLineCurve->Single length
		DWORD nowTime = 0x60; //当前动画执行时间 Class: BattleLineCurve->Double nowTime
		DWORD _globalTime = 0x28; //世界总时间 Class: GameBaseObject->Double _globalTime
		// 游戏哈希名
		DWORD CurShareState = 0x48; //游戏哈希名 Class: RuntimeLogicLayer->PlayableState CurShareState
		DWORD NameHash = 0x18; //游戏哈希名 Class: PlayableState->Int32 NameHash
		DWORD animClipName = 0x18; //角色名 Class: EventTrackSet->String animClipName


		struct
		{
			DWORD Heroid = 0x108;   //英雄ID Class: ActorModelPropertyData->Int32 _HeroTid
			DWORD Group = 0x118;    //团队 Class: ActorModelPropertyData->Int32 _Group
			DWORD WeaponId = 0x128;  //武器id Class: ActorModelPropertyData->Int32 _WeaponId
			DWORD RobotId = 0x130;   //机器人 Class: ActorModelPropertyData->Int32 _RobotId
			DWORD PlayerName = 0x1f8; //人物名称 Class: ActorModelPropertyData->String _Name
			DWORD SkillStatus = 0x198; //技能施放状态（0:未施放 1:前摇中 2:施放中） Class: ActorModelPropertyData->Int32 _SkillCastStatus
		} ProPerty;

		DWORD ActorKit = 0xc8; //招式 Class: ActorModel->ActorKit actorKit
		struct
		{
			DWORD BackingField = 0x38; //缓存行动根指针 Class: EntityKit->ActorAnimatorParams <AniParams>k__BackingField
			DWORD ActorPhysics = 0xD0; //缓存角色行动物理 Class: EntityKit->ActorKitPhysics actorPhysics
			DWORD HitSimulateData = 0xF0;   //状态 Class: EntityKit->ActionHitSimulateData actorKitHitSimulateData
			DWORD HitSimulatePtr = 0xE0;   //状态 Class: EntityKit->SkillComboData actorKitComboData
			DWORD actorKitBreakData = 0xF8; //Class: EntityKit->ActionBreakData actorKitBreakData
			struct
			{
				DWORD ReactionType = 0x54;				 //出刀状态 Class: ActionHitSimulateData->EHitReactionType ReactionType
				DWORD RangeReactionType = 0x58;			//攻击状态 Class: ActionHitSimulateData->EHitReactionType RangeReactionType
				DWORD ReactionExtraType = 0x5C;			//振刀状态 Class: ActionHitSimulateData->EHitReactionExtraType ReactionExtraType
				DWORD endureLevel = 0xD0;				 //当前状态 Class: ActionHitSimulateData->Int32 EndureLevel
				DWORD ActionDuration = 0x2C;			//僵直 Class: ActionHitSimulateData->Single ActionDuration
				DWORD ReactionParryType = 0xB4;			//磐石状态 Class: ActionHitSimulateData->Int32 ParryGroup
				DWORD XuListate = 0x70;			//蓄力层级 Class: SkillComboData->EComboCharge ChargeType
			} HitSimulate;
		} ActorKitS;

		struct
		{
			DWORD CameraAgent = 0x30; // Class: EntityKit->CameraAgent <cameraAgent>k__BackingField
		} EntityKitS;

		struct
		{
			DWORD CameraController = 0x88; // Class: CameraAgent->LXCameraController CameraController
		} CameraAgentS;

		struct
		{
			DWORD AdventureRig = 0x58; // Class: LXCameraController->AdventureRig <AdventureRig>k__BackingField
		} LXCameraControllerS;

		struct
		{
			DWORD Euler = 0x248; // Class: AdventureRig->Vector3 _Euler
		} AdventureRigS;
	} ActorModel;								//行动状态
	struct
	{
		DWORD _Instance = 0x8; // Class: GameBaseObject->GameBaseObject _Instance
		DWORD _fps = 0x18; // Class: GameBaseObject->AvgFPSManager _fps
	} GameBaseObject;

	struct
	{
		DWORD AvgRenderFPS = 0x14; // Class: AvgFPSManager->Int32 AvgRenderFPS
	} AvgFPSManager;


	struct
	{
		DWORD InteractiveManager = 0x228; //落物堆数据 Class: UserBattleData->InteractiveManager interactiveManager
		DWORD SpectatorData = 0x88; //判断观战 Class: UserData->UserDataSpectatorData spectatorData
		DWORD Skill_ID = 0x10; // //判断技能id Class: SkillData->Int32 fid
		DWORD skillState = 0x80;//判断技能状态 Class: SkillData->SkillState skillState
		DWORD ClientLeftCdTime = 0x44;//技能冷却 Class: SkillData->Single ClientLeftCdTime
		DWORD ClientLeftContinueTime = 0x4c;//技能持续时间 Class: SkillData->Single ClientLeftContinueTime
		DWORD logicData = 0x70;//技能持续时间 Class: SkillData->SkillConfig_Skill logicData
	} UserData; //人物数据

	struct
	{
		// 堆落
		DWORD interactiveId = 0x14; //堆落的interactiveID Class: InteractiveData->Int32 interactiveId
		DWORD status = 0x34; //堆落是否已打开0未打开 Class: InteractiveData->Int32 status
		DWORD position = 0x18; //堆落坐标 Class: InteractiveData->Vector3 position
	}ArticleData; //人物数据

	struct
	{
		// 魂玉
		DWORD fid = 0x340; //魂玉id Class: ActorModel->Int32 fid
		DWORD soulItemData = 0x90; //魂玉背包 Class: UserBattleData->UserBattleSoulItemData soulItemData
		DWORD dataDict = 0x10; //魂玉字典列表 Class: UserBattleSoulItemData->Dictionary`2 dataDict
		DWORD soulItemDataFid = 0x10; //魂玉id Class: PlayerBattleSoulItemData->Int32 fid
		DWORD _soulItemList = 0x40; //魂玉列表 Class: PlayerBattleSoulItemData->List`1 _soulItemList

		DWORD info = 0x10; //魂玉id Class: BattleSoulItemData->SoulItemData info
		DWORD fid_ = 0x18; //魂玉 Class: SoulItemData->Int32 fid_
		DWORD soul_item_tid_ = 0x1c; //魂玉 Class: SoulItemData->Int32 soul_item_tid_


		DWORD status = 0x34; //堆落是否已打开0未打开 Class: InteractiveData->Int32 status
		DWORD position = 0x18; //堆落坐标 Class: InteractiveData->Vector3 position
	}SoulData; //人物数据

	struct
	{
		// 背包物品
		DWORD itemData = 0x88; //背包数据 Class: UserBattleData->UserBattleItemInfo itemData
		DWORD dataDict = 0x10; //背包字典列表 Class: UserBattleItemInfo->Dictionary`2 dataDict
		DWORD itemDataFid = 0x10; //背包角色id Class: PlayerBattleItemInfo->Int32 fid
		DWORD itemDataList = 0x30; //背包物品列表 Class: PlayerBattleItemInfo->List`1 itemDataList

		DWORD battleItemInfo = 0x10; //物品信息 Class: BattleItemData->BattleItemInfo info
		DWORD item_id_ = 0x1c; //物品id Class: BattleItemInfo->Int32 item_id_
		DWORD listItems = 0x10; //List1->T[] _items
		DWORD listSize = 0x18; //List1->Int32 _size
	}BattleItemData; //人物数据
}