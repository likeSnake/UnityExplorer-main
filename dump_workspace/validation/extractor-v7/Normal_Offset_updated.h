#pragma once
namespace Offset
{
	inline uint64_t UnityPlayer = 0;
	inline uint64_t GameAssembly = 0;
	inline uint64_t NarakaBladepoint = 0;
	inline uint64_t MatrixAddr = 0;
	inline uint64_t Base_WindowBase = 0x2795D38;  //游戏窗口大小
	inline uint64_t m_GlobalTime = 0xe644808;            // 世界时间 Class: GameBaseObject
	inline uint64_t m_CharacterManager = 0xe642388;     // 世界地址  Class: CharacterManager
	inline uint64_t m_UserDataManager = 0xe644068;		// 技能建筑数据基址 Class: UserData 
	inline uint64_t m_CharactorSync = 0xe606490;		// 人物同步 Class: CharactorSync
	inline uint64_t m_EntityManager = 0xe643020;        //物品基址 Class: GameEntity
	inline uint64_t m_BuffManager = 0xE641868;        // BuffManager TypeInfo (class) RVA, DMA verified 2026-09-03
	inline uint64_t m_TransparentTagHandler = 0xE88A328; // TransparentTagHandler TypeInfo RVA, DMA verified 2026-09-03

	inline uint64_t Base_WindowBase2 = 0x3764380;  //游戏窗口大小
	inline uint64_t m_GlobalTime2 = 0x3755D18;            // 世界时间 Class: GameBaseObject
	inline uint64_t m_CharacterManager2 = 0x374C2B8;     // 世界地址  Class: CharacterManager
	inline uint64_t m_UserDataManager2 = 0x3772EA8;		// 技能建筑数据基址 Class: UserData 
	inline uint64_t m_CharactorSync2 = 0x374C368;		// PING人物同步 Class: CharactorSync
	inline uint64_t m_EntityManager2 = 0x3755DD0;        //物品基址 Class: GameEntity
	inline uint64_t m_BuffManager2 = 0x374AE20;        // BuffManager TypeInfo (class) RVA (TODO: update if different)
	inline uint64_t m_TransparentTagHandler2 = 0x376ABF8;        // Class: TransparentTagHandler, s_instance->refCharacterMgr fallback


	enum class GameAssemblyVariant
	{
		Default = 0,
		Super,
		SuperIBT
	};

	inline GameAssemblyVariant CurrentGameAssemblyVariant = GameAssemblyVariant::Default;

	inline bool IsGameAssemblyDefault()
	{
		return CurrentGameAssemblyVariant == GameAssemblyVariant::Default;
	}

	inline bool IsGameAssemblySuper()
	{
		return CurrentGameAssemblyVariant == GameAssemblyVariant::Super;
	}

	inline bool IsGameAssemblySuperIBT()
	{
		return CurrentGameAssemblyVariant == GameAssemblyVariant::SuperIBT;
	}

	inline void ApplyGameAssemblyOffsets(GameAssemblyVariant variant)
	{
		CurrentGameAssemblyVariant = variant;

		static const uint64_t kBase_WindowBase = Base_WindowBase;
		static const uint64_t kGlobalTime = m_GlobalTime;
		static const uint64_t kCharacterManager = m_CharacterManager;
		static const uint64_t kUserDataManager = m_UserDataManager;
		static const uint64_t kCharactorSync = m_CharactorSync;
		static const uint64_t kEntityManager = m_EntityManager;
		static const uint64_t kBuffManager = m_BuffManager;
		static const uint64_t kTransparentTagHandler = m_TransparentTagHandler;

		if (variant == GameAssemblyVariant::Super || variant == GameAssemblyVariant::SuperIBT) {
			Base_WindowBase = Base_WindowBase2;
			m_GlobalTime = m_GlobalTime2;
			m_CharacterManager = m_CharacterManager2;
			m_UserDataManager = m_UserDataManager2;
			m_CharactorSync = m_CharactorSync2;
			m_EntityManager = m_EntityManager2;
			m_BuffManager = m_BuffManager2;
			m_TransparentTagHandler = m_TransparentTagHandler2;
		}
		else {
			Base_WindowBase = kBase_WindowBase;
			m_GlobalTime = kGlobalTime;
			m_CharacterManager = kCharacterManager;
			m_UserDataManager = kUserDataManager;
			m_CharactorSync = kCharactorSync;
			m_EntityManager = kEntityManager;
			m_BuffManager = kBuffManager;
			m_TransparentTagHandler = kTransparentTagHandler;
		}
	}

