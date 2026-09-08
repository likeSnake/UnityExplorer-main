#pragma once
#include "log.h"

extern double myGlobalTime;
// 确保使用前刷新一次全局时间，避免初始值为 0
static inline double GetGlobalTimeSafe()
{
	
	return LocalPlayer_Data.worldPtrGlobalTime;
}

static float 距离计算(PlayerData obj) {
	// 假设 obj.O_Dis 已经定义并赋值
	float 自身出蓄前摇 = 0;
	if (obj.O_Dis > 1) {
		// 从 1 开始，每增加 0.1，减少 0.001
		float incrementjuli = (obj.O_Dis - 1) / 0.1f;  // 计算 obj.O_Dis 从 1 到 obj.O_Dis 之间的 0.1 增量
		自身出蓄前摇 -= incrementjuli * 0.001f;  // 每增加 0.1，减少 0.001
	}
	return 自身出蓄前摇;
}

static float 链剑A1后摇 = 0.085f;
static float 链剑走A1后摇 = 0.085f;
static float 链剑滑步A1后摇 = 0.085f;
static float 链剑B1后摇 = 0.050f;
static float 链剑走B1后摇 = 0.030f;
static float 链剑滑步B1后摇 = 0.030f;
static float 链剑A2后摇 = 0.412f;
static float 链剑B2后摇 = 0.412f;
static float 链剑CA后摇 = 0.12f;
static float 链剑滑铲CA后摇 = 0.13f;
static float 链剑升龙后摇 = 0.49f;
static float 链剑滑铲升龙后摇 = 0.95f;
static float 链剑跳A后摇 = 0.33f;
static float 链剑下批后摇 = 0.535f;

static float 长剑A1后摇 = 0.085f;
static float 长剑走A1后摇 = 0.085f;
static float 长剑滑步A1后摇 = 0.085f;
static float 长剑B1后摇 = 0.052f;
static float 长剑走B1后摇 = 0.030f;
static float 长剑滑步B1后摇 = 0.051f;
static float 长剑A2后摇 = 0.412f;
static float 长剑B2后摇 = 0.412f;
static float 长剑CA后摇 = 0.12f;
static float 长剑滑铲CA后摇 = 0.13f;
static float 长剑升龙后摇 = 0.49f;
static float 长剑滑铲升龙后摇 = 0.95f;
static float 长剑跳A后摇 = 0.33f;
static float 长剑下批后摇 = 0.535f;

static float 太刀A1后摇 = 0.050f;
static float 太刀走A1后摇 = 0.050f;
static float 太刀滑步A1后摇 = 0.050f;
static float 太刀B1后摇 = 0.060f;
static float 太刀走B1后摇 = 0.085f;
static float 太刀滑步B1后摇 = 0.085f;
static float 太刀A2后摇 = 0.412f;
static float 太刀B2后摇 = 0.412f;
static float 太刀CA后摇 = 0.12f;
static float 太刀滑铲CA后摇 = 1.011f;
static float 太刀升龙后摇 = 0.504f;
static float 太刀滑铲升龙后摇 = 0.70f;
static float 太刀跳A后摇 = 0.33f;
static float 太刀下批后摇 = 0.63f;

static float 横刀A1后摇 = 0.050f;
static float 横刀走A1后摇 = 0.050f;
static float 横刀滑步A1后摇 = 0.050f;
static float 横刀B1后摇 = 0.060f;
static float 横刀走B1后摇 = 0.085f;
static float 横刀滑步B1后摇 = 0.085f;
static float 横刀A2后摇 = 0.412f;
static float 横刀B2后摇 = 0.412f;
static float 横刀CA后摇 = 0.302f;
static float 横刀滑铲CA后摇 = 1.011f;
static float 横刀升龙后摇 = 0.504f;
static float 横刀滑铲升龙后摇 = 0.70f;
static float 横刀跳A后摇 = 0.33f;
static float 横刀下批后摇 = 0.41f;

