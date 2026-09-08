#include "aimbot.h"
#include <deque>
#include "log.h"

bool isRandomizer() {
	srand(time(0)); // 初始化随机数生成器
	int random = rand(); // 生成一个0到RAND_MAX之间的随机数
	return random == 0; // 判断随机数是否为0
}
struct ProjectileParams {
	float speed_mult;
	float gravity;
};
struct EntityHistory {
	std::deque<Vector3> positions;
	std::deque<uint64_t> times; // 时间戳（毫秒）
};

static std::map<uintptr_t, EntityHistory> entityHistoryMap;

static std::unordered_map<int, ProjectileParams> weapon_config = {
		{204, {1.25f, 0.0f}},  // 和尚
		{108, {1.5f, 0.0f}},  // 连弩
		{113, {2.5f, 0.0f}},  // 鸟铳
		{104, {(LocalPlayer_Data.MyBlueTime > 0.8f ? 2.0f : 1.5f), 2.35f}}, // 弓默认参数
		{112, {(LocalPlayer_Data.MyBlueTime > 0.8f ? 2.3f : 1.8f), 0.0f}},   // 五眼铳
		{110, {0.25f, 0.0f}},  // 喷火筒
		{111, {0.4f, 0.0f}},   // 一窝蜂
		{203, {1.6f, 0.0f}},   // 三娘水矛
		{206, {1.6f, 0.0f}},   // 神妙机甲
		{207, {1.6f, 0.0f}},   // 季莹莹大招
		{211, {1.6f, 0.0f}},   // 希拉大招
		{109, {0.25f, 5.0f}},  // 火炮（需根据距离调整）

};

Vector3 GetPosition(ULONG64 transform)
{
	Vector3 ret_value = {};
	ULONG64 matrix_list_base = 0;
	ULONG64 dependency_index_table_base = 0;
	ULONG64 transform_internal = transform;
	ULONG64 matrices = 0;
	mem.Read(transform_internal + 0x38, &matrices, 8);
	if (matrices == 0)
		return ret_value;
	int index = 0;
	mem.Read(transform_internal + 0x40, &index, 4);
	mem.Read(matrices + 0x18, &matrix_list_base, 8);
	if (matrix_list_base == 0)
		return ret_value;
	mem.Read(matrices + 0x20, &dependency_index_table_base, 8);
	if (dependency_index_table_base == 0)
		return ret_value;
	int index_relation = 0;
	mem.Read(dependency_index_table_base + index * 4, &index_relation, 4);
	float base_matrix3x4[64],
		* matrix3x4_buffer0 = (float*)((ULONG64)base_matrix3x4 + 16),
		* matrix3x4_buffer1 = (float*)((ULONG64)base_matrix3x4 + 32),
		* matrix3x4_buffer2 = (float*)((ULONG64)base_matrix3x4 + 48);
	mem.Read(matrix_list_base + index * 0x30, base_matrix3x4, 0x10);
	__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
	__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
	__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };
	while (index_relation >= 0)
	{
		ULONG matrix_relation_index = 6 * index_relation;
		mem.Read(matrix_list_base + 8 * matrix_relation_index, matrix3x4_buffer2, 0x10);
		__m128 v_0 = *(__m128*)matrix3x4_buffer2;
		mem.Read(matrix_list_base + 8 * matrix_relation_index + 32, matrix3x4_buffer0, 0x10);
		__m128 v_1 = *(__m128*)matrix3x4_buffer0;
		mem.Read(matrix_list_base + 8 * matrix_relation_index + 16, matrix3x4_buffer1, 0x10);
		__m128i v9 = *(__m128i*)matrix3x4_buffer1;
		__m128* v3 = (__m128*)base_matrix3x4;
		__m128 v10, v11, v12, v13, v14, v15, v16, v17;
		v10 = _mm_mul_ps(v_1, *v3);
		v11 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 0));
		v12 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 85));
		v13 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 142));
		v14 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 219));
		v15 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 170));
		v16 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 113));
		v17 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v11, xmmword_1410D1350), v13), _mm_mul_ps(_mm_mul_ps(v12, xmmword_1410D1360), v14)),
			_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 170))), _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v15, xmmword_1410D1360), v14),
				_mm_mul_ps(_mm_mul_ps(v11, xmmword_1410D1340), v16)), _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 85)))),
			_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v12, xmmword_1410D1340), v16), _mm_mul_ps(_mm_mul_ps(v15, xmmword_1410D1350), v13)),
				_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 0))), v10)), v_0);
		*v3 = v17;
		mem.Read(dependency_index_table_base + index_relation * 4, &index_relation, sizeof(index_relation));
	}
	ret_value = *(Vector3*)base_matrix3x4;
	return ret_value;
}



Vector3 GetBonePosition(uint64_t Entity, uint64_t  index) {
	auto object = mem.Read<uint64_t>(Entity + 0x60);
	object = mem.Read<uint64_t>(object + 0x10);
	object = mem.Read<uint64_t>(object + 0x30);
	auto bone_object = mem.Read<uint64_t>(object + 0x30);
	bone_object = mem.Read<uint64_t>(bone_object + 0x28);
	bone_object = mem.Read<uint64_t>(bone_object + 0x1E0);
	bone_object = mem.Read<uint64_t>(bone_object + 0x8);
	Vector3 spine_pos = GetPosition(mem.Read<uint64_t>(bone_object + index * 8));
	return spine_pos;
}