	// 内存振 / CrossFade hook：版本相关 RVA 与字段偏移集中在这里维护。
	// 运行时地址一律为模块基址 + RVA；对象字段一律为对象地址 + Offset。
	inline constexpr uint64_t MemoryShockUnresolvedRva = 0x111;
	inline uint64_t InitActorKitRefer = MemoryShockUnresolvedRva; // 运行时解析为 TransparentTagHandler TypeInfo RVA

	// IL2CPP runtime layout。通常比游戏字段稳定，但 Unity/IL2CPP 大版本变化时仍需复核。
	inline constexpr uint32_t Il2CppClassNameOffset = 0x10;
	inline constexpr uint32_t Il2CppClassParentOffset = 0x58;
	inline constexpr uint32_t Il2CppClassMethodsOffset = 0x98;
	inline constexpr uint32_t Il2CppClassMethodCountOffset = 0x120;
	inline constexpr uint32_t Il2CppClassStaticFieldsOffset = 0xB8;
	inline constexpr uint32_t Il2CppMethodPointerOffset = 0x00;
	inline constexpr uint32_t Il2CppMethodNameOffset = 0x18;
	inline constexpr uint32_t Il2CppMethodSlotOffset = 0x50;
	inline constexpr uint32_t Il2CppMethodParameterCountOffset = 0x52;
	inline constexpr uint32_t Il2CppStaticInstanceOffset = 0x08;

	// 当前 CrossFade 内存振共享对象链。
	inline constexpr uint32_t MemoryShockHookContextEntityKitOffset = 0x10; // hook rcx + 0x10 -> EntityKit，运行时观察
	inline constexpr uint32_t MemoryShockEntityKitActorModelOffset = 0x18; // Class: EntityKit->ActorModel <actorModel>k__BackingField
	inline constexpr uint32_t MemoryShockTransparentTagHandlerActorKitOffset = 0x70; // Class: TransparentTagHandler->ActorKit actorkit
	inline constexpr uint32_t MemoryShockTransparentTagHandlerCharacterManagerOffset = 0xD8; // TransparentTagHandler.refCharacterMgr
	inline constexpr uint32_t MemoryShockCharacterManagerMainPlayerOffset = 0x18; // CharacterManager.mainPlayerActorModel
	inline constexpr uint32_t MemoryShockInteractableVtableSlotOffset = 0x198;
	inline constexpr uint32_t MemoryShockCacheDelayBreakTypeOffset = 0x58; // ActionBreakData.CacheDelayBreakType

	// Legacy 内存振：旧实现保留值，具体类语义尚未全部还原，升级时必须逐项验证。
	inline constexpr uint64_t MemoryShockCodecaveHintRva = 0x55E5C; // NarakaBladepoint.exe 内只读 codecave 搜索起点
	inline constexpr uint32_t MemoryShockBootstrapChain0Offset = 0x18; // [待确认语义]
	inline constexpr uint32_t MemoryShockBootstrapChain1Offset = 0xC0; // [待确认语义]
	inline constexpr uint32_t MemoryShockBootstrapChain2Offset = 0x90; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyReferRva = 0x3EC8D92; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyReRva = 0x3EE3AB8; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyRefRva = 0x36F3250; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyJmpRva = 0xCBA5669; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyJmp2Rva = 0xCBA5630; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyJmp3Rva = 0xCBA55A8; // [待确认语义]
	inline constexpr uint64_t MemoryShockLegacyJmp4Rva = 0xCBA5669; // [待确认语义]
	inline constexpr uint32_t MemoryShockLegacyLocalActorChain0Offset = 0xA8; // [待确认语义]
	inline constexpr uint32_t MemoryShockLegacyLocalActorChain1Offset = 0x18; // [待确认语义]
	inline constexpr uint32_t MemoryShockLegacyTestFieldOffset = 0x48B; // test byte ptr [rax+offset]
	inline constexpr uint32_t MemoryShockLegacyCompareValue = 0x245A;
	inline constexpr uint32_t MemoryShockLegacyMoveFieldOffset = 0x122E8;
	inline constexpr uint32_t MemoryShockLegacyRefStateOffset = 0xE0; // [待确认语义]
	inline constexpr uint32_t MemoryShockLegacyRefObjectOffset = 0xB8; // [待确认语义]
	inline constexpr uint32_t MemoryShockLegacyRefCompareOffset = 0x18; // [待确认语义]