static float 双刀A1后摇 = 0.052f;
static float 双刀走A1后摇 = 0.052f;
static float 双刀滑步A1后摇 = 0.052f;
static float 双刀B1后摇 = 0.060f;
static float 双刀走B1后摇 = 0.085f;
static float 双刀滑步B1后摇 = 0.085f;
static float 双刀A2后摇 = 0.412f;
static float 双刀B2后摇 = 0.412f;
static float 双刀CA后摇 = 0.302f;
static float 双刀滑铲CA后摇 = 0.62f;
static float 双刀升龙后摇 = 0.40f;
static float 双刀滑铲升龙后摇 = 0.70f;
static float 双刀跳A后摇 = 0.33f;
static float 双刀下批后摇 = 0.41f;

static float 双戟A1后摇 = 0.052f;
static float 双戟走A1后摇 = 0.052f;
static float 双戟滑步A1后摇 = 0.052f;
static float 双戟B1后摇 = 0.060f;
static float 双戟走B1后摇 = 0.085f;
static float 双戟滑步B1后摇 = 0.085f;
static float 双戟A2后摇 = 0.412f;
static float 双戟B2后摇 = 0.412f;
static float 双戟CA后摇 = 0.302f;
static float 双戟滑铲CA后摇 = 0.79f;
static float 双戟升龙后摇 = 0.69f;
static float 双戟滑铲升龙后摇 = 0.69f;
static float 双戟跳A后摇 = 0.33f;
static float 双戟下批后摇 = 0.77f;

static float 双节棍A1后摇 = 0.12f;
static float 双节棍走A1后摇 = 0.115f;
static float 双节棍滑步A1后摇 = 0.115f;
static float 双节棍B1后摇 = 0.081f;
static float 双节棍走B1后摇 = 0.085f;
static float 双节棍滑步B1后摇 = 0.085f;
static float 双节棍A2后摇 = 0.76f;
static float 双节棍B2后摇 = 0.47f;
static float 双节棍CA后摇 = 0.302f;
static float 双节棍滑铲CA后摇 = 0.40f;
static float 双节棍升龙后摇 = 0.48f;
static float 双节棍滑铲升龙后摇 = 0.55f;
static float 双节棍跳A后摇 = 0.28f;
static float 双节棍下批后摇 = 0.75f;

static float 长棍A1后摇 = 0.285f;
static float 长棍走A1后摇 = 0.285f;
static float 长棍滑步A1后摇 = 0.285f;
static float 长棍B1后摇 = 0.085f;
static float 长棍走B1后摇 = 0.085f;
static float 长棍滑步B1后摇 = 0.085f;
static float 长棍A2后摇 = 0.310f;
static float 长棍B2后摇 = 0.310f;
static float 长棍CA后摇 = 0.350f;
static float 长棍滑铲CA后摇 = 0.79f;
static float 长棍升龙后摇 = 0.69f;
static float 长棍滑铲升龙后摇 = 0.69f;
static float 长棍跳A后摇 = 0.13f;
static float 长棍下批后摇 = 0.77f;

static float 长枪A1后摇 = 0.315f;
static float 长枪走A1后摇 = 0.315f;
static float 长枪滑步A1后摇 = 0.315f;
static float 长枪B1后摇 = 0.085f;
static float 长枪走B1后摇 = 0.085f;
static float 长枪滑步B1后摇 = 0.085f;
static float 长枪A2后摇 = 0.320f;
static float 长枪B2后摇 = 0.320f;
static float 长枪CA后摇 = 0.350f;
static float 长枪滑铲CA后摇 = 0.79f;
static float 长枪升龙后摇 = 0.69f;
static float 长枪滑铲升龙后摇 = 0.69f;
static float 长枪跳A后摇 = 0.13f;
static float 长枪下批后摇 = 0.77f;

static float 匕首A1后摇 = 0.085f;
static float 匕首走A1后摇 = 0.055f;
static float 匕首滑步A1后摇 = 0.065f;
static float 匕首B1后摇 = 0.065f;
static float 匕首走B1后摇 = 0.110f;
static float 匕首滑步B1后摇 = 0.110f;
static float 匕首A2后摇 = 0.120f;
static float 匕首B2后摇 = 0.120f;
static float 匕首CA后摇 = 0.120f;
static float 匕首滑铲CA后摇 = 0.120f;
static float 匕首升龙后摇 = 0.53f;
static float 匕首滑铲升龙后摇 = 0.56f;
static float 匕首跳A1后摇 = 0.15f;
static float 匕首跳A2后摇 = 0.15f;
static float 匕首下批后摇 = 0.15f;

