// ===== Runtime TypeInfo RVA Discovery =====
// Module: GameAssembly.dll
// Module Base: 0x2C713342000
// m_BuffManager = 0x8F80; // decimal: 36736 status=VERIFIED
//   klass=0x2C71334ABA0 slot=0x2C71334AF80 candidates=1 staticFieldsOffset=0xB8 staticFields=0x2C71334AE90 instanceOffset=0x0 instance=0x2C71334AEB0 instanceKlass=0x2C71334ABA0
// m_TransparentTagHandler = 0x8F88; // decimal: 36744 status=VERIFIED
//   klass=0x2C71334AD10 slot=0x2C71334AF88 candidates=1 staticFieldsOffset=0xB8 staticFields=0x2C71334AED0 instanceOffset=0x8 instance=0x2C71334AEF0 instanceKlass=0x2C71334AD10
// ==========================================

// ===== Offset.h Discovery Summary (v7) =====
// Active module: GameAssembly.dll base=0x2C713342000 variant=Normal
// Other variant: NOT_LOADED (no values available in this run)
// Keys preserve Offset.h paths. RVA, instance/static field offset and runtime VA are labelled separately.
// FOUND/CALIBRATED/VERIFIED/DERIVED = resolved; CANDIDATE requires validation; ABI_LAYOUT/CONFIG_CONSTANT are not discoveries.
// UNRESOLVED_LAYOUT preserves raw metadata in evidence; generic instance offsets below the object header are not usable offsets.
// 未加载的另一变体不填值；未找到/待确认/已跳过旧项均明确标注，不沿用历史偏移。
// Offset::UnityPlayer = NOT_LOADED; // status=NOT_LOADED module base unavailable
// Offset::GameAssembly = 0x2C713342000; // status=RUNTIME_VA module base VA; changes on restart
// Offset::NarakaBladepoint = NOT_LOADED; // status=NOT_LOADED module base unavailable
// Offset::MatrixAddr = UNRESOLVED; // status=UNRESOLVED live camera matrix VA; not a managed field/RVA; depends on camera/native layout
// Offset::Base_WindowBase = NOT_LOADED; // status=NOT_LOADED UnityPlayer module unavailable
// Offset::m_GlobalTime = NOT_FOUND; // status=NOT_FOUND class-not-found: GameBaseObject
// Offset::m_CharacterManager = NOT_FOUND; // status=NOT_FOUND class-not-found: CharacterManager
// Offset::m_UserDataManager = NOT_FOUND; // status=NOT_FOUND class-not-found: UserData
// Offset::m_CharactorSync = NOT_FOUND; // status=NOT_FOUND class-not-found: CharactorSync
// Offset::m_EntityManager = NOT_FOUND; // status=NOT_FOUND class-not-found: GameEntity
// Offset::m_BuffManager = 0x8F80; // status=FOUND Class: BuffManager [TypeInfo RVA] slot=0x2C71334AF80 klass=0x2C71334ABA0
// Offset::m_TransparentTagHandler = 0x8F88; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x2C71334AF88 klass=0x2C71334AD10
// Offset::Base_WindowBase2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_GlobalTime2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_CharacterManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_UserDataManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_CharactorSync2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_EntityManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_BuffManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_TransparentTagHandler2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::MemoryShockUnresolvedRva = 0x111; // status=CONFIG_CONSTANT application unresolved sentinel; not a discovered game RVA
// Offset::InitActorKitRefer = 0x8F88; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x2C71334AF88 klass=0x2C71334AD10
// Offset::Il2CppClassNameOffset = 0x10; // status=CALIBRATED runtime layout: cName
// Offset::Il2CppClassParentOffset = 0x58; // status=CALIBRATED runtime layout: cParent
// Offset::Il2CppClassMethodsOffset = 0x98; // status=CALIBRATED runtime layout: cMethods
// Offset::Il2CppClassMethodCountOffset = 0x118; // status=CALIBRATED runtime layout: cMethodCount
// Offset::Il2CppClassStaticFieldsOffset = 0xB8; // status=VERIFIED singleton instance klass back-reference
// Offset::Il2CppMethodPointerOffset = 0x0; // status=CALIBRATED runtime layout: mPtr
// Offset::Il2CppMethodNameOffset = 0x10; // status=CALIBRATED runtime layout: mName
// Offset::Il2CppMethodSlotOffset = 0x48; // status=CALIBRATED runtime layout: mSlot
// Offset::Il2CppMethodParameterCountOffset = 0x4A; // status=CALIBRATED runtime layout: mParamCount
// Offset::Il2CppStaticInstanceOffset = NOT_FOUND; // status=NOT_FOUND field-not-found: TransparentTagHandler.s_instance
// Offset::MemoryShockHookContextEntityKitOffset = UNRESOLVED; // status=UNRESOLVED actual hook RCX type unconfirmed; ActorKitInteractableDevice.agent is PlayerEnvironmentColliderAgent, not EntityKit
// Offset::MemoryShockEntityKitActorModelOffset = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::MemoryShockTransparentTagHandlerActorKitOffset = NOT_FOUND; // status=NOT_FOUND field-not-found: TransparentTagHandler.actorkit
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
// Offset::MemoryShockVirtualVtableSlotOffset = UNRESOLVED; // status=UNRESOLVED historical probe +0x190 does not point to module executable code; historical class byte offset; intended virtual method not specified
// Offset::MemoryShockVirtualBreakCacheOffset = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallARva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallBRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualCallCRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::MemoryShockVirtualReturnRva = UNRESOLVED; // status=UNRESOLVED semantic mapping unavailable; needs current native control-flow evidence
// Offset::AnimPlayableCrossFadeInFixedTimeNormal = NOT_FOUND; // status=NOT_FOUND class-not-found: .AnimPlayable
// Offset::AnimPlayableCrossFadeInFixedTimeSuper = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::FlushState = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKit
// Offset::CrossFadeInFixedTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .AnimPlayable
// Offset::LocalGenRangeSingleData = NOT_FOUND; // status=NOT_FOUND class-not-found: .RangeAttackEmitter
// Offset::op_Subtraction = SKIPPED_LEGACY; // status=SKIPPED_LEGACY unused old entry; user allowed skipping; operator overload declaring type/signature not specified
// Offset::Call_GetVelocity = SKIPPED_LEGACY; // status=SKIPPED_LEGACY unused old entry; user allowed skipping; no declaring class/signature or current consumer in Offset.h; velocity methods are ambiguous
// Offset::Character.lastSPLFramCount = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.AllAliveCharList = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.ArrayStart = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::Character.fpsFilter = NOT_FOUND; // status=NOT_FOUND class-not-found: .CharacterManager
// Offset::FpsFilter.fps = NOT_FOUND; // status=NOT_FOUND class-not-found: .FpsFilter
// Offset::ActorModel.ObjectMsg = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.EventTracksForLayer = NOT_FOUND; // status=NOT_FOUND class-not-found: .ObjectMessenger
// Offset::ActorModel._curAnimPlayableAgent = NOT_FOUND; // status=NOT_FOUND class-not-found: .ObjectMessenger
// Offset::ActorModel._runtimeLogicLayers = NOT_FOUND; // status=NOT_FOUND class-not-found: .AnimPlayable
// Offset::ActorModel.Transform = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.ActorWeapon = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.WeaponType = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorWeapon
// Offset::ActorModel.RuntimePropertyData = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.PropertyData = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.FestivalGiftBagData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserData
// Offset::ActorModel.PettyCashGiftData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserData
// Offset::ActorModel.battleData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserData
// Offset::ActorModel.skillData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::ActorModel.characterSkillDataDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleSkillData
// Offset::ActorModel.fighterEquipData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::ActorModel.cur_weapon_slot = NOT_FOUND; // status=NOT_FOUND class-not-found: .PlayerEquipInfoData
// Offset::ActorModel.cullingVisible = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
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
// Offset::ActorModel.ActorKit = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
// Offset::ActorModel.ActorKitS.BackingField = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.InteractableDevice = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.ActorKitInteractive = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.ActorPhysics = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.CurrentCollisionFlags = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorKitPhysics
// Offset::ActorModel.ActorKitS.HitSimulateData = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.HitSimulatePtr = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.ActorKitS.actorKitBreakData = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
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
// Offset::ActorModel.EntityKitS.CameraAgent = NOT_FOUND; // status=NOT_FOUND class-not-found: .EntityKit
// Offset::ActorModel.CameraAgentS.CameraController = NOT_FOUND; // status=NOT_FOUND class-not-found: .CameraAgent
// Offset::ActorModel.LXCameraControllerS.AdventureRig = NOT_FOUND; // status=NOT_FOUND class-not-found: .LXCameraController
// Offset::ActorModel.AdventureRigS.Euler = NOT_FOUND; // status=NOT_FOUND class-not-found: com.ootii.Cameras.AdventureRig
// Offset::GameBaseObject._Instance = NOT_FOUND; // status=NOT_FOUND class-not-found: .GameBaseObject
// Offset::GameBaseObject._fps = NOT_FOUND; // status=NOT_FOUND class-not-found: .GameBaseObject
// Offset::AvgFPSManager.AvgRenderFPS = NOT_FOUND; // status=NOT_FOUND class-not-found: .AvgFPSManager
// Offset::UserData.InteractiveManager = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::UserData.ClientDodgeConsumeData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserBattleData
// Offset::UserData.SpectatorData = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserData
// Offset::UserData.isSpectator = NOT_FOUND; // status=NOT_FOUND class-not-found: .UserDataSpectatorData
// Offset::UserData.Skill_ID = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.skillState = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.ClientLeftCdTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.ClientLeftContinueTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::UserData.logicData = NOT_FOUND; // status=NOT_FOUND class-not-found: .SkillData
// Offset::Il2CppList.Items = NOT_FOUND; // status=NOT_FOUND class-not-found: System.Collections.Generic.List`1
// Offset::Il2CppList.Size = NOT_FOUND; // status=NOT_FOUND class-not-found: System.Collections.Generic.List`1
// Offset::Il2CppList.FirstItem = 0x20; // status=ABI_LAYOUT x64 Il2CppArray object header: klass,monitor,bounds,max_length
// Offset::ClientDodgeConsumeData.DodgeConsumeIdx = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeData
// Offset::ClientDodgeConsumeData.DodgeConsumePaddingList = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeData
// Offset::ClientDodgeConsumeDataItem.Valid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ClientDodgeConsumeDataItem.CostValue = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ClientDodgeConsumeDataItem.RuntimeProp = NOT_FOUND; // status=NOT_FOUND class-not-found: .ClientDodgeConsumeDataItem
// Offset::ArticleData.interactiveId = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::ArticleData.status = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::ArticleData.position = NOT_FOUND; // status=NOT_FOUND class-not-found: .InteractiveData
// Offset::SoulData.fid = NOT_FOUND; // status=NOT_FOUND class-not-found: .ActorModel
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
// Offset::BattleItemData.listItems = NOT_FOUND; // status=NOT_FOUND class-not-found: System.Collections.Generic.List`1
// Offset::BattleItemData.listSize = NOT_FOUND; // status=NOT_FOUND class-not-found: System.Collections.Generic.List`1
// Offset::Buff._Instance = NOT_FOUND; // status=NOT_FOUND field-not-found: BuffManager._instance
// Offset::Buff.recordBuffHandlersDict = NOT_FOUND; // status=NOT_FOUND field-not-found: BuffManager._recordBuffHandlersDict
// Offset::Buff.BuffDict = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffInstanceIdList = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffTidList = NOT_FOUND; // status=NOT_FOUND class-not-found: .BuffHandler
// Offset::Buff.BuffId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.GroupId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.LeftTime = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.OwnerFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.CasterFid = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Offset::Buff.BuffIId = NOT_FOUND; // status=NOT_FOUND class-not-found: .Buff
// Inventory: total=221 resolved=13 other/unavailable=12 candidates=0 constants=2 unresolved/skipped=194
// ==========================================

