// ===== Runtime TypeInfo RVA Discovery =====
// Module: GameAssembly.dll
// Module Base: 0x1974E5E2000
// m_BuffManager = 0x8D50; // decimal: 36176 status=VERIFIED
//   klass=0x1974E5EA9B0 slot=0x1974E5EAD50 candidates=1 staticFieldsOffset=0xB8 staticFields=0x1974E5EAC60 instanceOffset=0x0 instance=0x1974E5EAC80 instanceKlass=0x1974E5EA9B0
// m_TransparentTagHandler = 0x8D58; // decimal: 36184 status=VERIFIED
//   klass=0x1974E5EAB00 slot=0x1974E5EAD58 candidates=1 staticFieldsOffset=0xB8 staticFields=0x1974E5EACA0 instanceOffset=0x8 instance=0x1974E5EACC0 instanceKlass=0x1974E5EAB00
// ==========================================

// ===== Offset.h Discovery Summary (v7) =====
// Active module: GameAssembly.dll base=0x1974E5E2000 variant=Normal
// Other variant: NOT_LOADED (no values available in this run)
// Keys preserve Offset.h paths. RVA, instance/static field offset and runtime VA are labelled separately.
// FOUND/CALIBRATED/VERIFIED/DERIVED = resolved; CANDIDATE requires validation; ABI_LAYOUT/CONFIG_CONSTANT are not discoveries.
// UNRESOLVED_LAYOUT preserves raw metadata in evidence; generic instance offsets below the object header are not usable offsets.
// 未加载的另一变体不填值；未找到/待确认/已跳过旧项均明确标注，不沿用历史偏移。
// Offset::UnityPlayer = NOT_LOADED; // status=NOT_LOADED module base unavailable
// Offset::GameAssembly = 0x1974E5E2000; // status=RUNTIME_VA module base VA; changes on restart
// Offset::NarakaBladepoint = NOT_LOADED; // status=NOT_LOADED module base unavailable
// Offset::MatrixAddr = UNRESOLVED; // status=UNRESOLVED live camera matrix VA; not a managed field/RVA; depends on camera/native layout
// Offset::Base_WindowBase = NOT_LOADED; // status=NOT_LOADED UnityPlayer module unavailable
// Offset::m_GlobalTime = NOT_FOUND; // status=NOT_FOUND class-not-found: GameBaseObject
// Offset::m_CharacterManager = NOT_FOUND; // status=NOT_FOUND class-not-found: CharacterManager
// Offset::m_UserDataManager = NOT_FOUND; // status=NOT_FOUND class-not-found: UserData
// Offset::m_CharactorSync = NOT_FOUND; // status=NOT_FOUND class-not-found: CharactorSync
// Offset::m_EntityManager = NOT_FOUND; // status=NOT_FOUND class-not-found: GameEntity
// Offset::m_BuffManager = 0x8D50; // status=FOUND Class: BuffManager [TypeInfo RVA] slot=0x1974E5EAD50 klass=0x1974E5EA9B0
// Offset::m_TransparentTagHandler = 0x8D58; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x1974E5EAD58 klass=0x1974E5EAB00
// Offset::Base_WindowBase2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_GlobalTime2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_CharacterManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_UserDataManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_CharactorSync2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_EntityManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_BuffManager2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::m_TransparentTagHandler2 = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant; this dump covers only GameAssembly.dll
// Offset::MemoryShockUnresolvedRva = 0x111; // status=CONFIG_CONSTANT application unresolved sentinel; not a discovered game RVA
// Offset::InitActorKitRefer = 0x8D58; // status=FOUND Class: TransparentTagHandler [TypeInfo RVA] slot=0x1974E5EAD58 klass=0x1974E5EAB00
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
// Instance: 0x1974e5ea860
internal class <Module> // TypeDefIndex: 100
{}

// Class: Sample 
// Namespace: Test.Game
// Instance: 0x1974e5e9000
// RVA: 0x8d48
// RVA: 36168
public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101
{
	// Properties
	public Int32 Count { get; set; }
}