static float 扇子吹风 = 0.210f;
static float 扇子A1后摇 = 0.075f;
static float 扇子走A1后摇 = 0.055f;
static float 扇子滑步A1后摇 = 0.065f;
static float 扇子B1后摇 = 0.065f;
static float 扇子走B1后摇 = 0.120f;
static float 扇子滑步B1后摇 = 0.120f;
static float 扇子A2后摇 = 0.120f;
static float 扇子B2后摇 = 0.120f;
static float 扇子CA后摇 = 0.120f;
static float 扇子滑铲CA后摇 = 0.120f;
static float 扇子升龙后摇 = 0.53f;
static float 扇子滑铲升龙后摇 = 0.56f;
static float 扇子跳A1后摇 = 0.25f;
static float 扇子跳A2后摇 = 0.30f;
static float 扇子下批后摇 = 0.19f;

static float 阔刀A1后摇 = 0.22f;
static float 阔刀走A1后摇 = 0.225f;
static float 阔刀滑步A1后摇 = 0.225f;
static float 阔刀B1后摇 = 0.325f;
static float 阔刀走B1后摇 = 0.325f;
static float 阔刀滑步B1后摇 = 0.325f;
static float 阔刀CA后摇 = 0.230f;
static float 阔刀滑铲CA后摇 = 0.235f;
static float 阔刀升龙后摇 = 0.29f;
static float 阔刀滑铲升龙后摇 = 0.23f;
static float 阔刀跳A后摇 = 0.61f;
static float 阔刀下批后摇 = 0.68f;

static float 斩马刀A1后摇 = 0.22f;
static float 斩马刀走A1后摇 = 0.225f;
static float 斩马刀滑步A1后摇 = 0.225f;
static float 斩马刀B1后摇 = 0.325f;
static float 斩马刀走B1后摇 = 0.325f;
static float 斩马刀滑步B1后摇 = 0.325f;
static float 斩马刀CA后摇 = 0.230f;
static float 斩马刀滑铲CA后摇 = 0.235f;
static float 斩马刀升龙后摇 = 0.29f;
static float 斩马刀滑铲升龙后摇 = 0.23f;
static float 斩马刀跳A后摇 = 0.61f;
static float 斩马刀下批后摇 = 0.68f;

static float 拳刃A1后摇 = 0.051f;
static float 拳刃走A1后摇 = 0.12f;
static float 拳刃滑步A1后摇 = 0.12f;
static float 拳刃B1后摇 = 0.079f;
static float 拳刃走B1后摇 = 0.080f;
static float 拳刃滑步B1后摇 = 0.075f;
static float 拳刃A2后摇 = 0.46f;
static float 拳刃B2后摇 = 0.15f;
static float 拳刃CA后摇 = 0.47f;
static float 拳刃滑铲CA后摇 = 0.15f;
static float 拳刃升龙后摇 = 0.53f;
static float 拳刃滑铲升龙后摇 = 0.56f;
static float 拳刃跳A1后摇 = 0.32f;
static float 拳刃跳A2后摇 = 0.33f;
static float 拳刃下批后摇 = 0.46f;

static float 飞刀A1后摇 = 0.12f;
static float 飞刀走A1后摇 = 0.12f;
static float 飞刀滑步A1后摇 = 0.12f;
static float 飞刀B1后摇 = 0.72f;
static float 飞刀走B1后摇 = 0.72f;
static float 飞刀滑步B1后摇 = 0.72f;
static float 飞刀A2后摇 = 0.46f;
static float 飞刀B2后摇 = 0.76f;
static float 飞刀CA后摇 = 0.47f;
static float 飞刀滑铲CA后摇 = 0.285f;
static float 飞刀升龙后摇 = 0.53f;
static float 飞刀滑铲升龙后摇 = 0.56f;
static float 飞刀跳A1后摇 = 0.265f;
static float 飞刀跳A2后摇 = 0.265f;
static float 飞刀下批后摇 = 0.46f;

static float 未知动作名 = 0.76f;


static bool flag = false;