	// 旧 FlushState 动画加速路径的身份链；该路径不是当前 InitMemoryShockHook 的主路径。
	inline constexpr uint32_t MemoryShockFlushIdentityChain0Offset = 0xA8; // [待确认语义]
	inline constexpr uint32_t MemoryShockFlushIdentityChain1Offset = 0x58; // [待确认语义]
	inline constexpr uint32_t MemoryShockFlushIdentityChain2Offset = 0x18; // [待确认语义]
	inline constexpr uint32_t MemoryShockFlushIdentityChain3Offset = 0x30; // [待确认语义]
	inline constexpr uint32_t MemoryShockFlushAnimPlayableOffset = 0x2C0; // 旧路径观察值；当前 dump 的 ActorKit 字段另见 AnimPlayableAgent

	// Virtual 内存振（当前普通 GameAssembly.dll 路径）。
	inline constexpr uint32_t MemoryShockVirtualVtableSlotOffset = 0x190;
	inline constexpr uint32_t MemoryShockVirtualBreakCacheOffset = 0x10; // 旧 shellcode 字段，[待确认语义]
	inline constexpr uint64_t MemoryShockVirtualCallARva = 0x12C6750; // [待确认语义]
	inline constexpr uint64_t MemoryShockVirtualCallBRva = 0x238ED00; // [待确认语义]
	inline constexpr uint64_t MemoryShockVirtualCallCRva = 0x13B00B0; // [待确认语义]
	inline constexpr uint64_t MemoryShockVirtualReturnRva = 0x36E1E73; // [待确认语义]

	// 2026-08-13 normal 来自 dump；super 由当前进程 AnimPlayable MethodInfo DMA 验证。
	inline constexpr uint64_t AnimPlayableCrossFadeInFixedTimeNormal = 0x5811350;
	inline constexpr uint64_t AnimPlayableCrossFadeInFixedTimeSuper = 0x797D760;
	// 通过 ActorModel->ActorKit->EntityKit klass->methods 链提取
	inline uint64_t FlushState = 0x1D7BE0;              //刷新状态 RVA (ActorKit.FlushState)
	inline uint64_t CrossFadeInFixedTime = AnimPlayableCrossFadeInFixedTimeNormal;

	inline uint64_t LocalGenRangeSingleData = 0x111;  //追踪
	inline uint64_t op_Subtraction = 0x1111;             //返回地址

	inline uint64_t Call_GetVelocity = 0x6C3C3D0;           //人物移动速c

	struct
	{
		DWORD lastSPLFramCount = 0xb8; //Class: CharacterManager->Int32 lastSPLFramCount
		DWORD AllAliveCharList = 0x50; // Class: CharacterManager->Dictionary`2 fid2AliveCharacter
		DWORD ArrayStart = 0x30;
		DWORD fpsFilter = 0x40; // FPS过滤器 Class: CharacterManager->FpsFilter fpsFilter
	} Character;
	struct
	{
		DWORD fps = 0x2C; // FPS值 Class: FpsFilter->Int32 fps
	} FpsFilter;

	struct
	{
		DWORD ObjectMsg = 0xd8; //对象消息 Class: ActorModel->ObjectMessenger objectMsg
		DWORD EventTracksForLayer = 0x30; //角色坐标 Class: ObjectMessenger->EventTrackSet[] EventTracksForLayer
		DWORD _curAnimPlayableAgent = 0x70; //当前时间 Class: ObjectMessenger->AnimPlayable _curAnimPlayableAgent
		DWORD _runtimeLogicLayers = 0x48; //当前时间 Class: AnimPlayable->RuntimeLogicLayer[] _runtimeLogicLayers

