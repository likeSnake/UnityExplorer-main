#pragma once
#include <cstdint>

namespace Offset {
inline uint64_t GameAssembly = 0;
inline uint64_t UnityPlayer = 0; // Added to support existing logic
inline uint64_t NarakaBladepoint = 0;
inline uint64_t MatrixAddr = 0;
inline uint64_t Base_WindowBase = 0x33E44B8;    // 游戏窗口大小
inline uint64_t Base_BuffManager = 0x33CDC10;   // 人物BUFF
inline uint64_t m_GlobalTime = 0x33D7968;       // 世界时间
inline uint64_t m_CharacterManager = 0x33CEEF0; // 世界地址 (GameAssembly_Super)
inline uint64_t m_UserDataManager = 0x33F1068;  // 技能建筑数据基址
inline uint64_t m_CharactorSync = 0x33CEF58;    // 人物同步
inline uint64_t m_EntityManager = 0x33D7A20;    // 物品基址

// inline uint64_t InitActorKitRefer = 0x;  // 内存振刀
inline uint64_t FlushState = 0xC5; // 刷新状态

inline uint64_t ActorKitRefer = 0xC; // 动画
inline uint64_t vtbale_ret = 0x1D;   // 返回

// inline uint64_t LocalGenRangeSingleData = 0x;  // 追踪
// inline uint64_t op_Subtraction          = 0x;  // 返回地址

// inline uint64_t Call_GetVelocity = 0x;  // 人物移动速c

struct Character_t {
  uint32_t AllAliveCharList = 0x58;
  uint32_t ArrayStart = 0x30;
};
inline Character_t Character;

struct ActorModel_t {
  uint32_t ObjectMsg = 0xD8; // 对象消息
  uint32_t EventTracksForLayer = 0x30;
  uint32_t Transform = 0x118; // 转换
  uint32_t ActorWeapon = 0xF8;
  uint32_t RuntimePropertyData = 0xA8;
  uint32_t PropertyData = 0x348;
  uint32_t ActorKit = 0xC8;

  struct ProPerty_t {
    uint32_t PlayerName = 0x1F8;
    uint32_t Heroid = 0x108;      // 英雄ID
    uint32_t Group = 0x118;       // 团队
    uint32_t WeaponId = 0x128;    // 武器id
    uint32_t RobotId = 0x130;     // 机器人
    uint32_t PlayerName = 0x1F8;  // 人物名称
    uint32_t SkillStatus = 0x198; // 技能施放状态（0:未施放 1:前摇中 2:施放中）
  } ProPerty;

  struct ActorKitS_t {
    uint32_t BackingField = 0x38;
    uint32_t ActorPhysics = 0xD0;
    uint32_t ActorKitSync = 0xC8;
    uint32_t currentCollisionFlags = 0xEC;
    uint32_t HitSimulateData = 0xF0;   // 状态
    uint32_t actorKitComboData = 0xE0; // 状态
    uint32_t actorKitBreakData = 0xF8;
    uint32_t PlayerCollideAgent = 0x1E8;
    uint32_t FlashType = 0x58;
    struct HitSimulate_t {
      uint32_t ReactionType = 0x54;      // 出刀状态
      uint32_t RangeReactionType = 0x58; // 攻击状态
      uint32_t ReactionExtraType = 0x5C; // 振刀状态
      uint32_t endureLevel = 0xD0;       // 当前状态
      uint32_t ActionDuration = 0x2C;    // 僵直
      uint32_t ReactionParryType = 0xB4; // 磐石状态
      uint32_t XuListate = 0x70;         // 蓄力状态
      uint32_t XuLilevel = 0x54;         // 蓄力层级
    } HitSimulate;
  } ActorKitS;
};
inline ActorModel_t ActorModel; // 行动状态

struct UserData_t {
  uint32_t EquipPackData = 0xA8;
  uint32_t InteractiveManager = 0x228;
  uint32_t SpectatorData = 0x88;
  uint32_t Skill_ID = 0x70;
  uint32_t skillState = 0x80;
  uint32_t ClientLeftCdTime = 0x44;
  uint32_t ClientLeftContinueTime = 0x4c;
};
inline UserData_t UserData; // 人物数据
} // namespace Offset