static auto SwAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	float 出招前摇 = 0.f;
	float 出招执行 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{

		出招后摇 = 长剑A1后摇;

	}
	//走A1
	else if (O_NameHash == 4109740294)
	{

		出招后摇 = 长剑走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{

		出招后摇 = 长剑滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{

		出招后摇 = 长剑B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{

		出招后摇 = 长剑走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{

		出招后摇 = 长剑滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{

		出招后摇 = 长剑A2后摇;

	}
	//B2
	else if (O_NameHash == 3064545035)
	{

		出招后摇 = 长剑B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{

		出招后摇 = 长剑CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{

		出招后摇 = 长剑滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{

		出招后摇 = 长剑升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{

		出招后摇 = 长剑滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{

		出招后摇 = 长剑跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{

		出招后摇 = 长剑下批后摇;

	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;
	出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/
}
//链剑
static auto LjAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	float 出招前摇 = 0.f;
	float 出招执行 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{

		出招后摇 = 链剑A1后摇;

	}
	//走A1
	else if (O_NameHash == 4109740294)
	{

		出招后摇 = 链剑走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{

		出招后摇 = 链剑滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{

		出招后摇 = 链剑B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{

		出招后摇 = 链剑走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{

		出招后摇 = 链剑滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{

		出招后摇 = 链剑A2后摇;

	}
	//B2
	else if (O_NameHash == 3064545035)
	{

		出招后摇 = 链剑B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{

		出招后摇 = 链剑CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{

		出招后摇 = 链剑滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{

		出招后摇 = 链剑升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{

		出招后摇 = 链剑滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{

		出招后摇 = 链剑跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{

		出招后摇 = 链剑下批后摇;

	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/
}

static auto KatanaAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	MyLog("敌人太刀招式哈希值",O_NameHash);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 太刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 太刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 太刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 太刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 太刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 太刀滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 太刀A2后摇;
	}
	//B2
	else if (O_NameHash == -1302707832)
	{
		出招后摇 = 太刀B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 太刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 太刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 太刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 太刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 1758954236)
	{
		出招后摇 = 太刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 太刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - FatPlayerData.ObjCurTimeForLogic);
	MyLog("ObjCurTime =", ObjCurTime);
	MyLog("ObjCurTimeForLogic =", FatPlayerData.ObjCurTimeForLogic);
	MyLog("GlobalTime =", GetGlobalTimeSafe());*/
	if (出招后摇 > 0.5) {
		return false;
	}
	MyLog("出招后摇=", 出招后摇);
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*if (Function::LogicKnife::BlueAttacklanding == true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(出招后摇 * 1000))); // 将秒转换为毫秒

		if (出招后摇 != 0 && ObjCurTime > 出招后摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			MyLog("太刀蓝顶");
			//Sleep(活化参数::蓝顶延迟);
			//振刀逻辑::执行长剑平A僵直抓取();
		}
	}*/
}
static auto BladeAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 阔刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 阔刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 阔刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 阔刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 阔刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 阔刀滑步B1后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 阔刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 阔刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 阔刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 阔刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 阔刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 阔刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/
}
static auto SpearAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 长枪A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 长枪走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 长枪滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 长枪B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 长枪走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 长枪滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 长枪A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 长枪B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 长枪CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 长枪滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 长枪升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 长枪滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 长枪跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 长枪下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(400);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/
}
static auto DaggeAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 匕首A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 匕首走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 匕首滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 匕首B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 匕首走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 匕首滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 匕首A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 匕首B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 匕首CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 匕首滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 匕首升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 匕首滑铲升龙后摇;
	}
	//跳A1
	else if (O_NameHash == 1758954236)
	{
		出招后摇 = 匕首跳A1后摇;
	}
	//跳A2
	else if (O_NameHash == 533770858)
	{
		出招后摇 = 匕首跳A2后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 匕首下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(250);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/


}
static auto nunchucksAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 双节棍A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 双节棍走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 双节棍滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 双节棍B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 双节棍走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 双节棍滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 双节棍A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 双节棍B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 双节棍CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 双节棍滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 双节棍升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 双节棍滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 双节棍跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 双节棍下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/

}
static auto twinbladesAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 双刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 双刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 双刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 双刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 双刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 双刀滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 双刀A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 双刀B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 双刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 双刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 双刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 双刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 双刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 双刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
}
static auto rodAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 长棍A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 长棍走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 长棍滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 长棍B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 长棍走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 长棍滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 长棍A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 长棍B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 长棍CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 长棍滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 长棍升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 长棍滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 长棍跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 长棍下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(400);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/

}
static auto SaberAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 斩马刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 斩马刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 斩马刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 斩马刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 斩马刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 斩马刀滑步B1后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 斩马刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 斩马刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 斩马刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 斩马刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 斩马刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 斩马刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/
}
static auto dualhalberdAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 双刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 双刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 双刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 双刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 双刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 双刀滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 双刀A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 双刀B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 双刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 双刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 双刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 双刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 双刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 双刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/

}
static auto fanAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	// 扇子吹风
	if (O_NameHash == 3011659083) {
		出招后摇 = 扇子吹风;
	}
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 扇子A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 扇子走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 扇子滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 扇子B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 扇子走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 扇子滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 扇子A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 扇子B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 扇子CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 扇子滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 扇子升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 扇子滑铲升龙后摇;
	}
	//跳A1
	else if (O_NameHash == 1758954236)
	{
		出招后摇 = 扇子跳A1后摇;
	}
	//跳A2
	else if (O_NameHash == 533770858)
	{
		出招后摇 = 扇子跳A2后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 扇子下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(250);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/


}
static auto hengdaoAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 横刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 横刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 横刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 横刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 横刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 横刀滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 横刀A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 横刀B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 横刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 横刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 横刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 横刀滑铲升龙后摇;
	}
	//跳A
	else if (O_NameHash == 285948504)
	{
		出招后摇 = 横刀跳A后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 横刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
}
static auto punchAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 拳刃A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 拳刃走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 拳刃滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 拳刃B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 拳刃走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 拳刃滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 拳刃A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 拳刃B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 匕首CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 拳刃滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 拳刃升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 拳刃滑铲升龙后摇;
	}
	//跳A1
	else if (O_NameHash == 1758954236)
	{
		出招后摇 = 拳刃跳A1后摇;
	}
	//跳A2
	else if (O_NameHash == 533770858)
	{
		出招后摇 = 拳刃跳A2后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 拳刃下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}
	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/


}
static auto KnifeAttack(PlayerData obj, float 招式前摇, float 武器前摇 = 0)
{
	auto O_NameHash = mem.Read<unsigned int>(obj.Cache_NameHash + 0x18);
	MyLog("飞刀动作名:", O_NameHash);
	float 出招后摇 = 0.f;
	//站A1
	if (O_NameHash == 770101480)
	{
		出招后摇 = 飞刀A1后摇;
	}
	//走A1
	else if (O_NameHash == 4109740294)
	{
		出招后摇 = 飞刀走A1后摇;
	}
	//滑步A1
	else if (O_NameHash == 1140511039)
	{
		出招后摇 = 飞刀滑步A1后摇;
	}
	//站B1
	else if (O_NameHash == 799043249)
	{
		出招后摇 = 飞刀B1后摇;
	}
	//走B1
	else if (O_NameHash == 1682617495)
	{
		出招后摇 = 飞刀走B1后摇;
	}
	//跑B1
	else if (O_NameHash == 1102868326)
	{
		出招后摇 = 飞刀滑步B1后摇;
	}
	//A2
	else if (O_NameHash == 3035595090)
	{
		出招后摇 = 飞刀A2后摇;
	}
	//B2
	else if (O_NameHash == 3064545035)
	{
		出招后摇 = 飞刀B2后摇;
	}
	//CA
	else if (O_NameHash == 3498263638)
	{
		出招后摇 = 飞刀CA后摇;
	}
	//滑步CA
	else if (O_NameHash == 971039075)
	{
		出招后摇 = 飞刀滑铲CA后摇;
	}
	//升龙
	else if (O_NameHash == 4086852895)
	{
		出招后摇 = 飞刀升龙后摇;
	}
	//滑铲升龙
	else if (O_NameHash == 452708394)
	{
		出招后摇 = 飞刀滑铲升龙后摇;
	}
	//跳A1
	else if (O_NameHash == 1758954236)
	{
		出招后摇 = 飞刀跳A1后摇;
	}
	//跳A2
	else if (O_NameHash == 533770858)
	{
		出招后摇 = 飞刀跳A2后摇;
	}
	else if (O_NameHash == 574090087)
	{
		出招后摇 = 飞刀B1后摇;
	}
	//跳下批(前者状态2，后者状态0，前者能拼刀，后者不能)
	else if (O_NameHash == 1787903141 || O_NameHash == 152476916)
	{
		出招后摇 = 飞刀下批后摇;
	}
	else {
		//MyLogTrue("未知动作名，请反馈作者:", O_NameHash);
		出招后摇 = 未知动作名;
	}
	出招后摇 += Function::侧键博弈::白刀蓝顶延迟;出招后摇 -= 武器前摇;
	MyLog("出招后摇=", 出招后摇);
	if (出招后摇 > 0.5) {
		return false;
	}
	ULONGLONG WaitTIme = GetTickCount64() + 1500;
	while (WaitTIme > GetTickCount64()) {
		auto ObjCurTime = (LocalPlayer_Data.worldPtrGlobalTime - FatPlayerData.ObjCurTimeForLogic);
		if (出招后摇 == 0) {
			break;
		}
		if (ObjCurTime > 出招后摇 || LocalPlayer_Data.MyBlueTime > 0.505) {
			MyLog("后摇判断--蓝顶出刀");
			break;
		}
		Sleep(1);
	}

	return true;
	/*auto ObjCurTime = (GetGlobalTimeSafe() - mem.Read<double>(obj.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime));
	if (Function::LogicKnife::BlueAttacklanding == true)
	{
		if (出招后摇 != 0 && ObjCurTime > 出招后摇 / 3 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime <= 0.49))
		{
			Sleep(活化参数::蓝顶延迟);
			振刀逻辑::CollideKnife(); Sleep(300);
		}
	}
	if (ObjCurTime < 0.06)return;

	else if ((O_NameHash == 971039075 || O_NameHash == 452708394) && 出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}
	else if (出招后摇 != 0 && ObjCurTime < 出招后摇 - 招式前摇 && (flag ? true : LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.MyBlueTime > 0.5f))
	{

		振刀逻辑::CollideKnife(); Sleep(500);
	}*/


}

//长剑
static auto SwBlueAttackWrite(PlayerData obj)
{
	//float 长剑前摇 = Function::侧键博弈::白刀蓝顶延迟;
	float 武器前摇 = 0.017;

	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//长剑剑气
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_(light|heavy)_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长剑剑气;
		flag = false;
	}
	//长剑凤凰羽左右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_sw(_|_run_|_sprint_)attack_(light|heavy)_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长剑凤凰羽;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//太刀
static auto KatanaBlueAttackWrite2(PlayerData obj)
{
	//myGlobalTime = GlobalTime;
	//Global::WorldPtr.GlobalTime = mem.Read<double>(Cache_GetGlobalTime_PTR + Offset::ActorModel._globalTime);
	//MyLog("时间值：", GlobalTime);
	float 武器前摇 = 0;
	auto 自身出蓄前摇 = 0.0f;  // 初始化

	武器前摇 += 距离计算(obj);

	//太刀百裂
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_(light|heavy)_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::太刀百裂;
		flag = false;
	}
	//太刀噬魂斩左右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_katana(_|_run_|_sprint_)attack_(light|heavy)_charge_soul_(01|02|03)")))
	{
		自身出蓄前摇 = BlueAttackFront::太刀噬魂斩;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		return false;
		break;
	}
}
//阔刀
static auto BladeBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//站走右蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade(_|_run_)attack_heavy_01_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::阔刀站走右蓄;
		flag = false;
	}
	//滑步右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_sprint_attack_heavy_01_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::阔刀滑步右蓄;
		flag = false;
	}
	//左蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_charge(_|_run_)(02|03|04)")))
	{
		if (LocalPlayer_Data.MyBlueTime > 0.5f && LocalPlayer_Data.MyBlueTime < 0.99f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄1段;
			flag = false;
		}
		else if (LocalPlayer_Data.MyBlueTime > 0.99f && LocalPlayer_Data.MyBlueTime < 0.149f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄2段;
			flag = false;
		}
		else if (LocalPlayer_Data.MyBlueTime > 0.149f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄3段;
			flag = false;
		}
	}
	//站走(包括风雷步)
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_charge_run_(02|03|04|start|)_(01|soul)")))
	{
		if (LocalPlayer_Data.MyBlueTime > 0.5f && LocalPlayer_Data.MyBlueTime < 0.99f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄1段;
			flag = false;
		}
		else if (LocalPlayer_Data.MyBlueTime > 0.99f && LocalPlayer_Data.MyBlueTime < 0.149f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄2段;
			flag = false;
		}
		else if (LocalPlayer_Data.MyBlueTime > 0.149f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左蓄3段;
			flag = false;
		}
	}
	//左右
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_heavy_03_(pre|charge)")))
	{
		if (LocalPlayer_Data.MyBlueTime < 0.49f)
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左右;
			flag = true;
		}
		else
		{
			自身出蓄前摇 = BlueAttackFront::阔刀左右捏蓄;
			flag = true;
		}
	}
	//右右
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_blade_attack_heavy_02_(pre|charge)")))
	{
		自身出蓄前摇 = BlueAttackFront::阔刀右右;
		flag = true;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//长枪
static auto SpearBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//长枪左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长枪左蓄;
		flag = false;
	}
	//穿心脚左蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_light_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长枪左蓄;
		flag = false;
	}

	//长枪右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长枪右蓄;
		flag = false;
	}
	//穿心脚右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_spear(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长枪右蓄;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//匕首