// Image 0: Test.dll - 256

// Class: <Module> 
// Namespace: 
// Instance: 0x2c71334aa30
internal class <Module> // TypeDefIndex: 100
{}

// Class: Sample 
// Namespace: Test.Game
// Instance: 0x2c713349000
// RVA: 0x8f78
// RVA: 36728
[Serializable]
public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101
{
	// Fields
	public Int32 m_count; // offset:0x10
	private static readonly Boolean s_flag; // offset:0x0
	public const String Name; // size:0x8

	// Properties
	public Int32 Count { get; set; }

	// Methods:6
	// RVA: 0x1010 VA: 0x2c713343010
	public Int32 get_Count() { }
	// RVA: 0x1020 VA: 0x2c713343020
	public Void set_Count(Int32 value) { }
	// RVA: 0x1000 VA: 0x2c713343000
	public Void .ctor() { }
	// RVA: 0x1030 VA: 0x2c713343030
	public Void DoWork(Int32 a, ref Single b, out Int32 c) { }
	// RVA: 0x1040 VA: 0x2c713343040 Slot: 4
	public virtual Void Virtual() { }
	// RVA: 0x VA: 0x0
	public Void NoPointer() { }
}

// Class: MyEnum 
// Namespace: Test.Game
// Instance: 0x2c713349610
public enum MyEnum // TypeDefIndex: 102
{
	// Fields
	public Int32 value__; // offset:0x10
	public const MyEnum ValueA; // size:0x4
}

