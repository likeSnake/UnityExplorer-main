#pragma once
#include "Data.h"

Vector3 GetPrediction(Vector3 Velocity, Vector3 TarPos, Vector3 MyPos, int Type);
void AimBot(int ScreenWidth, int ScreenHeight, float x, float y, float AimSpeed, float smoothness);
Vector3 GetBonePosition(uint64_t Entity, uint64_t index);
Vector3 GetPosition(ULONG64 transform);
Vector3 CalcVelocity(uintptr_t entityId, const Vector3& curPos);
void AimAtTarget(uintptr_t actorModel, float ScreenCenterX, float ScreenCenterY, Vector3 targetPos, Vector3 MyPos, int weaponType);
VOID ZhuiZong(Vector3 BonePos);

