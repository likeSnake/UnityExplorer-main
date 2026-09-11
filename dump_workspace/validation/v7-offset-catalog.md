# v7 Offset.h 映射核对

这是对既有 dump 的离线映射审计，不是 v7 DLL 实机运行结果。

Source: F:\gua\UnityExplorer-main\tools\..\dump_workspace\inject_test\out_super\dump.cs
Inventory: 221 / 221, exact coverage.

大小写严格匹配；同名类按字段唯一命中消歧；CrossFade 校验七参数类型；另一变体的方法不作比较。

| Offset.h 变量 | 当前 dump 类/成员 | dump 值 | Offset.h 原值 | 对比 |
|---|---|---|---|---|
| m_GlobalTime | .GameBaseObject ->  | OTHER_VARIANT | 0xE644808 | unresolved |
| m_CharacterManager | .CharacterManager ->  | OTHER_VARIANT | 0xE642388 | unresolved |
| m_UserDataManager | .UserData -> battleData | OTHER_VARIANT | 0xE644068 | unresolved |
| m_CharactorSync | MobileNetwork.CharactorSync ->  | OTHER_VARIANT | 0xE606490 | unresolved |
| m_EntityManager | GameLogic.Common.GameEntity ->  | OTHER_VARIANT | 0xE643020 | unresolved |
| m_BuffManager | .BuffManager ->  | OTHER_VARIANT | 0xE641868 | unresolved |
| m_TransparentTagHandler | .TransparentTagHandler ->  | OTHER_VARIANT | 0xE88A328 | unresolved |
| m_GlobalTime2 | .GameBaseObject -> TypeInfo RVA | 0x3755D18 | 0x3755D18 | SAME |
| m_CharacterManager2 | .CharacterManager -> TypeInfo RVA | 0x374C2B8 | 0x374C2B8 | SAME |
| m_UserDataManager2 | .UserData -> TypeInfo RVA | 0x3772EA8 | 0x3772EA8 | SAME |
| m_CharactorSync2 | MobileNetwork.CharactorSync -> TypeInfo RVA | 0x374C368 | 0x374C368 | SAME |
| m_EntityManager2 | GameLogic.Common.GameEntity -> TypeInfo RVA | 0x3755DD0 | 0x3755DD0 | SAME |
| m_BuffManager2 | .BuffManager -> TypeInfo RVA | 0x374AE20 | 0x374AE20 | SAME |
| m_TransparentTagHandler2 | .TransparentTagHandler -> TypeInfo RVA | 0x376ABF8 | 0x376ABF8 | SAME |
| InitActorKitRefer | .TransparentTagHandler -> TypeInfo RVA | 0x376ABF8 | MemoryShockUnresolvedRva | SYMBOLIC_INITIALIZER |
| Il2CppStaticInstanceOffset | .TransparentTagHandler -> s_instance | 0x8 | 0x8 | SAME |
| MemoryShockEntityKitActorModelOffset | .EntityKit -> <actorModel>k__BackingField | 0x18 | 0x18 | SAME |
| MemoryShockTransparentTagHandlerActorKitOffset | .TransparentTagHandler -> actorkit | 0x70 | 0x70 | SAME |
| MemoryShockTransparentTagHandlerCharacterManagerOffset | .TransparentTagHandler -> refCharacterMgr | 0xD8 | 0xD8 | SAME |
| MemoryShockCharacterManagerMainPlayerOffset | .CharacterManager -> mainPlayerActorModel | 0x18 | 0x18 | SAME |
| MemoryShockCacheDelayBreakTypeOffset | Code.Module_Battle.ActionBreakData -> CacheDelayBreakType | 0x58 | 0x58 | SAME |
| AnimPlayableCrossFadeInFixedTimeNormal | .AnimPlayable -> CrossFadeInFixedTime | OTHER_VARIANT | 0x5811350 | unresolved |
| AnimPlayableCrossFadeInFixedTimeSuper | .AnimPlayable -> CrossFadeInFixedTime | 0x797D760 | 0x797D760 | SAME |
| FlushState | .ActorKit -> FlushState | NOT_FOUND | 0x1D7BE0 | unresolved |
| CrossFadeInFixedTime | .AnimPlayable -> CrossFadeInFixedTime | 0x797D760 | AnimPlayableCrossFadeInFixedTimeNormal | SYMBOLIC_INITIALIZER |
| LocalGenRangeSingleData | .RangeAttackEmitter -> LocalGenRangeSingleData | 0x5FECD50 | 0x111 | CHANGED |
| Character.lastSPLFramCount | .CharacterManager -> lastSPLFramCount | 0xB8 | 0xB8 | SAME |
| Character.AllAliveCharList | .CharacterManager -> fid2AliveCharacter | 0x50 | 0x50 | SAME |
| Character.fpsFilter | .CharacterManager -> fpsFilter | 0x40 | 0x40 | SAME |
| FpsFilter.fps | .FpsFilter -> fps | 0x2C | 0x2C | SAME |
| ActorModel.ObjectMsg | .ActorModel -> objectMsg | 0xD8 | 0xD8 | SAME |
| ActorModel.EventTracksForLayer | .ObjectMessenger -> EventTracksForLayer | 0x30 | 0x30 | SAME |
| ActorModel._curAnimPlayableAgent | .ObjectMessenger -> _curAnimPlayableAgent | 0x70 | 0x70 | SAME |
| ActorModel._runtimeLogicLayers | .AnimPlayable -> _runtimeLogicLayers | 0x48 | 0x48 | SAME |
| ActorModel.Transform | .ActorModel -> FastTransform | 0x128 | 0x128 | SAME |
| ActorModel.ActorWeapon | .ActorModel -> actorWeapon | 0xF8 | 0xF8 | SAME |
| ActorModel.WeaponType | .ActorWeapon -> WeaponType | 0x68 | 0x68 | SAME |
| ActorModel.RuntimePropertyData | .ActorModel -> RuntimePropertyData | 0xA8 | 0xA8 | SAME |
| ActorModel.PropertyData | .ActorModel -> propertyData | 0x358 | 0x358 | SAME |
| ActorModel.FestivalGiftBagData | .UserData -> FestivalGiftBagData | 0xC8 | 0xC8 | SAME |
| ActorModel.PettyCashGiftData | .UserData -> PettyCashGiftData | 0xC0 | 0xC0 | SAME |
| ActorModel.battleData | .UserData -> battleData | 0x38 | 0x38 | SAME |
| ActorModel.skillData | .UserBattleData -> skillData | 0x118 | 0x118 | SAME |
| ActorModel.characterSkillDataDict | .UserBattleSkillData -> characterSkillDataDict | 0x10 | 0x10 | SAME |
| ActorModel.fighterEquipData | .UserBattleData -> fighterEquipData | 0x70 | 0x70 | SAME |
| ActorModel.cur_weapon_slot | .PlayerEquipInfoData -> cur_weapon_slot | 0x20 | 0x20 | SAME |
| ActorModel.cullingVisible | .ActorModel -> cullingVisible | 0x6C | 0x6C | SAME |
| ActorModel.shrtt | MobileNetwork.CharactorSync -> shrtt | 0x68 | 0x68 | SAME |
| ActorModel.currentTime | .BattleLineCurve -> currentTime | 0x58 | 0x58 | SAME |
| ActorModel.length | .BattleLineCurve -> length | 0x54 | 0x54 | SAME |
| ActorModel.nowTime | .BattleLineCurve -> nowTime | 0x60 | 0x60 | SAME |
| ActorModel._globalTime | .GameBaseObject -> _globalTime | 0x28 | 0x28 | SAME |
| ActorModel.CurShareState | .RuntimeLogicLayer -> CurShareState | 0x48 | 0x48 | SAME |
| ActorModel.NameHash | .PlayableState -> NameHash | 0x18 | 0x18 | SAME |
| ActorModel.animClipName | Leihuo.EventTrackSet -> animClipName | 0x18 | 0x18 | SAME |
| ActorModel.ProPerty.Heroid | .ActorModelPropertyData -> _HeroTid | 0x108 | 0x108 | SAME |
| ActorModel.ProPerty.Group | .ActorModelPropertyData -> _Group | 0x118 | 0x118 | SAME |
| ActorModel.ProPerty.WeaponId | .ActorModelPropertyData -> _WeaponId | 0x128 | 0x128 | SAME |
| ActorModel.ProPerty.RobotId | .ActorModelPropertyData -> _RobotId | 0x130 | 0x130 | SAME |
| ActorModel.ProPerty.BeControlledStatus | .ActorModelPropertyData -> _BeControlledStatus | 0x140 | 0x140 | SAME |
| ActorModel.ProPerty.HeroBattleStatus | .ActorModelPropertyData -> _HeroBattleStatus | 0x14C | 0x14C | SAME |
| ActorModel.ProPerty.ComboId | .ActorModelPropertyData -> _ComboId | 0x158 | 0x158 | SAME |
| ActorModel.ProPerty.CurrentStageType | .ActorModelPropertyData -> _CurrentStageType | 0x160 | 0x160 | SAME |
| ActorModel.ProPerty.PlayerName | .ActorModelPropertyData -> _Name | 0x200 | 0x200 | SAME |
| ActorModel.ProPerty.SkillStatus | .ActorModelPropertyData -> _SkillCastStatus | 0x198 | 0x198 | SAME |
| ActorModel.ProPerty.StereoDodgeType | .ActorModelPropertyData -> _StereoDodgeType | 0x1A0 | 0x1A0 | SAME |
| ActorModel.ProPerty.LastStateHash | .ActorModelPropertyData -> _LastStateHash | 0x1AC | 0x1AC | SAME |
| ActorModel.ProPerty.CurrentUseItemSkinId | .ActorModelPropertyData -> _CurrentUseItemSkinId | 0x1B4 | 0x1B4 | SAME |
| ActorModel.ProPerty.LastBeControlledStatus | .ActorModelPropertyData -> _LastBeControlledStatus | 0x1C8 | 0x1C8 | SAME |
| ActorModel.ProPerty.LastBaseStateHash | .ActorModelPropertyData -> _LastBaseStateHash | 0x1DC | 0x1DC | SAME |
| ActorModel.ProPerty.UniqueSkillState | .ActorModelPropertyData -> _UniqueSkillState | 0x1F4 | 0x1F4 | SAME |
| ActorModel.ProPerty.JiguansanState | .ActorModelPropertyData -> _JiguansanState | 0x1F8 | 0x1F8 | SAME |
| ActorModel.ActorKit | .ActorModel -> actorKit | 0xC8 | 0xC8 | SAME |
| ActorModel.ActorKitS.BackingField | .EntityKit -> <AniParams>k__BackingField | 0x38 | 0x38 | SAME |
| ActorModel.ActorKitS.InteractableDevice | .EntityKit -> actorKitInteractableDevice | 0x90 | 0x90 | SAME |
| ActorModel.ActorKitS.ActorKitInteractive | .EntityKit -> actorKitInteractive | 0xD0 | 0xD0 | SAME |
| ActorModel.ActorKitS.ActorPhysics | .EntityKit -> actorPhysics | 0xE0 | 0xE0 | SAME |
| ActorModel.ActorKitS.CurrentCollisionFlags | .ActorKitPhysics -> _currentCollisionFlags | 0xF4 | 0xF4 | SAME |
| ActorModel.ActorKitS.HitSimulateData | .EntityKit -> actorKitHitSimulateData | 0x100 | 0x100 | SAME |
| ActorModel.ActorKitS.HitSimulatePtr | .EntityKit -> actorKitComboData | 0xF0 | 0xF0 | SAME |
| ActorModel.ActorKitS.actorKitBreakData | .EntityKit -> actorKitBreakData | 0x108 | 0x108 | SAME |
| ActorModel.ActorKitS.AnimPlayableAgent | .ActorKit -> <AnimPlayableAgent>k__BackingField | 0x2E8 | 0x2E8 | SAME |
| ActorModel.ActorKitS.HitSimulate.ReactionType | Code.Module_Battle.ActionHitSimulateData -> ReactionType | 0x54 | 0x54 | SAME |
| ActorModel.ActorKitS.HitSimulate.RangeReactionType | Code.Module_Battle.ActionHitSimulateData -> RangeReactionType | 0x58 | 0x58 | SAME |
| ActorModel.ActorKitS.HitSimulate.ReactionExtraType | Code.Module_Battle.ActionHitSimulateData -> ReactionExtraType | 0x5C | 0x5C | SAME |
| ActorModel.ActorKitS.HitSimulate.endureLevel | Code.Module_Battle.ActionHitSimulateData -> EndureLevel | 0xD0 | 0xD0 | SAME |
| ActorModel.ActorKitS.HitSimulate.ActionDuration | Code.Module_Battle.ActionHitSimulateData -> ActionDuration | 0x2C | 0x2C | SAME |
| ActorModel.ActorKitS.HitSimulate.ReactionParryType | Code.Module_Battle.ActionHitSimulateData -> ParryGroup | 0xB4 | 0xB4 | SAME |
| ActorModel.ActorKitS.HitSimulate.XuListate | Code.Module_Battle.SkillComboData -> ChargeType | 0x70 | 0x70 | SAME |
| ActorModel.ActorKitInteractiveS.TargetInteractiveFID | .ActorKitInteractive -> m_targetInteractiveFID | 0x10 | 0x10 | SAME |
| ActorModel.ActorKitInteractiveS.CanRescueActor | .ActorKitInteractive -> canRescueActor | 0x28 | 0x28 | SAME |
| ActorModel.ActorKitInteractiveS.CanStealActor | .ActorKitInteractive -> canStealActor | 0x30 | 0x30 | SAME |
| ActorModel.ActorKitInteractiveS.DyingExecuteActorFid | .ActorKitInteractive -> DyingExecuteActorFid | 0x38 | 0x38 | SAME |
| ActorModel.ActorKitInteractiveS.InteractiveType | .ActorKitInteractive -> m_interactiveType | 0x58 | 0x58 | SAME |
| ActorModel.ActorKitInteractiveS.IsMainPlayer | .ActorKitInteractive -> isMainPlayer | 0x5C | 0x5C | SAME |
| ActorModel.ActorKitInteractiveS.Status | .ActorKitInteractive -> m_status | 0x60 | 0x60 | SAME |
| ActorModel.ActorKitInteractiveS.IType | .ActorKitInteractive -> m_itype | 0x64 | 0x64 | SAME |
| ActorModel.ActorKitInteractiveS.RescueFid | .ActorKitInteractive -> rescue_fid | 0x68 | 0x68 | SAME |
| ActorModel.ActorKitInteractiveS.NowShowInteractiveType | .ActorKitInteractive -> now_showinteractiveType | 0x70 | 0x70 | SAME |
| ActorModel.RuntimeLogicLayerS.LayerIndex | .RuntimeLogicLayer -> layerIndex | 0x10 | 0x10 | SAME |
| ActorModel.RuntimeLogicLayerS.LayerType | .RuntimeLogicLayer -> LayerType | 0x14 | 0x14 | SAME |
| ActorModel.RuntimeLogicLayerS.LayerWeight | .RuntimeLogicLayer -> _layerWeight | 0x18 | 0x18 | SAME |
| ActorModel.RuntimeLogicLayerS.CurShareState | .RuntimeLogicLayer -> CurShareState | 0x48 | 0x48 | SAME |
| ActorModel.RuntimeLogicLayerS.CurStateStartTimeOffset | .RuntimeLogicLayer -> CurStateStartTimeOffset | 0x50 | 0x50 | SAME |
| ActorModel.RuntimeLogicLayerS.CurDuration | .RuntimeLogicLayer -> CurDuration | 0x54 | 0x54 | SAME |
| ActorModel.RuntimeLogicLayerS.CurTimeForLogicState | .RuntimeLogicLayer -> _curTimeForLogicState | 0x60 | 0x60 | SAME |
| ActorModel.RuntimeLogicLayerS.TransitionDuration | .RuntimeLogicLayer -> TransitionDuration | 0x74 | 0x74 | SAME |
| ActorModel.RuntimeLogicLayerS.TransitionPercent | .RuntimeLogicLayer -> _transitionPercent | 0x78 | 0x78 | SAME |
| ActorModel.RuntimeLogicLayerS.CurIsLogicState | .RuntimeLogicLayer -> CurIsLogicState | 0x7C | 0x7C | SAME |
| ActorModel.RuntimeLogicLayerS.TransitionFinish | .RuntimeLogicLayer -> transitionFinish | 0x7E | 0x7E | SAME |
| ActorModel.PlayableStateS.AnimatorName | .PlayableState -> animatorName | 0x10 | 0x10 | SAME |
| ActorModel.PlayableStateS.NameHash | .PlayableState -> NameHash | 0x18 | 0x18 | SAME |
| ActorModel.PlayableStateS.TagHash | .PlayableState -> TagHash | 0x1C | 0x1C | SAME |
| ActorModel.EntityKitS.CameraAgent | .EntityKit -> <cameraAgent>k__BackingField | 0x30 | 0x30 | SAME |
| ActorModel.CameraAgentS.CameraController | .CameraAgent -> CameraController | 0x88 | 0x88 | SAME |
| ActorModel.LXCameraControllerS.AdventureRig | .LXCameraController -> <AdventureRig>k__BackingField | 0x58 | 0x58 | SAME |
| ActorModel.AdventureRigS.Euler | com.ootii.Cameras.AdventureRig -> _Euler | 0x250 | 0x250 | SAME |
| GameBaseObject._Instance | .GameBaseObject -> _Instance | 0x8 | 0x8 | SAME |
| GameBaseObject._fps | .GameBaseObject -> _fps | 0x18 | 0x18 | SAME |
| AvgFPSManager.AvgRenderFPS | .AvgFPSManager -> AvgRenderFPS | 0x14 | 0x14 | SAME |
| UserData.InteractiveManager | .UserBattleData -> interactiveManager | 0x240 | 0x240 | SAME |
| UserData.ClientDodgeConsumeData | .UserBattleData -> clientDodgeConsumeData | 0x228 | 0x228 | SAME |
| UserData.SpectatorData | .UserData -> spectatorData | 0x90 | 0x90 | SAME |
| UserData.isSpectator | .UserDataSpectatorData -> isSpectator | 0x48 | 0x48 | SAME |
| UserData.Skill_ID | .SkillData -> fid | 0x10 | 0x10 | SAME |
| UserData.skillState | .SkillData -> skillState | 0x78 | 0x78 | SAME |
| UserData.ClientLeftCdTime | .SkillData -> ClientLeftCdTime | 0x40 | 0x40 | SAME |
| UserData.ClientLeftContinueTime | .SkillData -> ClientLeftContinueTime | 0x48 | 0x48 | SAME |
| UserData.logicData | .SkillData -> logicData | 0x68 | 0x68 | SAME |
| Il2CppList.Items | System.Collections.Generic.List`1 -> _items | 0x0 | 0x10 | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| Il2CppList.Size | System.Collections.Generic.List`1 -> _size | 0x0 | 0x18 | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| ClientDodgeConsumeData.DodgeConsumeIdx | .ClientDodgeConsumeData -> dodge_consume_idx | 0x10 | 0x10 | SAME |
| ClientDodgeConsumeData.DodgeConsumePaddingList | .ClientDodgeConsumeData -> dodgeConsumePaddingList | 0x18 | 0x18 | SAME |
| ClientDodgeConsumeDataItem.Valid | .ClientDodgeConsumeDataItem -> valid | 0x10 | 0x10 | SAME |
| ClientDodgeConsumeDataItem.CostValue | .ClientDodgeConsumeDataItem -> cost_value | 0x14 | 0x14 | SAME |
| ClientDodgeConsumeDataItem.RuntimeProp | .ClientDodgeConsumeDataItem -> rt_prop | 0x18 | 0x18 | SAME |
| ArticleData.interactiveId | .InteractiveData -> interactiveId | 0x14 | 0x14 | SAME |
| ArticleData.status | .InteractiveData -> status | 0x34 | 0x34 | SAME |
| ArticleData.position | .InteractiveData -> position | 0x18 | 0x18 | SAME |
| SoulData.fid | .ActorModel -> fid | 0x350 | 0x350 | SAME |
| SoulData.soulItemData | .UserBattleData -> soulItemData | 0x98 | 0x98 | SAME |
| SoulData.dataDict | .UserBattleSoulItemData -> dataDict | 0x10 | 0x10 | SAME |
| SoulData.soulItemDataFid | .PlayerBattleSoulItemData -> fid | 0x10 | 0x10 | SAME |
| SoulData._soulItemList | .PlayerBattleSoulItemData -> _soulItemList | 0x40 | 0x40 | SAME |
| SoulData.info | .BattleSoulItemData -> info | 0x10 | 0x10 | SAME |
| SoulData.fid_ | .SoulItemData -> fid_ | 0x18 | 0x18 | SAME |
| SoulData.soul_item_tid_ | .SoulItemData -> soul_item_tid_ | 0x1C | 0x1C | SAME |
| SoulData.status | .InteractiveData -> status | 0x34 | 0x34 | SAME |
| SoulData.position | .InteractiveData -> position | 0x18 | 0x18 | SAME |
| BattleItemData.itemData | .UserBattleData -> itemData | 0x90 | 0x90 | SAME |
| BattleItemData.dataDict | .UserBattleItemInfo -> dataDict | 0x10 | 0x10 | SAME |
| BattleItemData.itemDataFid | .PlayerBattleItemInfo -> fid | 0x10 | 0x10 | SAME |
| BattleItemData.itemDataList | .PlayerBattleItemInfo -> itemDataList | 0x30 | 0x30 | SAME |
| BattleItemData.battleItemInfo | .BattleItemData -> info | 0x10 | 0x10 | SAME |
| BattleItemData.item_id_ | .BattleItemInfo -> item_id_ | 0x1C | 0x1C | SAME |
| BattleItemData.count_ | .BattleItemInfo -> count_ | 0x20 | 0x20 | SAME |
| BattleItemData.listItems | System.Collections.Generic.List`1 -> _items | 0x0 | 0x10 | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| BattleItemData.listSize | System.Collections.Generic.List`1 -> _size | 0x0 | 0x18 | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| Buff._Instance | .BuffManager -> _instance | 0x0 | 0x0 | SAME |
| Buff.recordBuffHandlersDict | .BuffManager -> _recordBuffHandlersDict | 0x10 | 0x10 | SAME |
| Buff.BuffDict | .BuffHandler -> _mBuffDict | 0x30 | 0x30 | SAME |
| Buff.BuffInstanceIdList | .BuffHandler -> <BuffInstanceIdList>k__BackingField | 0x38 | 0x38 | SAME |
| Buff.BuffTidList | .BuffHandler -> <BuffTidList>k__BackingField | 0x40 | 0x40 | SAME |
| Buff.BuffId | .Buff -> <BuffId>k__BackingField | 0x20 | 0x20 | SAME |
| Buff.GroupId | .Buff -> <GroupId>k__BackingField | 0x24 | 0x24 | SAME |
| Buff.LeftTime | .Buff -> leftTime | 0x34 | 0x34 | SAME |
| Buff.OwnerFid | .Buff -> ownerFid | 0x84 | 0x84 | SAME |
| Buff.CasterFid | .Buff -> casterFid | 0x88 | 0x88 | SAME |
| Buff.BuffIId | .Buff -> <BuffIId>k__BackingField | 0x8C | 0x8C | SAME |

Managed mappings: matched=156 missing/ambiguous=5 other-variant=8.

## 数值不符（列出 dump 原值；有效性看状态，禁止跨变体复制）

| 变量 | Offset.h 原值 | dump 值 | 依据 | 状态 |
|---|---|---|---|---|
| LocalGenRangeSingleData | 0x111 | 0x5FECD50 | RangeAttackEmitter -> LocalGenRangeSingleData | CHANGED |
| Il2CppList.Items | 0x10 | 0x0 | List`1 -> _items | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| Il2CppList.Size | 0x18 | 0x0 | List`1 -> _size | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| BattleItemData.listItems | 0x10 | 0x0 | List`1 -> _items | UNRESOLVED_LAYOUT (raw dump value; do not apply) |
| BattleItemData.listSize | 0x18 | 0x0 | List`1 -> _size | UNRESOLVED_LAYOUT (raw dump value; do not apply) |

## 注释映射不符（精确声明以 dump 为准）

| 变量 | Offset.h 注释 | dump 声明 |
|---|---|---|
