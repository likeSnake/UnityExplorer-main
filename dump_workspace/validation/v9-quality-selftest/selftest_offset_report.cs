// ===== Offset.h Discovery Summary (v7) =====
// Active module: GameAssembly_Super.dll base=0x20127410000 variant=Super/SuperIBT
// Other variant: NOT_LOADED (no values available in this run)
// Keys preserve Offset.h paths. RVA, instance/static field offset and runtime VA are labelled separately.
// FOUND/CALIBRATED/VERIFIED/DERIVED = resolved; CANDIDATE requires validation; ABI_LAYOUT/CONFIG_CONSTANT are not discoveries.
// UNRESOLVED_LAYOUT preserves raw metadata in evidence; generic instance offsets below the object header are not usable offsets.
// 未加载的另一变体不填值；未找到/待确认/已跳过旧项均明确标注，不沿用历史偏移。
// Offset::UnityPlayer = 0x20127410000; // status=RUNTIME_VA module base VA; changes on restart
// Offset::GameAssembly = 0x20127410000; // status=RUNTIME_VA module base VA; changes on restart
// Offset::NarakaBladepoint = NOT_LOADED; // status=NOT_LOADED module base unavailable
// Offset::MatrixAddr = UNRESOLVED; // status=UNRESOLVED live camera matrix VA; not a managed field/RVA; depends on camera/native layout
// Offset::Base_WindowBase = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_GlobalTime = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_CharacterManager = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_UserDataManager = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_CharactorSync = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_EntityManager = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_BuffManager = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::m_TransparentTagHandler = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::Base_WindowBase2 = NOT_FOUND; // status=NOT_FOUND module=UnityPlayer_LVB.dll client=1280x720 chain=slot->+0xB8->+0x8->width@0x14,height@0x18 candidates=0
// Offset::m_GlobalTime2 = NOT_FOUND; // status=NOT_FOUND class-not-found: GameBaseObject
// Offset::m_CharacterManager2 = NOT_FOUND; // status=NOT_FOUND class-not-found: CharacterManager
// Offset::m_UserDataManager2 = 0x180; // status=FOUND Class: UserData [TypeInfo RVA] slot=0x20127410180 klass=0x201274308D0
// Offset::m_CharactorSync2 = NOT_FOUND; // status=NOT_FOUND class-not-found: CharactorSync
// Offset::m_EntityManager2 = NOT_FOUND; // status=NOT_FOUND class-not-found: GameEntity
// Offset::m_BuffManager2 = NOT_FOUND; // status=NOT_FOUND class-not-found: BuffManager
// Offset::m_TransparentTagHandler2 = 0x100; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x20127410100 klass=0x20127430560
// Offset::MemoryShockUnresolvedRva = 0x111; // status=CONFIG_CONSTANT application unresolved sentinel; not a discovered game RVA
// Offset::InitActorKitRefer = 0x100; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x20127410100 klass=0x20127430560
// Offset::Il2CppClassNameOffset = 0x10; // status=CALIBRATED runtime layout: cName
// Offset::Il2CppClassParentOffset = 0x58; // status=CALIBRATED runtime layout: cParent
// Offset::Il2CppClassMethodsOffset = 0x98; // status=CALIBRATED runtime layout: cMethods
// Offset::Il2CppClassMethodCountOffset = 0x110; // status=CALIBRATED runtime layout: cMethodCount
// Offset::Il2CppClassStaticFieldsOffset = 0xB8; // status=VERIFIED singleton instance klass back-reference
// Offset::Il2CppMethodPointerOffset = 0x0; // status=CALIBRATED runtime layout: mPtr
// Offset::Il2CppMethodNameOffset = 0x10; // status=CALIBRATED runtime layout: mName
// Offset::Il2CppMethodSlotOffset = 0x40; // status=CALIBRATED runtime layout: mSlot
// Offset::Il2CppMethodParameterCountOffset = 0x42; // status=CALIBRATED runtime layout: mParamCount
// Offset::Il2CppStaticInstanceOffset = NOT_FOUND; // status=NOT_FOUND field-not-found: TransparentTagHandler.s_instance
// Offset::MemoryShockHookContextEntityKitOffset = UNRESOLVED; // status=UNRESOLVED actual hook RCX type unconfirmed; ActorKitInteractableDevice.agent is PlayerEnvironmentColliderAgent, not EntityKit
// Offset::MemoryShockEntityKitActorModelOffset = 0x28; // status=FOUND Class: EntityKit->Object <actorModel>k__BackingField [instance]
// Offset::MemoryShockTransparentTagHandlerActorKitOffset = 0x78; // status=FOUND Class: TransparentTagHandler->Object actorkit [instance]
// Offset::MemoryShockTransparentTagHandlerCharacterManagerOffset = NOT_FOUND; // status=NOT_FOUND field-not-found: TransparentTagHandler.refCharacterMgr
// Offset::MemoryShockCharacterManagerMainPlayerOffset = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::MemoryShockInteractableVtableSlotOffset = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractableDevice
// Offset::MemoryShockCacheDelayBreakTypeOffset = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionBreakData
// Offset::MemoryShockCodecaveHintRva = UNRESOLVED; // status=UNRESOLVED native EXE allocation/search hint; no stable managed semantic
// Offset::MemoryShockBootstrapChain0Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockBootstrapChain1Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockBootstrapChain2Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockLegacyReferRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyReRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyRefRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyJmpRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyJmp2Rva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyJmp3Rva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyJmp4Rva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyLocalActorChain0Offset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyLocalActorChain1Offset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyTestFieldOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyCompareValue = UNRESOLVED; // status=UNRESOLVED legacy comparison immediate; target state semantics not established
// Offset::MemoryShockLegacyMoveFieldOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyRefStateOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyRefObjectOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockLegacyRefCompareOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockFlushIdentityChain0Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockFlushIdentityChain1Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockFlushIdentityChain2Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockFlushIdentityChain3Offset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockFlushAnimPlayableOffset = UNRESOLVED; // status=UNRESOLVED legacy hook context chain; not interchangeable with ActorKit fields
// Offset::MemoryShockVirtualVtableSlotOffset = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::MemoryShockVirtualBreakCacheOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallARva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallBRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallCRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualReturnRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::AnimPlayableCrossFadeInFixedTimeNormal = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly_Super.dll
// Offset::AnimPlayableCrossFadeInFixedTimeSuper = 0x10020; // status=FOUND Class: AnimPlayable->CrossFadeInFixedTime params=7 MethodInfo=0x201274311C0 VA=0x20127420020 slot=0xFFFF
// Offset::FlushState = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKit
// Offset::CrossFadeInFixedTime = 0x10020; // status=FOUND Class: AnimPlayable->CrossFadeInFixedTime params=7 MethodInfo=0x201274311C0 VA=0x20127420020 slot=0xFFFF
// Offset::LocalGenRangeSingleData = NOT_FOUND; // status=NOT_FOUND class-not-found: .RangeAttackEmitter
// Offset::op_Subtraction = SKIPPED_LEGACY; // status=SKIPPED_LEGACY unused old entry; user allowed skipping; operator overload declaring type/signature not specified
// Offset::Call_GetVelocity = SKIPPED_LEGACY; // status=SKIPPED_LEGACY unused old entry; user allowed skipping; no declaring class/signature or current consumer in Offset.h; velocity methods are ambiguous
// Offset::Character.lastSPLFramCount = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.AllAliveCharList = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.ArrayStart = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.fpsFilter = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::FpsFilter.fps = NOT_FOUND; // status=NOT_FOUND class-not-found: .FpsFilter
// Offset::ActorModel.ObjectMsg = 0xE8; // status=FOUND Class: ActorModel->Object objectMsg [instance]
// Offset::ActorModel.EventTracksForLayer = NOT_FOUND; // status=NOT_FOUND class-not-found: .ObjectMessenger
// Offset::ActorModel._curAnimPlayableAgent = NOT_FOUND; // status=NOT_FOUND class-not-found: .ObjectMessenger
// Offset::ActorModel._runtimeLogicLayers = NOT_FOUND; // status=NOT_FOUND field-not-found: AnimPlayable._runtimeLogicLayers
// Offset::ActorModel.Transform = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.FastTransform
// Offset::ActorModel.ActorWeapon = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.actorWeapon
// Offset::ActorModel.WeaponType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorWeapon
// Offset::ActorModel.RuntimePropertyData = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.RuntimePropertyData
// Offset::ActorModel.PropertyData = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.propertyData
// Offset::ActorModel.FestivalGiftBagData = NOT_FOUND; // status=NOT_FOUND field-not-found: UserData.FestivalGiftBagData
// Offset::ActorModel.PettyCashGiftData = NOT_FOUND; // status=NOT_FOUND field-not-found: UserData.PettyCashGiftData
// Offset::ActorModel.battleData = 0x90; // status=FOUND Class: UserData->Object battleData [instance]
// Offset::ActorModel.skillData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::ActorModel.characterSkillDataDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleSkillData
// Offset::ActorModel.fighterEquipData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::ActorModel.cur_weapon_slot = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerEquipInfoData
// Offset::ActorModel.cullingVisible = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.cullingVisible
// Offset::ActorModel.shrtt = NOT_FOUND; // status=NOT_FOUND class-not-found: MobileNetwork.CharactorSync
// Offset::ActorModel.currentTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleLineCurve
// Offset::ActorModel.length = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleLineCurve
// Offset::ActorModel.nowTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleLineCurve
// Offset::ActorModel._globalTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .GameBaseObject
// Offset::ActorModel.CurShareState = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.NameHash = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayableState
// Offset::ActorModel.animClipName = NOT_FOUND; // status=NOT_FOUND class-not-found: Leihuo.EventTrackSet
// Offset::ActorModel.ProPerty.Heroid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.Group = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.WeaponId = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.RobotId = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.BeControlledStatus = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.HeroBattleStatus = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.ComboId = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.CurrentStageType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.PlayerName = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.SkillStatus = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.StereoDodgeType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.LastStateHash = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.CurrentUseItemSkinId = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.LastBeControlledStatus = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.LastBaseStateHash = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.UniqueSkillState = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ProPerty.JiguansanState = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModelPropertyData
// Offset::ActorModel.ActorKit = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.actorKit
// Offset::ActorModel.ActorKitS.BackingField = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.<AniParams>k__BackingField
// Offset::ActorModel.ActorKitS.InteractableDevice = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorKitInteractableDevice
// Offset::ActorModel.ActorKitS.ActorKitInteractive = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorKitInteractive
// Offset::ActorModel.ActorKitS.ActorPhysics = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorPhysics
// Offset::ActorModel.ActorKitS.CurrentCollisionFlags = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitPhysics
// Offset::ActorModel.ActorKitS.HitSimulateData = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorKitHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulatePtr = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorKitComboData
// Offset::ActorModel.ActorKitS.actorKitBreakData = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.actorKitBreakData
// Offset::ActorModel.ActorKitS.AnimPlayableAgent = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKit
// Offset::ActorModel.ActorKitS.HitSimulate.ReactionType = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.endureLevel = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.ReactionParryType = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.ActionHitSimulateData
// Offset::ActorModel.ActorKitS.HitSimulate.XuListate = NOT_FOUND; // status=NOT_FOUND class-not-found: Code.Module_Battle.SkillComboData
// Offset::ActorModel.ActorKitInteractiveS.TargetInteractiveFID = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.CanRescueActor = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.CanStealActor = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.DyingExecuteActorFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.InteractiveType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.IsMainPlayer = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.Status = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.IType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.RescueFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.ActorKitInteractiveS.NowShowInteractiveType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitInteractive
// Offset::ActorModel.RuntimeLogicLayerS.LayerIndex = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.LayerType = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.LayerWeight = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.CurShareState = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.CurStateStartTimeOffset = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.CurDuration = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.CurTimeForLogicState = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.TransitionDuration = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.TransitionPercent = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.CurIsLogicState = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.RuntimeLogicLayerS.TransitionFinish = NOT_FOUND; // status=NOT_FOUND class-not-found: .RuntimeLogicLayer
// Offset::ActorModel.PlayableStateS.AnimatorName = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayableState
// Offset::ActorModel.PlayableStateS.NameHash = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayableState
// Offset::ActorModel.PlayableStateS.TagHash = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayableState
// Offset::ActorModel.EntityKitS.CameraAgent = NOT_FOUND; // status=NOT_FOUND field-not-found: EntityKit.<cameraAgent>k__BackingField
// Offset::ActorModel.CameraAgentS.CameraController = NOT_FOUND; // status=NOT_FOUND class-not-found: .CameraAgent
// Offset::ActorModel.LXCameraControllerS.AdventureRig = NOT_FOUND; // status=NOT_FOUND class-not-found: .LXCameraController
// Offset::ActorModel.AdventureRigS.Euler = NOT_FOUND; // status=NOT_FOUND class-not-found: com.ootii.Cameras.AdventureRig
// Offset::GameBaseObject._Instance = NOT_FOUND; // status=NOT_FOUND class-not-found: .GameBaseObject
// Offset::GameBaseObject._fps = NOT_FOUND; // status=NOT_FOUND class-not-found: .GameBaseObject
// Offset::AvgFPSManager.AvgRenderFPS = NOT_FOUND; // status=NOT_FOUND class-not-found: .AvgFPSManager
// Offset::UserData.InteractiveManager = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::UserData.ClientDodgeConsumeData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::UserData.SpectatorData = NOT_FOUND; // status=NOT_FOUND field-not-found: UserData.spectatorData
// Offset::UserData.isSpectator = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserDataSpectatorData
// Offset::UserData.Skill_ID = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.skillState = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.ClientLeftCdTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.ClientLeftContinueTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.logicData = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::Il2CppList.Items = UNRESOLVED_LAYOUT; // status=UNRESOLVED_LAYOUT Class: List`1->Object _items [instance] rawOffset=0x0; generic definition has no confirmed instance layout
// Offset::Il2CppList.Size = NOT_FOUND; // status=NOT_FOUND field-not-found: List`1._size
// Offset::Il2CppList.FirstItem = 0x20; // status=ABI_LAYOUT x64 Il2CppArray object header: klass,monitor,bounds,max_length
// Offset::ClientDodgeConsumeData.DodgeConsumeIdx = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeData
// Offset::ClientDodgeConsumeData.DodgeConsumePaddingList = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeData
// Offset::ClientDodgeConsumeDataItem.Valid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ClientDodgeConsumeDataItem.CostValue = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ClientDodgeConsumeDataItem.RuntimeProp = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ArticleData.interactiveId = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::ArticleData.status = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::ArticleData.position = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::SoulData.fid = NOT_FOUND; // status=NOT_FOUND field-not-found: ActorModel.fid
// Offset::SoulData.soulItemData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::SoulData.dataDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleSoulItemData
// Offset::SoulData.soulItemDataFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerBattleSoulItemData
// Offset::SoulData._soulItemList = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerBattleSoulItemData
// Offset::SoulData.info = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleSoulItemData
// Offset::SoulData.fid_ = NOT_FOUND; // status=NOT_FOUND class-not-found: .SoulItemData
// Offset::SoulData.soul_item_tid_ = NOT_FOUND; // status=NOT_FOUND class-not-found: .SoulItemData
// Offset::SoulData.status = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::SoulData.position = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::BattleItemData.itemData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::BattleItemData.dataDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleItemInfo
// Offset::BattleItemData.itemDataFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerBattleItemInfo
// Offset::BattleItemData.itemDataList = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerBattleItemInfo
// Offset::BattleItemData.battleItemInfo = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleItemData
// Offset::BattleItemData.item_id_ = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleItemInfo
// Offset::BattleItemData.count_ = NOT_FOUND; // status=NOT_FOUND class-not-found: .BattleItemInfo
// Offset::BattleItemData.listItems = UNRESOLVED_LAYOUT; // status=UNRESOLVED_LAYOUT Class: List`1->Object _items [instance] rawOffset=0x0; generic definition has no confirmed instance layout
// Offset::BattleItemData.listSize = NOT_FOUND; // status=NOT_FOUND field-not-found: List`1._size
// Offset::Buff._Instance = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffManager
// Offset::Buff.recordBuffHandlersDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffManager
// Offset::Buff.BuffDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffInstanceIdList = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffTidList = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.GroupId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.LeftTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.OwnerFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.CasterFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.BuffIId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Inventory: total=221 resolved=20 other/unavailable=11 candidates=0 constants=2 unresolved/skipped=188
// ==========================================