// Class: MyStruct 
// Namespace: Test.Game
// Instance: 0x2c713349850
public struct MyStruct : IFoo, IBar // TypeDefIndex: 103
{
	// Fields
	public Int32 x; // offset:0x10
}

// Class: Arrays 
// Namespace: Test.Game
// Instance: 0x2c713349a40
public class Arrays // TypeDefIndex: 104
{
	// Fields
	public Byte[] data; // offset:0x18
	public Int32[,] grid; // offset:0x20
	public BaseClass boxed; // offset:0x28
	public BaseClass indexed; // offset:0x30
}

// Class: Object 
// Namespace: System
// Instance: 0x2c7133400c0
public class Object // TypeDefIndex: 105
{}

// Class: BaseClass 
// Namespace: Test.Game
// Instance: 0x2c713340230
public class BaseClass // TypeDefIndex: 106
{}

// Class: IFoo 
// Namespace: Test.Game
// Instance: 0x2c7133403b0
public interface IFoo // TypeDefIndex: 107
{}

// Class: Extra1 
// Namespace: Test.Game
// Instance: 0x2c713349da0
public class Extra1 // TypeDefIndex: 108
{
	// Fields
	public Int32 f1; // offset:0x10

	// Properties
	public Void P1 { get; }

	// Methods:2
	// RVA: 0x1100 VA: 0x2c713343100
	public Void M1() { }
	// RVA: 0x1110 VA: 0x2c713343110
	public Void M2() { }
}