static auto DaggeBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//匕首左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::匕首左蓄;
		flag = false;
	}
	//匕首右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::匕首右蓄;
		flag = false;
	}
	//匕首亢龙有悔
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dagger(_|_run_|_sprint_)attack_light_(01|02|03)_charge_soul")))
	{
		自身出蓄前摇 = BlueAttackFront::匕首亢龙有悔;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//双节棍
static auto nunchucksBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//双节棍左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双节棍左蓄;
		flag = false;
	}
	//双节棍右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双节棍右蓄;
		flag = false;
	}
	//扬鞭劲
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_light_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双节棍扬鞭劲;
		flag = false;
	}
	//虎啸龙咆
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_nunchucks(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双节棍右蓄;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//双刀
static auto twinbladesBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0.011;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//双刀左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_charge_enter_idle_02")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀左蓄;
		flag = false;
	}
	//双刀右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀右蓄;
		flag = false;
	}
	//双刀走蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_light_charge_run_02")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀左蓄;
		flag = false;
	}
	//乾坤日月斩
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_twinblades(_|_run_|_sprint_)attack_heavy_charge_soul_(01|02|03)")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀右蓄;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇,武器前摇);
		break;
	default:
		break;
	}
}
//长棍
static auto rodBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//棍左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_light_01_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长棍左蓄;
		flag = false;
	}
	//棍右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_rod(_|_run_|_sprint_)attack_heavy_01_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::长棍右蓄;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//斩马刀