void AimBot(int ScreenWidth, int ScreenHeight, float x, float y, float AimSpeed, float smoothness)
{
	// 计算目标偏移
	float targetx = 0.0f, targety = 0.0f;
	float centerX = static_cast<float>(ScreenWidth);
	float centerY = static_cast<float>(ScreenHeight);

	if (x != 0.0f) {
		targetx = (x > centerX) ? -(centerX - x) : (x - centerX);
		targetx /= AimSpeed;
		
		if ((x > centerX && targetx + centerX > centerX * 2) ||
			(x < centerX && targetx + centerX < 0))
			targetx = 0.0f;
	}

	if (y != 0.0f) {
		targety = (y > centerY) ? -(centerY - y) : (y - centerY);
		targety /= AimSpeed;
		if ((y > centerY && targety + centerY > centerY * 2) ||
			(y < centerY && targety + centerY < 0))
			targety = 0.0f;
	}

	// 平滑处理
	targetx /= smoothness;
	targety /= smoothness;

	// 随机微调
	if (isRandomizer()) {
		if (std::abs(targetx) < 1.0f)
			targetx = (targetx > 0.0f) ? 1.0f : (targetx < 0.0f ? -1.0f : 0.0f);
		if (std::abs(targety) < 1.0f)
			targety = (targety > 0.0f) ? 1.0f : (targety < 0.0f ? -1.0f : 0.0f);
	}

	Move_mouse(static_cast<int>(targetx), static_cast<int>(targety));
}

Vector3 CalcVelocity(uintptr_t entityId, const Vector3& curPos) {
	constexpr size_t kHistorySize = 5;

	constexpr size_t kSmoothWindow = 3; // 平滑窗口大小

	auto& history = entityHistoryMap[entityId];
	uint64_t curTimeMs = GetTickCount64();

	// 存储当前坐标和时间
	history.positions.push_back(curPos);
	history.times.push_back(curTimeMs);
	if (history.positions.size() > kHistorySize) {
		history.positions.pop_front();
		history.times.pop_front();
	}
	// 不足2帧时返回0
	if (history.positions.size() < 2) return Vector3(0, 0, 0);

	// 计算滑动平均速度
	Vector3 velocitySum(0, 0, 0);
	size_t count = 0;
	for (size_t i = 1; i < history.positions.size(); ++i) {
		uint64_t dt_ms = history.times[i] - history.times[i - 1];
		if (dt_ms == 0) continue;
		float dt = dt_ms / 1000.0f;
		Vector3 v = (history.positions[i] - history.positions[i - 1]) / dt;
		velocitySum = velocitySum + v;
		++count;
		if (count >= kSmoothWindow) break; 
	}
	if (count == 0) return Vector3(0, 0, 0);
	return velocitySum / static_cast<float>(count);
}

Vector3 GetPrediction(Vector3 Velocity, Vector3 TarPos, Vector3 MyPos, int Type) {
	float speed = 100.0f;
	float gravity = 0.0f;
	// 根据武器类型调整速度和重力
	auto it = weapon_config.find(Type);
	if (it != weapon_config.end()) {
		speed *= it->second.speed_mult;
		gravity = it->second.gravity;
	}

	float distance = TarPos.DistTo(MyPos);
	float flightduration = distance / speed;
	Vector3 ReturnVec = {0,0,0};


	ReturnVec = TarPos + Velocity * flightduration ;
	
	ReturnVec.y += gravity * 0.5f * flightduration * flightduration;


	return ReturnVec;
}