// Class: Extra2 
// Namespace: Test.Game
// Instance: 0x2c71334a0b0
public class Extra2 // TypeDefIndex: 109
{
	// Fields
	public Int32 f1; // offset:0x10
	public Int32 f2; // offset:0x14

	// Properties
	public Void P1 { get; }
	public Void P2 { get; }

	// Methods:3
	// RVA: 0x1200 VA: 0x2c713343200
	public Void M1() { }
	// RVA: 0x1210 VA: 0x2c713343210
	public Void M2() { }
	// RVA: 0x1220 VA: 0x2c713343220
	public Void M3() { }
}

// Class: Extra3 
// Namespace: Test.Game
// Instance: 0x2c71334a4e0
public class Extra3 // TypeDefIndex: 10a
{
	// Fields
	public Int32 f1; // offset:0x10
	public Int32 f2; // offset:0x14
	public Int32 f3; // offset:0x18

	// Properties
	public Void P1 { get; }
	public Void P2 { get; }
	public Void P3 { get; }

	// Methods:4
	// RVA: 0x1300 VA: 0x2c713343300
	public Void M1() { }
	// RVA: 0x1310 VA: 0x2c713343310
	public Void M2() { }
	// RVA: 0x1320 VA: 0x2c713343320
	public Void M3() { }
	// RVA: 0x1330 VA: 0x2c713343330
	public Void M4() { }
}

// Class: BuffManager 
// Namespace: 
// Instance: 0x2c71334aba0
// RVA: 0x8f80
// RVA: 36736
public class BuffManager // TypeDefIndex: 10b
{}

// Class: TransparentTagHandler 
// Namespace: 
// Instance: 0x2c71334ad10
// RVA: 0x8f88
// RVA: 36744
public class TransparentTagHandler // TypeDefIndex: 10c
{}