		DWORD Transform = 0x128; //角色坐标 Class: ActorModel->Transform FastTransform					
		DWORD ActorWeapon = 0xf8; //武器结构 Class: ActorModel->ActorWeapon actorWeapon
		DWORD WeaponType = 0x68; //武器类型 Class: ActorWeapon->Int32 WeaponType
		DWORD RuntimePropertyData = 0xa8; // Class: ActorModel->RuntimePlayerPropertyData RuntimePropertyData
		DWORD PropertyData = 0x358; // Class: ActorModel->ActorModelPropertyData propertyData
		// 技能偏移
		DWORD FestivalGiftBagData = 0xc8; // Class: UserData->UserDataFestivalGiftBagActData FestivalGiftBagData
		DWORD PettyCashGiftData = 0xc0; // Class: UserData->UserDataPettyCashGiftActData PettyCashGiftData
		DWORD battleData = 0x38; // Class: UserData->UserBattleData battleData
		DWORD skillData = 0x118; // Class: UserBattleData->UserBattleSkillData skillData
		DWORD characterSkillDataDict = 0x10; // Class: UserBattleSkillData->Dictionary`2 characterSkillDataDict
		// 武器索引
		DWORD fighterEquipData = 0x70; // Class: UserBattleData->UserBattleEquipData fighterEquipData

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
			DWORD BeControlledStatus = 0x140; // Class: ActorModelPropertyData->Int32 _BeControlledStatus
			DWORD HeroBattleStatus = 0x14c; // Class: ActorModelPropertyData->Int32 _HeroBattleStatus
			DWORD ComboId = 0x158; // Class: ActorModelPropertyData->Int32 _ComboId
			DWORD CurrentStageType = 0x160; // Class: ActorModelPropertyData->Int32 _CurrentStageType
			DWORD PlayerName = 0x200; //人物名称 Class: ActorModelPropertyData->String _Name
			DWORD SkillStatus = 0x198; //技能施放状态（0:未施放 1:前摇中 2:施放中） Class: ActorModelPropertyData->Int32 _SkillCastStatus
			DWORD StereoDodgeType = 0x1a0; // Class: ActorModelPropertyData->Int32 _StereoDodgeType
			DWORD LastStateHash = 0x1ac; // Class: ActorModelPropertyData->Int32 _LastStateHash
			DWORD CurrentUseItemSkinId = 0x1b4; // Class: ActorModelPropertyData->Int32 _CurrentUseItemSkinId
			DWORD LastBeControlledStatus = 0x1c8; // Class: ActorModelPropertyData->Int32 _LastBeControlledStatus
			DWORD LastBaseStateHash = 0x1dc; // Class: ActorModelPropertyData->Int32 _LastBaseStateHash
			DWORD UniqueSkillState = 0x1f4; // Class: ActorModelPropertyData->Int32 _UniqueSkillState
			DWORD JiguansanState = 0x1f8; // Class: ActorModelPropertyData->Int32 _JiguansanState
		} ProPerty;