//追踪
VOID ZhuiZong(Vector3 BonePos) {
	uint64_t m_ItemPointer = mem.Read<uint64_t>(Offset::GameAssembly + Offset::m_EntityManager);
	m_ItemPointer = mem.Read<uint64_t>(m_ItemPointer + 0xB8);
	m_ItemPointer = mem.Read<uint64_t>(m_ItemPointer + 0x8);
	m_ItemPointer = mem.Read<uint64_t>(m_ItemPointer + 0x28);
	uint32_t m_ItemCount = mem.Read<uint32_t>(m_ItemPointer + 0x24);
	uint64_t m_InitialAddress = mem.Read<uint64_t>(m_ItemPointer + 0x18) + 0x28;
	for (uint32_t i = 0; i < m_ItemCount; i++) {
		uint64_t m_ItemAddress = mem.Read<uint64_t>(static_cast<uint64_t>(m_InitialAddress) + static_cast<uint64_t>(i) * 0x10);
		if (m_ItemAddress <= 0) {
			continue;
		}
		uint64_t m_ItemPointer = mem.Read<uint64_t>(m_ItemAddress);
		if (m_ItemPointer <= 0) {
			continue;
		}
		uint64_t m_ItemType = mem.Read<uint64_t>(m_ItemPointer + 0x10);
		char m_ItemTypeTxt[32];
		mem.Read(m_ItemType, &m_ItemTypeTxt, sizeof(m_ItemTypeTxt));
		m_ItemTypeTxt[sizeof(m_ItemTypeTxt) - 1] = '\0';
		if (!strcmp((const char*)m_ItemTypeTxt, "RangeAgentEntity")) {
			uint32_t m_ItemID = mem.Read<uint32_t>(mem.Read<uint64_t>(m_ItemAddress + 0x58) + 0x30);
			uint64_t m_CoordinatePointer = GetItemPosAddr(m_ItemAddress);
			uint32_t m_CoordinateIndex = mem.Read<uint32_t>(m_CoordinatePointer + 0x40);
			uint64_t m_CoordinatePosition = mem.Read<uint64_t>(mem.Read<uint64_t>(m_CoordinatePointer + 0x38) + 0x18);
			m_CoordinatePointer = m_CoordinatePosition + static_cast<uint64_t>(m_CoordinateIndex) * 0x30;
			Vector3 m_ItemPos = GetItemPos(m_CoordinatePointer);

			float m_Distance = GetPlayerDis(m_ItemPos, BonePos);
			if (m_Distance <= 30.f) {
				mem.Write<float>(m_CoordinatePointer, BonePos.x);
				mem.Write<float>(m_CoordinatePointer + 4, BonePos.y);
				mem.Write<float>(m_CoordinatePointer + 8, BonePos.z);
			}
		}
	}
}


void AimAtTarget(uintptr_t actorModel, float ScreenCenterX, float ScreenCenterY , Vector3 targetPos, Vector3 MyPos, int weaponType)
{
	static long lastTime = 0;
	static QWORD lastTarget = 0;
	static Vector3 lastPos = { 0, 0, 0 }, velocity = { 0, 0, 0 };

	Vector3 predictedPos;
	double weaponSpeed = 0, bulletTravelTime = 0;
	long currentTime = GetTickCount64();
	long deltaTime = currentTime - lastTime;

	// 控制调用频率
	//if (deltaTime > 10)
	{
		// 目标切换时重置
		Sleep(Function::AiMBot::总平滑度);
		if (actorModel != lastTarget)
		{
			lastTarget = actorModel;
			lastPos = targetPos;
		}

		// 初始化上次坐标
		if (lastPos.x == 0 && lastPos.y == 0 && lastPos.z == 0)
		{
			lastPos = targetPos;
		}

		// 计算移动速度（米/秒）
		if (lastTarget != 0 && actorModel == lastTarget && deltaTime > 0)
		{
			float dt = deltaTime / 1000.0f;
			velocity.x = (targetPos.x - lastPos.x) / dt * Function::AiMBot::Dt;
			velocity.y = (targetPos.y - lastPos.y) / dt * Function::AiMBot::Dt;
			velocity.z = (targetPos.z - lastPos.z) / dt * Function::AiMBot::Dt;
		}

		// 预测目标位置

		predictedPos = GetPrediction(velocity, targetPos, MyPos, weaponType);
		/*weaponSpeed = GetWeaponSpeed(weaponType);
		bulletTravelTime = actorModel.O_Dis / weaponSpeed;
		predictedPos.x = targetPos.x + velocity.x * bulletTravelTime;
		predictedPos.y = targetPos.y + velocity.y * bulletTravelTime;
		predictedPos.z = targetPos.z + velocity.z * bulletTravelTime;*/

		// 更新时间和坐标
		lastTime = currentTime;
		lastPos = targetPos;

		// 投影到屏幕坐标
		Vec2 screenPos;
		Vec4 clipPos;
		if (WorldToScreen(MATRIX, predictedPos, screenPos, clipPos))
		{
		
			float sensitivityX = Function::AiMBot::smoothnessX;
			float sensitivityY = Function::AiMBot::smoothnessY;

			// 计算相对屏幕中心的偏移
			screenPos.x = (screenPos.x - ScreenCenterX) * sensitivityX;
			screenPos.y = (screenPos.y - ScreenCenterY) * sensitivityY;

			// 限速、平滑、死区处理
			float moveX = screenPos.x;
			float moveY = screenPos.y;
			float dist = sqrtf(moveX * moveX + moveY * moveY);

			//MyLogTrue("移动距离", dist);
			float maxSpeed = Function::AiMBot::maxSpeed;   // 大范围移动限制
			float smoothingFactor = Function::AiMBot::smoothingFactor; // 小范围平滑系数
			float deadZone = Function::AiMBot::deadZone;    // 死区阈值
			float lockError = Function::AiMBot::lockError;

			if (lockError > 0.0f && dist <= lockError)
			{
				return;
			}

			if (fabs(moveX) < deadZone && fabs(moveY) < deadZone)
			{
				// 死区内不移动
				return;
			}
			else
			{
				if (dist > maxSpeed)
				{
					float ratio = maxSpeed / dist;
					moveX *= ratio;
					moveY *= ratio;
				}
				else
				{
					moveX *= smoothingFactor;
					moveY *= smoothingFactor;
				}
			}
			Move_mouse(static_cast<int>(moveX), static_cast<int>(moveY));
			
		}
	}
}