static auto SaberBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	if (LocalPlayer_Data.MyBlueTime > 0.5f)
	{
		//站走左蓄
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber(_|_run_)attack_light_01_charge")))
		{
			自身出蓄前摇 = BlueAttackFront::斩马刀左蓄;
			flag = false;
		}
		//滑步左蓄
		if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_sprint_attack_light_01_charge")))
		{
			自身出蓄前摇 = BlueAttackFront::斩马刀左蓄;
			flag = false;
		}
		//右蓄(left|right|back|front)
		else if ((regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_enter_idle_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run(_|_start_)(01|05)")) ||
			regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_turn_square_(left|right|back|front)_05")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run_turn_back_(left|right|back|front)_05"))))
		{
			自身出蓄前摇 = BlueAttackFront::斩马刀右蓄;
			flag = false;
		}
		//炽焰斩
		else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber(_|_sprint_|_run_)attack_light_soul_01_charge")))
		{
			自身出蓄前摇 = BlueAttackFront::斩马刀炽焰斩;
			flag = false;
		}
	}
	//左右
	if ((regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_heavy_03_pre")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_enter_idle_03")) ||
		regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run_start_01")) || regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_turn_square_(left|right|back|front)_05")) ||
		regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"(male|female)_saber_attack_charge_run_turn_back_(left|right|back|front)_05"))))
	{
		自身出蓄前摇 = BlueAttackFront::斩马刀左右;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//双戟
static auto dualhalberdBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//双刀左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_charge_enter_idle_02")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀左蓄;
		flag = false;
	}
	//双刀右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀右蓄;
		flag = false;
	}
	//双刀走蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_light_charge_run_02")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀左蓄;
		flag = false;
	}
	//乾坤日月斩
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_dualhalberd(_|_run_|_sprint_)attack_heavy_charge_soul_(01|02|03)")))
	{
		自身出蓄前摇 = BlueAttackFront::双刀乾坤日月斩;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//扇子
static auto fanBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//匕首左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::扇子左蓄;
		flag = false;
	}
	//匕首右蓄
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::拳刃右蓄;
		flag = false;
	}
	//匕首亢龙有悔
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_fan(_|_run_|_sprint_)attack_heavy_soul_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::扇子缠龙奔野;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	default:
		break;
	}
}
//横刀
static auto hengdaoBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//横刀左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::横刀左蓄;
		flag = false;
	}
	//横刀右蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::横刀右蓄;
		flag = false;
	}
	//横刀极光碎云闪
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_hengdao(_|_run_|_sprint_)attack_heavy_charge_soul_01")))
	{
		自身出蓄前摇 = BlueAttackFront::横刀极光碎云闪;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
	default:
		break;
	}
}
//拳刃
static auto punchBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//拳刃左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::拳刃左蓄;
		flag = false;
	}
	//拳刃右蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::拳刃右蓄;
		flag = false;
	}
	//拳刃太极弄云手
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_punch(_|_run_|_sprint_)attack_light_charge_soul_01")))
	{
		自身出蓄前摇 = BlueAttackFront::拳刃太极弄云手;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
	default:
		break;
	}
}
//飞刀
static auto knifeBlueAttackWrite(PlayerData obj)
{
	float 武器前摇 = 0;
	float 自身出蓄前摇 = 0.f;
	武器前摇 += 距离计算(obj);
	//拳刃左蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_light_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::飞刀左蓄;
		flag = false;
	}
	//拳刃右蓄
	if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_heavy_(01|02|03)_charge")))
	{
		自身出蓄前摇 = BlueAttackFront::飞刀右蓄;
		flag = false;
	}
	//拳刃
	else if (regex_match(LocalPlayer_Data.MyActionName.c_str(), wregex(L"male_knife(_|_run_|_sprint_)attack_light_charge_soul_01")))
	{
		自身出蓄前摇 = BlueAttackFront::飞刀断月千刃舞;
		flag = false;
	}

	switch (obj.O_ActorWeapon)
	{
	case 101:
		return SwAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 102:
		return KatanaAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 103:
		return BladeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 105:
		return SpearAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 106:
		return DaggeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 116:
		return nunchucksAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 118:
		return twinbladesAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 119:
		return SaberAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 120:
		return rodAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 121:
		return dualhalberdAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 122:
		return fanAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 123:
		return hengdaoAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 124:
		return punchAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 127:
		return KnifeAttack(obj, 自身出蓄前摇, 武器前摇);
		break;
	case 128:
		return LjAttack(obj, 自身出蓄前摇, 武器前摇);
	default:
		break;
	}
}
//出蓄抓白刀
static auto BlueAttackWriteA(PlayerData obj)
{

	switch (LocalPlayer_Data.MyWeaponType)
	{
	case 101:
		SwBlueAttackWrite(obj);
		break;
	case 102:
		KatanaBlueAttackWrite2(obj);
		break;
	case 103:
		BladeBlueAttackWrite(obj);
		break;
	case 105:
		SpearBlueAttackWrite(obj);
		break;
	case 106:
		DaggeBlueAttackWrite(obj);
		break;
	case 116:
		nunchucksBlueAttackWrite(obj);
		break;
	case 118:
		twinbladesBlueAttackWrite(obj);
		break;
	case 119:
		SaberBlueAttackWrite(obj);
		break;
	case 120:
		rodBlueAttackWrite(obj);
		break;
	case 121:
		dualhalberdBlueAttackWrite(obj);
		break;
	case 122:
		fanBlueAttackWrite(obj);
		break;
	case 123:
		hengdaoBlueAttackWrite(obj);
		break;
	case 124:
		punchBlueAttackWrite(obj);
		break;
	case 127:
		knifeBlueAttackWrite(obj);
		break;
	case 128:
		// 链剑

		break;
	default:
		break;
	}

}