		DWORD ActorKit = 0xc8; //招式 Class: ActorModel->ActorKit actorKit
		struct
		{
			DWORD BackingField = 0x38; //缓存行动根指针 Class: EntityKit->ActorAnimatorParams <AniParams>k__BackingField
			DWORD InteractableDevice = 0x90; // Class: EntityKit->IEntityKitInteractableDevice actorKitInteractableDevice
			DWORD ActorKitInteractive = 0xd0; // Class: EntityKit->IEntityKitInteractive actorKitInteractive
			DWORD ActorPhysics = 0xe0; //缓存角色行动物理 Class: EntityKit->ActorKitPhysics actorPhysics
			DWORD CurrentCollisionFlags = 0xf4; //落地碰撞标记 Class: ActorKitPhysics->CollisionFlags _currentCollisionFlags
			DWORD HitSimulateData = 0x100;   //状态 Class: EntityKit->ActionHitSimulateData actorKitHitSimulateData
			DWORD HitSimulatePtr = 0xf0;   //状态 Class: EntityKit->SkillComboData actorKitComboData
			DWORD actorKitBreakData = 0x108; //Class: EntityKit->ActionBreakData actorKitBreakData
			DWORD AnimPlayableAgent = 0x2e8; // Class: ActorKit->AnimPlayable <AnimPlayableAgent>k__BackingField
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
			// Thread.cpp: EntityKit.actorKitInteractive 声明为 IEntityKitInteractive；以下为 ActorKitInteractive 实现类布局，读取前应核对实际实例类。
			DWORD TargetInteractiveFID = 0x10; // Class: ActorKitInteractive->Int32 m_targetInteractiveFID
			DWORD CanRescueActor = 0x28; // Class: ActorKitInteractive->ActorKit canRescueActor
			DWORD CanStealActor = 0x30; // Class: ActorKitInteractive->ActorKit canStealActor
			DWORD DyingExecuteActorFid = 0x38; // Class: ActorKitInteractive->Int32 DyingExecuteActorFid
			DWORD InteractiveType = 0x58; // Class: ActorKitInteractive->ActorInteractiveType m_interactiveType
			DWORD IsMainPlayer = 0x5c; // Class: ActorKitInteractive->Boolean isMainPlayer
			DWORD Status = 0x60; // Class: ActorKitInteractive->ActorInteractiveStatus m_status
			DWORD IType = 0x64; // Class: ActorKitInteractive->InteractiveType m_itype
			DWORD RescueFid = 0x68; // Class: ActorKitInteractive->Int32 rescue_fid
			DWORD NowShowInteractiveType = 0x70; // Class: ActorKitInteractive->ActorInteractiveType now_showinteractiveType
		} ActorKitInteractiveS;

		struct
		{
			// Thread.cpp: AnimPlayable._runtimeLogicLayers 数组元素 -> RuntimeLogicLayer 实例。
			DWORD LayerIndex = 0x10; // Class: RuntimeLogicLayer->Int32 layerIndex
			DWORD LayerType = 0x14; // Class: RuntimeLogicLayer->ELogicLayer LayerType
			DWORD LayerWeight = 0x18; // Class: RuntimeLogicLayer->Single _layerWeight
			DWORD CurShareState = 0x48; // Class: RuntimeLogicLayer->PlayableState CurShareState
			DWORD CurStateStartTimeOffset = 0x50; // Class: RuntimeLogicLayer->Single CurStateStartTimeOffset
			DWORD CurDuration = 0x54; // Class: RuntimeLogicLayer->Single CurDuration
			DWORD CurTimeForLogicState = 0x60; // Class: RuntimeLogicLayer->Single _curTimeForLogicState
			DWORD TransitionDuration = 0x74; // Class: RuntimeLogicLayer->Single TransitionDuration
			DWORD TransitionPercent = 0x78; // Class: RuntimeLogicLayer->Single _transitionPercent
			DWORD CurIsLogicState = 0x7c; // Class: RuntimeLogicLayer->Boolean CurIsLogicState
			DWORD TransitionFinish = 0x7e; // Class: RuntimeLogicLayer->Boolean transitionFinish
		} RuntimeLogicLayerS;

		struct
		{
			// Thread.cpp: RuntimeLogicLayer.CurShareState -> PlayableState 实例。
			DWORD AnimatorName = 0x10; // Class: PlayableState->String animatorName
			DWORD NameHash = 0x18; // Class: PlayableState->Int32 NameHash
			DWORD TagHash = 0x1c; // Class: PlayableState->Int32 TagHash
		} PlayableStateS;

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
			DWORD Euler = 0x250; // Class: AdventureRig->Vector3 _Euler
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
		DWORD InteractiveManager = 0x240; //落物堆数据 Class: UserBattleData->InteractiveManager interactiveManager
		DWORD ClientDodgeConsumeData = 0x228; // Class: UserBattleData->ClientDodgeConsumeData clientDodgeConsumeData
		DWORD SpectatorData = 0x90; //判断观战 Class: UserData->UserDataSpectatorData spectatorData
		DWORD isSpectator = 0x48; //是否观战 Class: UserDataSpectatorData->Boolean isSpectator
		DWORD Skill_ID = 0x10; // //判断技能id Class: SkillData->Int32 fid
		DWORD skillState = 0x78;//判断技能状态 Class: SkillData->SkillState skillState
		DWORD ClientLeftCdTime = 0x40;//技能冷却 Class: SkillData->Single ClientLeftCdTime
		DWORD ClientLeftContinueTime = 0x48;//技能持续时间 Class: SkillData->Single ClientLeftContinueTime
		DWORD logicData = 0x68;//技能持续时间 Class: SkillData->SkillConfig_Skill logicData
	} UserData; //人物数据

	struct
	{
		DWORD Items = 0x10; // Class: List`1->T[] _items
		DWORD Size = 0x18; // Class: List`1->Int32 _size
		DWORD FirstItem = 0x20; // Class: T[] first element
	} Il2CppList;

	struct
	{
		DWORD DodgeConsumeIdx = 0x10; // Class: ClientDodgeConsumeData->Int32 dodge_consume_idx
		DWORD DodgeConsumePaddingList = 0x18; // Class: ClientDodgeConsumeData->List`1 dodgeConsumePaddingList
	} ClientDodgeConsumeData;

	struct
	{
		DWORD Valid = 0x10; // Class: ClientDodgeConsumeDataItem->Boolean valid
		DWORD CostValue = 0x14; // Class: ClientDodgeConsumeDataItem->Int32 cost_value
		DWORD RuntimeProp = 0x18; // Class: ClientDodgeConsumeDataItem->Int32 rt_prop
	} ClientDodgeConsumeDataItem;

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
		DWORD fid = 0x350; //魂玉id Class: ActorModel->Int32 fid
		DWORD soulItemData = 0x98; //魂玉背包 Class: UserBattleData->UserBattleSoulItemData soulItemData
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
		DWORD itemData = 0x90; //背包数据 Class: UserBattleData->UserBattleItemInfo itemData
		DWORD dataDict = 0x10; //背包字典列表 Class: UserBattleItemInfo->Dictionary`2 dataDict
		DWORD itemDataFid = 0x10; //背包角色id Class: PlayerBattleItemInfo->Int32 fid
		DWORD itemDataList = 0x30; //背包物品列表 Class: PlayerBattleItemInfo->List`1 itemDataList

		DWORD battleItemInfo = 0x10; //物品信息 Class: BattleItemData->BattleItemInfo info
		DWORD item_id_ = 0x1c; //物品id Class: BattleItemInfo->Int32 item_id_
		DWORD count_ = 0x20; //物品数量 Class: BattleItemInfo->Int32 count_
		DWORD listItems = 0x10; //List1->T[] _items
		DWORD listSize = 0x18; //List1->Int32 _size
	}BattleItemData; //人物数据

	struct
	{
		DWORD _Instance = 0x0;                 // static field; Class: BuffManager->BuffManager _instance
		DWORD recordBuffHandlersDict = 0x10;   // Class: BuffManager->Dictionary`2 _recordBuffHandlersDict
		DWORD BuffDict = 0x30;                 // Class: BuffHandler->Dictionary`2 _mBuffDict
		DWORD BuffInstanceIdList = 0x38;       // Class: BuffHandler->List`1 <BuffInstanceIdList>k__BackingField
		DWORD BuffTidList = 0x40;              // Class: BuffHandler->List`1 <BuffTidList>k__BackingField
		DWORD BuffId = 0x20;                   // Class: Buff->Int32 <BuffId>k__BackingField
		DWORD GroupId = 0x24;                  // Class: Buff->Int32 <GroupId>k__BackingField
		DWORD LeftTime = 0x34;                 // Class: Buff->Single leftTime
		DWORD OwnerFid = 0x84;                 // Class: Buff->Int32 ownerFid
		DWORD CasterFid = 0x88;                // Class: Buff->Int32 casterFid
		DWORD BuffIId = 0x8c;                  // Class: Buff->Int32 <BuffIId>k__BackingField
	} Buff;
}
