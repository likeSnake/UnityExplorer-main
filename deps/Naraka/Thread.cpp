#include "Data.h"
#include "Thread.h"
#include "Menu.h"
#include "Tool.h"
#include "log.h"
#include "Hook.h"
#include "ShockKnifeLogic.h"
#include "HeroActivation.h"
#include "SkillActivate.h"
#include "AttackShockKnife.h"
//#include "AttackWrite.h"
#include "aimbot.h"
#include "AttackDuanXu.h"
#include "autoCombo.h"
#include "FakeThread.h"
#include <shared_mutex>
#include <mutex>
#include <array>
#include <future>
#include <sstream> 
#include <mmsystem.h>
#include <powrprof.h>
#include "landing.h"
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "PowrProf.lib")
#ifndef GUID_STANDBYIDLE_TIMEOUT
static const GUID GUID_STANDBYIDLE_TIMEOUT = { 0x29f6c1db, 0x86da, 0x48c5, { 0x9f, 0xdb, 0xf2, 0xb6, 0x7b, 0x1f, 0x44, 0xda } };
#endif
#ifndef GUID_HIBERNATEIDLE_TIMEOUT
static const GUID GUID_HIBERNATEIDLE_TIMEOUT = { 0x9d7815a6, 0x7ee4, 0x497e, { 0x88, 0x88, 0x51, 0x5a, 0x05, 0xf0, 0x23, 0x64 } };
#endif
#ifndef GUID_HYBRID_SLEEP
static const GUID GUID_HYBRID_SLEEP = { 0x94ac6d29, 0x73ce, 0x41a6, { 0x80, 0x9f, 0x63, 0x63, 0xba, 0x21, 0xb4, 0x7e } };
#endif
static DWORD WINAPI Cache_Player_World_Data2(LPVOID lpParam);
static DWORD WINAPI LocalPlayer_Object_Data2(LPVOID lpParam);
static DWORD WINAPI GetItemData(LPVOID lpParam);
static DWORD WINAPI GetUserBattleData(LPVOID lpParam);
static DWORD WINAPI BonesTraverseThread(LPVOID lpParam);
static DWORD WINAPI SmarThread(LPVOID lpParam);
static DWORD WINAPI ShockKnifeThread(LPVOID lpParam);
static DWORD WINAPI HeroActivationThread(LPVOID lpParam);
static DWORD WINAPI LogicKnifeThread(LPVOID lpParam);
static DWORD WINAPI AimbotThread(LPVOID lpParam);
static DWORD WINAPI ErgodicThread(LPVOID lpParam);
#define 自瞄键 atoi(Function::AiMBot::AimKey)
static bool 是否连接Net = false;
static bool 是否HOOK = false;
std::vector<HANDLE> g_hThreads;  // 动态线程句柄容器
HANDLE g_hExitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
// 线程退出 专用锁
std::atomic<bool> g_bExit;
// 放蓄 专用锁
std::atomic<bool> g_xuli;
// 增加配置保存专用锁
std::mutex g_configMutex;
static UINT g_timePeriod = 1;
static std::once_flag g_timePeriodInitFlag;
static std::once_flag g_timePeriodStopFlag;

static void InitTimePeriodOnce() {
	TIMECAPS tc{};
	if (timeGetDevCaps(&tc, sizeof(tc)) == TIMERR_NOERROR) {
		g_timePeriod = tc.wPeriodMin;
	}
	timeBeginPeriod(g_timePeriod);
}

static void ApplyHighPriorityAndDisableThrottling()
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS); // HIGH_PRIORITY_CLASS   REALTIME_PRIORITY_CLASS 

#if defined(PROCESS_POWER_THROTTLING_CURRENT_VERSION) && defined(PROCESS_POWER_THROTTLING_EXECUTION_SPEED)
	PROCESS_POWER_THROTTLING_STATE state{};
	state.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	state.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
	state.StateMask = 0;
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &state, sizeof(state));
#endif
}

static void DisableThreadPowerThrottling(HANDLE hThread)
{
#if defined(THREAD_POWER_THROTTLING_CURRENT_VERSION) && defined(THREAD_POWER_THROTTLING_EXECUTION_SPEED)
	THREAD_POWER_THROTTLING_STATE state{};
	state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
	state.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
	state.StateMask = 0;
	SetThreadInformation(hThread, ThreadPowerThrottling, &state, sizeof(state));
#else
	(void)hThread;
#endif
}

static bool WritePowerSettingValues(const GUID& scheme, const GUID& subgroup, const GUID& setting, DWORD acValue, DWORD dcValue)
{
	DWORD status = PowerWriteACValueIndex(nullptr, &scheme, &subgroup, &setting, acValue);
	if (status != ERROR_SUCCESS) {
		return false;
	}
	status = PowerWriteDCValueIndex(nullptr, &scheme, &subgroup, &setting, dcValue);
	return status == ERROR_SUCCESS;
}

static void ApplyHighPerformancePowerPlanAndDisableLock()
{
	GUID scheme{};
	bool hasActiveScheme = false;
	bool isHighPerf = false;

	GUID* activeScheme = nullptr;
	if (PowerGetActiveScheme(nullptr, &activeScheme) == ERROR_SUCCESS && activeScheme) {
		scheme = *activeScheme;
		hasActiveScheme = true;
		isHighPerf = IsEqualGUID(scheme, GUID_MIN_POWER_SAVINGS);
		LocalFree(activeScheme);
		if (isHighPerf) {
			//MyLogTrue("电源模式已是高性能");
			return;
		}
	}

	if (!isHighPerf) {
		GUID highPerf = GUID_MIN_POWER_SAVINGS;
		DWORD setStatus = PowerSetActiveScheme(nullptr, &highPerf);
		if (setStatus == ERROR_SUCCESS) {
			scheme = highPerf;
			hasActiveScheme = true;
			//MyLogTrue("已设置电源方案高性能");
		} else if (!hasActiveScheme) {
			//MyLogTrue("读取电源方案失败");
			return;
		} else {
			//MyLogTrue("切换电源高性能失败");
		}
	}

	if (!hasActiveScheme) {
		return;
	}

	WritePowerSettingValues(scheme, GUID_VIDEO_SUBGROUP, GUID_VIDEO_POWERDOWN_TIMEOUT, 0, 0);
	WritePowerSettingValues(scheme, GUID_SLEEP_SUBGROUP, GUID_STANDBYIDLE_TIMEOUT, 0, 0);
	WritePowerSettingValues(scheme, GUID_SLEEP_SUBGROUP, GUID_HIBERNATEIDLE_TIMEOUT, 0, 0);
	WritePowerSettingValues(scheme, GUID_SLEEP_SUBGROUP, GUID_HYBRID_SLEEP, 0, 0);

	PowerSetActiveScheme(nullptr, &scheme);

	SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETSCREENSAVESECURE, 0, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
}
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	auto SafeSave = []() {
		std::lock_guard<std::mutex> lock(g_configMutex);
		// 提前保存配置
		if (Function::Shock::F_ShockKnife) {
			保存模拟振配置();
		}
		保存活化配置();
		保存其他参数();
		振刀逻辑::CollideKnife();
		SendKeyUpEx(KEY_W);
		SendKeyUpEx(KEY_S);
		SendKeyUpEx(KEY_A);
		SendKeyUpEx(KEY_D);
		SendKeyUpEx(KEY_F);
		SendKeyUpEx(KEY_G);
		SendKeyUpEx(KEY_LEFTSHIFT);
		SendKeyUpEx(KEY_SPACEBAR);
		};
	switch (dwCtrlType) {
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		// 立即执行保存操作
		SafeSave();
		// 恢复高精度到默认值
		std::call_once(g_timePeriodStopFlag, [] { timeEndPeriod(g_timePeriod); });
		// 设置原子退出标志
		g_bExit.store(true, std::memory_order_release);
		// 触发事件通知线程退出
		if (g_hExitEvent) {
			::SetEvent(g_hExitEvent);
		}
		return TRUE;
	}
	return FALSE;
}
void 链剑参数初始化()
{
	振刀参数::距离::链剑.左3 = 7.0f;
	振刀参数::距离::链剑.右3 = 7.0f;
	振刀参数::距离::链剑.左蓄力 = 12.f;
	振刀参数::距离::链剑.右蓄力 = 12.f;
	振刀参数::距离::链剑.地龙滚堂刹 = 16.f;
	振刀参数::距离::链剑.火龙卷云 = 14.f;
	振刀参数::距离::链剑.断罪碎蜂 = 14.5f;
	振刀参数::距离::链剑.壁击 = 14.f;
	振刀参数::距离::链剑.蓄力追击 = 7.f;
	振刀参数::角度::链剑.左3 = 90.f;
	振刀参数::角度::链剑.右3 = 90.f;
	振刀参数::角度::链剑.左蓄力 = 90.f;
	振刀参数::角度::链剑.右蓄力 = 90.f;
	振刀参数::角度::链剑.地龙滚堂刹 = 90.f;
	振刀参数::角度::链剑.火龙卷云 = 90.f;
	振刀参数::角度::链剑.断罪碎蜂 = 90.5f;
	振刀参数::角度::链剑.壁击 = 90.f;
	振刀参数::角度::链剑.蓄力追击 = 90.f;
	振刀参数::延迟::链剑.左3 = 0.25f;
	振刀参数::延迟::链剑.右3 = 0.249f;
	振刀参数::延迟::链剑.左蓄力 = 0.13f;
	振刀参数::延迟::链剑.右蓄力 = 0.1f;
	振刀参数::延迟::链剑.地龙滚堂刹 = 0.f;
	振刀参数::延迟::链剑.火龙卷云 = 0.f;
	振刀参数::延迟::链剑.断罪碎蜂 = 0.5f;
	振刀参数::延迟::链剑.壁击 = 0.f;
	振刀参数::延迟::链剑.蓄力追击 = 0.f;
}
void 初始化振刀方式参数()
{
	// { 0"拼刀", 1"站震", 2"跳震", 3"后闪震", 4"闪避" };
	振刀参数::振刀方式::长剑.左3 =1;
	振刀参数::振刀方式::长剑.右3 = 1;
	振刀参数::振刀方式::长剑.左剑气1 = 1;
	振刀参数::振刀方式::长剑.左剑气2 = 1;
	振刀参数::振刀方式::长剑.右剑气2 = 1;
	振刀参数::振刀方式::长剑.右剑气1 = 1;
	振刀参数::振刀方式::长剑.凤凰羽1 = 1;
	振刀参数::振刀方式::长剑.凤凰羽2 = 1;
	振刀参数::振刀方式::长剑.苍牙 = 1;
	振刀参数::振刀方式::长剑.跳斩 = 1;
	振刀参数::振刀方式::长剑.壁击 = 1;
	振刀参数::振刀方式::长剑.七星夺窍 = 1;
	振刀参数::振刀方式::长剑.天外飞仙 = 1;
	振刀参数::振刀方式::链剑.左3 = 1;
	振刀参数::振刀方式::链剑.右3 = 1;
	振刀参数::振刀方式::链剑.左蓄力1 = 6;
	振刀参数::振刀方式::链剑.左蓄力2 = 1;
	振刀参数::振刀方式::链剑.右蓄力2 = 1;
	振刀参数::振刀方式::链剑.右蓄力1 = 6;
	振刀参数::振刀方式::链剑.地龙滚堂刹 = 1;
	振刀参数::振刀方式::链剑.苍牙 = 1;
	振刀参数::振刀方式::链剑.跳斩 = 1;
	振刀参数::振刀方式::链剑.壁击 = 1;
	振刀参数::振刀方式::链剑.火龙卷云 = 1;
	振刀参数::振刀方式::链剑.断罪碎蜂 = 1;
	振刀参数::振刀方式::链剑.蓄力追击 = 1;
	振刀参数::振刀方式::太刀.左3 = 1;
	振刀参数::振刀方式::太刀.右3 = 1;
	振刀参数::振刀方式::太刀.左蓄1 = 7;
	振刀参数::振刀方式::太刀.左蓄2 = 7;
	振刀参数::振刀方式::太刀.右蓄2 = 7;
	振刀参数::振刀方式::太刀.右蓄1 = 7;
	振刀参数::振刀方式::太刀.刹那斩1 = 1;
	振刀参数::振刀方式::太刀.苍牙 = 1;
	振刀参数::振刀方式::太刀.惊雷 = 6;
	振刀参数::振刀方式::太刀.壁击 = 7;
	振刀参数::振刀方式::太刀.青鬼 = 1;
	振刀参数::振刀方式::太刀.同源 = 1;
	振刀参数::振刀方式::太刀.噬魂斩1 = 1;
	振刀参数::振刀方式::太刀.噬魂斩2 = 1;
	振刀参数::振刀方式::阔刀.左左 = 6;
	振刀参数::振刀方式::阔刀.左右 = 7;
	振刀参数::振刀方式::阔刀.右右 = 6;
	振刀参数::振刀方式::阔刀.右左 = 6;
	振刀参数::振刀方式::阔刀.左蓄一段 = 6;
	振刀参数::振刀方式::阔刀.左蓄二段 = 6;
	振刀参数::振刀方式::阔刀.左蓄三段 = 6;
	振刀参数::振刀方式::阔刀.右蓄1 = 3;
	振刀参数::振刀方式::阔刀.右蓄2 = 3;
	振刀参数::振刀方式::阔刀.右右蓄 = 6;
	振刀参数::振刀方式::阔刀.雷刀 = 1;
	振刀参数::振刀方式::阔刀.壁击 = 1;
	振刀参数::振刀方式::阔刀.同源 = 1;
	振刀参数::振刀方式::阔刀.雷刀下劈 = 1;
	振刀参数::振刀方式::阔刀.翻江倒海 = 1;
	振刀参数::振刀方式::长枪.左3 = 1;
	振刀参数::振刀方式::长枪.右3 = 1;
	振刀参数::振刀方式::长枪.左蓄 = 3;
	振刀参数::振刀方式::长枪.六合枪 = 1;
	振刀参数::振刀方式::长枪.右蓄 = 6;
	振刀参数::振刀方式::长枪.大圣游 = 6;
	振刀参数::振刀方式::长枪.风卷云残 = 1;
	振刀参数::振刀方式::长枪.壁击 = 1;
	振刀参数::振刀方式::长枪.同源 = 1;
	振刀参数::振刀方式::长枪.双环扫 = 1;
	振刀参数::振刀方式::长枪.龙王破 = 1;
	振刀参数::振刀方式::长枪.穿心脚 = 1;
	振刀参数::振刀方式::匕首.左3 = 1;
	振刀参数::振刀方式::匕首.右3 = 1;
	振刀参数::振刀方式::匕首.左蓄 = 1;
	振刀参数::振刀方式::匕首.右蓄 = 1;
	振刀参数::振刀方式::匕首.鬼反 = 1;
	振刀参数::振刀方式::匕首.荆轲献匕 = 1;
	振刀参数::振刀方式::匕首.壁击 = 1;
	振刀参数::振刀方式::匕首.鬼刃暗扎 = 1;
	振刀参数::振刀方式::匕首.鬼哭神嚎 = 1;
	振刀参数::振刀方式::匕首.亢龙有悔 = 1;
	振刀参数::振刀方式::双截棍.左右3 = 1;
	振刀参数::振刀方式::双截棍.左蓄 = 3;
	振刀参数::振刀方式::双截棍.右蓄 = 7;
	振刀参数::振刀方式::双截棍.飞踢 = 1;
	振刀参数::振刀方式::双截棍.壁击 = 1;
	振刀参数::振刀方式::双截棍.扬鞭劲 = 7;
	振刀参数::振刀方式::双截棍.横栏 = 3;
	振刀参数::振刀方式::双截棍.龙虎乱舞 = 1;
	振刀参数::振刀方式::双截棍.三龙灭阳棍 = 1;
	振刀参数::振刀方式::双刀.左右3 = 1;
	振刀参数::振刀方式::双刀.左蓄 = 6;
	振刀参数::振刀方式::双刀.右蓄1 = 7;
	振刀参数::振刀方式::双刀.右蓄2 = 7;
	振刀参数::振刀方式::双刀.铁马残红 = 1;
	振刀参数::振刀方式::双刀.壁击 = 1;
	振刀参数::振刀方式::双刀.乾坤日月斩 = 7;
	振刀参数::振刀方式::双刀.惊雷 = 6;
	振刀参数::振刀方式::双刀.八斩刀 = 1;
	振刀参数::振刀方式::双刀.分水斩 = 1;
	振刀参数::振刀方式::长棍.右3 = 1;
	振刀参数::振刀方式::长棍.左3 = 1;
	振刀参数::振刀方式::长棍.左蓄 = 1;
	振刀参数::振刀方式::长棍.右蓄 = 1;
	振刀参数::振刀方式::长棍.壁击 = 1;
	振刀参数::振刀方式::长棍.同源 = 1;
	振刀参数::振刀方式::长棍.腾云式 = 1;
	振刀参数::振刀方式::长棍.乱点天宫 = 1;
	振刀参数::振刀方式::长棍.桶劲 = 7;
	振刀参数::振刀方式::长棍.五情七灭镇 = 1;
	振刀参数::振刀方式::长棍.双环扫 = 1;
	振刀参数::振刀方式::长棍.少林棍 = 1;
	振刀参数::振刀方式::长棍.镇地撑天 = 7;
	振刀参数::振刀方式::斩马刀.左左 = 1;
	振刀参数::振刀方式::斩马刀.左右 = 1;
	振刀参数::振刀方式::斩马刀.右右 = 1;
	振刀参数::振刀方式::斩马刀.右左 = 1;
	振刀参数::振刀方式::斩马刀.左蓄1 = 3;
	振刀参数::振刀方式::斩马刀.左蓄2 = 3;
	振刀参数::振刀方式::斩马刀.左蓄3 = 3;
	振刀参数::振刀方式::斩马刀.右蓄1 = 6;
	振刀参数::振刀方式::斩马刀.右蓄2 = 6;
	振刀参数::振刀方式::斩马刀.右蓄3 = 6;
	振刀参数::振刀方式::斩马刀.柄击 = 1;
	振刀参数::振刀方式::斩马刀.壁击 = 1;
	振刀参数::振刀方式::斩马刀.同源 = 1;
	振刀参数::振刀方式::斩马刀.惊雷 = 1;
	振刀参数::振刀方式::斩马刀.炽焰斩 = 7;
	振刀参数::振刀方式::斩马刀.过关斩将 = 1;
	振刀参数::振刀方式::斩马刀.奔雷入阵 = 1;
	振刀参数::振刀方式::双戟.左3 = 1;
	振刀参数::振刀方式::双戟.右3 = 1;
	振刀参数::振刀方式::双戟.左蓄 = 6;
	振刀参数::振刀方式::双戟.右蓄1 = 7;
	振刀参数::振刀方式::双戟.右蓄2 = 7;
	振刀参数::振刀方式::双戟.惊雷 = 1;
	振刀参数::振刀方式::双戟.勾旋斩 = 1;
	振刀参数::振刀方式::双戟.探海蛟 = 1;
	振刀参数::振刀方式::双戟.钩挂 = 1;
	振刀参数::振刀方式::双戟.壁击 = 1;
	振刀参数::振刀方式::双戟.战龙在天 = 1;
	振刀参数::振刀方式::双戟.形意钩 = 7;
	振刀参数::振刀方式::扇子.左3 = 1;
	振刀参数::振刀方式::扇子.右3 = 1;
	振刀参数::振刀方式::扇子.左蓄 = 6;
	振刀参数::振刀方式::扇子.右蓄 = 7;
	振刀参数::振刀方式::扇子.双开圆 = 1;
	振刀参数::振刀方式::扇子.三风摆 = 1;
	振刀参数::振刀方式::扇子.鬼反 = 1;
	振刀参数::振刀方式::扇子.惊雷 = 1;
	振刀参数::振刀方式::扇子.壁击 = 1;
	振刀参数::振刀方式::扇子.缠龙奔野 = 1;
	振刀参数::振刀方式::扇子.泽风上六 = 1;
	振刀参数::振刀方式::横刀.左3 = 1;
	振刀参数::振刀方式::横刀.右3 = 1;
	振刀参数::振刀方式::横刀.左蓄 = 1;
	振刀参数::振刀方式::横刀.右蓄1 = 7;
	振刀参数::振刀方式::横刀.右蓄2 = 7;
	振刀参数::振刀方式::横刀.苍牙 = 1;
	振刀参数::振刀方式::横刀.壁击 = 1;
	振刀参数::振刀方式::横刀.同源 = 1;
	振刀参数::振刀方式::横刀.踏空闪 = 1;
	振刀参数::振刀方式::横刀.破千军 = 1;
	振刀参数::振刀方式::横刀.极光破云闪 = 1;
	振刀参数::振刀方式::横刀.乾坤一掷 = 1;
	振刀参数::振刀方式::拳刃.左3 = 1;
	振刀参数::振刀方式::拳刃.右3 = 1;
	振刀参数::振刀方式::拳刃.左蓄力1 = 6;
	振刀参数::振刀方式::拳刃.左蓄力2 = 6;
	振刀参数::振刀方式::拳刃.右蓄力1 = 6;
	振刀参数::振刀方式::拳刃.右蓄力2 = 6;
	振刀参数::振刀方式::拳刃.右蓄力2段 = 6;
	振刀参数::振刀方式::拳刃.百裂腿 = 6;
	振刀参数::振刀方式::拳刃.破空拳 = 7;
	振刀参数::振刀方式::拳刃.苍牙 = 1;
	振刀参数::振刀方式::拳刃.壁击 = 1;
	振刀参数::振刀方式::拳刃.太极弄云手 = 1;
	振刀参数::振刀方式::飞刀.左3 = 1;
	振刀参数::振刀方式::飞刀.右3 = 1;
	振刀参数::振刀方式::飞刀.左蓄力 = 6;
	振刀参数::振刀方式::飞刀.左蓄力2段 = 1;
	振刀参数::振刀方式::飞刀.右蓄力 = 6;
	振刀参数::振刀方式::飞刀.掌心雷 = 1;
	振刀参数::振刀方式::飞刀.苍牙 = 1;
	振刀参数::振刀方式::飞刀.太极 = 1;
	振刀参数::振刀方式::飞刀.壁击 = 1;
	振刀参数::振刀方式::飞刀.断月千刃舞 = 1;
	振刀参数::振刀方式::飞刀.万点寒梅 = 7;
}
void 初始化振刀距离参数数据()
{
	振刀参数::距离::长剑.左3 = 7.0f;
	振刀参数::距离::长剑.右3 = 7.0f;
	振刀参数::距离::长剑.左剑气 = 12.f;
	振刀参数::距离::长剑.右剑气 = 12.f;
	振刀参数::距离::长剑.凤凰羽 = 14.f;
	振刀参数::距离::长剑.苍牙 = 14.f;
	振刀参数::距离::长剑.跳斩 = 5.5f;
	振刀参数::距离::长剑.壁击 = 14.f;
	振刀参数::距离::长剑.七星夺窍 = 12.f;
	振刀参数::距离::长剑.天外飞仙 = 15.f;
	振刀参数::距离::链剑.左3 = 7.0f;
	振刀参数::距离::链剑.右3 = 7.0f;
	振刀参数::距离::链剑.左蓄力 = 12.f;
	振刀参数::距离::链剑.右蓄力 = 12.f;
	振刀参数::距离::链剑.地龙滚堂刹 = 16.f;
	振刀参数::距离::链剑.火龙卷云 = 14.f;
	振刀参数::距离::链剑.断罪碎蜂 = 14.5f;
	振刀参数::距离::链剑.壁击 = 14.f;
	振刀参数::距离::链剑.蓄力追击 = 7.f;
	振刀参数::距离::太刀.左3 = 6.5f;
	振刀参数::距离::太刀.右3 = 6.5f;
	振刀参数::距离::太刀.左蓄 = 5.3f;
	振刀参数::距离::太刀.右蓄 = 5.3f;
	振刀参数::距离::太刀.苍牙 = 5.f;
	振刀参数::距离::太刀.刹那斩 = 5.5f;
	振刀参数::距离::太刀.壁击 = 14.f;
	振刀参数::距离::太刀.惊雷 = 7.5f;
	振刀参数::距离::太刀.惊雷十劫 = 10.5f;	
	振刀参数::距离::太刀.青鬼 = 6.0f;
	振刀参数::距离::太刀.同源 = 7.5f;
	振刀参数::距离::太刀.噬魂斩 = 12.5f;
	振刀参数::距离::阔刀.左左 = 6.5f;
	振刀参数::距离::阔刀.左右 = 5.5f;
	振刀参数::距离::阔刀.右右 = 4.2f;
	振刀参数::距离::阔刀.右左 = 4.3f;
	振刀参数::距离::阔刀.壁击 = 10.5f;
	振刀参数::距离::阔刀.左蓄一段 = 7.f;
	振刀参数::距离::阔刀.左蓄二段 = 7.f;
	振刀参数::距离::阔刀.左蓄三段 = 7.f;
	振刀参数::距离::阔刀.右蓄 = 7.f;
	振刀参数::距离::阔刀.右右蓄 = 8.5f;
	振刀参数::距离::阔刀.雷刀 = 5.f;
	振刀参数::距离::阔刀.雷刀下劈 = 4.5f;
	振刀参数::距离::阔刀.同源 = 7.5f;
	振刀参数::距离::长枪.左3 = 4.5f;
	振刀参数::距离::长枪.右3 = 4.5f;
	振刀参数::距离::长枪.左蓄 = 4.5f;
	振刀参数::距离::长枪.六合枪 = 8.5f;
	振刀参数::距离::长枪.右蓄 = 8.3f;
	振刀参数::距离::长枪.大圣游 = 6.2f;
	振刀参数::距离::长枪.同源 = 6.2f;
	振刀参数::距离::长枪.壁击 = 12.5f;
	振刀参数::距离::长枪.风卷云残 = 5.f;
	振刀参数::距离::长枪.双环扫 = 4.5f;
	振刀参数::距离::长枪.龙王破 = 13.f;
	振刀参数::距离::长枪.穿心脚 = 10.f;
	振刀参数::距离::匕首.左3 = 4.5f;
	振刀参数::距离::匕首.右3 = 4.f;
	振刀参数::距离::匕首.左蓄 = 5.8f;
	振刀参数::距离::匕首.右蓄 = 4.f;
	振刀参数::距离::匕首.鬼反 = 5.5f;
	振刀参数::距离::匕首.壁击 = 12.5f;
	振刀参数::距离::匕首.荆轲献匕 = 8.f;
	振刀参数::距离::匕首.鬼刃暗扎 = 6.5f;
	振刀参数::距离::匕首.鬼哭神嚎 = 6.f;
	振刀参数::距离::匕首.亢龙有悔 = 10.f;
	振刀参数::距离::双截棍.左右3 = 4.5f;
	振刀参数::距离::双截棍.左蓄 = 6.f;
	振刀参数::距离::双截棍.壁击 = 12.5f;
	振刀参数::距离::双截棍.右蓄 = 4.5f;
	振刀参数::距离::双截棍.飞踢 = 4.5f;
	振刀参数::距离::双截棍.扬鞭劲 = 4.5f;
	振刀参数::距离::双截棍.横栏 = 3.5f;
	振刀参数::距离::双截棍.龙虎乱舞 = 6.f;
	振刀参数::距离::双刀.左右3 = 4.5f;
	振刀参数::距离::双刀.左蓄 = 5.5f;
	振刀参数::距离::双刀.右蓄 = 4.8f;
	振刀参数::距离::双刀.壁击 = 12.5f;
	振刀参数::距离::双刀.铁马残红 = 5.f;
	振刀参数::距离::双刀.乾坤日月斩 = 9.f;
	振刀参数::距离::双刀.惊雷 = 4.5f;
	振刀参数::距离::双刀.八斩刀 = 6.f;
	振刀参数::距离::双刀.分水斩 = 4.5f;
	振刀参数::距离::长棍.左3 = 4.5f;
	振刀参数::距离::长棍.右3 = 4.5f;
	振刀参数::距离::长棍.少林棍 = 4.5f;
	振刀参数::距离::长棍.左蓄 = 5.7f;
	振刀参数::距离::长棍.右蓄 = 6.8f;
	振刀参数::距离::长棍.壁击 = 12.5f;
	振刀参数::距离::长棍.腾云式 = 7.5f;
	振刀参数::距离::长棍.乱点天宫 = 5.f;
	振刀参数::距离::长棍.桶劲 = 8.f;
	振刀参数::距离::长棍.同源 = 5.7f;
	振刀参数::距离::长棍.五情七灭镇 = 13.f;
	振刀参数::距离::长棍.双环扫 = 4.5f;
	振刀参数::距离::长棍.镇地撑天 = 5.5f;
	振刀参数::距离::斩马刀.左左 = 6.f;
	振刀参数::距离::斩马刀.左右 = 5.5f;
	振刀参数::距离::斩马刀.右右 = 4.f;
	振刀参数::距离::斩马刀.右左 = 4.5f;
	振刀参数::距离::斩马刀.左蓄 = 5.f;
	振刀参数::距离::斩马刀.右蓄 = 5.8f;
	振刀参数::距离::斩马刀.柄击 = 6.5f;
	振刀参数::距离::斩马刀.惊雷 = 5.f;
	振刀参数::距离::斩马刀.同源 = 5.f;
	振刀参数::距离::斩马刀.壁击 = 12.5f;
	振刀参数::距离::斩马刀.炽焰斩 = 7.f;
	振刀参数::距离::斩马刀.过关斩将 = 8.3f;
	振刀参数::距离::斩马刀.奔雷入阵 = 4.f;
	振刀参数::距离::双戟.左右3 = 4.5f;
	振刀参数::距离::双戟.左蓄 = 5.f;
	振刀参数::距离::双戟.右蓄 = 6.f;
	振刀参数::距离::双戟.勾旋斩 = 5.f;
	振刀参数::距离::双戟.探海蛟 = 7.f;
	振刀参数::距离::双戟.钩挂 = 6.f;
	振刀参数::距离::双戟.壁击 = 12.5f;
	振刀参数::距离::双戟.战龙在天 = 6.4f;
	振刀参数::距离::双戟.形意钩 = 11.f;
	振刀参数::距离::扇子.左3 = 4.5f;
	振刀参数::距离::扇子.右3 = 4.f;
	振刀参数::距离::扇子.左蓄 = 8.f;
	振刀参数::距离::扇子.右蓄 = 8.f;
	振刀参数::距离::扇子.双开圆 = 6.f;
	振刀参数::距离::扇子.三风摆 = 9.5f;
	振刀参数::距离::扇子.鬼反 = 6.f;
	振刀参数::距离::扇子.惊雷 = 5.5f;
	振刀参数::距离::扇子.壁击 = 12.5f;
	振刀参数::距离::扇子.缠龙奔野 = 11.5f;
	振刀参数::距离::扇子.泽风上六 = 5.5f;
	振刀参数::距离::横刀.左3 = 4.5f;
	振刀参数::距离::横刀.右3 = 4.8f;
	振刀参数::距离::横刀.左蓄 = 5.f;
	振刀参数::距离::横刀.右蓄 = 6.5f;
	振刀参数::距离::横刀.苍牙 = 5.5f;
	振刀参数::距离::横刀.壁击 = 12.5f;
	振刀参数::距离::横刀.同源 = 6.5f;
	振刀参数::距离::横刀.踏空闪 = 4.5f;
	振刀参数::距离::横刀.破千军 = 5.f;
	振刀参数::距离::横刀.乾坤一掷 = 10.f;
	振刀参数::距离::横刀.极光破云闪 = 8.f;
	振刀参数::距离::拳刃.左3 = 4.5f;
	振刀参数::距离::拳刃.右3 = 4.5f;
	振刀参数::距离::拳刃.左蓄力 = 4.f;
	振刀参数::距离::拳刃.右蓄力 = 8.f;
	振刀参数::距离::拳刃.右蓄力2段 = 8.f;
	振刀参数::距离::拳刃.苍牙 = 4.5f;
	振刀参数::距离::拳刃.百裂腿 = 5.5f;
	振刀参数::距离::拳刃.破空拳 = 4.5f;
	振刀参数::距离::拳刃.壁击 = 12.5f;
	振刀参数::距离::拳刃.太极弄云手 = 8.f;
	振刀参数::距离::飞刀.左3 = 4.5f;
	振刀参数::距离::飞刀.右3 = 4.5f;
	振刀参数::距离::飞刀.左蓄力 = 9.5f;
	振刀参数::距离::飞刀.左蓄力2段 = 13.5f;
	振刀参数::距离::飞刀.右蓄力 = 13.5f;
	振刀参数::距离::飞刀.掌心雷 = 14.5f;
	振刀参数::距离::飞刀.苍牙 = 5.5f;
	振刀参数::距离::飞刀.太极 = 7.5f;
	振刀参数::距离::飞刀.壁击 = 12.5f;
	振刀参数::距离::飞刀.断月千刃舞 = 14.5f;
	振刀参数::距离::飞刀.万点寒梅 = 7.5f;
}
void 初始化振刀角度参数数据()
{
	振刀参数::角度::长剑.左3 = 90.f;
	振刀参数::角度::长剑.右3 = 90.f;
	振刀参数::角度::长剑.左剑气 = 90.f;
	振刀参数::角度::长剑.右剑气 = 60.f;
	振刀参数::角度::长剑.凤凰羽 = 90.f;
	振刀参数::角度::长剑.苍牙 = 90.f;
	振刀参数::角度::长剑.跳斩 = 360.f;
	振刀参数::角度::长剑.壁击 = 90.f;
	振刀参数::角度::长剑.七星夺窍 = 45.f;
	振刀参数::角度::长剑.天外飞仙 = 120.f;
	振刀参数::角度::链剑.左3 = 90.f;
	振刀参数::角度::链剑.右3 = 90.f;
	振刀参数::角度::链剑.左蓄力 = 360.f;
	振刀参数::角度::链剑.右蓄力 = 360.f;
	振刀参数::角度::链剑.地龙滚堂刹 = 360.f;
	振刀参数::角度::链剑.火龙卷云 = 360.f;
	振刀参数::角度::链剑.断罪碎蜂 = 360.5f;
	振刀参数::角度::链剑.壁击 = 360.f;
	振刀参数::角度::链剑.蓄力追击 = 360.f;
	振刀参数::角度::太刀.左3 = 90.f;
	振刀参数::角度::太刀.右3 = 90.f;
	振刀参数::角度::太刀.左蓄  = 360.f;
	振刀参数::角度::太刀.右蓄 = 360.f;
	振刀参数::角度::太刀.苍牙 = 90.f;
	振刀参数::角度::太刀.壁击 = 360.f;
	振刀参数::角度::太刀.刹那斩 = 120.f;
	振刀参数::角度::太刀.惊雷十劫 = 90.f;
	振刀参数::角度::太刀.青鬼 = 90.f;
	振刀参数::角度::太刀.同源 = 90.f;
	振刀参数::角度::太刀.噬魂斩 = 360.f;
	振刀参数::角度::阔刀.左左 = 90.f;
	振刀参数::角度::阔刀.左右 = 360.f;
	振刀参数::角度::阔刀.右右 = 360.f;
	振刀参数::角度::阔刀.右左 = 90.f;
	振刀参数::角度::阔刀.壁击 = 360.f;
	振刀参数::角度::阔刀.同源 = 90.f;
	振刀参数::角度::阔刀.左蓄一段 = 90.f;
	振刀参数::角度::阔刀.左蓄二段 = 90.f;
	振刀参数::角度::阔刀.左蓄三段 = 190.f;
	振刀参数::角度::阔刀.右蓄 = 360.f;
	振刀参数::角度::阔刀.右右蓄= 90.f;
	振刀参数::角度::长枪.左3 = 90.f;
	振刀参数::角度::长枪.右3 = 90.f;
	振刀参数::角度::长枪.左蓄 = 90.f;
	振刀参数::角度::长枪.六合枪 = 90.f;
	振刀参数::角度::长枪.右蓄 = 90.f;
	振刀参数::角度::长枪.壁击 = 90.f;
	振刀参数::角度::长枪.同源 = 90.f;
	振刀参数::角度::长枪.大圣游 = 90.f;
	振刀参数::角度::长枪.双环扫 = 90.f;
	振刀参数::角度::长枪.风卷云残 = 90.f;
	振刀参数::角度::长枪.龙王破 = 360.f;
	振刀参数::角度::长枪.穿心脚 = 360.f;
	振刀参数::角度::匕首.左3 = 90.f;
	振刀参数::角度::匕首.右3 = 90.f;
	振刀参数::角度::匕首.左蓄 = 90.f;
	振刀参数::角度::匕首.右蓄 = 90.f;
	振刀参数::角度::匕首.鬼反 = 90.f;
	振刀参数::角度::匕首.壁击 = 90.f;
	振刀参数::角度::匕首.荆轲献匕 = 90.f;
	振刀参数::角度::匕首.鬼刃暗扎 = 90.f;
	振刀参数::角度::匕首.亢龙有悔 = 90.f;
	振刀参数::角度::双截棍.左右3 = 90.f;
	振刀参数::角度::双截棍.左蓄 = 90.f;
	振刀参数::角度::双截棍.右蓄 = 90.f;
	振刀参数::角度::双截棍.飞踢 = 90.f;
	振刀参数::角度::双截棍.壁击 = 90.f;
	振刀参数::角度::双截棍.扬鞭劲 = 90.f;
	振刀参数::角度::双截棍.横栏 = 60.f;
	振刀参数::角度::双刀.左右3 = 90.f;
	振刀参数::角度::双刀.左蓄 = 60.f;
	振刀参数::角度::双刀.右蓄 = 90.f;
	振刀参数::角度::双刀.壁击 = 90.f;
	振刀参数::角度::双刀.分水斩 = 90.f;
	振刀参数::角度::双刀.铁马残红 = 90.f;
	振刀参数::角度::双刀.乾坤日月斩 = 360.f;
	振刀参数::角度::双刀.惊雷 = 90.f;
	振刀参数::角度::双刀.八斩刀 = 60.f;
	振刀参数::角度::长棍.左3 = 90.f;
	振刀参数::角度::长棍.右3 = 90.f;
	振刀参数::角度::长棍.左蓄 = 90.f;
	振刀参数::角度::长棍.右蓄 = 90.f;
	振刀参数::角度::长棍.壁击 = 90.f;
	振刀参数::角度::长棍.腾云式 = 90.f;
	振刀参数::角度::长棍.乱点天宫 = 90.f;
	振刀参数::角度::长棍.桶劲 = 360.f;
	振刀参数::角度::长棍.同源 = 90.f;
	振刀参数::角度::长棍.五情七灭镇 = 90.f;
	振刀参数::角度::长棍.双环扫 = 90.f;
	振刀参数::角度::长棍.少林棍 = 90.f;
	振刀参数::角度::长棍.镇地撑天 = 120.f;
	振刀参数::角度::斩马刀.左左 = 90.f;
	振刀参数::角度::斩马刀.左右 = 90.f;
	振刀参数::角度::斩马刀.右右 = 30.f;
	振刀参数::角度::斩马刀.右左 = 90.f;
	振刀参数::角度::斩马刀.左蓄 = 360.f;
	振刀参数::角度::斩马刀.右蓄 = 360.f;
	振刀参数::角度::斩马刀.柄击 = 90.f;
	振刀参数::角度::斩马刀.壁击 = 90.f;
	振刀参数::角度::斩马刀.同源 = 90.f;
	振刀参数::角度::斩马刀.惊雷 = 90.f;
	振刀参数::角度::斩马刀.炽焰斩 = 360.f;
	振刀参数::角度::斩马刀.过关斩将 = 360.f;
	振刀参数::角度::斩马刀.奔雷入阵 = 360.f;
	振刀参数::角度::双戟.左右3 = 90.f;
	振刀参数::角度::双戟.左蓄 = 90.f;
	振刀参数::角度::双戟.右蓄= 90.f;
	振刀参数::角度::双戟.勾旋斩 = 90.f;
	振刀参数::角度::双戟.探海蛟 = 360.f;
	振刀参数::角度::双戟.钩挂 = 90.f;
	振刀参数::角度::双戟.壁击 = 90.f;
	振刀参数::角度::双戟.战龙在天 = 90.f;
	振刀参数::角度::双戟.形意钩 = 90.f;
	振刀参数::角度::扇子.左3 = 90.f;
	振刀参数::角度::扇子.右3 = 90.f;
	振刀参数::角度::扇子.左蓄 = 90.f;
	振刀参数::角度::扇子.右蓄 = 90.f;
	振刀参数::角度::扇子.双开圆 = 90.f;
	振刀参数::角度::扇子.三风摆 = 90.f;
	振刀参数::角度::扇子.鬼反 = 90.f;
	振刀参数::角度::扇子.惊雷 = 90.f;
	振刀参数::角度::扇子.壁击 = 90.f;
	振刀参数::角度::扇子.缠龙奔野 = 90.f;
	振刀参数::角度::扇子.泽风上六 = 90.f;
	振刀参数::角度::横刀.左3 = 90.f;
	振刀参数::角度::横刀.右3 = 90.f;
	振刀参数::角度::横刀.左蓄 = 90.f;
	振刀参数::角度::横刀.右蓄 = 90.f;
	振刀参数::角度::横刀.苍牙 = 90.f;
	振刀参数::角度::横刀.壁击 = 90.f;
	振刀参数::角度::横刀.同源 = 90.f;
	振刀参数::角度::横刀.踏空闪 = 360.f;
	振刀参数::角度::横刀.破千军 = 90.f;
	振刀参数::角度::横刀.极光破云闪 = 180.f;
	振刀参数::角度::横刀.乾坤一掷 = 180.f;
	振刀参数::角度::拳刃.左3 = 90.f;
	振刀参数::角度::拳刃.右3 = 90.f;
	振刀参数::角度::拳刃.左蓄力 = 90.f;
	振刀参数::角度::拳刃.右蓄力 = 90.f;
	振刀参数::角度::拳刃.右蓄力2段 = 90.f;
	振刀参数::角度::拳刃.苍牙 = 90.f;
	振刀参数::角度::拳刃.壁击 = 90.f;
	振刀参数::角度::拳刃.百裂腿 = 90.f;
	振刀参数::角度::拳刃.破空拳 = 90.f;
	振刀参数::角度::拳刃.太极弄云手 = 360.f;
	振刀参数::角度::飞刀.左3 = 90.f;
	振刀参数::角度::飞刀.右3 = 90.f;
	振刀参数::角度::飞刀.左蓄力 = 90.f;
	振刀参数::角度::飞刀.左蓄力2段 = 90.f;
	振刀参数::角度::飞刀.右蓄力 = 90.f;
	振刀参数::角度::飞刀.苍牙 = 90.f;
	振刀参数::角度::飞刀.掌心雷 = 360.f;
	振刀参数::角度::飞刀.太极 = 90.f;
	振刀参数::角度::飞刀.壁击 = 90.f;
	振刀参数::角度::飞刀.断月千刃舞 = 90.f;
	振刀参数::角度::飞刀.万点寒梅 = 90.f;
}
void 初始化振刀延迟参数数据()
{
	振刀参数::延迟::长剑.左3 = 0.25f;
	振刀参数::延迟::长剑.右3 = 0.249f;
	振刀参数::延迟::长剑.左剑气 = 0.13f;
	振刀参数::延迟::长剑.右剑气 = 0.1f;
	振刀参数::延迟::长剑.凤凰羽 = 0.f;
	振刀参数::延迟::长剑.苍牙 = 0.f;
	振刀参数::延迟::长剑.跳斩 = 0.f;
	振刀参数::延迟::长剑.七星夺窍 = 0.f;
	振刀参数::延迟::长剑.壁击 = 0.f;
	振刀参数::延迟::长剑.天外飞仙 = 0.f;
	振刀参数::延迟::太刀.左3 = 0.24f;
	振刀参数::延迟::太刀.右3 = 0.223f;
	振刀参数::延迟::太刀.左蓄 = 0.f;
	振刀参数::延迟::太刀.右蓄 = 0.f;
	振刀参数::延迟::太刀.苍牙 = 0.f;
	振刀参数::延迟::太刀.刹那斩 = 0.f;
	振刀参数::延迟::太刀.右惊雷 = 0.f;	//惊雷十劫和此参数一样
	振刀参数::延迟::太刀.青鬼 = 0.f;
	振刀参数::延迟::太刀.青鬼 = 0.f;
	振刀参数::延迟::太刀.同源 = 0.f;
	振刀参数::延迟::太刀.噬魂斩 = 0.f;
	振刀参数::延迟::太刀.壁击 = 0.f;
	振刀参数::延迟::阔刀.左左 = 0.35f;
	振刀参数::延迟::阔刀.左右 = 0.28f;
	振刀参数::延迟::阔刀.右右 = 0.25f;
	振刀参数::延迟::阔刀.右左 = 0.25f;
	振刀参数::延迟::阔刀.左蓄一段 = 0.2f;
	振刀参数::延迟::阔刀.左蓄二段 = 0.2f;
	振刀参数::延迟::阔刀.左蓄三段 = 0.05f;
	振刀参数::延迟::阔刀.右蓄 = 0.f;
	振刀参数::延迟::阔刀.右右蓄 = 0.f;
	振刀参数::延迟::阔刀.雷刀 = 0.1f;
	振刀参数::延迟::阔刀.同源 = 0.1f;
	振刀参数::延迟::阔刀.雷刀下劈 = 0.35f;
	振刀参数::延迟::阔刀.壁击 = 0.f;
	振刀参数::延迟::长枪.左3 = 0.15f;
	振刀参数::延迟::长枪.右3 = 0.31f;
	振刀参数::延迟::长枪.左蓄 = 0.2f;
	振刀参数::延迟::长枪.六合枪 = 0.f;
	振刀参数::延迟::长枪.右蓄 = 0.105f;
	振刀参数::延迟::长枪.大圣游 = 0.f;
	振刀参数::延迟::长枪.风卷云残 = 0.f;
	振刀参数::延迟::长枪.双环扫 = 0.f;
	振刀参数::延迟::长枪.龙王破 = 0.075f;
	振刀参数::延迟::长枪.穿心脚 = 0.f;
	振刀参数::延迟::长枪.壁击 = 0.f;
	振刀参数::延迟::长枪.同源 = 0.f;
	振刀参数::延迟::匕首.左3 = 0.105f;
	振刀参数::延迟::匕首.右3 = 0.105f;
	振刀参数::延迟::匕首.左蓄 = 0.f;
	振刀参数::延迟::匕首.右蓄 = 0.f;
	振刀参数::延迟::匕首.鬼反 = 0.05f;
	振刀参数::延迟::匕首.壁击 = 0.05f;
	振刀参数::延迟::匕首.荆轲献匕 = 0.f;
	振刀参数::延迟::匕首.鬼刃暗扎 = 0.f;
	振刀参数::延迟::匕首.鬼哭神嚎 = 0.f;
	振刀参数::延迟::匕首.亢龙有悔 = 0.f;
	振刀参数::延迟::双截棍.左右3 = 0.1f;
	振刀参数::延迟::双截棍.左蓄 = 0.f;
	振刀参数::延迟::双截棍.右蓄 = 0.f;
	振刀参数::延迟::双截棍.飞踢 = 0.f;
	振刀参数::延迟::双截棍.扬鞭劲 = 0.f;
	振刀参数::延迟::双截棍.横栏 = 0.f;
	振刀参数::延迟::双截棍.壁击 = 0.f;
	振刀参数::延迟::双截棍.龙虎乱舞 = 0.85f;
	振刀参数::延迟::双刀.左右3 = 0.1f;
	振刀参数::延迟::双刀.左蓄 = 0.077f;
	振刀参数::延迟::双刀.右蓄 = 0.f;
	振刀参数::延迟::双刀.铁马残红 = 0.f;
	振刀参数::延迟::双刀.乾坤日月斩 = 0.f;
	振刀参数::延迟::双刀.惊雷 = 0.25f;
	振刀参数::延迟::双刀.壁击 = 0.f;
	振刀参数::延迟::双刀.八斩刀 = 0.f;
	振刀参数::延迟::双刀.分水斩 = 0.1f;
	振刀参数::延迟::长棍.左3 = 0.25f;
	振刀参数::延迟::长棍.右3 = 0.35f;
	振刀参数::延迟::长棍.少林棍 = 0.11f;
	振刀参数::延迟::长棍.左蓄 = 0.095f;
	振刀参数::延迟::长棍.右蓄 = 0.f;
	振刀参数::延迟::长棍.腾云式 = 0.f;
	振刀参数::延迟::长棍.乱点天宫 = 0.f;
	振刀参数::延迟::长棍.桶劲 = 0.f;
	振刀参数::延迟::长棍.壁击 = 0.f;
	振刀参数::延迟::长棍.同源 = 0.f;
	振刀参数::延迟::长棍.五情七灭镇 = 0.f;
	振刀参数::延迟::长棍.双环扫 = 0.f;
	振刀参数::延迟::长棍.倒海棍 = 0.f;
	振刀参数::延迟::长棍.镇地撑天 = 0.f;
	振刀参数::延迟::斩马刀.左左 = 0.25f;
	振刀参数::延迟::斩马刀.左右 = 0.f;
	振刀参数::延迟::斩马刀.右右 = 0.25f;
	振刀参数::延迟::斩马刀.右左 = 0.35f;
	振刀参数::延迟::斩马刀.左蓄 = 0.f;
	振刀参数::延迟::斩马刀.右蓄 = 0.075f;
	振刀参数::延迟::斩马刀.柄击 = 0.11f;
	振刀参数::延迟::斩马刀.惊雷 = 0.f;
	振刀参数::延迟::斩马刀.同源 = 0.f;
	振刀参数::延迟::斩马刀.壁击 = 0.f;
	振刀参数::延迟::斩马刀.炽焰斩 = 0.f;
	振刀参数::延迟::斩马刀.奔雷入阵 = 0.055f;
	振刀参数::延迟::斩马刀.过关斩将 = 0.149f;
	振刀参数::延迟::双戟.左右3 = 0.15f;
	振刀参数::延迟::双戟.左蓄 = 0.f;
	振刀参数::延迟::双戟.右蓄 = 0.f;
	振刀参数::延迟::双戟.勾旋斩 = 0.14f;
	振刀参数::延迟::双戟.探海蛟 = 0.f;
	振刀参数::延迟::双戟.钩挂 = 0.13f;
	振刀参数::延迟::双戟.壁击 = 0.f;
	振刀参数::延迟::双戟.战龙在天 = 0.f;
	振刀参数::延迟::双戟.形意钩 = 0.11f;
	振刀参数::延迟::扇子.左3 = 0.1f;
	振刀参数::延迟::扇子.右3 = 0.1f;
	振刀参数::延迟::扇子.左蓄 = 0.f;
	振刀参数::延迟::扇子.右蓄 = 0.f;
	振刀参数::延迟::扇子.双开圆 = 0.f;
	振刀参数::延迟::扇子.三风摆 = 0.f;
	振刀参数::延迟::扇子.鬼反 = 0.f;
	振刀参数::延迟::扇子.惊雷 = 0.f;
	振刀参数::延迟::扇子.壁击 = 0.f;
	振刀参数::延迟::扇子.缠龙奔野 = 0.f;
	振刀参数::延迟::扇子.泽风上六 = 0.f;
	振刀参数::延迟::横刀.左3 = 0.1f;
	振刀参数::延迟::横刀.右3 = 0.1f;
	振刀参数::延迟::横刀.左蓄 = 0.13f;
	振刀参数::延迟::横刀.右蓄 = 0.f;
	振刀参数::延迟::横刀.苍牙 = 0.f;
	振刀参数::延迟::横刀.壁击 = 0.f;
	振刀参数::延迟::横刀.同源 = 0.f;
	振刀参数::延迟::横刀.踏空闪 = 0.f;
	振刀参数::延迟::横刀.破千军 = 0.f;
	振刀参数::延迟::横刀.极光破云闪 = 0.f;
	振刀参数::延迟::横刀.乾坤一掷 = 0.f;
	振刀参数::延迟::拳刃.左3 = 0.1f;
	振刀参数::延迟::拳刃.右3 = 0.1f;
	振刀参数::延迟::拳刃.左蓄力 = 0.05f;
	振刀参数::延迟::拳刃.右蓄力 = 0.05f;
	振刀参数::延迟::拳刃.右蓄力2段 = 0.08f;
	振刀参数::延迟::拳刃.苍牙 = 0.f;
	振刀参数::延迟::拳刃.壁击 = 0.f;
	振刀参数::延迟::拳刃.百裂腿 = 0.11f;
	振刀参数::延迟::拳刃.破空拳 = 0.f;
	振刀参数::延迟::拳刃.太极弄云手 = 0.07f;
	振刀参数::延迟::飞刀.左3 = 0.1f;
	振刀参数::延迟::飞刀.右3 = 0.1f;
	振刀参数::延迟::飞刀.左蓄力 = 0.05f;
	振刀参数::延迟::飞刀.左蓄力2段 = 0.095f;
	振刀参数::延迟::飞刀.右蓄力 = 0.135f;
	振刀参数::延迟::飞刀.苍牙 = 0.1f;
	振刀参数::延迟::飞刀.太极 = 0.07f;
	振刀参数::延迟::飞刀.壁击 = 0.f;
	振刀参数::延迟::飞刀.掌心雷 = 0.125f;
	振刀参数::延迟::飞刀.断月千刃舞 = 0.f;
	振刀参数::延迟::飞刀.万点寒梅 = 0.f;
}
void 初始化间隔配置() {
	振刀参数::闪避::长剑.左3 = 70;
	振刀参数::闪避::长剑.右3 = 70;
	振刀参数::闪避::长剑.左剑气1 = 70;
	振刀参数::闪避::长剑.左剑气2 = 70;
	振刀参数::闪避::长剑.右剑气2 = 70;
	振刀参数::闪避::长剑.右剑气1 = 70;
	振刀参数::闪避::长剑.凤凰羽1 = 70;
	振刀参数::闪避::长剑.凤凰羽2 = 70;
	振刀参数::闪避::长剑.苍牙 = 70;
	振刀参数::闪避::长剑.跳斩 = 70;
	振刀参数::闪避::长剑.壁击 = 70;
	振刀参数::闪避::长剑.七星夺窍 = 70;
	振刀参数::闪避::长剑.天外飞仙 = 70;
	振刀参数::闪避::链剑.左3 = 70;
	振刀参数::闪避::链剑.右3 = 70;
	振刀参数::闪避::链剑.左蓄力1 = 70;
	振刀参数::闪避::链剑.左蓄力2 = 70;
	振刀参数::闪避::链剑.右蓄力2 = 70;
	振刀参数::闪避::链剑.右蓄力1 = 70;
	振刀参数::闪避::链剑.地龙滚堂刹 = 70;
	振刀参数::闪避::链剑.苍牙 = 70;
	振刀参数::闪避::链剑.跳斩 = 70;
	振刀参数::闪避::链剑.壁击 = 70;
	振刀参数::闪避::链剑.火龙卷云 = 70;
	振刀参数::闪避::链剑.断罪碎蜂 = 70;
	振刀参数::闪避::链剑.蓄力追击 = 70;
	振刀参数::闪避::太刀.左3 = 70;
	振刀参数::闪避::太刀.右3 = 70;
	振刀参数::闪避::太刀.左蓄1 = 70;
	振刀参数::闪避::太刀.左蓄2 = 70;
	振刀参数::闪避::太刀.右蓄2 = 70;
	振刀参数::闪避::太刀.右蓄1 = 70;
	振刀参数::闪避::太刀.刹那斩1 = 70;
	振刀参数::闪避::太刀.苍牙 = 70;
	振刀参数::闪避::太刀.惊雷 = 70;
	振刀参数::闪避::太刀.壁击 = 70;
	振刀参数::闪避::太刀.青鬼 = 70;
	振刀参数::闪避::太刀.同源 = 70;
	振刀参数::闪避::太刀.噬魂斩1 = 70;
	振刀参数::闪避::太刀.噬魂斩2 = 70;
	振刀参数::闪避::阔刀.左左 = 70;
	振刀参数::闪避::阔刀.左右 = 70;
	振刀参数::闪避::阔刀.右右 = 70;
	振刀参数::闪避::阔刀.右左 = 70;
	振刀参数::闪避::阔刀.左蓄一段 = 70;
	振刀参数::闪避::阔刀.左蓄二段 = 70;
	振刀参数::闪避::阔刀.左蓄三段 = 70;
	振刀参数::闪避::阔刀.右蓄1 = 70;
	振刀参数::闪避::阔刀.右蓄2 = 70;
	振刀参数::闪避::阔刀.右右蓄 = 70;
	振刀参数::闪避::阔刀.雷刀 = 70;
	振刀参数::闪避::阔刀.壁击 = 70;
	振刀参数::闪避::阔刀.同源 = 70;
	振刀参数::闪避::阔刀.雷刀下劈 = 70;
	振刀参数::闪避::阔刀.翻江倒海 = 70;
	振刀参数::闪避::长枪.左3 = 70;
	振刀参数::闪避::长枪.右3 = 70;
	振刀参数::闪避::长枪.左蓄 = 70;
	振刀参数::闪避::长枪.六合枪 = 70;
	振刀参数::闪避::长枪.右蓄 = 70;
	振刀参数::闪避::长枪.大圣游 = 70;
	振刀参数::闪避::长枪.风卷云残 = 70;
	振刀参数::闪避::长枪.壁击 = 70;
	振刀参数::闪避::长枪.同源 = 70;
	振刀参数::闪避::长枪.双环扫 = 70;
	振刀参数::闪避::长枪.龙王破 = 70;
	振刀参数::闪避::长枪.穿心脚 = 70;
	振刀参数::闪避::匕首.左3 = 70;
	振刀参数::闪避::匕首.右3 = 70;
	振刀参数::闪避::匕首.左蓄 = 70;
	振刀参数::闪避::匕首.右蓄 = 70;
	振刀参数::闪避::匕首.鬼反 = 70;
	振刀参数::闪避::匕首.荆轲献匕 = 70;
	振刀参数::闪避::匕首.壁击 = 70;
	振刀参数::闪避::匕首.鬼刃暗扎 = 70;
	振刀参数::闪避::匕首.鬼哭神嚎 = 70;
	振刀参数::闪避::匕首.亢龙有悔 = 70;
	振刀参数::闪避::双截棍.左右3 = 70;
	振刀参数::闪避::双截棍.左蓄 = 70;
	振刀参数::闪避::双截棍.右蓄 = 70;
	振刀参数::闪避::双截棍.飞踢 = 70;
	振刀参数::闪避::双截棍.壁击 = 70;
	振刀参数::闪避::双截棍.扬鞭劲 = 70;
	振刀参数::闪避::双截棍.横栏 = 70;
	振刀参数::闪避::双截棍.龙虎乱舞 = 70;
	振刀参数::闪避::双截棍.三龙灭阳棍 = 70;
	振刀参数::闪避::双刀.左右3 = 70;
	振刀参数::闪避::双刀.左蓄 = 70;
	振刀参数::闪避::双刀.右蓄1 = 70;
	振刀参数::闪避::双刀.右蓄2 = 70;
	振刀参数::闪避::双刀.铁马残红 = 70;
	振刀参数::闪避::双刀.壁击 = 70;
	振刀参数::闪避::双刀.乾坤日月斩 = 70;
	振刀参数::闪避::双刀.惊雷 = 70;
	振刀参数::闪避::双刀.八斩刀 = 70;
	振刀参数::闪避::双刀.分水斩 = 70;
	振刀参数::闪避::长棍.右3 = 70;
	振刀参数::闪避::长棍.左3 = 70;
	振刀参数::闪避::长棍.左蓄 = 70;
	振刀参数::闪避::长棍.右蓄 = 70;
	振刀参数::闪避::长棍.壁击 = 70;
	振刀参数::闪避::长棍.同源 = 70;
	振刀参数::闪避::长棍.腾云式 = 70;
	振刀参数::闪避::长棍.乱点天宫 = 70;
	振刀参数::闪避::长棍.桶劲 = 70;
	振刀参数::闪避::长棍.五情七灭镇 = 70;
	振刀参数::闪避::长棍.双环扫 = 70;
	振刀参数::闪避::长棍.少林棍 = 70;
	振刀参数::闪避::长棍.镇地撑天 = 70;
	振刀参数::闪避::斩马刀.左左 = 70;
	振刀参数::闪避::斩马刀.左右 = 70;
	振刀参数::闪避::斩马刀.右右 = 70;
	振刀参数::闪避::斩马刀.右左 = 70;
	振刀参数::闪避::斩马刀.左蓄1 = 70;
	振刀参数::闪避::斩马刀.左蓄2 = 70;
	振刀参数::闪避::斩马刀.左蓄3 = 70;
	振刀参数::闪避::斩马刀.右蓄1 = 70;
	振刀参数::闪避::斩马刀.右蓄2 = 70;
	振刀参数::闪避::斩马刀.右蓄3 = 70;
	振刀参数::闪避::斩马刀.柄击 = 70;
	振刀参数::闪避::斩马刀.壁击 = 70;
	振刀参数::闪避::斩马刀.同源 = 70;
	振刀参数::闪避::斩马刀.惊雷 = 70;
	振刀参数::闪避::斩马刀.炽焰斩 = 70;
	振刀参数::闪避::斩马刀.过关斩将 = 70;
	振刀参数::闪避::斩马刀.奔雷入阵 = 70;
	振刀参数::闪避::双戟.左3 = 70;
	振刀参数::闪避::双戟.右3 = 70;
	振刀参数::闪避::双戟.左蓄 = 70;
	振刀参数::闪避::双戟.右蓄1 = 70;
	振刀参数::闪避::双戟.右蓄2 = 70;
	振刀参数::闪避::双戟.惊雷 = 70;
	振刀参数::闪避::双戟.勾旋斩 = 70;
	振刀参数::闪避::双戟.探海蛟 = 70;
	振刀参数::闪避::双戟.钩挂 = 70;
	振刀参数::闪避::双戟.壁击 = 70;
	振刀参数::闪避::双戟.战龙在天 = 70;
	振刀参数::闪避::双戟.形意钩 = 70;
	振刀参数::闪避::扇子.左3 = 70;
	振刀参数::闪避::扇子.右3 = 70;
	振刀参数::闪避::扇子.左蓄 = 70;
	振刀参数::闪避::扇子.右蓄 = 70;
	振刀参数::闪避::扇子.双开圆 = 70;
	振刀参数::闪避::扇子.三风摆 = 70;
	振刀参数::闪避::扇子.鬼反 = 70;
	振刀参数::闪避::扇子.惊雷 = 70;
	振刀参数::闪避::扇子.壁击 = 70;
	振刀参数::闪避::扇子.缠龙奔野 = 70;
	振刀参数::闪避::扇子.泽风上六 = 70;
	振刀参数::闪避::横刀.左3 = 70;
	振刀参数::闪避::横刀.右3 = 70;
	振刀参数::闪避::横刀.左蓄 = 70;
	振刀参数::闪避::横刀.右蓄1 = 70;
	振刀参数::闪避::横刀.右蓄2 = 70;
	振刀参数::闪避::横刀.苍牙 = 70;
	振刀参数::闪避::横刀.壁击 = 70;
	振刀参数::闪避::横刀.同源 = 70;
	振刀参数::闪避::横刀.踏空闪 = 70;
	振刀参数::闪避::横刀.破千军 = 70;
	振刀参数::闪避::横刀.极光破云闪 = 70;
	振刀参数::闪避::横刀.乾坤一掷 = 70;
	振刀参数::闪避::拳刃.左3 = 70;
	振刀参数::闪避::拳刃.右3 = 70;
	振刀参数::闪避::拳刃.左蓄力1 = 70;
	振刀参数::闪避::拳刃.左蓄力2 = 70;
	振刀参数::闪避::拳刃.右蓄力1 = 70;
	振刀参数::闪避::拳刃.右蓄力2 = 70;
	振刀参数::闪避::拳刃.右蓄力2段 = 70;
	振刀参数::闪避::拳刃.百裂腿 = 70;
	振刀参数::闪避::拳刃.破空拳 = 70;
	振刀参数::闪避::拳刃.苍牙 = 70;
	振刀参数::闪避::拳刃.壁击 = 70;
	振刀参数::闪避::拳刃.太极弄云手 = 70;
	振刀参数::闪避::飞刀.左3 = 70;
	振刀参数::闪避::飞刀.右3 = 70;
	振刀参数::闪避::飞刀.左蓄力 = 70;
	振刀参数::闪避::飞刀.左蓄力2段 = 70;
	振刀参数::闪避::飞刀.右蓄力 = 70;
	振刀参数::闪避::飞刀.掌心雷 = 70;
	振刀参数::闪避::飞刀.苍牙 = 70;
	振刀参数::闪避::飞刀.太极 = 70;
	振刀参数::闪避::飞刀.壁击 = 70;
	振刀参数::闪避::飞刀.断月千刃舞 = 70;
	振刀参数::闪避::飞刀.万点寒梅 = 70;
}
std::shared_mutex m_DataMutex; //缓存指针专用锁
// 缓存指针
DWORD WINAPI Cache_Player_World_Data2(LPVOID lpParam)
{
	vector<ObjData> m_ListData;
	ObjData tempData;
	static uint64_t lastMatrixAddr = 0;
	while (true)
	{
		auto start = chrono::high_resolution_clock::now();
		Global::WorldPtr.Character->GetCharacterManagerPtr();	// 缓存CharacterManager
		if (!Global::WorldPtr.Character->CharacterManagerPtr) {
			if (lastMatrixAddr != 0) {
				Offset::MatrixAddr = lastMatrixAddr;
			} else {
				Offset::MatrixAddr = 0;
			}
			std::this_thread::sleep_for(chrono::milliseconds(500));
			continue;
		}



		Global::WorldPtr.Character->Cache_PlayerCount_ActorModelList();	// 为后续做缓存
		Global::WorldPtr.UserData->GetUserDataManager();			// 缓存UserDataManager
		auto matrixAddr = Tool::GetMatrixAddr(Global::WorldPtr.Character->CharacterManagerPtr); // 矩阵
		if (matrixAddr > 0x10000) {
			Offset::MatrixAddr = matrixAddr;
			lastMatrixAddr = matrixAddr;
		} else {
			Offset::MatrixAddr = lastMatrixAddr;
		}
		Global::Cache_LocalPlayer.L_ActorModel = Global::WorldPtr.Character->GetLocalActorModel(); // 缓存自身指针
		Global::Cache_LocalPlayer.Cache_WeaponIndex_PTR = Global::WorldPtr.UserData->Cache_WeaponIndex();
		Global::Cache_LocalPlayer.Cache_WeaponIndex_1PTR = Global::WorldPtr.UserData->GetWeaponList(0);
		Global::Cache_LocalPlayer.Cache_WeaponIndex_2PTR = Global::WorldPtr.UserData->GetWeaponList(1);
		if (!Global::Cache_LocalPlayer.L_ActorModel) {
			this_thread::sleep_for(chrono::milliseconds(200)); // Sleep if local player is not valid
			continue;
		}
		if (!Global::Cache_LocalPlayer.L_ActorModel)
		{
			MyLogTrue("L_ActorModel为空");
			data_buffers.SwapBuffers();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		Global::Cache_LocalPlayer.L_ActorKit = Global::Cache_LocalPlayer.L_ActorModel->GetActorKit();
		if (!Global::Cache_LocalPlayer.L_ActorKit) {
			MyLogTrue("L_ActorKit为空");
			data_buffers.SwapBuffers();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		// 缓存ActorModel
		if (Global::Cache_LocalPlayer.L_ActorKit)
		{
			Global::Cache_LocalPlayer.Cache_CameraAgent_PTR = Global::Cache_LocalPlayer.L_ActorKit->GetCameraAgent();
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_CameraAgent_PTR = NULL;
		}
		if (Global::Cache_LocalPlayer.Cache_CameraAgent_PTR)
		{
			Global::Cache_LocalPlayer.Cache_CameraController_PTR = GetCameraController(Global::Cache_LocalPlayer.Cache_CameraAgent_PTR);
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_CameraController_PTR = NULL;
		}
		if (Global::Cache_LocalPlayer.Cache_CameraController_PTR)
		{
			Global::Cache_LocalPlayer.Cache_AdventureRig_PTR = GetAdventureRig(Global::Cache_LocalPlayer.Cache_CameraController_PTR);
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_AdventureRig_PTR = NULL;
		}

		Global::Cache_LocalPlayer.L_HitSimulate = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulateData();
		Global::Cache_LocalPlayer.Cache_Bluetime_PTR = Global::Cache_LocalPlayer.L_ActorKit->Cache_BlueTime();

		Global::Cache_LocalPlayer.L_PropertyData = Global::Cache_LocalPlayer.L_ActorModel->GetActorModelPropertyData();
		Global::Cache_LocalPlayer.L_RuntimeProperty = Global::Cache_LocalPlayer.L_ActorModel->GetRuntimePropertyData();
		Global::Cache_LocalPlayer.L_ObjectMessenger = Global::Cache_LocalPlayer.L_ActorModel->GetObjectMessenger();
		if (!Global::Cache_LocalPlayer.L_PropertyData || !Global::Cache_LocalPlayer.L_RuntimeProperty || !Global::Cache_LocalPlayer.L_ObjectMessenger) {
			std::this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		// 缓存ActorModel下面各项指针
		Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetEventTracksForLayer();
		Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetActorWeaponPtr();
		Global::Cache_LocalPlayer.Cache_ActorPhysics_PTR = Global::Cache_LocalPlayer.L_ActorKit ? Global::Cache_LocalPlayer.L_ActorKit->Cache_ActorPhysics() : 0;
		Global::Cache_LocalPlayer.Cache_ActorRoot_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetActorRootPtr();
		Global::Cache_LocalPlayer.Cache_PropertData_PTR = Global::Cache_LocalPlayer.L_RuntimeProperty->Cache_PropertData();
		Global::Cache_LocalPlayer.Cache_PropertyDataID = Global::Cache_LocalPlayer.L_RuntimeProperty->GetPropertyDataID();
		Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR = Global::Cache_LocalPlayer.L_ObjectMessenger->Cache_GetGlobalTime();
		Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR = Global::Cache_LocalPlayer.L_ObjectMessenger->Cache_GetCurTimeForLogicState();
		Global::Cache_LocalPlayer.Cache_AvgFPSStatic_PTR = 0;
		Global::Cache_LocalPlayer.CharactorSyncManagerPtr = Global::WorldPtr.Character->GetCharactorSyncManager();		//缓存GetCharactorSyncManager
		if (Global::WorldPtr.Character && Global::WorldPtr.Character->CharacterManagerPtr)
		{
			Global::Cache_LocalPlayer.fpsFilterPtr = mem.Read<uintptr_t>(Global::WorldPtr.Character->CharacterManagerPtr + Offset::Character.fpsFilter);
		}


		/*if (Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR)
		{
			auto gameBaseObj = mem.Read<uintptr_t>(Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR + Offset::GameBaseObject._Instance);
			if (gameBaseObj)
			{
				auto fpsManager = mem.Read<uintptr_t>(gameBaseObj + Offset::GameBaseObject._fps);
				if (fpsManager)
				{
					auto fpsKlass = mem.Read<uintptr_t>(fpsManager);
					Global::Cache_LocalPlayer.Cache_AvgFPSStatic_PTR = mem.Read<uintptr_t>(fpsKlass + 0xB8);
				}
			}
		}*/
		LocalPlayer_Data.MyTeam = Global::Cache_LocalPlayer.L_PropertyData->GetTeam();
		LocalPlayer_Data.MyHero = Global::Cache_LocalPlayer.L_PropertyData->GetHeroTID();
		// 优化：读取数据提前缓存
		Global::MySkillData.MySkill = Global::WorldPtr.UserData->GetSkillID(LocalPlayer_Data.MyHero);
		auto PlayerCount = Global::WorldPtr.Character->GetPlayerCount();
		m_ListData.clear();
		m_ListData.reserve(PlayerCount);  // 预分配内存
		// 这里可以进行多线程优化
		for (int i = 0; i < PlayerCount; i++)
		{
			auto List = Global::WorldPtr.Character->GetActorModelList(i);
			if (List == Global::Cache_LocalPlayer.L_ActorModel || List == nullptr) continue;
			tempData.O_PropertyData = List->GetActorModelPropertyData();
			if (!tempData.O_PropertyData) continue;
			if (LocalPlayer_Data.MyTeam == tempData.O_PropertyData->GetTeam()) continue;
			// 缓存对象
			tempData.O_ActorModel = List;
			tempData.O_ActorKit = List->GetActorKit();
			if (!tempData.O_ActorKit) continue;
			tempData.O_HitSimulate = tempData.O_ActorKit->GetActionHitSimulateData();
			tempData.O_HitSimulatePtr = tempData.O_ActorKit->GetActionHitSimulatePtr();
			tempData.O_RuntimeProperty = List->GetRuntimePropertyData();
			tempData.O_ObjectMessenger = List->GetObjectMessenger();
			if (!tempData.O_RuntimeProperty || !tempData.O_ObjectMessenger) continue;
			// 缓存ActorModel下面各项指针
			tempData.Cache_EventTracksForLayer_PTR = List->Cache_GetEventTracksForLayer();
			tempData.Cache_ActorWeapon_PTR = List->Cache_GetActorWeaponPtr();
			tempData.Cache_ActorRoot_PTR = List->Cache_GetActorRootPtr();
			tempData.Cache_Bluetime_PTR = tempData.O_ActorKit->Cache_BlueTime();
			tempData.Cache_PropertData_PTR = tempData.O_RuntimeProperty->Cache_PropertData();
			tempData.Cache_PropertyDataID = tempData.O_RuntimeProperty->GetPropertyDataID();
			tempData.Cache_GetCurTimeForLogicState_PTR = tempData.O_ObjectMessenger->Cache_GetCurTimeForLogicState();
			tempData.Cache_GetNameHash_PTR = tempData.Cache_GetCurTimeForLogicState_PTR;
			// 以下缓存不经常变动的数据
			tempData.ISRobot = tempData.O_PropertyData->GetISRobotId();
			tempData.Team = tempData.O_PropertyData->GetTeam();
			tempData.HeroId = tempData.O_PropertyData->GetHeroTID();
			tempData.PlayerName = tempData.O_PropertyData->GetPlayerName();
			tempData.O_Skill = {};
			if (Global::WorldPtr.UserData && Global::WorldPtr.UserData->UserDataManagerPtr)
			{
				int enemyFid = mem.Read<int>((uintptr_t)tempData.O_ActorModel + Offset::SoulData.fid);
				if (enemyFid != 0)
				{
					tempData.O_Skill = Global::WorldPtr.UserData->GetSkillIDByFid(tempData.HeroId, enemyFid);
				}
			}
			//tempData.O_PropertyData->GetCharactorSyncManager();
			//tempData.ping = tempData.O_PropertyData->GetPing();
			m_ListData.push_back(tempData);
		}
		{
			std::unique_lock lock(m_DataMutex); // 加锁减少锁的使用次数
			g_GameData = std::move(m_ListData);
		}
		auto stop = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
		this_thread::sleep_for(chrono::milliseconds(duration));
	}
	return 0;
}
DWORD WINAPI LocalPlayer_Object_Data2(LPVOID lpParam)
{
	constexpr size_t ENEMY_SCATTER_BATCH = 16; // 每批最多读多少个敌人，减少 Scatter 句柄/执行次数
	constexpr float ENEMY_FULLDATA_DISTANCE = 40.0f;
	auto per_second_start = std::chrono::steady_clock::now();
	uint64_t read_count = 0;
	static int last_nonzero_energy = 0;
	static int last_nonzero_range_reaction_type = 0;
	static ActionType last_nonzero_endure_level = (ActionType)0;
	static float last_nonzero_bluetime = 0.0f;
	static float last_bluetime_value = 0.0f;
	static int bluetime_zero_streak = 0;
	static int bluetime_same_streak = 0;
	auto report_read_stats = [&](bool did_read) {
		if (did_read) {
			++read_count;
		}
		auto per_second_now = std::chrono::steady_clock::now();
		if (per_second_now - per_second_start >= std::chrono::seconds(1))
		{
			g_localplayer_reads_per_second.store(read_count, std::memory_order_release);
			read_count = 0;
			per_second_start = per_second_now;
		}
	};
	while (true)
	{
		bool did_read = false;
		//MyLog("鼠标左键状态:", !振刀逻辑::判断鼠标左键是否按下());
		// 只在第一次进入时启动网络监听
		if (Function::Kmbox::Net::NetState == 0 && !是否连接Net)
		{
			//root_kmNet_mouse_left(1);
			cout << "启动监听端口:" << atoi(Function::Kmbox::Net::Port) << endl;
			是否连接Net = true;
			kmNet_monitor(atoi(Function::Kmbox::Net::Port));	// 启动监听端口
		}
		auto start = std::chrono::high_resolution_clock::now();
		// ========= 阶段 1：拷贝当前缓存的 ObjData 列表（快照） =========
		std::vector<ObjData> current_players;
		{
			// 建议在 Cache_Player_World_Data 那边用 std::unique_lock 写 g_GameData
			std::shared_lock lock(m_DataMutex);
			current_players = g_GameData;   // 拷贝一份，避免长时间持有锁
		}
		//LocalPlayer_Data.敌人数量5M内 = 0;
		auto& writeBuffer = data_buffers.GetWriteBuffer();
		writeBuffer.clear();
		writeBuffer.reserve(current_players.size());
		// 如果本地玩家 Actor 还没缓存好，直接等待
		if (!Global::Cache_LocalPlayer.L_ActorModel)
		{
			//MyLogTrue("L_ActorModel为空");
			report_read_stats(false);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		// ========= 阶段 2：本地玩家一次性缓存（单线程） =========
		// ActorKit / HitSimulate 这些指针每帧更新一次即可
		//Global::Cache_LocalPlayer.L_ActorKit = Global::Cache_LocalPlayer.L_ActorModel->GetActorKit();
		if (!Global::Cache_LocalPlayer.L_ActorKit) {
			//MyLogTrue("L_ActorKit为空");
			data_buffers.SwapBuffers();
			report_read_stats(false);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		
		Global::Cache_LocalPlayer.L_HitSimulatePtr = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulatePtr();

		// 自身背包 / 武器 / 魂玉容量（这些读一次就够）
		LocalPlayer_Data.myNowBag = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull);
		LocalPlayer_Data.myNowBagWeapon = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.weaponMaxID * 4ull);
		LocalPlayer_Data.myNowBagsoul = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.soulItemMaxID * 4ull);
		// 魂玉背包实际列表：PlayerBattleSoulItemData._soulItemList offset 0x40
		if(false){
			std::vector<int> soulItemTidList;
			uintptr_t soulItemListPtr = 0;
			int localFid = 0;
			int soulItemDataFid = 0;
			if (Global::Cache_LocalPlayer.L_ActorModel)
			{
				localFid = mem.Read<int>((uintptr_t)Global::Cache_LocalPlayer.L_ActorModel + Offset::SoulData.fid);
			}
			if (Global::WorldPtr.UserData && Global::WorldPtr.UserData->UserDataManagerPtr)
			{
				auto userBattleData = mem.Read<uintptr_t>(Global::WorldPtr.UserData->UserDataManagerPtr + Offset::ActorModel.battleData);
				auto userBattleSoulItemData = userBattleData ? mem.Read<uintptr_t>(userBattleData + Offset::SoulData.soulItemData) : 0;
				auto soulItemDict = userBattleSoulItemData ? mem.Read<uintptr_t>(userBattleSoulItemData + Offset::SoulData.dataDict) : 0;
				auto soulItemEntries = soulItemDict ? mem.Read<uintptr_t>(soulItemDict + 0x18) : 0;
				auto playerSoulItemData = soulItemEntries ? mem.Read<uintptr_t>(soulItemEntries + 0x30) : 0;
				if (playerSoulItemData)
				{
					soulItemDataFid = mem.Read<int>(playerSoulItemData + Offset::SoulData.soulItemDataFid);
					soulItemListPtr = mem.Read<uintptr_t>(playerSoulItemData + Offset::SoulData._soulItemList);
				}
			}
			//MyLogTrue("魂玉背包列表Ptr: 0x", std::hex, soulItemListPtr, std::dec, " 本地fid:", localFid, " 列表fid:", soulItemDataFid);
			if (soulItemListPtr)
			{
				auto soulItemArray = mem.Read<uintptr_t>(soulItemListPtr + Offset::SoulData.info);
				int soulItemCount = mem.Read<int>(soulItemListPtr + Offset::SoulData.fid_);
				soulItemTidList.reserve(soulItemCount);
				for (int i = 0; i < soulItemCount; ++i)
				{
					auto battleSoulItemPtr = mem.Read<uintptr_t>(soulItemArray + 0x20 + i * 0x08);
					if (!battleSoulItemPtr)
					{
						continue;
					}
					auto soulItemInfoPtr = mem.Read<uintptr_t>(battleSoulItemPtr + 0x10);
					if (!soulItemInfoPtr)
					{
						continue;
					}
					int soulItemFid = mem.Read<int>(soulItemInfoPtr + 0x18);
					int soulItemTid = mem.Read<int>(soulItemInfoPtr + Offset::SoulData.soul_item_tid_);
					//MyLogTrue("魂玉ID:", soulItemTid, " fid:", soulItemFid);
					soulItemTidList.push_back(soulItemTid);
				}
			}
			LocalPlayer_Data.mySoulItemTidList.swap(soulItemTidList);
		}
		// 背包物品实际列表：PlayerBattleItemInfo.itemDataList offset 0x30
		if(false){
			std::vector<int> bagItemTidList;
			uintptr_t itemDataListPtr = 0;
			if (Global::WorldPtr.UserData && Global::WorldPtr.UserData->UserDataManagerPtr)
			{
				auto userBattleData = mem.Read<uintptr_t>(Global::WorldPtr.UserData->UserDataManagerPtr + Offset::ActorModel.battleData);
				auto userBattleItemData = userBattleData ? mem.Read<uintptr_t>(userBattleData + Offset::BattleItemData.itemData) : 0;
				auto itemDict = userBattleItemData ? mem.Read<uintptr_t>(userBattleItemData + Offset::BattleItemData.dataDict) : 0;
				auto itemEntries = itemDict ? mem.Read<uintptr_t>(itemDict + 0x18) : 0;
				auto playerItemData = itemEntries ? mem.Read<uintptr_t>(itemEntries + 0x30) : 0;
				if (playerItemData)
				{
					itemDataListPtr = mem.Read<uintptr_t>(playerItemData + Offset::BattleItemData.itemDataList);
				}
			}
			if (itemDataListPtr)
			{
				auto itemArray = mem.Read<uintptr_t>(itemDataListPtr + Offset::BattleItemData.listItems);
				int itemCount = mem.Read<int>(itemDataListPtr + Offset::BattleItemData.listSize);
				bagItemTidList.reserve(itemCount);
				for (int i = 0; i < itemCount; ++i)
				{
					auto battleItemDataPtr = mem.Read<uintptr_t>(itemArray + 0x20 + i * 0x08);
					if (!battleItemDataPtr)
					{
						continue;
					}
					auto battleItemInfoPtr = mem.Read<uintptr_t>(battleItemDataPtr + Offset::BattleItemData.battleItemInfo);
					if (!battleItemInfoPtr)
					{
						continue;
					}
					int itemTid = mem.Read<int>(battleItemInfoPtr + Offset::BattleItemData.item_id_);
					//MyLogTrue("物品ID:", itemTid);
					bagItemTidList.push_back(itemTid);
				}
			}
			LocalPlayer_Data.myBagItemTidList.swap(bagItemTidList);
		}

		if (Global::Cache_LocalPlayer.CharactorSyncManagerPtr)
		{
			float shrtt = mem.Read<float>(Global::Cache_LocalPlayer.CharactorSyncManagerPtr + Offset::ActorModel.shrtt);//Class: CharactorSync->Single shrtt
			//MyLog("游戏延迟:", shrtt);
			if (shrtt > 0.0f)
			{
				LocalPlayer_Data.MyPing = (shrtt * 2.0f) * 1000.0f;
			}
		}
		
		// 一次性用 Scatter 批量读本地玩家所有需要的 runtime 数据
		{
			auto handle = mem.CreateScatterHandle();
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulatePtr + Offset::ActorModel.ActorKitS.HitSimulate.XuListate,
				&LocalPlayer_Data.XuListate);
			float blue_time = 0.0f;
			mem.AddScatterReadRequest<float>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.Cache_Bluetime_PTR + 0x24 + 3 * 0xC,
				&blue_time);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_PTR + Offset::ActorModel.cur_weapon_slot,
				&LocalPlayer_Data.MyWeaponIndex);
			ActionType endure_level = (ActionType)0;
			mem.AddScatterReadRequest<ActionType>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel,
				&endure_level);
			int range_reaction_type = 0;
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType,
				&range_reaction_type);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType,
				&LocalPlayer_Data.MyReactionType);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType,
				&LocalPlayer_Data.MyReactionExtraType);
			mem.AddScatterReadRequest<float>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration,
				&LocalPlayer_Data.MyActionDuration);
			mem.AddScatterReadRequest<double>(
				handle,
				Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime,
				&LocalPlayer_Data.MyCurTimeForLogic);
			mem.AddScatterReadRequest<uintptr_t>(
				handle,
				Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.CurShareState,
				&LocalPlayer_Data.Cache_NameHash);
			mem.AddScatterReadRequest<double>(
				handle,
				Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR + Offset::ActorModel._globalTime,
				&Global::WorldPtr.GlobalTime);
			mem.AddScatterReadRequest<double>(
				handle,
				Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR + Offset::ActorModel._globalTime,
				&LocalPlayer_Data.worldPtrGlobalTime);
			if (Global::Cache_LocalPlayer.fpsFilterPtr)
			{
				mem.AddScatterReadRequest<int>(
					handle,
					Global::Cache_LocalPlayer.fpsFilterPtr + Offset::FpsFilter.fps,
					&LocalPlayer_Data.MyFPS);
			}
		
			mem.AddScatterReadRequest<WeaponType>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR + Offset::ActorModel.WeaponType,
				&LocalPlayer_Data.MyWeaponType);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_1PTR + 0x14,
				&LocalPlayer_Data.My_1weapon);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_2PTR + 0x14,
				&LocalPlayer_Data.My_2weapon);
			mem.AddScatterReadRequest<int>(
				handle,
				(uintptr_t)Global::Cache_LocalPlayer.L_PropertyData + Offset::ActorModel.ProPerty.SkillStatus,
				&LocalPlayer_Data.MySkillState);
			int cur_energy = 0;
			mem.AddScatterReadRequest<int>(
				handle,
				Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.CurEnergyID * 4ull,
				&cur_energy);
			mem.AddScatterReadRequest<int>(
				handle,
				Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.CurshieldID * 4ull,
				&LocalPlayer_Data.MyCurshield);
			mem.AddScatterReadRequest<Vector3>(
				handle,
				Global::Cache_LocalPlayer.Cache_ActorRoot_PTR + 0x90,
				&LocalPlayer_Data.MyPos);
			if (Global::Cache_LocalPlayer.Cache_AdventureRig_PTR)
			{
				mem.AddScatterReadRequest<Vector3>(
					handle,
					Global::Cache_LocalPlayer.Cache_AdventureRig_PTR + Offset::ActorModel.AdventureRigS.Euler,
					&LocalPlayer_Data.CameraEuler);
			}
			else
			{
				LocalPlayer_Data.CameraEuler = Vector3(0.f, 0.f, 0.f);
			}
			mem.ExecuteReadScatter(handle);
			if (range_reaction_type > 0) {
				last_nonzero_range_reaction_type = range_reaction_type;
				LocalPlayer_Data.MyRangeReactionType = range_reaction_type;
			} else if (last_nonzero_range_reaction_type > 0) {
				LocalPlayer_Data.MyRangeReactionType = last_nonzero_range_reaction_type;
			}
			if (endure_level != (ActionType)0) {
				last_nonzero_endure_level = endure_level;
				LocalPlayer_Data.MyEndureLevel = endure_level;
			} else if (last_nonzero_endure_level != (ActionType)0) {
				LocalPlayer_Data.MyEndureLevel = last_nonzero_endure_level;
			}
			if (blue_time > 0.0f) {
				bluetime_zero_streak = 0;
				const float blue_diff = blue_time - last_bluetime_value;
				if (blue_diff < 0.0001f && blue_diff > -0.0001f) {
					++bluetime_same_streak;
				} else {
					bluetime_same_streak = 1;
					last_bluetime_value = blue_time;
				}
				if (bluetime_same_streak >= 3) {
					LocalPlayer_Data.MyBlueTime = 0.0f;
					last_nonzero_bluetime = 0.0f;
				} else {
					last_nonzero_bluetime = blue_time;
					LocalPlayer_Data.MyBlueTime = blue_time;
				}
			} else {
				bluetime_same_streak = 0;
				last_bluetime_value = 0.0f;
				++bluetime_zero_streak;
				if (bluetime_zero_streak >= 2) {
					LocalPlayer_Data.MyBlueTime = 0.0f;
				} else if (last_nonzero_bluetime > 0.0f) {
					LocalPlayer_Data.MyBlueTime = last_nonzero_bluetime;
				}
			}
			if (cur_energy > 0) {
				last_nonzero_energy = cur_energy;
				LocalPlayer_Data.MyCurEnergy = cur_energy;
			} else if (last_nonzero_energy > 0) {
				LocalPlayer_Data.MyCurEnergy = last_nonzero_energy;
			}
			LocalPlayer_Data.CameraPitch = LocalPlayer_Data.CameraEuler.x;
			LocalPlayer_Data.My_NameHash = mem.Read<int>(LocalPlayer_Data.Cache_NameHash + Offset::ActorModel.NameHash);
			mem.CloseScatterHandle(handle);
		}
		// 这些调用很轻量，直接调
		LocalPlayer_Data.IsSpectator = Global::WorldPtr.UserData->IsSpectator();
		LocalPlayer_Data._showExtraEvent = Global::WorldPtr.UserData->IsShowExtraEvent();
		// 本地玩家当前动作名指针 & 文本
		LocalPlayer_Data.Cache_ActionNamePtr =
			mem.Read<uintptr_t>(mem.Read<uintptr_t>(Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR + 0x20) + Offset::ActorModel.animClipName);
		{
			wchar_t MyName[80] = { 0 };
			mem.Read((uintptr_t)LocalPlayer_Data.Cache_ActionNamePtr + 0x14, MyName, sizeof(MyName));
			LocalPlayer_Data.MyActionName = MyName;
		}
		// 派生结果
		LocalPlayer_Data.MyCurTime = Global::WorldPtr.GlobalTime - LocalPlayer_Data.MyCurTimeForLogic;
		LocalPlayer_Data.My_ActorRootPtr = Global::Cache_LocalPlayer.Cache_ActorRoot_PTR;
		LocalPlayer_Data.Weapon_1 = JudgeWeapon(LocalPlayer_Data.My_1weapon, 1);
		LocalPlayer_Data.Weapon_2 = JudgeWeapon(LocalPlayer_Data.My_2weapon, 2);
		CharactorSyncManager tempData2;
		//tempData2.GetCharactorSyncManager();
		//LocalPlayer_Data.MyPing = tempData2.GamePing();
		did_read = true;
		// ========= 阶段 3：敌人数据批量 Scatter 读取（降低单轮耗时，提升自身数据刷新频率） =========
		const size_t total = current_players.size();
		if (total == 0)
		{
			data_buffers.SwapBuffers();
			report_read_stats(did_read);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		struct EnemyScatterItem
		{
			const ObjData* src = nullptr;
			PlayerData pd{};
			std::array<wchar_t, 80> action_name{};
		};

		const Vector3 myPosSnapshot = LocalPlayer_Data.MyPos;
		const double globalTimeSnapshot = Global::WorldPtr.GlobalTime;

		int nearCount = 0;
		std::vector<EnemyScatterItem> batch;
		batch.reserve(ENEMY_SCATTER_BATCH);
		std::vector<EnemyScatterItem*> full_read;
		full_read.reserve(ENEMY_SCATTER_BATCH);

		for (size_t base = 0; base < total; base += ENEMY_SCATTER_BATCH)
		{
			const size_t end = std::min<size_t>(base + ENEMY_SCATTER_BATCH, total);
			batch.clear();
			full_read.clear();

			auto base_handle = mem.CreateScatterHandle();
			for (size_t i = base; i < end; ++i)
			{
				const auto& PlayerTemp = current_players[i];
				// 无效对象 / 自己已经死的情况跳过
				if (PlayerTemp.O_ActorModel == nullptr ||
					(Global::Cache_LocalPlayer.L_ActorModel == PlayerTemp.O_ActorModel &&
						!PlayerTemp.O_ActorModel->IsAlive()))
				{
					continue;
				}

				batch.emplace_back();
				auto& item = batch.back();
				item.src = &PlayerTemp;
				item.pd.ISRobot = PlayerTemp.ISRobot;
				item.pd.Team = PlayerTemp.Team;
				item.pd.HeroId = PlayerTemp.HeroId;
				item.pd.PlayerName = PlayerTemp.PlayerName;
				item.pd.HeroName = GetHeroType(item.pd.HeroId);

				// 敌人 ActionNamePtr（链读取）
				item.pd.Cache_ActionNamePtr =
					mem.Read<uintptr_t>(mem.Read<uintptr_t>(PlayerTemp.Cache_EventTracksForLayer_PTR + 0x20) + Offset::ActorModel.animClipName);

				// Scatter 读取（一次执行覆盖该批次所有敌人）
				if (item.pd.Cache_ActionNamePtr)
				{
					mem.AddScatterReadRequest(
						base_handle,
						item.pd.Cache_ActionNamePtr + 0x14,
						item.action_name.data(),
						sizeof(item.action_name));
				}
				mem.AddScatterReadRequest<uintptr_t>(
					base_handle,
					PlayerTemp.Cache_GetNameHash_PTR + Offset::ActorModel.CurShareState,
					&item.pd.Cache_NameHash);
				mem.AddScatterReadRequest<bool>(
					base_handle,
					(uintptr_t)PlayerTemp.O_ActorModel + Offset::ActorModel.cullingVisible,
					&item.pd.O_Visible);
				mem.AddScatterReadRequest<WeaponType>(
					base_handle,
					PlayerTemp.Cache_ActorWeapon_PTR + Offset::ActorModel.WeaponType,
					&item.pd.O_ActorWeapon);
				mem.AddScatterReadRequest<int>(
					base_handle,
					(uintptr_t)PlayerTemp.O_PropertyData + Offset::ActorModel.ProPerty.WeaponId,
					&item.pd.O_WeaponID);
				mem.AddScatterReadRequest<int>(
					base_handle,
					PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.CurHpID * 4ull,
					&item.pd.O_CurHp);
				mem.AddScatterReadRequest<int>(
					base_handle,
					PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.HpMaxID * 4ull,
					&item.pd.O_MaxHp);
				mem.AddScatterReadRequest<int>(
					base_handle,
					PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.CurshieldID * 4ull,
					&item.pd.O_Curshield);
				mem.AddScatterReadRequest<int>(
					base_handle,
					PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.shieldMaxID * 4ull,
					&item.pd.O_Maxshield);
				mem.AddScatterReadRequest<int>(
					base_handle,
					PlayerTemp.Cache_PropertData_PTR + 0x30,
					&item.pd.O_CurAnger);
				mem.AddScatterReadRequest<Vector3>(
					base_handle,
					PlayerTemp.Cache_ActorRoot_PTR + 0x90,
					&item.pd.O_Pos);
			}

			if (!batch.empty())
			{
				mem.ExecuteReadScatter(base_handle);
			}
			mem.CloseScatterHandle(base_handle);

			for (auto& item : batch)
			{
				const auto& PlayerTemp = *item.src;
				// 派生数据计算（使用快照，避免同轮被更新导致不一致）
				item.pd.O_ActionName = item.action_name.data();
				item.pd.O_Dis = item.pd.O_Pos.DistTo(myPosSnapshot);
				item.pd.Cache_GetCurTimeForLogicState_PTR = PlayerTemp.Cache_GetCurTimeForLogicState_PTR;
				item.pd.Cache_ActorRootPtr = PlayerTemp.Cache_ActorRoot_PTR;
				item.pd.Cache_HitSimulate = (uintptr_t)PlayerTemp.O_HitSimulate;
				item.pd.Cache_PropertyData = (uintptr_t)PlayerTemp.O_PropertyData;
				item.pd.O_ActorModel = (uintptr_t)PlayerTemp.O_ActorModel;
				item.pd.O_ActorModel1 = PlayerTemp.O_ActorModel;
				item.pd.IsSoul = (PlayerTemp.O_ActorModel && !PlayerTemp.O_ActorModel->IsAlive());
				item.pd.O_HitSimulate = (uintptr_t)PlayerTemp.O_HitSimulate;
				if (item.pd.O_Dis <= ENEMY_FULLDATA_DISTANCE)
				{
					item.pd.O_Skill = PlayerTemp.O_Skill;
					full_read.push_back(&item);
				}
				if (item.pd.O_Dis <= 7.0f)
				{
					++nearCount;
				}
			}

			if (!full_read.empty())
			{
				auto full_handle = mem.CreateScatterHandle();
				for (auto* item : full_read)
				{
					const auto& PlayerTemp = *item->src;
					mem.AddScatterReadRequest<ActionType>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel,
						&item->pd.O_EndureLevel);
					mem.AddScatterReadRequest<int>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulatePtr + Offset::ActorModel.ActorKitS.HitSimulate.XuListate,
						&item->pd.O_XuListate);
					mem.AddScatterReadRequest<int>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType,
						&item->pd.O_RangeReactionType);
					mem.AddScatterReadRequest<int>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType,
						&item->pd.O_ReactionType);
					mem.AddScatterReadRequest<int>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType,
						&item->pd.O_ReactionExtraType);
					mem.AddScatterReadRequest<float>(
						full_handle,
						(uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration,
						&item->pd.O_ActionDuration);
					mem.AddScatterReadRequest<float>(
						full_handle,
						PlayerTemp.Cache_Bluetime_PTR + 0x24 + 3 * 0xC,
						&item->pd.O_BlueTime);
					mem.AddScatterReadRequest<double>(
						full_handle,
						PlayerTemp.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime,
						&item->pd.ObjCurTimeForLogic);
					mem.AddScatterReadRequest<Vector4>(
						full_handle,
						PlayerTemp.Cache_ActorRoot_PTR + 0xA0,
						&item->pd.O_Quat);
				}
				mem.ExecuteReadScatter(full_handle);
				mem.CloseScatterHandle(full_handle);
			}

			for (auto& item : batch)
			{
				if (item.pd.O_Dis <= ENEMY_FULLDATA_DISTANCE)
				{
					item.pd.O_NameHash = mem.Read<int>(item.pd.Cache_NameHash + 0x18);
					item.pd.ObjCurTime = globalTimeSnapshot - item.pd.ObjCurTimeForLogic;
					item.pd.O_Direction = fabsf(
						math::AngleDifference(
							math::get_enityangle(item.pd.O_Quat),
							math::VectorToRotationYaw(
								math::FindLookAtVector(item.pd.O_Pos, myPosSnapshot))));
				}
				writeBuffer.push_back(std::move(item.pd));
			}
		}

		LocalPlayer_Data.敌人数量5M内 = nearCount;
		data_buffers.SwapBuffers();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		report_read_stats(did_read);
		//MyLogTrue("执行时间: ", duration.count() / 1000.0, " ms");  // 输出毫秒，保留小数
		// 控制频率
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}

//DWORD WINAPI MyPlayer_Data(LPVOID lpParam){}
//缓存指针
DWORD WINAPI Cache_Player_World_Data(LPVOID lpParam)
{
	vector<ObjData> m_ListData;
	ObjData tempData;
	while (true)
	{
		auto start = chrono::high_resolution_clock::now();
		Global::WorldPtr.Character->GetCharacterManagerPtr();		//缓存CharacterManager
		
		Global::WorldPtr.Character->Cache_PlayerCount_ActorModelList();	//为后续做缓存
		Global::WorldPtr.UserData->GetUserDataManager();			//缓存UserDataManager
		Offset::MatrixAddr = Tool::GetMatrixAddr(Global::WorldPtr.Character->CharacterManagerPtr);   //矩阵
		Global::Cache_LocalPlayer.L_ActorModel = Global::WorldPtr.Character->GetLocalActorModel();		//缓存自身指针
		Global::Cache_LocalPlayer.Cache_WeaponIndex_PTR = Global::WorldPtr.UserData->Cache_WeaponIndex();
		Global::Cache_LocalPlayer.Cache_WeaponIndex_1PTR = Global::WorldPtr.UserData->GetWeaponList(0);
		Global::Cache_LocalPlayer.Cache_WeaponIndex_2PTR = Global::WorldPtr.UserData->GetWeaponList(1);
		if (!Global::Cache_LocalPlayer.L_ActorModel) {
			this_thread::sleep_for(chrono::milliseconds(1000)); // Sleep if local player is not valid
			continue;
		}
		/*Global::MySkillData.L_LocalSkillData = Global::WorldPtr.UserData->GetAllSkillDataList();
		Global::MySkillData.Cache_Skill_PTR = Global::MySkillData.L_LocalSkillData->Cache_Skill(0);
		Global::MySkillData.Cache_SkillID = mem.Read<uintptr_t>((uintptr_t)Global::MySkillData.L_LocalSkillData->Cache_Skill(1) + 0x60);*/
		//缓存ActorModel
		Global::Cache_LocalPlayer.L_PropertyData = Global::Cache_LocalPlayer.L_ActorModel->GetActorModelPropertyData();
		Global::Cache_LocalPlayer.L_ActorKit = Global::Cache_LocalPlayer.L_ActorModel->GetActorKit();
		if (!Global::Cache_LocalPlayer.L_ActorKit) {
			data_buffers.SwapBuffers();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		//Global::Cache_LocalPlayer.L_HitSimulate = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulateData();
		//Global::Cache_LocalPlayer.L_HitSimulatePtr = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulatePtr();
		Global::Cache_LocalPlayer.L_RuntimeProperty = Global::Cache_LocalPlayer.L_ActorModel->GetRuntimePropertyData();
		Global::Cache_LocalPlayer.L_ObjectMessenger = Global::Cache_LocalPlayer.L_ActorModel->GetObjectMessenger();
		//缓存ActorModel下面各项指针
		Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetEventTracksForLayer();
		Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetActorWeaponPtr();
		//Global::Cache_LocalPlayer.Cache_Pos_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetPosPtr();
		Global::Cache_LocalPlayer.Cache_ActorPhysics_PTR = Global::Cache_LocalPlayer.L_ActorKit ? Global::Cache_LocalPlayer.L_ActorKit->Cache_ActorPhysics() : 0;
		Global::Cache_LocalPlayer.Cache_ActorRoot_PTR = Global::Cache_LocalPlayer.L_ActorModel->Cache_GetActorRootPtr();
		//Global::Cache_LocalPlayer.Cache_Bluetime_PTR = Global::Cache_LocalPlayer.L_ActorKit->Cache_BlueTime();
		Global::Cache_LocalPlayer.Cache_PropertData_PTR = Global::Cache_LocalPlayer.L_RuntimeProperty->Cache_PropertData();
		Global::Cache_LocalPlayer.Cache_PropertyDataID = Global::Cache_LocalPlayer.L_RuntimeProperty->GetPropertyDataID();
		Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR = Global::Cache_LocalPlayer.L_ObjectMessenger->Cache_GetGlobalTime();
		Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR = Global::Cache_LocalPlayer.L_ObjectMessenger->Cache_GetCurTimeForLogicState();
		Global::Cache_LocalPlayer.Cache_AvgFPSStatic_PTR = 0;
		Global::Cache_LocalPlayer.CharactorSyncManagerPtr = Global::WorldPtr.Character->GetCharactorSyncManager();		//缓存GetCharactorSyncManager
		if (Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR)
		{
			auto gameBaseObj = mem.Read<uintptr_t>(Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR + Offset::GameBaseObject._Instance);
			if (gameBaseObj)
			{
				auto fpsManager = mem.Read<uintptr_t>(gameBaseObj + Offset::GameBaseObject._fps);
				if (fpsManager)
				{
					auto fpsKlass = mem.Read<uintptr_t>(fpsManager);
					Global::Cache_LocalPlayer.Cache_AvgFPSStatic_PTR = mem.Read<uintptr_t>(fpsKlass + 0xB8);
				}
			}
		}
		LocalPlayer_Data.MyTeam = Global::Cache_LocalPlayer.L_PropertyData->GetTeam();
		LocalPlayer_Data.MyHero = Global::Cache_LocalPlayer.L_PropertyData->GetHeroTID();
		//LocalPlayer_Data.playerNowBag = Global::Cache_LocalPlayer.L_PropertyData->PlayerBag(Global::Cache_LocalPlayer.Cache_PropertData_PTR);
		//std::cout << "扩容:" << LocalPlayer_Data.playerNowBag << endl;
		Global::MySkillData.MySkill = Global::WorldPtr.UserData->GetSkillID(LocalPlayer_Data.MyHero);
		/*cout << "F技能CD: " << Global::MySkillData.MySkill.F_CD << "F技能ID: " << Global::MySkillData.MySkill.F_SkillID << "F技能状态: " << Global::MySkillData.MySkill.F_State << "F技能持续CD: " << Global::MySkillData.MySkill.F_LeftContinueTime<<endl;
		cout << "V技能CD: " << Global::MySkillData.MySkill.V_CD << "V技能ID: " << Global::MySkillData.MySkill.V_SkillID << "V技能d状态: " << Global::MySkillData.MySkill.V_State << "V技能持续CD: " << Global::MySkillData.MySkill.V_LeftContinueTime<<endl;*/
		Global::Cache_LocalPlayer.Cache_GetNameHash_PTR = Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR;
		auto PlayerCount = Global::WorldPtr.Character->GetPlayerCount();
		m_ListData.clear();
		for (int i = 0; i < PlayerCount; i++)
		{
			auto List = Global::WorldPtr.Character->GetActorModelList(i);
			if (List == Global::Cache_LocalPlayer.L_ActorModel || List == NULL) continue;
			tempData.O_PropertyData = List->GetActorModelPropertyData();
			if (!tempData.O_PropertyData) continue;
			if (LocalPlayer_Data.MyTeam == tempData.O_PropertyData->GetTeam()) continue;
			//缓存对象
			tempData.O_ActorModel = List;
			tempData.O_ActorKit = List->GetActorKit();
			if (!tempData.O_ActorKit) continue;
			tempData.O_HitSimulate = tempData.O_ActorKit->GetActionHitSimulateData();
			tempData.O_HitSimulatePtr = tempData.O_ActorKit->GetActionHitSimulatePtr();
			tempData.O_RuntimeProperty = List->GetRuntimePropertyData();
			tempData.O_ObjectMessenger = List->GetObjectMessenger();
			if (!tempData.O_RuntimeProperty || !tempData.O_ObjectMessenger) continue;
			//缓存ActorModel下面各项指针
			tempData.Cache_EventTracksForLayer_PTR = List->Cache_GetEventTracksForLayer();
			tempData.Cache_ActorWeapon_PTR = List->Cache_GetActorWeaponPtr();
			//tempData.Cache_Pos_PTR = List->Cache_GetPosPtr();
			tempData.Cache_ActorRoot_PTR = List->Cache_GetActorRootPtr();
			tempData.Cache_Bluetime_PTR = tempData.O_ActorKit->Cache_BlueTime();
			tempData.Cache_PropertData_PTR = tempData.O_RuntimeProperty->Cache_PropertData();
			tempData.Cache_PropertyDataID = tempData.O_RuntimeProperty->GetPropertyDataID();
			tempData.Cache_GetCurTimeForLogicState_PTR = tempData.O_ObjectMessenger->Cache_GetCurTimeForLogicState();
			tempData.Cache_GetNameHash_PTR = tempData.Cache_GetCurTimeForLogicState_PTR;
			//以下缓存不经常变动的数据
			tempData.ISRobot = tempData.O_PropertyData->GetISRobotId();
			tempData.Team = tempData.O_PropertyData->GetTeam();
			tempData.HeroId = tempData.O_PropertyData->GetHeroTID();
			tempData.PlayerName = tempData.O_PropertyData->GetPlayerName();
			//int mySoulTest = tempData.O_PropertyData->GetHeroSoul();
			//tempData.O_PropertyData->GetCharactorSyncManager();
			//tempData.ping = tempData.O_PropertyData->GetPing();
			CharactorSyncManager tempData2;
			tempData2.GetCharactorSyncManager();
			tempData.ping = tempData2.GamePing();
			//tempData.playerNowBag = tempData2.PlayerBag(Global::Cache_LocalPlayer.Cache_PropertData_PTR);
			//std::cout << "状态:" << mySoulTest << endl;
			//std::cout << "延迟:" << ping1 << endl;
			//std::cout << "扩容:" << tempData.playerNowBag << endl;
			m_ListData.push_back(tempData);
		}
		{
			/*std::unique_lock lock(m_DataMutex);*/
			g_GameData = m_ListData;
		}
		auto stop = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
		/*std::cout << "Read time: " << duration.count() << " ms | ID:" << m_ListData.size() << endl;*/
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	return 0;
}
//读取玩家数据
DWORD WINAPI LocalPlayer_Object_Data(LPVOID lpParam) {
	const int max_threads = 20; // 最大并发线程数
	const int chunk_size = 2;  // 每个线程处理人数
	PlayerData tempData{};
	if (Function::Kmbox::Net::NetState == 0 && !是否连接Net)
	{
		是否连接Net = true;
		kmNet_monitor(atoi(Function::Kmbox::Net::Port));		//启动监听端口
	}
	while (true) {
		auto start = std::chrono::high_resolution_clock::now();
		// 阶段1：数据准备
		std::vector<ObjData> current_players;
		{
			/*std::unique_lock lock(m_DataMutex);*/
			current_players = g_GameData;
		}
		//LocalPlayer_Data.敌人数量5M内 = 0;
		auto& writeBuffer = data_buffers.GetWriteBuffer();
		writeBuffer.clear();
		writeBuffer.reserve(current_players.size());
		size_t total = current_players.size();
		size_t thread_count = (total + chunk_size - 1) / chunk_size;
		if (thread_count > max_threads) thread_count = max_threads;
		std::vector<std::vector<PlayerData>> thread_results(thread_count);
		std::vector<std::future<void>> futures;
		// 此处暂时用于获取自身的数据
		LocalPlayer_Data.myNowBag = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull);
		LocalPlayer_Data.myNowBagWeapon = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.weaponMaxID * 4ull);
		LocalPlayer_Data.myNowBagsoul = mem.Read<int>(Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.soulItemMaxID * 4ull);
		Global::Cache_LocalPlayer.L_ActorKit = Global::Cache_LocalPlayer.L_ActorModel->GetActorKit();
		if (!Global::Cache_LocalPlayer.L_ActorKit) {
			data_buffers.SwapBuffers();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		if (Global::Cache_LocalPlayer.L_ActorKit)
		{
			Global::Cache_LocalPlayer.Cache_CameraAgent_PTR = Global::Cache_LocalPlayer.L_ActorKit->GetCameraAgent();
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_CameraAgent_PTR = NULL;
		}
		if (Global::Cache_LocalPlayer.Cache_CameraAgent_PTR)
		{
			Global::Cache_LocalPlayer.Cache_CameraController_PTR = GetCameraController(Global::Cache_LocalPlayer.Cache_CameraAgent_PTR);
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_CameraController_PTR = NULL;
		}
		if (Global::Cache_LocalPlayer.Cache_CameraController_PTR)
		{
			Global::Cache_LocalPlayer.Cache_AdventureRig_PTR = GetAdventureRig(Global::Cache_LocalPlayer.Cache_CameraController_PTR);
		}
		else
		{
			Global::Cache_LocalPlayer.Cache_AdventureRig_PTR = NULL;
		}
		Global::Cache_LocalPlayer.L_HitSimulate = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulateData();
		Global::Cache_LocalPlayer.L_HitSimulatePtr = Global::Cache_LocalPlayer.L_ActorKit->GetActionHitSimulatePtr();
		Global::Cache_LocalPlayer.Cache_Bluetime_PTR = Global::Cache_LocalPlayer.L_ActorKit->Cache_BlueTime();
		LocalPlayer_Data.XuListate = mem.Read <int>(Global::Cache_LocalPlayer.L_HitSimulatePtr + Offset::ActorModel.ActorKitS.HitSimulate.XuListate);
		LocalPlayer_Data.MyBlueTime = mem.Read <float>((uintptr_t)Global::Cache_LocalPlayer.Cache_Bluetime_PTR + 0x24 + 3 * 0xC);
		LocalPlayer_Data.MyWeaponIndex = mem.Read<int>((uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_PTR + 0x20);
		LocalPlayer_Data.MyEndureLevel = mem.Read<ActionType>((uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel);
		wchar_t MyName[80] = { NULL };
		mem.Read((uintptr_t)LocalPlayer_Data.Cache_ActionNamePtr + 0x14, MyName, sizeof(MyName));
		LocalPlayer_Data.MyActionName = MyName;
		//LocalPlayer_Data.MyActionName = mem.Read<uintptr_t>((uintptr_t)LocalPlayer_Data.Cache_ActionNamePtr + 0x14);
		LocalPlayer_Data.IsSpectator = Global::WorldPtr.UserData->IsSpectator();
		LocalPlayer_Data._showExtraEvent = Global::WorldPtr.UserData->IsShowExtraEvent();
		//mem.AddScatterReadRequest(handle, (uintptr_t)LocalPlayer_Data.Cache_ActionNamePtr + 0x14, &MyName, sizeof(MyName));
		//mem.AddScatterReadRequest<ActionType>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel, &LocalPlayer_Data.MyEndureLevel);
		//mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_PTR + 0x20, &LocalPlayer_Data.MyWeaponIndex);
		///mem.AddScatterReadRequest<float>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_Bluetime_PTR + 0x24 + 3 * 0xC, &LocalPlayer_Data.MyBlueTime);
		//cout << "扩容获取:" << LocalPlayer_Data.myNowBag << endl;
		//cout << "武器:" << LocalPlayer_Data.myNowBagWeapon << endl;
		//cout << "魂玉获取:" << LocalPlayer_Data.myNowBagsoul << endl;
		//cout << "蓄力状态:" << LocalPlayer_Data.XuListate << endl;
		//cout << "进蓝时间:" << LocalPlayer_Data.MyBlueTime << endl;
		//cout << "当前武器位:" << LocalPlayer_Data.MyWeaponIndex << endl;
		//cout << "是否为观战模式:" << LocalPlayer_Data.IsSpectator << endl;
		//cout << "是否打开背包:" << LocalPlayer_Data._showExtraEvent << endl;
		for (size_t t = 0; t < thread_count; ++t) {
			size_t begin = t * chunk_size;
			size_t end = min(begin + chunk_size, total);
			futures.emplace_back(std::async(std::launch::async, [&, begin, end, t]() {
				for (size_t i = begin; i < end; ++i) {
					const auto& PlayerTemp = current_players[i];
					PlayerData tempData{};
					if (PlayerTemp.O_ActorModel == nullptr ||
						(Global::Cache_LocalPlayer.L_ActorModel == PlayerTemp.O_ActorModel && !PlayerTemp.O_ActorModel->IsAlive())) {
						continue;
						//mem.AddScatterReadRequest<int>(handle, Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull, &LocalPlayer_Data.MyCurkuorong);
					}
					tempData.ISRobot = PlayerTemp.ISRobot;
					tempData.Team = PlayerTemp.Team;
					tempData.HeroId = PlayerTemp.HeroId;
					tempData.PlayerName = PlayerTemp.PlayerName;
					tempData.HeroName = GetHeroType(tempData.HeroId);
					LocalPlayer_Data.Cache_ActionNamePtr = mem.Read<uintptr_t>(mem.Read<uintptr_t>(Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR + 0x20) + 0x18);
					tempData.Cache_ActionNamePtr = mem.Read<uintptr_t>(mem.Read<uintptr_t>(PlayerTemp.Cache_EventTracksForLayer_PTR + 0x20) + 0x18);
					auto handle = mem.CreateScatterHandle();
					//wchar_t MyName[80] = { NULL };
					//mem.AddScatterReadRequest(handle, (uintptr_t)LocalPlayer_Data.Cache_ActionNamePtr + 0x14, &MyName, sizeof(MyName));
					//mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulatePtr + Offset::ActorModel.ActorKitS.HitSimulate.XuListate, &LocalPlayer_Data.XuListate);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType, &LocalPlayer_Data.MyRangeReactionType);
					//mem.AddScatterReadRequest<ActionType>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel, &LocalPlayer_Data.MyEndureLevel);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType, &LocalPlayer_Data.MyReactionType);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType, &LocalPlayer_Data.MyReactionExtraType);
					//mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionParryType, &LocalPlayer_Data.MyReactionParryType);
					//mem.AddScatterReadRequest<int>(handle, Global::Cache_LocalPlayer.Cache_ActorPhysics_PTR + Offset::ActorModel.ActorKitS.currentCollisionFlags, &LocalPlayer_Data.MyCollisionFlags);
					//mem.AddScatterReadRequest<float>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_Bluetime_PTR + 0x24 + 3 * 0xC, &LocalPlayer_Data.MyBlueTime);
					mem.AddScatterReadRequest<double>(handle, Global::Cache_LocalPlayer.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime, &LocalPlayer_Data.MyCurTimeForLogic);
					mem.AddScatterReadRequest<uintptr_t>(handle, Global::Cache_LocalPlayer.Cache_GetNameHash_PTR + 0x48, &LocalPlayer_Data.Cache_NameHash);
					mem.AddScatterReadRequest<float>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration, &LocalPlayer_Data.MyActionDuration);
					mem.AddScatterReadRequest<double>(handle, Global::Cache_LocalPlayer.Cache_GetGlobalTime_PTR + Offset::ActorModel._globalTime, &Global::WorldPtr.GlobalTime); // Class: GameBaseObject->Double _globalTime
					mem.AddScatterReadRequest<WeaponType>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_ActorWeapon_PTR + Offset::ActorModel.WeaponType, &LocalPlayer_Data.MyWeaponType);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_1PTR + 0x14, &LocalPlayer_Data.My_1weapon);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.Cache_WeaponIndex_2PTR + 0x14, &LocalPlayer_Data.My_2weapon);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)Global::Cache_LocalPlayer.L_PropertyData + Offset::ActorModel.ProPerty.SkillStatus, &LocalPlayer_Data.MySkillState);
					mem.AddScatterReadRequest<int>(handle, Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.CurEnergyID * 4ull, &LocalPlayer_Data.MyCurEnergy);// Class: RuntimeDevicePropertyData->List`1 propertyCallbacks
					mem.AddScatterReadRequest<int>(handle, Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.CurshieldID * 4ull, &LocalPlayer_Data.MyCurshield);
					//cout << "指针2:" << Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull << endl;
					//mem.AddScatterReadRequest<int>(handle, Global::Cache_LocalPlayer.Cache_PropertData_PTR + 0x20 + Global::Cache_LocalPlayer.Cache_PropertyDataID.bagMaxID * 4ull, &LocalPlayer_Data.MyCurkuorong);
					mem.AddScatterReadRequest<Vector3>(handle, Global::Cache_LocalPlayer.Cache_ActorRoot_PTR + 0x90, &LocalPlayer_Data.MyPos);
					wchar_t Name[80] = { NULL };
					mem.AddScatterReadRequest(handle, tempData.Cache_ActionNamePtr + 0x14, &Name, sizeof(Name));
					mem.AddScatterReadRequest<ActionType>(handle, (uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.endureLevel, &tempData.O_EndureLevel);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_HitSimulatePtr + Offset::ActorModel.ActorKitS.HitSimulate.XuListate, &tempData.O_XuListate);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.RangeReactionType, &tempData.O_RangeReactionType);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionType, &tempData.O_ReactionType);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ReactionExtraType, &tempData.O_ReactionExtraType);
					mem.AddScatterReadRequest<float>(handle, (uintptr_t)PlayerTemp.O_HitSimulate + Offset::ActorModel.ActorKitS.HitSimulate.ActionDuration, &tempData.O_ActionDuration);
					mem.AddScatterReadRequest<float>(handle, PlayerTemp.Cache_Bluetime_PTR + 0x24 + 3 * 0xC, &tempData.O_BlueTime);
					mem.AddScatterReadRequest<uintptr_t>(handle, PlayerTemp.Cache_GetNameHash_PTR + 0x48, &tempData.Cache_NameHash);
					mem.AddScatterReadRequest<bool>(handle, (uintptr_t)PlayerTemp.O_ActorModel + Offset::ActorModel.cullingVisible, &tempData.O_Visible);
					mem.AddScatterReadRequest<WeaponType>(handle, PlayerTemp.Cache_ActorWeapon_PTR + Offset::ActorModel.WeaponType, &tempData.O_ActorWeapon);
					mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_PropertyData + Offset::ActorModel.ProPerty.WeaponId, &tempData.O_WeaponID);
					/*mem.AddScatterReadRequest<int>(handle, (uintptr_t)PlayerTemp.O_PropertyData + Offset::ActorModel.ProPerty.SkillStatus, &tempData.O_SkillStatus);*/
					mem.AddScatterReadRequest<int>(handle, PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.CurHpID * 4ull, &tempData.O_CurHp);
					mem.AddScatterReadRequest<int>(handle, PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.HpMaxID * 4ull, &tempData.O_MaxHp);
					mem.AddScatterReadRequest<int>(handle, PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.CurshieldID * 4ull, &tempData.O_Curshield);
					mem.AddScatterReadRequest<int>(handle, PlayerTemp.Cache_PropertData_PTR + 0x20 + PlayerTemp.Cache_PropertyDataID.shieldMaxID * 4ull, &tempData.O_Maxshield);
					mem.AddScatterReadRequest<int>(handle, PlayerTemp.Cache_PropertData_PTR + 0x30, &tempData.O_CurAnger);
					mem.AddScatterReadRequest<double>(handle, PlayerTemp.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime, &tempData.ObjCurTimeForLogic);
					mem.AddScatterReadRequest<Vector3>(handle, PlayerTemp.Cache_ActorRoot_PTR + 0x90, &tempData.O_Pos);
					mem.AddScatterReadRequest<Vector4>(handle, PlayerTemp.Cache_ActorRoot_PTR + 0xA0, &tempData.O_Quat);
					mem.ExecuteReadScatter(handle);
					mem.CloseScatterHandle(handle);
					//LocalPlayer_Data.dw = MyName;
					LocalPlayer_Data.My_NameHash = mem.Read<int>(LocalPlayer_Data.Cache_NameHash + 0x18);
					LocalPlayer_Data.MyPing = PlayerTemp.ping;
					LocalPlayer_Data.MyCurTime = Global::WorldPtr.GlobalTime - LocalPlayer_Data.MyCurTimeForLogic;
					LocalPlayer_Data.My_ActorRootPtr = Global::Cache_LocalPlayer.Cache_ActorRoot_PTR;
					LocalPlayer_Data.Weapon_1 = JudgeWeapon(LocalPlayer_Data.My_1weapon);
					LocalPlayer_Data.Weapon_2 = JudgeWeapon(LocalPlayer_Data.My_2weapon);
					tempData.O_ActionName = Name;
					tempData.O_Dis = tempData.O_Pos.DistTo(LocalPlayer_Data.MyPos);
					if (Global::WorldPtr.UserData && Global::WorldPtr.UserData->UserDataManagerPtr)
					{
						int enemyFid = mem.Read<int>((uintptr_t)PlayerTemp.O_ActorModel + Offset::SoulData.fid);
						if (enemyFid != 0)
						{
							tempData.O_Skill = Global::WorldPtr.UserData->GetSkillIDByFid(tempData.HeroId, enemyFid);
						}
					}
					tempData.ObjCurTime = Global::WorldPtr.GlobalTime - tempData.ObjCurTimeForLogic;
					tempData.Cache_GetCurTimeForLogicState_PTR = PlayerTemp.Cache_GetCurTimeForLogicState_PTR;
					tempData.Cache_ActorRootPtr = PlayerTemp.Cache_ActorRoot_PTR;
					tempData.Cache_HitSimulate = (uintptr_t)PlayerTemp.O_HitSimulate;
					tempData.Cache_PropertyData = (uintptr_t)PlayerTemp.O_PropertyData;
					tempData.O_ActorModel = (uintptr_t)PlayerTemp.O_ActorModel;
					tempData.O_ActorModel1 = PlayerTemp.O_ActorModel;
					tempData.IsSoul = (PlayerTemp.O_ActorModel && !PlayerTemp.O_ActorModel->IsAlive());
					tempData.O_HitSimulate = (uintptr_t)PlayerTemp.O_HitSimulate;
					/*tempData.O_Direction = GetAngle(tempData.O_Quat, tempData.O_Pos, LocalPlayer_Data.MyPos);*/
					tempData.O_Direction = fabsf(math::AngleDifference(math::get_enityangle(tempData.O_Quat), math::VectorToRotationYaw(math::FindLookAtVector(tempData.O_Pos, LocalPlayer_Data.MyPos))));
					if (tempData.O_Dis <= 8.f) { LocalPlayer_Data.敌人数量5M内++; }
					thread_results[t].push_back(tempData);
				}
				}));
		}
		for (auto& f : futures) f.get();
		// 合并所有线程结果
		for (const auto& vec : thread_results) {
			writeBuffer.insert(writeBuffer.end(), vec.begin(), vec.end());
		}
		data_buffers.SwapBuffers();
		/*auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << "Process time: " << duration.count() << " ms | ID:" << current_players.size() << endl;*/
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
//读取物品
DWORD WINAPI GetItemData(LPVOID lpParam)
{
	vector<ItemInfoData> m_ListData;
	ItemInfoData tempData;
	const uint32_t kMaxEntityCount = 200000;
	while (true)
	{
		uint64_t managerRoot = mem.Read<uint64_t>(Offset::GameAssembly + Offset::m_EntityManager);
		if (managerRoot < 0x10000) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		managerRoot = mem.Read<uint64_t>(managerRoot + 0xB8);
		if (managerRoot < 0x10000) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		managerRoot = mem.Read<uint64_t>(managerRoot + 0x8);
		if (managerRoot < 0x10000) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t GameEntityManager = mem.Read<uint64_t>(managerRoot + 0x28); // Class: GameEntity->GameEntityManager entityManager + Class: GameEntityManager->HashSet`1 entities
		if (GameEntityManager < 0x10000) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t arrayBase = mem.Read<uint64_t>(GameEntityManager + 0x18);
		if (arrayBase < 0x10000) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t Array = arrayBase + 0x28;
		uint32_t Count = mem.Read<uint32_t>(GameEntityManager + 0x24);
		if (Count > kMaxEntityCount) {
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		m_ListData.clear();
		for (int A = 0; A < Count; A++)
		{
			uint64_t GameEntity = mem.Read<uint64_t>(static_cast<uint64_t>(Array) + static_cast<uint64_t>(A) * 0x10);
			if (GameEntity <= 0) { continue; }
			uint64_t m_ItemPointer = mem.Read<uint64_t>(GameEntity);
			if (m_ItemPointer <= 0) {
				continue;
			}
			uint64_t m_ItemType = mem.Read<uint64_t>(m_ItemPointer + 0x10);
			char m_ItemTypeTxt[32];
			mem.Read(m_ItemType, &m_ItemTypeTxt, sizeof(m_ItemTypeTxt));
			m_ItemTypeTxt[sizeof(m_ItemTypeTxt) - 1] = '\0';
			if (!strcmp((const char*)m_ItemTypeTxt, "DropItemEntity"))
			{
				uint32_t ItemID = mem.Read<uint32_t>(mem.Read<uint64_t>(GameEntity + 0x58) + 0x28);
				if (ItemID == 0 || ItemID > 4000000) { continue; }
				uint64_t Itemtransform = GetItemPosAddr(GameEntity);
				if (Itemtransform <= 0) { continue; }
				uint32_t  ItemIndex = mem.Read<uint32_t>(Itemtransform + 0x40);
				uint64_t ItemPosPtr = mem.Read<uint64_t>(mem.Read<uint64_t>(Itemtransform + 0x38) + 0x18);
				if (itemTable.count(ItemID) != 0)
				{
					tempData.ID = ItemID;
					//auto& [Name, Color, itemType] = itemTable[tempData.ID];
					auto [Name, Color, itemType] = GetItemInfo(tempData.ID);
					tempData.Pos = mem.Read<Vector3>(ItemPosPtr + ItemIndex * 48);
					tempData.Name = Name;
					tempData.Color = Color;
					tempData.ItemType = itemType;
					m_ListData.push_back(tempData);
				}
				else if (Function::Flag::显示未知物品ID) {
					tempData.ID = ItemID;
					tempData.Pos = mem.Read<Vector3>(ItemPosPtr + ItemIndex * 48);
					tempData.Name = "ID:" + std::to_string(ItemID);
					tempData.Color = ImColor(255, 255, 255);
					tempData.ItemType = 0;
					m_ListData.push_back(tempData);
				}
				if (Function::Flag::数据遍历 == true)
				{
					if (Function::Flag::物品数据 == true)
					{
						auto& [Name, Color, itemType] = itemTable[ItemID];
						cout << "ItemID:" << ItemID << " Name:" << Name << " Color:" << Color << " ItemType:" << itemType << endl;
					}
					this_thread::sleep_for(chrono::milliseconds(500));
				}
			}
		}
		{
			全_物品数据 = m_ListData;
		}
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	return 0;
}
//读取堆堆任务
DWORD WINAPI GetUserBattleData(LPVOID lpParam)
{
	vector<InteractiveInfoData> m_ListData;
	InteractiveInfoData tempData;
	const uint32_t kMaxInteractiveCount = 200000;
	while (true)
	{
		uint64_t userDataRoot = mem.Read<uint64_t>(Offset::GameAssembly + Offset::m_UserDataManager);
		if (userDataRoot == 0) {
			//MyLogTrue("userDataRoot1 == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		userDataRoot = mem.Read<uint64_t>(userDataRoot + 0xB8 + 0);
		if (userDataRoot == 0) {
			//MyLogTrue("userDataRoot2 == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t userDataManager = mem.Read<uint64_t>(userDataRoot + 0);
		if (userDataManager == 0) {
			//MyLogTrue("userDataManager == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t userBattleData = mem.Read<uint64_t>(userDataManager + 0x30);
		if (userBattleData == 0) {
			//MyLogTrue("userBattleData == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t interactiveManagerPtr = mem.Read<uint64_t>(userBattleData + Offset::UserData.InteractiveManager);
		if (interactiveManagerPtr == 0) {
			//MyLogTrue("interactiveManagerPtr == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t InteractiveManager = mem.Read<uint64_t>(interactiveManagerPtr + 0x10);
		if (InteractiveManager == 0) {
			//MyLogTrue("InteractiveManager == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint64_t InteractiveArray = mem.Read<uint64_t>(InteractiveManager + 0x18);
		if (InteractiveArray == 0) {
			//MyLogTrue("InteractiveArray == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		uint32_t InteractiveCount = mem.Read<uint32_t>(InteractiveArray + 0x18);
		if (InteractiveCount > kMaxInteractiveCount) {
			//MyLogTrue("InteractiveCount == null");
			this_thread::sleep_for(chrono::milliseconds(200));
			continue;
		}
		//MyLogTrue("堆数量:", InteractiveCount);
		m_ListData.clear();
		for (uint32_t A = 0; A < InteractiveCount; A++)
		{
			uint64_t GameEntity = mem.Read<uint64_t>(static_cast<uint64_t>(InteractiveArray) + 0x18 + static_cast<uint64_t>(A) * 0x18);
			if (!GameEntity) { continue; }
			uint32_t InteractiveId = mem.Read<uint32_t>(GameEntity + Offset::ArticleData.interactiveId);
			uint32_t status = mem.Read<uint32_t>(GameEntity + Offset::ArticleData.status);
			if (InteractiveTable.count(InteractiveId) != 0 && status == 0)
			{
				//LOG(to_string(InteractiveId).c_str());
				//LOG("\n");
				tempData.ID = InteractiveId;
				tempData.status = status;
				auto& [Name, Color, itemType] = InteractiveTable[tempData.ID];
				tempData.Pos = mem.Read<Vector3>(GameEntity + Offset::ArticleData.position);
				tempData.Name = Name;
				tempData.Color = Color;
				tempData.ItemType = itemType;
				m_ListData.push_back(tempData);
			}
			else if (Function::Flag::显示未知堆ID && status == 0)
			{
				tempData.ID = InteractiveId;
				tempData.status = status;
				tempData.Pos = mem.Read<Vector3>(GameEntity + Offset::ArticleData.position);
				tempData.Name = "ID:" + std::to_string(InteractiveId);
				tempData.Color = ImColor(255, 255, 255);
				tempData.ItemType = 0;
				m_ListData.push_back(tempData);
			}
			if (Function::Flag::数据遍历 == true)
			{
				if (Function::Flag::堆堆数据 == true && status == 0)
				{
					auto& [Name, Color, itemType] = InteractiveTable[InteractiveId];
					cout << "InteractiveId:" << InteractiveId << " Name:" << Name << " Color:" << Color << " itemType:" << itemType << endl;
				}
				this_thread::sleep_for(chrono::milliseconds(500));
			}
		}
		{
			//std::lock_guard<std::mutex> lock(g_InteractiveMutex);
			全_金堆数据 = m_ListData;
		}
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	return 0;
}
//自动博弈
DWORD WINAPI SmarThread(LPVOID lpParam)
{
	int countTest = 0;
	int last_read_index = -1;
	ObjData targetPlay;
	bool needPass = false;
	bool 武器日志 = false;
	while (true)
	{
		if (Function::Flag::性能测试开关) {
			Function::Flag::性能测试开关 = false;
			int tempI = 0;
			ULONGLONG WaitTIme1 = GetTickCount64() + 500;
			//std::call_once(g_timePeriodInitFlag, InitTimePeriodOnce);
			while (WaitTIme1 > GetTickCount64()) {
				MyLogTrue("性能测试--循环次数：", tempI);
				tempI++;
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
				Sleep(1);
			}
			MyLogTrue("性能测试--结束");
		}
		
		//MyLog("fps:", LocalPlayer_Data.MyFPS);
		//MyLog("我上次闪避时间: ", 我上次闪避时间);
		//MyLog("敌人数量5m:", LocalPlayer_Data.敌人数量5M内);
		//wcout << "动作名=" << LocalPlayer_Data.MyActionName << "动作哈希=" << LocalPlayer_Data.My_NameHash << endl;
		//MyLog("我的攻击状态:", LocalPlayer_Data.MyRangeReactionType);
		//wcout << "敌人动作名=" << FatPlayerData.O_ActionName << "攻击状态=" << FatPlayerData.O_RangeReactionType << "等级=" << FatPlayerData.O_EndureLevel << "-敌人行动时长=" << FatPlayerData.ObjCurTime << endl;
		//awwcout << "动作名=" << LocalPlayer_Data.MyActionName << "我的攻击状态=" << LocalPlayer_Data.MyRangeReactionType << "-捏蓝时间=" << LocalPlayer_Data.MyBlueTime << "等级=" << LocalPlayer_Data.MyEndureLevel << "-我的行动时长=" << LocalPlayer_Data.MyCurTime << endl;
		//auto ObjCurTime = (Global::WorldPtr.GlobalTime - FatPlayerData.ObjCurTimeForLogic);
		//MyLog("ObjCurTime=", ObjCurTime);
		//MyLog("GlobalTime =", Global::WorldPtr.GlobalTime);
		//double 动画总时间 = mem.Read<double>(FatPlayerData.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.length);
		//double nowTime = mem.Read<double>(FatPlayerData.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.nowTime);
		//double 动作时间 = mem.Read<double>(FatPlayerData.Cache_GetCurTimeForLogicState_PTR + Offset::ActorModel.currentTime);
		//MyLog("当前动作时间", FatPlayerData.ObjCurTimeForLogic);
		//MyLog("动画总时间", 动画总时间);
		//MyLog("当前动画执行时间", nowTime);
		//MyLog("视角测试--" , LocalPlayer_Data.CameraPitch); >20
		//MyLog("我的状态：", LocalPlayer_Data.MyRangeReactionType);
		//MyLog("敌人O_EndureLevel：", FatPlayerData.O_EndureLevel);
		//if (!Function::Shock::Self_play) continue;
		//if (LocalPlayer_Data.MyWeaponType != WeaponType::Sw) {
		//	continue;
		//}
		if (振刀逻辑::侧键是否按下()) // 鼠标侧键按下
		{
			是否半自动模式 = false;
			needPass = true;
			if (Global::Cache_LocalPlayer.L_ActorModel == nullptr) {continue;}
			/*闪避总线程*/
			if (Function::HeroActivation::IsDodge) {
				if (LocalPlayer_Data.MyEndureLevel == 6 && !拼刀状态(LocalPlayer_Data.MyActionName) && !JudgePanshi(LocalPlayer_Data.MyActionName)) {
					if (Judgefly(LocalPlayer_Data.MyActionName) && Function::侧键博弈::切拳起身) {
						string 标识 = "起身--";
						MyLog(标识, "开始起身");
						int sendKey = KEY_A;
						if (rand() % 2) {
							sendKey = KEY_D;
						}

						root_sendKeyPress(KEY_0_CPARENTHESIS);
						Sleep(10);
						root_sendKeyUp(KEY_0_CPARENTHESIS);

						ULONGLONG WaitTIme1 = GetTickCount64() + 1500;
						while (WaitTIme1 > GetTickCount64()) {
							Sleep(1);
							//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
							if (LocalPlayer_Data.MyWeaponType == WeaponType::Node)
							{
								MyLog(标识, "切拳");
								break;
							}
							else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName) || !振刀逻辑::侧键是否按下())
							{

								root_sendKeyUpAll();
								break;
							}

						}

						root_sendKeyPress(sendKey);
						
						ULONGLONG WaitTIme = GetTickCount64() + 5000;
						while (WaitTIme > GetTickCount64()) {
							Sleep(1);
							//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
							if (倒地翻滚状态(LocalPlayer_Data.MyActionName))
							{
								MyLog(标识, "翻滚状态");
								break;
							}
							else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName) 
								|| !振刀逻辑::侧键是否按下() || (!倒地状态(LocalPlayer_Data.MyActionName) && !Judgefly(LocalPlayer_Data.MyActionName)))
							{
								
								root_sendKeyUpAll();
								break;
							}
						}

						Sleep(10);
						//倒地状态(LocalPlayer_Data.MyActionName)
						root_sendKeyPress(上次武器按键);
						Sleep(10);
						root_sendKeyUp(上次武器按键);
						
						ULONGLONG WaitTImebb1 = GetTickCount64() + 500;
						while (WaitTImebb1 > GetTickCount64()) {
							Sleep(1);
							//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
							if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && LocalPlayer_Data.MyWeaponType != 0)
							{
								MyLog(标识, "切回武器");
								break;
							}
							else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName) || !振刀逻辑::侧键是否按下())
							{

								root_sendKeyUpAll();
								break;
							}
							
						}
						root_sendKeyUp(sendKey);
						Sleep(50);
						continue;
						bool 出巴掌 = (rand() % 3 < 2);
						if (FatPlayerData.O_Dis < 7) {
							// 直接扇一个巴掌
							root_kmNet_mouse_right(0);
							root_kmNet_mouse_right(1);
							int 对面闪避 = false;
							ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
							while (WaitTIme2 > GetTickCount64()) {
								Sleep(1);
								//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
								if (LocalPlayer_Data.MyBlueTime >= 0.22 && (出巴掌 || FatPlayerData.O_BlueTime > 0.1))
								{
									MyLog(标识, "扇巴掌，出蓄力时间-", LocalPlayer_Data.MyBlueTime);
									break;
								}
								else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName))
								{

									root_sendKeyUpAll();
									break;
								}
								else if (LocalPlayer_Data.MyBlueTime >= 0.3) {
									对面闪避 = true;
									break;
								}
								//else if (LocalPlayer_Data.MyBlueTime >= 0.18 && JudgeShortFlash(FatPlayerData.O_ActionName)) {
								//	// 蓄力过程中对面闪了
								//	对面闪避 = true;
								//	break;
								//}
							}
							if (对面闪避) {
								MyLog(标识, "c切刀");
								root_sendKeyPress(KEY_C);
								int WaitTImeTime = 0;
								ULONGLONG WaitTImebb = GetTickCount64() + 1000;
								while (WaitTImebb > GetTickCount64()) {
									//MyLog("c切刀 循环次数:", WaitTImeTime);
									if (WaitTImeTime > 80 || JudgeCrouch(LocalPlayer_Data.MyActionName)) {

										break;
									}
									else if (JudgeZhenDaoing(LocalPlayer_Data.MyActionName)) {
										root_sendKeyUpAll();
										break;
									}
									WaitTImeTime++;
									Sleep(1);
								}
								振刀逻辑::CollideKnife();

								root_sendKeyPress(上次武器按键);

								Sleep(10);

								root_sendKeyUp(上次武器按键);

								root_sendKeyUp(KEY_C);
								Sleep(15);
								root_sendKeyUpAll();
							}
							else {
								root_kmNet_mouse_right(0);
								ULONGLONG WaitTIme12 = GetTickCount64() + 500;
								while (WaitTIme12 > GetTickCount64()) {
									Sleep(1);
									//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
									if (LocalPlayer_Data.MyEndureLevel == 5)
									{
										MyLog(标识, "出巴掌蓄力后摇结束");
										break;
									}
									else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName))
									{

										root_sendKeyUpAll();
										break;
									}
								}

								root_sendKeyUpAll();
								root_sendKeyPress(KEY_S);
								root_sendKeyPress(KEY_LEFTSHIFT);
								ULONGLONG WaitTImeb1 = GetTickCount64() + 1500;
								while (WaitTImeb1 > GetTickCount64()) {
									if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
										break;
									}
									if (振刀逻辑::是否特殊状态())
									{
										root_sendKeyUpAll();
										break;
									}
									Sleep(1);
								}


								root_sendKeyPress(上次武器按键);


								root_kmNet_mouse_left(1);
								Sleep(10);
								root_sendKeyUp(KEY_LEFTSHIFT);
								root_sendKeyUp(KEY_S);
								root_sendKeyUp(上次武器按键);
								ULONGLONG WaitTImeb2 = GetTickCount64() + 1500;
								while (WaitTImeb2 > GetTickCount64()) {
									if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && LocalPlayer_Data.MyWeaponType != 0) {
										MyLog("切刀成功");
										break;
									}
									if (振刀逻辑::是否特殊状态())
									{
										root_sendKeyUpAll();
										break;
									}
									Sleep(1);
								}
							}
							
						}
						/*else if(FatPlayerData.O_Dis < 10){
							
							root_kmNet_mouse_right(0);
							root_kmNet_mouse_right(1);
							int 对面闪避 = false;
							int WaitTIme2Temp = 0;
							ULONGLONG WaitTIme2 = GetTickCount64() + 500;
							while (WaitTIme2 > GetTickCount64()) {
								Sleep(1);
								//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
								if (WaitTIme2Temp > 150)
								{
									MyLog(标识, "断巴掌，蓄力时间-", LocalPlayer_Data.MyBlueTime);
									break;
								}
								else if (挂墙状态(LocalPlayer_Data.MyActionName) || 拼刀状态(LocalPlayer_Data.MyActionName))
								{

									root_sendKeyUpAll();
									break;
								}
								WaitTIme2Temp++;
							}

							root_sendKeyPress(KEY_S);
							root_sendKeyPress(KEY_LEFTSHIFT);
							ULONGLONG WaitTImeb1 = GetTickCount64() + 1500;
							while (WaitTImeb1 > GetTickCount64()) {
								if (JudgeShortFlash(LocalPlayer_Data.MyActionName)) {
									break;
								}
								if (振刀逻辑::是否特殊状态())
								{
									root_sendKeyUpAll();
									break;
								}
								Sleep(1);
							}

							root_kmNet_mouse_right(0);
							root_sendKeyPress(上次武器按键);


							root_kmNet_mouse_left(1);
							Sleep(10);
							root_sendKeyUp(KEY_LEFTSHIFT);
							root_sendKeyUp(KEY_S);
							root_sendKeyUp(上次武器按键);
							ULONGLONG WaitTImeb2 = GetTickCount64() + 1500;
							while (WaitTImeb2 > GetTickCount64()) {
								if (LocalPlayer_Data.MyWeaponType != WeaponType::Node && LocalPlayer_Data.MyWeaponType != 0) {
									MyLog("切刀成功");
									break;
								}
								if (振刀逻辑::是否特殊状态())
								{
									root_sendKeyUpAll();
									break;
								}
								Sleep(1);
							}

						}
						root_sendKeyPress(上次武器按键);
						Sleep(10);
						root_sendKeyUp(上次武器按键);*/
					}
					else {
						if (Global::MySkillData.MySkill.F_CD <= 0) {
						}
						if (LocalPlayer_Data.MyActionDuration >= 0.6
							) {
							if (LocalPlayer_Data.MyCurEnergy >= 2000) {
								if (FatPlayerData.O_Dis <= 5) {
									MyLog("僵直闪");
									root_sendKeyUpAll();
									振刀逻辑::ShortDodgeExAll();
									Sleep(10);
									continue;
								}
							}
						}
						else {
						}
					}
					
				}
			}
			
			/*横栏处理*/
			if (横栏状态(LocalPlayer_Data.MyActionName)) {
				MyLog("横栏状态 直接捏蓝");
				root_sendKeyUpAll();
				continue;
			}
			
			/*挂墙状态处理*/
			if (挂墙状态(LocalPlayer_Data.MyActionName)) {
				MyLog("挂墙状态 壁击");
				root_sendKeyUpAll();
				root_kmNet_mouse_left(1);
				Sleep(50);
				root_kmNet_mouse_left(0);
				continue;
			}
			/*振刀处决方法*/
			if (判断振刀成功(LocalPlayer_Data.MyActionName)) {
				string 标识 = "振刀处决逻辑--";
				MyLog("振刀成功");
				int 振刀处决跳出 = 0;
				ULONGLONG WaitTIme2 = GetTickCount64() + 100;
				while (WaitTIme2 > GetTickCount64()) {
					if (LocalPlayer_Data.MyEndureLevel == 31) {
						MyLog("振刀成功 进行处决");
						MouseReleaseNoCheck();
						Sleep(400);
						if (振刀逻辑::是否特殊状态())
						{
							root_sendKeyUpAll();
							continue;
						}
						root_kmNet_mouse_left(1);
						ULONGLONG WaitTIme = GetTickCount64() + 500;
						while (WaitTIme > GetTickCount64()) {
							Sleep(1);
							//wcout << "动作名=" << LocalPlayer_Data.MyActionName << endl;
							if (LocalPlayer_Data.MyEndureLevel == 30)
							{
								MyLog(标识, "处决中");
								break;
							}
							else if (挂墙状态(LocalPlayer_Data.MyActionName) || LocalPlayer_Data.MyEndureLevel == 6 || 拼刀状态(LocalPlayer_Data.MyActionName))
							{
								振刀处决跳出 = 1;
								root_sendKeyUpAll();
								break;
							}
						}
						

						root_kmNet_mouse_left(0);

						if (LocalPlayer_Data.MyRangeReactionType == 12) {
							MyLog(标识, "状态12 准备跳A");
							if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {
								Sleep(1700);
							}
							else if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades || LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
								Sleep(800);
							}
							else if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana || LocalPlayer_Data.MyWeaponType == WeaponType::nunchucks) {
								Sleep(1000);
							}
							else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
								Sleep(700);
							}
							else if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
								Sleep(1000);
							}
							else if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
								Sleep(1100);
							}
							else {
								Sleep(1000);
							}

							MyLog(标识, "处决后双跳A");
							if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao || LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
								振刀逻辑::处决后双跳A();
							}
							else {
								振刀逻辑::处决后跳A();
							}
						}
					}
				}
				if (振刀处决跳出 != 0) {
					root_sendKeyUpAll();
					Sleep(1);
					continue;
				}
			}
			/*敌人横刀一段出蓄处理*/
			if (横刀左蓄一段状态(FatPlayerData.O_ActionName)) {
				MyLog("[博弈循环框架]敌人横刀左蓄 进行后短闪");
				if (LocalPlayer_Data.MyBlueTime < 0.5) {
					if (LocalPlayer_Data.MyCurEnergy >= 5000) {
						振刀逻辑::ShortDodgeExAll();
						root_sendKeyUpAll();
						continue;
					}
					振刀逻辑::BlueToRed();
					root_sendKeyUpAll();
					continue;
				}
				if (LocalPlayer_Data.MyBlueTime >= 0.5) {
					振刀逻辑::CollideKnife();
					continue;
				}
				if (LocalPlayer_Data.MyCurEnergy >= 5000) {
					振刀逻辑::ShortDodgeExAll();
					root_sendKeyUpAll();
					continue;
				}
				振刀逻辑::BlueToRed();
				root_sendKeyUpAll();
				continue;
			}
			if (GetTickCount64() - LocalPlayer_Data.上次振刀时间 < 500 && LocalPlayer_Data.MyEndureLevel != 6) {
				MyLog("距离上次振刀时间过短--振刀过程中...");
				Sleep(10);
				continue;
			}
			/*自己横刀一段出蓄处理*/
			if (横刀左蓄一段状态(LocalPlayer_Data.MyActionName)) {
				MyLog("自己横刀一段出蓄处理1");
				if (JudgeZhenDaoing(FatPlayerData.O_ActionName)) {
					root_sendKeyUpAll();
					root_kmNet_mouse_left(1);
					Sleep(20);
					root_kmNet_mouse_left(0);
					振刀逻辑::执行横刀平A僵直抓取();
					continue;
				}
			}
			/*纪莹莹鬼魂状态*/
			/*升龙抓高差*/
			if (false) {
				if (挂墙状态(FatPlayerData.O_ActionName) || JudgeJump(FatPlayerData.O_ActionName) || 二段跳跃状态(FatPlayerData.O_ActionName)) {
					float myY = FatPlayerData.O_Pos.y - LocalPlayer_Data.MyPos.y;
					if (JudgeZhanLi(LocalPlayer_Data.MyActionName) && FatPlayerData.O_Dis <= 7.2f && myY > 0 && myY < 6.f) {
						// 升龙抓
						bool 升龙抓成功 = false;
						root_sendKeyUp(KEY_C);
						Sleep(1);
						root_sendKeyPress(KEY_C);
						ULONGLONG WaitTIme4 = GetTickCount64() + 500;
						while (WaitTIme4 > GetTickCount64()) {
							if (JudgeSquat(LocalPlayer_Data.MyActionName)) {
								升龙抓成功 = true;
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								root_sendKeyUpAll();
								升龙抓成功 = false;
								break;;
							}
							Sleep(1);
						}
						if (!升龙抓成功) {
							continue;
						}
						振刀逻辑::左右键直接松开();
						Sleep(1);
						root_kmNet_mouse_right(1);
						root_sendKeyUp(KEY_C);
						ULONGLONG WaitTIme5 = GetTickCount64() + 500;
						while (WaitTIme5 > GetTickCount64()) {
							if (CB状态(LocalPlayer_Data.MyActionName)) {
								MyLog("进入CB状态");
								升龙抓成功 = true;
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								升龙抓成功 = false;
								root_sendKeyUpAll();
								break;
							}
							Sleep(1);
						}
						if (!升龙抓成功) {
							continue;
						}
						root_kmNet_mouse_right(0);
						ULONGLONG WaitTIme14 = GetTickCount64() + 500;
						while (WaitTIme14 > GetTickCount64()) {
							if (LocalPlayer_Data.MyRangeReactionType == 12) {
								MyLog("升龙出刀中");
								升龙抓成功 = true;
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								升龙抓成功 = false;
								root_sendKeyUpAll();
								break;
							}
							Sleep(1);
						}
						if (!升龙抓成功) {
							continue;
						}
						ULONGLONG WaitTIme6 = GetTickCount64() + 800;
						while (WaitTIme6 > GetTickCount64()) {
							if (LocalPlayer_Data.MyRangeReactionType == 10) {
								MyLog("升龙出刀结束");
								升龙抓成功 = true;
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								升龙抓成功 = false;
								root_sendKeyUpAll();
								break;
							}
							Sleep(1);
						}
						if (!升龙抓成功) {
							continue;
						}
						bool 升龙是否命中 = false;
						int temp7 = 0;
						// 检查是否击中
						ULONGLONG WaitTIme7 = GetTickCount64() + 500;
						while (WaitTIme7 > GetTickCount64()) {
							if (FatPlayerData.O_EndureLevel == 6 && !拼刀状态(FatPlayerData.O_ActionName) && !JudgePanshi(FatPlayerData.O_ActionName)) {
								// 命中并且僵直
								MyLog("升龙命中");
								升龙是否命中 = true;
								break;
							}
							if (temp7 > 50) {
								MyLog("超时 -- 升龙未命中");
								升龙是否命中 = false;;
								break;
							}
							if (振刀逻辑::是否特殊状态())
							{
								升龙是否命中 = false;;
								root_sendKeyUpAll();
								break;
							}
							temp7++;
							Sleep(1);
						}
						if (升龙是否命中) {
							// 等待自己落地 或者 直接跳A
							ULONGLONG WaitTIme8 = GetTickCount64() + 2000;
							while (WaitTIme8 > GetTickCount64()) {
								// 检查对方状态
								//if(释放受击技能(FatPlayerData.O_ActionName) || )
								if (落地状态(FatPlayerData.O_ActionName)) {
									MyLog("升龙 落地");
									break;
								}
								if (振刀逻辑::是否特殊状态())
								{
									root_sendKeyUpAll();
									break;
								}
								Sleep(1);
							}
							/*root_kmNet_mouse_right(1);
							ULONGLONG WaitTIme8 = GetTickCount64() + 1500;
							while (WaitTIme8 > GetTickCount64()) {
								if (!Judgefly(FatPlayerData.O_ActionName)) {
									break;
								}
								if (LocalPlayer_Data.MyBlueTime >= 0.51 && LocalPlayer_Data.MyEndureLevel == 20) {
									break;
								}
								if (振刀逻辑::是否特殊状态())
								{
									root_sendKeyUpAll();
									return;
								}
								Sleep(1);
							}*/
						}
					}
				}
			}
			if(LocalPlayer_Data.MyHero == HeroType::wutian){
				if (武田F状态(LocalPlayer_Data.MyActionName)) {
					MyLog("武田夺刀中");
					/*ULONGLONG WaitTIme2 = GetTickCount64() + 1000;
					while (WaitTIme2 > GetTickCount64()) {
						if (!武田F状态(LocalPlayer_Data.MyActionName)) {
							break;
						}
						Sleep(1);
					}*/
					root_sendKeyUpAll();
					continue;
				}
			}
			// 获取最近玩家
			//auto nearest_player = data_buffers.GetNearestPlayer();
			
			//countTest++;
			//MyLog("计数", countTest);
			if(LocalPlayer_Data.MyWeaponType == WeaponType::Katana){
				//MyLog("太刀博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::太刀博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
				//MyLog("双刀博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::双刀博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
				//MyLog("双戟博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::双戟博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::Dagge || LocalPlayer_Data.MyWeaponType == WeaponType::fan) {
				//MyLog("匕首博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::匕首博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {
				//MyLog("横刀博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::横刀博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
				//MyLog("长剑博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::长剑博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::nunchucks) {
				//MyLog("双截棍博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::双截棍博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::rod) {
				//MyLog("长棍博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::长棍博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::saber){
				//MyLog("斩马刀博弈"); 
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::斩马刀博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
				//MyLog("长枪博弈"); 
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::长枪博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::lj) {
				//MyLog("链剑博弈"); 
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::链剑博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
				//MyLog("拳刃博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::拳刃博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
				//MyLog("飞刀博弈");
				上次武器按键 = LocalPlayer_Data.MyWeaponIndex == 0 ? KEY_1_EXCLAMATION_MARK : KEY_2_AT;
				振刀逻辑::飞刀博弈();
			}
			else if (LocalPlayer_Data.MyWeaponType == WeaponType::Node) {
				MyLog("空手博弈"); 
				振刀逻辑::空手博弈();
			}
		}
		else
		{
			if (needPass) {
				needPass = false;
				MyLog("解键");
				root_sendKeyUpAll();
				MouseReleaseNoCheck();
				//MouseLeftorRightUp();
			}
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
//骨骼
DWORD WINAPI BonesTraverseThread(LPVOID lpParam)
{
	vector<Vec2> m_bonesData;
	Bone_D boneData;
	while (true)
	{
		if (!Global::Cache_LocalPlayer.L_ActorModel) {
			this_thread::sleep_for(chrono::milliseconds(1000)); // Sleep if local player is not valid
			continue;
		}
		if (!Function::ESP::骨骼) continue;;
		m_bonesData.clear();
		auto PlayerCount = Global::WorldPtr.Character->GetPlayerCount();
		for (int i = 0; i < PlayerCount; i++)
		{
			auto List = Global::WorldPtr.Character->GetActorModelList(i);
			if (List == Global::Cache_LocalPlayer.L_ActorModel || List == NULL) continue;
			auto PropertyData = List->GetActorModelPropertyData();
			if (LocalPlayer_Data.MyTeam == PropertyData->GetTeam()) continue;
			if (!List->IsAlive() || !List->IsVisible()) continue;
			auto ISRobot = PropertyData->GetISRobotId();
			Vector3 NeckPosition = GetBonePosition(reinterpret_cast<uintptr_t>(List), boneData.脖子);
			Vector3 WaistPosition = GetBonePosition(reinterpret_cast<uintptr_t>(List), boneData.盆骨);
			Vector3 previous, current;
			Vec2 p1, c1;
			for (auto& a : boneData.BoneList) {
				previous = Vector3{ 0.f, 0.f, 0.f };
				for (int bone : a) {
					current = bone == boneData.脖子 ? NeckPosition : (bone == boneData.盆骨 ? WaistPosition : GetBonePosition(reinterpret_cast<uintptr_t>(List), bone));
					if (previous.x == 0.f) {
						previous = current;
						continue;
					}
					WorldToScreen_AIM(MATRIX, previous, p1);
					WorldToScreen_AIM(MATRIX, current, c1);
					Gui.Line(Vec2(p1.x, p1.y), Vec2(c1.x, c1.y), (ISRobot ? Function::ESP::人机颜色 : Function::ESP::玩家颜色), 2);
					previous = current;
				}
			}	
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
std::atomic<bool> onDoing{ false };
//振刀
DWORD WINAPI ShockKnifeThread(LPVOID lpParam) {
	std::cout.setf(std::ios::fixed);
	std::cout.precision(3);
	int last_read_index = -1;  // 跟踪已处理的数据版本
	while (true) {
		if (!Function::Shock::F_ShockKnife) continue;
			if (data_buffers.WaitForData(std::chrono::milliseconds(50))) {
			const auto& readBuffer = data_buffers.GetReadBuffer();
			const int current_index = data_buffers.GetCurrentIndex();
			if (current_index != last_read_index) {
				last_read_index = current_index;
				for (auto const& list : readBuffer) {
					if (list.IsSoul) continue;
					if (list.O_Dis > 15.f /*|| !list.O_Visible*/ /*|| list.O_Direction > 125*/) continue;
					if (Function::Shock::只振视野范围内) {
						if (!list.O_Visible || list.O_Direction > 125)continue;
					}
					// male_dualhalberd_shock_01  male_dualhalberd_shock_01 male_hengdao_shock_01 
					if(JudgeZhenDao(LocalPlayer_Data.MyActionName)){
						//MyLog("振刀状态");
						if (Function::LogicKnife::auto_pickup_weapon && !onDoing) {
							onDoing = true;
							std::thread([&]() {
								const int totalDurationMs = 1000;    // 总时长 1 秒
								const int intervalMs = 500;          // 间隔 0.5 秒
								int elapsed = 0;
								while (elapsed < totalDurationMs) {
									SendKeyPressEx(KEY_E);
									Sleep(15);
									SendKeyUpEx(KEY_E);
									std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
									elapsed += intervalMs;
								}
								onDoing = false;
								}).detach(); // 分离线程
						}
						continue;
					}
					//if (/*!Tool::IsMeleeWeapon(LocalPlayer_Data.MyWeaponType) || !JudgeStatus(LocalPlayer_Data.MyEndureLevel) ||*/
					//	JudgeZhenDao(LocalPlayer_Data.MyActionName)) continue;
					if (LocalPlayer_Data.MyHero == HeroType::shenjiang || LocalPlayer_Data.MyHero == HeroType::dafo ||
						LocalPlayer_Data.MyHero == HeroType::tiewei || 英雄技能不振(LocalPlayer_Data.MyActionName) || (!振刀参数::特殊处理::胡为开大振刀 && 胡为开大状态(LocalPlayer_Data.MyActionName))) continue;

					JudgeActionName(list);
					//int endure = GetEndureLevel(list.Cache_HitSimulate);
					//if (endure == 20 || endure == 22 || list.O_WeaponID == WeaponType::Blade || list.O_WeaponID == WeaponType::saber) {
					//	JudgeActionName(list);
					//	// Shock(list);
					//}
				}
			}
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
//闪避活化
DWORD WINAPI HeroActivationThread(LPVOID lpParam)
{
	int last_read_index = -1;  // 跟踪已处理的数据版本
	while (true)
	{
		if (data_buffers.WaitForData(std::chrono::milliseconds(50))) {
			const auto& readBuffer = data_buffers.GetReadBuffer();
			const int current_index = data_buffers.GetCurrentIndex();
			// 仅处理新数据
			if (current_index != last_read_index) {
				last_read_index = current_index;
				// 遍历玩家数据
				for (const auto& list : readBuffer)
				{
					if (list.IsSoul) continue;
					if (list.O_Dis > 35.f) continue;
					if (英雄技能不振(LocalPlayer_Data.MyActionName)) continue;
					//闪避活化
					if (StrStrW(list.O_ActionName.c_str(), L"hero") && Function::HeroActivation::IsDodge)
					{
						DodgeSkill(list);
					}
					if (Function::HeroActivation::技能活化)
					{
						SkillActivate(list);
					}
				}
			}
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
std::atomic<bool> isPaused(false);  // 标识线程是否需要暂停
DWORD WINAPI LianZhaoThread(LPVOID lpParam)
{
	int last_read_index = -1;  // 跟踪已处理的数据版本
	while (true)
	{
		if (振刀逻辑::侧键是否按下()) {
			// 按下 VK_XBUTTON2 时暂停线程
			isPaused.store(true);
			while (振刀逻辑::侧键是否按下()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 等待直到按键释放
			}
			// 按键释放后恢复线程
			isPaused.store(false);
		}
		if (!isPaused.load()) {
			if (data_buffers.WaitForData(std::chrono::milliseconds(50))) {
				const auto& readBuffer = data_buffers.GetReadBuffer();
				const int current_index = data_buffers.GetCurrentIndex();
				// 仅处理新数据
				if (current_index != last_read_index) {
					last_read_index = current_index;
					// 遍历玩家数据
					for (auto const& list : readBuffer)
					{
						if (list.IsSoul) continue;
						if (LocalPlayer_Data.MyHero == HeroType::shenjiang || LocalPlayer_Data.MyHero == HeroType::dafo ||
							LocalPlayer_Data.MyHero == HeroType::tiewei) continue;
						if (!Tool::IsMeleeWeapon(LocalPlayer_Data.MyWeaponType)) continue;
						if (list.O_Dis > 8.f || !list.O_Visible || (fabs(list.O_Pos.y - LocalPlayer_Data.MyPos.y) > 4.5f &&
							fabs(list.O_Pos.y - LocalPlayer_Data.MyPos.y) < -4.5f)) continue;
						if (LocalPlayer_Data.敌人数量5M内 >= 2) continue; // 如果5M内敌人数量大于2，则只博弈3M内的敌人
						if (LocalPlayer_Data.MyEndureLevel == 11)
						{
							if (Function::LogicKnife::AutoCombo)
							{
								AutoCombo(list);
							}
						}
					}
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 控制线程的休眠频率
	}
	return 0;
}

void 半自动出蓄(PlayerData list, const std::vector<PlayerData>& enemies) {
	
	是否半自动模式 = true;

	if (Function::半自动博弈::自动断蓄 && LocalPlayer_Data.MyBlueTime > Function::半自动博弈::断续切刀时间) {
		string 武器名 = "半自动--";
		if (false) {
			// cc重置
			if (LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(武器名, "蓄力一段后CC重置蓄力");
				振刀逻辑::半自动断蓄力(); Sleep(400);
				return;
			}
		}
		else {
			// 切刀重置
			if (LocalPlayer_Data.MyEndureLevel == 20) {
				MyLog(武器名, "蓄力一段后C滚轮切刀重置蓄力");
				振刀逻辑::半自动断蓄力(); Sleep(400);
				return;
			}
		}
	}
	
	if ((Function::半自动博弈::白刀抓总开关 || LocalPlayer_Data.MyWeaponType == WeaponType::Dagge || LocalPlayer_Data.MyWeaponType == WeaponType::fan) && LocalPlayer_Data.敌人数量5M内 < 2 && list.O_Dis < 4.5f && LocalPlayer_Data.MyBlueTime < 0.49 && LocalPlayer_Data.MyEndureLevel == 20 && !AB3蓄力状态(LocalPlayer_Data.MyActionName) && LocalPlayer_Data.XuListate == 5) {

		if (LocalPlayer_Data.MyWeaponType == WeaponType::Katana) {
			string 武器名 = "半自动太刀--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行太刀平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行太刀平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行太刀平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行太刀平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::twinblades) {
			string 武器名 = "半自动双刀--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行双刀平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行双刀平A僵直抓取();
						return;
					}

				}

			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (twinbladesBlueAttackWrite(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行双刀平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行双刀平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::dualhalberd) {
			string 武器名 = "半自动双戟--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行双戟平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行双戟平A僵直抓取();
						return;
					}
				}
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (twinbladesBlueAttackWrite(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行双戟平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行双戟平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::Dagge || LocalPlayer_Data.MyWeaponType == WeaponType::fan) {
			string 武器名 = "半自动匕首扇子--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行匕首扇子平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行匕首扇子平A僵直抓取();
						return;
					}
				}
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行匕首扇子平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行匕首扇子平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::hengdao) {
			string 武器名 = "半自动横刀--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行横刀平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行横刀平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行横刀平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行横刀平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::Sw) {
			string 武器名 = "半自动长剑--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行长剑平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行长剑平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行长剑平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行长剑平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::nunchucks) {
			string 武器名 = "半自动双截棍--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行双截棍平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行双截棍平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行双截棍平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行双截棍平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::rod) {
			string 武器名 = "半自动长棍--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行长棍平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行长棍平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (KatanaBlueAttackWrite2(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行长棍平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行长棍平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::Spear) {
			string 武器名 = "半自动太刀--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行长枪平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行长枪平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (SpearBlueAttackWrite(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行长枪平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行长枪平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::lj) {
			string 武器名 = "半自动链剑--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行链剑平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行链剑平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (SwBlueAttackWrite(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行链剑平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行链剑平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
			string 武器名 = "半自动拳刃--";
			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1)) {
				if (Function::半自动博弈::白刀抓长闪 && JudgeLongFlash(list.O_ActionName)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓长闪最远距离) {
						wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 长闪");
						振刀逻辑::执行拳刃平A僵直抓取();
						return;
					}
				}
				else if (list.O_ActorWeapon != WeaponType::punch && Function::半自动博弈::白刀抓短闪 && 判断短闪哈希(list.O_NameHash)) {
					if (list.O_Dis <= Function::半自动博弈::白刀抓短闪最远距离 && 振刀逻辑::抓短闪时间判断()) {
						//wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
						MyLog(武器名, "蓄力1.0前抓 短闪--动作哈希值=", list.O_NameHash);
						振刀逻辑::执行拳刃平A僵直抓取();
						return;
					}
				}
			}

			if (Function::半自动博弈::白刀抓受击 && 被壁击状态(list.O_ActionName) && list.O_ActionDuration > 0.8) {
				MyLog(武器名, "蓄力1.0前 抓壁击");
				振刀逻辑::通用拳切百裂();
				return;
			}

			if (JudgeYuanCheng() || list.O_WeaponID == WeaponType::Node) {
				MyLog(武器名, "蓄力1.0前抓远程/空手");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}

			if ((JudgeZhanLi(list.O_ActionName) || JudgeGouSuo(list.O_ActionName)) && list.O_BlueTime < 0.1 && list.O_EndureLevel != 20 && list.O_EndureLevel != 11) {
				//	wcout << "敌人动作=" << list.O_ActionName << "-捏蓝时间=" << list.O_BlueTime << endl; // 敌人动作=male_blade_attack_charge_idle_01
				MyLog(武器名, "蓄力1.0前抓站立/跑动/走动");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓蓄力后摇 && ShakeBack(list.O_ActionName) && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前抓出蓄力后摇");
				Sleep(Function::半自动博弈::白刀抓蓄力后摇延迟);
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}

			if (JudgeJump(list.O_ActionName) || JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓出跳蹲");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}


			if (Function::半自动博弈::白刀蓝顶 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
				MyLog(武器名, "蓄力1.0前进入蓝顶判断");
				//MyLog("时间1", LocalPlayer_Data.worldPtrGlobalTime);
				if (punchBlueAttackWrite(list)) {
					MyLog(武器名, "蓄力1.0前蓝顶抓");
					if (LocalPlayer_Data.MyBlueTime >= 0.5) {
						振刀逻辑::CollideKnife();
					}
					else {
						振刀逻辑::执行拳刃平A僵直抓取();
					}
					return;
				}
			}

			if (Function::半自动博弈::白刀抓振 && JudgeZhenDaoing(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0前抓振刀");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}

			if ((list.O_EndureLevel == 5 || list.O_EndureLevel == 1) &&
				!JudgeShortFlash(list.O_ActionName)) {
				// 不是长闪或短闪状态时，只要满足O_EndureLevel条件，执行振刀逻辑
				wcout << "敌人动作=" << list.O_ActionName << "-距离=" << list.O_Dis << endl;
				MyLog(武器名, "蓄力1.0前抓 状态5/1");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}

			if (Function::半自动博弈::白刀抓受击 && 受击站立状态(list.O_ActionName)) {
				wcout << "我的动作=" << LocalPlayer_Data.MyActionName << endl;
				MyLog(武器名, "蓄力1.0前抓 受击站立状态");
				振刀逻辑::执行拳刃平A僵直抓取();
				return;
			}
		}
		else if (LocalPlayer_Data.MyWeaponType == WeaponType::punch) {
			string 武器名 = "半自动飞刀--";
				
		}

	}

	string 武器名 = "半自动--";

	if ((Function::半自动博弈::蓄力抓总开关)&& LocalPlayer_Data.MyBlueTime >= 0.5 && LocalPlayer_Data.MyEndureLevel == 20 && LocalPlayer_Data.XuListate == 5 /*&& !Function::LogicKnife::BlueAttackDuanXu_fast*/) {
		auto CanCollideKnifeForEnemy = [&](const PlayerData& enemy) -> bool {
			if (Function::半自动博弈::蓄力抓白刀 && enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20) {
				return true;
			}

			if (Function::半自动博弈::蓄力抓振 && JudgeZhenDaoing(enemy.O_ActionName) && LocalPlayer_Data.MyBlueTime < 1.15) {
				if (enemy.O_ActorWeapon == WeaponType::nunchucks) {
					return LocalPlayer_Data.MyBlueTime < 1.01;
				}
				return true;
			}

			if (Function::半自动博弈::蓄力抓倒地 && 倒地状态(enemy.O_ActionName)) {
				return true;
			}

			if (Function::半自动博弈::蓄力抓受击 && enemy.O_EndureLevel == 6) {
				return true;
			}

			if (enemy.O_RangeReactionType == 12 && enemy.O_EndureLevel != 20) {
				if (Function::半自动博弈::蓄力抓长闪 && JudgeLongFlash(enemy.O_ActionName)) {
					return true;
				}

				if (Function::半自动博弈::蓄力抓短闪 && JudgeLongFlash(enemy.O_ActionName)) {
					return true;
				}

				if (Function::半自动博弈::蓄力抓起跳 && JudgeJump(enemy.O_ActionName)) {
					return true;
				}

				if (Function::半自动博弈::蓄力抓下蹲 && JudgeCrouch(enemy.O_ActionName) || JudgeSquat(enemy.O_ActionName)) {
					return true;
				}
			}

			return false;
		};

		if (LocalPlayer_Data.敌人数量5M内 >= 2) {
			for (const auto& enemy : enemies) {
				if (enemy.IsSoul || enemy.O_Dis > 7.0f) {
					continue;
				}
				if (!CanCollideKnifeForEnemy(enemy)) {
					return;
				}
			}
		}

		if (Function::半自动博弈::蓄力抓白刀 && list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
			MyLog(武器名, "蓄力1.0后抓出刀");
			振刀逻辑::CollideKnife();
			return;
		}

		if (Function::半自动博弈::蓄力抓振 && JudgeZhenDaoing(list.O_ActionName) && LocalPlayer_Data.MyBlueTime < 1.15) {
			if (list.O_ActorWeapon == WeaponType::nunchucks) {
				if (LocalPlayer_Data.MyBlueTime < 1.01) {
					Sleep(Function::半自动博弈::蓄力抓振延迟 + 20);
					MyLog(武器名, "蓄力1.0后双截棍抓振-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
					振刀逻辑::CollideKnife();
				}

			}
			else {
				Sleep(Function::半自动博弈::蓄力抓振延迟);
				MyLog(武器名, "蓄力1.0后抓振刀-我的蓄力时间", LocalPlayer_Data.MyBlueTime);
				振刀逻辑::CollideKnife();
			}

			return;
		}

		if (Function::半自动博弈::蓄力抓倒地 && 倒地状态(list.O_ActionName)) {
			MyLog(武器名, "蓄力1.0后抓倒地");
			振刀逻辑::CollideKnife();
			return;
		}

		if (Function::半自动博弈::蓄力抓受击 && list.O_EndureLevel == 6) {
			MyLog(武器名, " 1.0后抓受击状态");
			振刀逻辑::CollideKnife();
			return;
		}

		if (list.O_RangeReactionType == 12 && list.O_EndureLevel != 20) {
			if (Function::半自动博弈::蓄力抓长闪 && JudgeLongFlash(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0后抓长闪");
				振刀逻辑::CollideKnife();
				return;
			}

			if (Function::半自动博弈::蓄力抓短闪 && JudgeLongFlash(list.O_ActionName)) {
				MyLog(武器名, "蓄力1.0后抓短闪");
				振刀逻辑::CollideKnife();
				return;
			}

			if (Function::半自动博弈::蓄力抓起跳 && JudgeJump(list.O_ActionName)) {
				MyLog("蓄力1.0后抓跳", 武器名);
				振刀逻辑::CollideKnife();
				return;
			}

			if (Function::半自动博弈::蓄力抓下蹲 && JudgeCrouch(list.O_ActionName) || JudgeSquat(list.O_ActionName)) {
				MyLog("蓄力1.0后抓蹲", 武器名);
				振刀逻辑::CollideKnife();
				return;
			}
		}
	}
}

//出蓄博弈
std::atomic<bool> isPaused2(false);  // 标识线程是否需要暂停
DWORD WINAPI LogicKnifeThread(LPVOID lpParam)
{
	string 武器名 = "自动出蓄--";
	int last_read_index = -1;  // 跟踪已处理的数据版本
	while (true)
	{
		if (振刀逻辑::侧键是否按下() || !Function::半自动博弈::总开关) {
			// 按下 VK_XBUTTON2 时暂停线程
			isPaused2.store(true);
			while (振刀逻辑::侧键是否按下() || !Function::半自动博弈::总开关) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 等待直到按键释放
			}
			// 按键释放后恢复线程
			isPaused2.store(false);
		}
		if (!isPaused2.load()) {
			if (data_buffers.WaitForData(std::chrono::milliseconds(50))) {
				const auto& readBuffer = data_buffers.GetReadBuffer();
				const int current_index = data_buffers.GetCurrentIndex();
				// 仅处理新数据
				if (current_index != last_read_index) {
					last_read_index = current_index;
					// 遍历玩家数据
					for (auto const& list : readBuffer)
					{
						if (list.IsSoul) continue;
						if (LocalPlayer_Data.MyHero == HeroType::shenjiang || LocalPlayer_Data.MyHero == HeroType::dafo ||
							LocalPlayer_Data.MyHero == HeroType::tiewei) continue;
						if (!Tool::IsMeleeWeapon(LocalPlayer_Data.MyWeaponType)) continue;
						if (list.O_Dis > 7.5f || !list.O_Visible || (fabs(list.O_Pos.y - LocalPlayer_Data.MyPos.y) > 4.5f &&
							fabs(list.O_Pos.y - LocalPlayer_Data.MyPos.y) < -4.5f)) continue;
						
						//if (LocalPlayer_Data.XuListate == 5)
						//{
						//	BlueAttackDuanXu(list);
						//	if (Function::LogicKnife::BlueAttackWrite)
						//	{
						//		BlueAttackWriteA(list);
						//	}
						//	if (Function::LogicKnife::BlueAttackShock)
						//	{
						//		float 振刀后摇 = 0.f;
						//		if (list.O_ActorWeapon == nunchucks) // 双截棍振刀判定时间比其他武器多0.1S
						//			振刀后摇 += 0.1f;
						//		if (StrStrW(list.O_ActionName.c_str(), L"emptystep_back_01"))
						//			BlueAttackRed(list, 振刀后摇 + 0.347 + (活化参数::出蓄抓振延迟 / 1000));
						//		else if (StrStrW(list.O_ActionName.c_str(), L"emptystep_back_02"))
						//			BlueAttackRed(list, 振刀后摇 + 0.477 + (活化参数::出蓄抓振延迟 / 1000));
						//		else if (StrStrW(list.O_ActionName.c_str(), L"emptystep_back_03"))
						//			BlueAttackRed(list, 振刀后摇 + 0.447 + (活化参数::出蓄抓振延迟 / 1000));
						//		else if (StrStrW(list.O_ActionName.c_str(), L"emptystep_back_04"))
						//			BlueAttackRed(list, 振刀后摇 + 0.347 + (活化参数::出蓄抓振延迟 / 1000));
						//	}
						//}
						if (LocalPlayer_Data.XuListate == 5) {
							半自动出蓄(list, readBuffer);
						}
						
					}
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 控制线程的休眠频率
	}
	return 0;
}

//自瞄
DWORD WINAPI AimbotThread(LPVOID lpParam)
{
	
	Bone_D boneData;
	int last_read_index = -1;  // 跟踪已处理的数据版本
	uint64_t Entity = NULL;
	float m_NearestDistance = 0.0f;
	while (true)
	{
		博弈键按下 = mem.GetKeyboard()->IsKeyDown(Function::侧键博弈::博弈热键);
		if (JudgeDuanFlash(FatPlayerData.O_ActionName) && !英雄技能状态(FatPlayerData.O_ActionName) && GetTickCount64() - 敌人上次闪避时间 >= 500) {
			MyLog("敌人闪避时间更新");
			wcout << "动作名=" << FatPlayerData.O_ActionName << "动作哈希=" << FatPlayerData.O_NameHash << endl;
			敌人上次闪避时间 = GetTickCount64();
		}
		if (JudgeDuanFlash(LocalPlayer_Data.MyActionName) && !英雄技能状态(FatPlayerData.O_ActionName) && GetTickCount64() - 我上次闪避时间 >= 500) {
			我上次闪避时间 = GetTickCount64();
		}

		auto ScreenCenterX = Gui.Window.Size.x * 0.5f;
		auto ScreenCenterY = Gui.Window.Size.y * 0.5f;
		if (!Function::AiMBot::AiMBotState) {
			Entity = NULL;
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		if (!Tool::IsMeleeWeaponY(LocalPlayer_Data.MyWeaponType))
		{
			Entity = NULL;
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		if (Tool::IsMeleeWeaponY(LocalPlayer_Data.MyWeaponType))
		{
			if (Entity == NULL)
			{
				if (data_buffers.WaitForData(std::chrono::milliseconds(50))) {
					const auto& readBuffer = data_buffers.GetReadBuffer();
					const int current_index = data_buffers.GetCurrentIndex();
					// 仅处理新数据
					if (current_index != last_read_index) {
						last_read_index = current_index;
						Vec2 PScreenPos{ 0,0 };
						for (auto const& CurList : readBuffer)
						{
							if (CurList.IsSoul) continue;
							if (LocalPlayer_Data.MyHero == HeroType::dafo && CurList.O_Dis < 5.f) continue;
							if (LocalPlayer_Data.MyWeaponType == 109 && CurList.O_Dis > 40.f) continue;
							if (CurList.O_Dis > 90.f || CurList.O_Dis < 3.f || !CurList.O_Visible)continue;
							if (WorldToScreen_AIM(MATRIX, CurList.O_Pos, PScreenPos))
							{
								float m_CurrentDistance = GetAimDis(PScreenPos.x, PScreenPos.y, ScreenCenterX, ScreenCenterY);
								if (m_CurrentDistance <= Function::AiMBot::AimRange)
								{
									if (Entity == NULL)
									{
										m_NearestDistance = m_CurrentDistance;
										Entity = CurList.O_ActorModel;
									}
									else if (m_CurrentDistance <= m_NearestDistance)
									{
										m_NearestDistance = m_CurrentDistance;
										Entity = CurList.O_ActorModel;
									}
								}
							}
						}
					}
				}
			}
			if (Entity != NULL)
			{
				Vector3 a_BonePos = GetBonePosition(Entity, boneData.头部);
				if (mem.GetKeyboard()->IsKeyDown(Function::AiMBot::自瞄热键))
				{
					AimAtTarget(Entity, ScreenCenterX, ScreenCenterY, a_BonePos, LocalPlayer_Data.MyPos, LocalPlayer_Data.MyWeaponType);
				}
				else
				{
					Entity = NULL;
				}
			}
		}	
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	return 0;
}
// 输出运算符重载
std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}
DWORD WINAPI ErgodicThread(LPVOID lpParam)
{
	while (true)
	{
		/*if (Function::Shock::M_ShockKnife && !是否HOOK)
		{
			Hook::KnifeStartHook();
			是否HOOK = true;
		}*/
		int last_read_index = -1;  // 跟踪已处理的数据版本
		if (Function::Flag::数据遍历)
		{
			if (Function::Flag::全局数据)
			{
				cout << "**********************全局数据*************************" << endl;
				cout << "人物指针" << Global::WorldPtr.Character << endl;
				cout << "人物行动根指针" << Global::Cache_LocalPlayer.Cache_ActorRoot_PTR << endl;
				cout << "堆堆技能指针" << Global::WorldPtr.UserData << endl;
				cout << "人物事件指针" << Global::Cache_LocalPlayer.Cache_EventTracksForLayer_PTR << endl;
				cout << "人物属性指针" << Global::Cache_LocalPlayer.Cache_PropertData_PTR << endl;
				cout << "人物动作指针" << Global::Cache_LocalPlayer.L_ActorKit << endl;
				cout << "人物对象指针" << Global::Cache_LocalPlayer.L_ActorModel << endl;
				cout << "对象消息指针" << Global::Cache_LocalPlayer.L_ObjectMessenger << endl;
				cout << "全局时间=" << Global::WorldPtr.GlobalTime << endl;
				cout << "当前时间=" << Global::WorldPtr.now << endl;
				cout << "延迟" << Global::WorldPtr.ping << endl;
			}
			this_thread::sleep_for(chrono::milliseconds(200));
			if (Function::Flag::人物数据)
			{
				//if (data_buffers.WaitForData(std::chrono::milliseconds(100)))
				{
					cout << "*****************************自己数据**********************************" << endl;
					cout << "人物ID=" << LocalPlayer_Data.MyHero << endl;
					cout << "队伍ID=" << LocalPlayer_Data.MyTeam << endl;
					cout << "武器=" << LocalPlayer_Data.MyWeaponType << endl;
					cout << "武器位=" << LocalPlayer_Data.MyWeaponIndex << endl;
					cout << "5M内敌人数量=" << LocalPlayer_Data.敌人数量5M内 << endl;
					cout << "1号武器=" << LocalPlayer_Data.Weapon_1 << endl;
					cout << "2号武器=" << LocalPlayer_Data.Weapon_2 << endl;
					wcout << "动作=" << GetActorName(LocalPlayer_Data.Cache_ActionNamePtr) << endl; 
					cout << "动作哈希=" << mem.Read<int>(LocalPlayer_Data.Cache_NameHash + 0x18) << endl;
					cout << "技能释放=" << LocalPlayer_Data.MySkillState << endl;
					cout << "F技能ID=" << Global::MySkillData.MySkill.F_SkillID << endl;
					cout << "F技能状态=" << Global::MySkillData.MySkill.F_State << endl;
					cout << "F技能CD=" << Global::MySkillData.MySkill.F_CD << endl;
					cout << "F技能持续CD=" << Global::MySkillData.MySkill.V_LeftContinueTime << endl;
					cout << "V技能ID=" << Global::MySkillData.MySkill.V_SkillID << endl;
					cout << "V技能状态=" << Global::MySkillData.MySkill.V_State << endl;
					cout << "V技能CD=" << Global::MySkillData.MySkill.V_CD << endl;
					cout << "V技能持续CD=" << Global::MySkillData.MySkill.V_LeftContinueTime << endl;
					cout << "蓄力层数=" << LocalPlayer_Data.XuListate << endl;
					cout << "是否拼刀=" << LocalPlayer_Data.MyReactionType << endl;
					cout << "是否振刀=" << LocalPlayer_Data.MyReactionExtraType << endl;
					cout << "攻击状态=" << LocalPlayer_Data.MyRangeReactionType << endl;
					cout << "僵直=" << LocalPlayer_Data.MyActionDuration << endl;
					cout << "我的当前精力=" << LocalPlayer_Data.MyCurEnergy << endl;
					cout << "我的当前护甲=" << LocalPlayer_Data.MyCurshield << endl;
					cout << "我的行动时长=" << LocalPlayer_Data.MyCurTime << endl;
					cout << "我的进蓝时间=" << LocalPlayer_Data.MyBlueTime << endl;
					cout << "当前状态=" << LocalPlayer_Data.MyEndureLevel << endl;
					cout << "坐标:" << LocalPlayer_Data.MyPos << endl;
					cout << "游戏延迟:" << LocalPlayer_Data.MyPing << endl;
					cout << "游戏帧率:" << LocalPlayer_Data.MyFPS << endl;
					cout << "扩容格子:" << LocalPlayer_Data.MyCurkuorong << endl;
					cout << "1号武器id:" << LocalPlayer_Data.My_1weapon << endl;
					cout << "2号武器id:" << LocalPlayer_Data.My_2weapon << endl;
					cout << "1号武器品质:" << LocalPlayer_Data.Weapon_1_quality << endl;
					cout << "2号武器品质:" << LocalPlayer_Data.Weapon_2_quality << endl;
				}
			}
			this_thread::sleep_for(chrono::milliseconds(1));
			if (Function::Flag::对象数据)
			{
				cout << "**********************对象数据*************************" << endl;
				cout << "英雄ID=" << FatPlayerData.HeroId << endl;
				cout << "距离=" << FatPlayerData.O_Dis << endl;
				cout << "名称=" << FatPlayerData.PlayerName << endl;
				cout << "可见=" << FatPlayerData.O_Visible << endl;
				cout << "武器ID=" << FatPlayerData.O_WeaponID << endl;
				cout << "武器=" << FatPlayerData.O_ActorWeapon << endl;

				cout << "F技能ID=" << FatPlayerData.O_Skill.F_SkillID << endl;
				cout << "F技能状态=" << FatPlayerData.O_Skill.F_State << endl;
				cout << "F技能CD=" << FatPlayerData.O_Skill.F_CD << endl;
				cout << "F技能持续CD=" << FatPlayerData.O_Skill.V_LeftContinueTime << endl;

				cout << "V技能ID=" << FatPlayerData.O_Skill.V_SkillID << endl;
				cout << "V技能状态=" << FatPlayerData.O_Skill.V_State << endl;
				cout << "V技能CD=" << FatPlayerData.O_Skill.V_CD << endl;
				cout << "V技能持续CD=" << FatPlayerData.O_Skill.V_LeftContinueTime << endl;

				cout << "队伍ID=" << FatPlayerData.Team << endl;
				wcout << "动作名称=" << GetActorName(FatPlayerData.Cache_ActionNamePtr) << endl;
				cout << "动作哈希=" << mem.Read<int>(FatPlayerData.Cache_NameHash + 0x18) << endl;
				cout << "当前状态=" << FatPlayerData.O_EndureLevel << endl;
				//cout << "当前缓存状态=" << GetEndureLevel(FatPlayerData.Cache_HitSimulate) << endl;
				cout << "蓄力层数=" << FatPlayerData.O_XuListate << endl;
				cout << "对象蓄力时间=" << FatPlayerData.O_BlueTime << endl;
				cout << "攻击状态=" << FatPlayerData.O_RangeReactionType << endl;
				cout << "技能释放=" << FatPlayerData.O_SkillStatus << endl;
				cout << "振刀状态=" << FatPlayerData.O_ReactionExtraType << endl;
				cout << "是否拼刀=" << FatPlayerData.O_ReactionType << endl;
				cout << "人物朝向=" << FatPlayerData.O_Direction << endl;
				cout << "对象行动时长=" << FatPlayerData.ObjCurTime << endl;
				cout << "HP=" << FatPlayerData.O_MaxHp << endl;
				cout << "护甲=" << FatPlayerData.O_Maxshield << endl;
				cout << "僵直=" << FatPlayerData.O_ActionDuration << endl;
				//cout << "朝向:" << FatPlayerData.O_Quat << endl;
				//cout << "速度:" << FatPlayerData.O_Velocity << endl;
				cout << "坐标:" << FatPlayerData.O_Pos << endl;
				/*if (data_buffers.WaitForData(std::chrono::milliseconds(100))) {
					const auto& readBuffer = data_buffers.GetReadBuffer();
					const int current_index = data_buffers.GetCurrentIndex();
					// 仅处理新数据
					if (current_index != last_read_index) {
						last_read_index = current_index;
						for (auto const& List : readBuffer)
						{
							if (List.O_Dis > 25.f) continue;
							cout << "**********************对象数据*************************" << endl;
							cout << "英雄ID=" << List.HeroId << endl;
							cout << "距离=" << List.O_Dis << endl;
							cout << "名称=" << List.PlayerName << endl;
							cout << "可见=" << List.O_Visible << endl;
							cout << "武器ID=" << List.O_WeaponID << endl;
							cout << "武器=" << List.O_ActorWeapon << endl;
							cout << "队伍ID=" << List.Team << endl;
							wcout << "动作名称=" << GetActorName(List.Cache_ActionNamePtr) << endl;
							cout << "动作哈希=" << mem.Read<int>(List.Cache_NameHash + 0x18) << endl;
							cout << "当前状态=" << List.O_EndureLevel << endl;
							//cout << "当前缓存状态=" << GetEndureLevel(List.Cache_HitSimulate) << endl;
							cout << "蓄力层数=" << List.O_XuListate << endl;
							cout << "对象蓄力时间=" << List.O_BlueTime << endl;
							cout << "攻击状态=" << List.O_RangeReactionType << endl;
							cout << "技能释放=" << List.O_SkillStatus << endl;
							cout << "振刀状态=" << List.O_ReactionExtraType << endl;
							cout << "是否拼刀=" << List.O_ReactionType << endl;
							cout << "人物朝向=" << List.O_Direction << endl;
							cout << "对象行动时长=" << List.ObjCurTime << endl;
							cout << "HP=" << List.O_MaxHp << endl;
							cout << "护甲=" << List.O_Maxshield << endl;
							cout << "僵直=" << List.O_ActionDuration << endl;
							//cout << "朝向:" << List.O_Quat << endl;
							//cout << "速度:" << List.O_Velocity << endl;
							cout << "坐标:" << List.O_Pos << endl;
						}
					}
				}*/
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	return 0;
}
DWORD WINAPI RenderThread(LPVOID lpParam)
{
	Gui.MyWindow(DrawRun);
	return 0;
}


struct ThreadConfig {
	LPTHREAD_START_ROUTINE function;
	int priority;
	DWORD_PTR coreMask;  // 核心偏好位（按位序映射到高性能核心列表）
	const char* name;
};

 bool kEnableDynamicAffinity = false; // 开关：true=动态高性能核映射；false=固定4核方案

static std::vector<DWORD_PTR> GetHighPerformanceCoreMasks()
{
	DWORD length = 0;
	GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
	std::vector<BYTE> buffer(length);
	std::vector<DWORD_PTR> coreMasks;

	if (length != 0 && GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &length)) {
		struct CoreEntry {
			BYTE efficiency;
			DWORD_PTR mask;
		};
		std::vector<CoreEntry> entries;

		BYTE* walker = buffer.data();
		BYTE* end = buffer.data() + length;
		while (walker < end) {
			auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(walker);
			if (info->Relationship == RelationProcessorCore) {
				const auto& processor = info->Processor;
				for (WORD groupIndex = 0; groupIndex < processor.GroupCount; ++groupIndex) {
					const GROUP_AFFINITY& affinity = processor.GroupMask[groupIndex];
					if (affinity.Group != 0 || affinity.Mask == 0) continue;

					DWORD_PTR mask = affinity.Mask;
					while (mask) {
						DWORD_PTR lowestBit = mask & (~mask + 1);
						entries.push_back({ processor.EfficiencyClass, lowestBit });
						mask &= (mask - 1);
					}
				}
			}
			walker += info->Size;
		}

		if (!entries.empty()) {
			const BYTE bestClass = std::min_element(
				entries.begin(), entries.end(),
				[](const CoreEntry& a, const CoreEntry& b) { return a.efficiency < b.efficiency; })->efficiency;

			entries.erase(
				std::remove_if(entries.begin(), entries.end(),
					[bestClass](const CoreEntry& entry) { return entry.efficiency != bestClass; }),
				entries.end());

			std::sort(entries.begin(), entries.end(),
				[](const CoreEntry& a, const CoreEntry& b) {
					if (a.efficiency != b.efficiency) return a.efficiency < b.efficiency;
					return a.mask < b.mask;
				});

			for (const auto& entry : entries) {
				coreMasks.push_back(entry.mask);
			}
		}
	}

	if (coreMasks.empty()) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		const auto maxBits = static_cast<DWORD>(sizeof(DWORD_PTR) * 8);
		const DWORD count = std::min<DWORD>(sysInfo.dwNumberOfProcessors, maxBits);
		for (DWORD i = 0; i < count; ++i) {
			coreMasks.push_back(static_cast<DWORD_PTR>(1) << i);
		}
	}

	if (coreMasks.empty()) {
		coreMasks.push_back(1);
	}

	return coreMasks;
}

static DWORD_PTR MapToPreferredCoreMask(DWORD_PTR requestedMask, const std::vector<DWORD_PTR>& preferredCores)
{
	if (preferredCores.empty()) {
		return 1;
	}

	std::vector<size_t> requestedIndices;
	for (size_t i = 0; i < sizeof(DWORD_PTR) * 8; ++i) {
		if (requestedMask & (static_cast<DWORD_PTR>(1) << i)) {
			requestedIndices.push_back(i);
		}
	}

	if (requestedIndices.empty()) {
		return preferredCores.front();
	}

	DWORD_PTR finalMask = 0;
	for (size_t idx : requestedIndices) {
		finalMask |= preferredCores[idx % preferredCores.size()];
	}

	return finalMask ? finalMask : preferredCores.front();
}

static DWORD GetPhysicalCoreCount()
{
	DWORD length = 0;
	if (!GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || length == 0) {
			SYSTEM_INFO sysInfo{};
			GetSystemInfo(&sysInfo);
			return sysInfo.dwNumberOfProcessors ? sysInfo.dwNumberOfProcessors : 1;
		}
	}

	std::vector<BYTE> buffer(length);
	if (!GetLogicalProcessorInformationEx(RelationProcessorCore,
		reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
		&length)) {
		SYSTEM_INFO sysInfo{};
		GetSystemInfo(&sysInfo);
		return sysInfo.dwNumberOfProcessors ? sysInfo.dwNumberOfProcessors : 1;
	}

	DWORD coreCount = 0;
	BYTE* walker = buffer.data();
	BYTE* end = buffer.data() + length;
	while (walker < end) {
		auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(walker);
		if (info->Relationship == RelationProcessorCore) {
			++coreCount;
		}
		if (info->Size == 0) {
			break;
		}
		walker += info->Size;
	}

	if (coreCount == 0) {
		SYSTEM_INFO sysInfo{};
		GetSystemInfo(&sysInfo);
		return sysInfo.dwNumberOfProcessors ? sysInfo.dwNumberOfProcessors : 1;
	}

	return coreCount;
}

static DWORD GetLogicalProcessorCountAll()
{
	DWORD length = 0;
	if (!GetLogicalProcessorInformationEx(RelationGroup, nullptr, &length)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || length == 0) {
			SYSTEM_INFO si{}; GetSystemInfo(&si);
			return si.dwNumberOfProcessors ? si.dwNumberOfProcessors : 1;
		}
	}

	std::vector<BYTE> buffer(length);
	if (!GetLogicalProcessorInformationEx(
		RelationGroup,
		reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
		&length)) {
		SYSTEM_INFO si{}; GetSystemInfo(&si);
		return si.dwNumberOfProcessors ? si.dwNumberOfProcessors : 1;
	}

	DWORD total = 0;
	BYTE* p = buffer.data();
	BYTE* end = buffer.data() + length;
	while (p < end) {
		auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(p);
		if (info->Relationship == RelationGroup) {
			const GROUP_RELATIONSHIP& g = info->Group;
			for (WORD i = 0; i < g.ActiveGroupCount; ++i)
				total += g.GroupInfo[i].ActiveProcessorCount;
		}
		if (info->Size == 0) break;
		p += info->Size;
	}

	if (total == 0) {
		SYSTEM_INFO si{}; GetSystemInfo(&si);
		total = si.dwNumberOfProcessors;
	}
	return total ? total : 1;
}


// 固定4核绑定方案（0x1-0x8对应Core0-Core3），可通过开关切换动态映射
constexpr ThreadConfig THREAD_CONFIGS[] = {
	// 函数指针              优先级                     核心掩码
	{Cache_Player_World_Data2,  THREAD_PRIORITY_NORMAL,          0x01, "Cache_Player_World_Data2"}, // Core0 缓存数据
	{LocalPlayer_Object_Data2,  THREAD_PRIORITY_TIME_CRITICAL,   0x08, "LocalPlayer_Object_Data2"},// Core3 对象各种数据独占（最高优先级）
	{GetItemData,               THREAD_PRIORITY_BELOW_NORMAL,    0x01, "GetItemData"}, // Core0 物资遍历
	{GetUserBattleData,         THREAD_PRIORITY_BELOW_NORMAL,    0x01, "GetUserBattleData"}, // Core0 物堆遍历
	{ShockKnifeThread,          THREAD_PRIORITY_TIME_CRITICAL,   0x08, "ShockKnifeThread"}, // Core2 振刀
	{LogicKnifeThread,          THREAD_PRIORITY_NORMAL,          0x04, "LogicKnifeThread"}, // Core1 智能放蓄
	//{LianZhaoThread,            THREAD_PRIORITY_NORMAL,          0x04, "LianZhaoThread"}, // Core1 连招
	{HeroActivationThread,      THREAD_PRIORITY_ABOVE_NORMAL,    0x02, "HeroActivationThread"},// Core2 技能活化
	{AimbotThread,              THREAD_PRIORITY_NORMAL,          0x02, "AimbotThread"},// Core1 自瞄 
	//{BonesTraverseThread,       THREAD_PRIORITY_BELOW_NORMAL,    0x02},// Core1 骨骼遍历
	{SmarThread,                THREAD_PRIORITY_TIME_CRITICAL,   0x04, "SmarThread"},// Core1 技能 自动博弈
	{RenderThread,              THREAD_PRIORITY_NORMAL,          0x02, "RenderThread"},// Core1 透视 菜单
	{ErgodicThread,             THREAD_PRIORITY_LOWEST,          0x01, "ErgodicThread"} // Core0 后台遍历
};


// 固定12核绑定方案
constexpr ThreadConfig THREAD_CONFIGS12[] = {
	// 函数指针              优先级                     核心掩码
	
	{LocalPlayer_Object_Data2,  THREAD_PRIORITY_TIME_CRITICAL,   0x02, "LocalPlayer_Object_Data2"},//  对象各种数据独占（最高优先级）
	{ShockKnifeThread,          THREAD_PRIORITY_TIME_CRITICAL,   0x04, "ShockKnifeThread"}, //  振刀
	{SmarThread,                THREAD_PRIORITY_TIME_CRITICAL,   0x08, "SmarThread"},//  技能 自动博弈
	{LogicKnifeThread,          THREAD_PRIORITY_NORMAL,          0x20, "LogicKnifeThread"}, //  智能放蓄

	{GetItemData,               THREAD_PRIORITY_BELOW_NORMAL,    0x40, "GetItemData"}, //  物资遍历
	{GetUserBattleData,         THREAD_PRIORITY_BELOW_NORMAL,    0x100, "GetUserBattleData"}, //  物堆遍历
	
	{Cache_Player_World_Data2,  THREAD_PRIORITY_NORMAL,          0x10, "Cache_Player_World_Data2"}, //  缓存数据
	//{LianZhaoThread,            THREAD_PRIORITY_NORMAL,          0x04, "LianZhaoThread"}, //  连招
	{HeroActivationThread,      THREAD_PRIORITY_ABOVE_NORMAL,    0x80, "HeroActivationThread"},//  技能活化
	{AimbotThread,              THREAD_PRIORITY_NORMAL,          0x200, "AimbotThread"},//  自瞄 
	//{BonesTraverseThread,       THREAD_PRIORITY_BELOW_NORMAL,    0x02},//  骨骼遍历
	{RenderThread,              THREAD_PRIORITY_NORMAL,          0x100, "RenderThread"},//  透视 菜单
	{ErgodicThread,             THREAD_PRIORITY_LOWEST,          0x200, "ErgodicThread"} //  后台遍历
};

// 固定10核绑定方案
constexpr ThreadConfig THREAD_CONFIGS10[] = {
	// 函数指针              优先级                     核心掩码
	{Cache_Player_World_Data2,  THREAD_PRIORITY_NORMAL,          0x01, "Cache_Player_World_Data2"}, //  缓存数据
	{LocalPlayer_Object_Data2,  THREAD_PRIORITY_TIME_CRITICAL,   0x08, "LocalPlayer_Object_Data2"},//  对象各种数据独占（最高优先级）
	{ShockKnifeThread,          THREAD_PRIORITY_TIME_CRITICAL,   0x04, "ShockKnifeThread"}, //  振刀
	{SmarThread,                THREAD_PRIORITY_TIME_CRITICAL,   0x10, "SmarThread"},	//  技能 自动博弈
	{LogicKnifeThread,          THREAD_PRIORITY_NORMAL,          0x20, "LogicKnifeThread"}, //  智能放蓄
	{AimbotThread,              THREAD_PRIORITY_NORMAL,          0x40, "AimbotThread"},//  自瞄 
	{HeroActivationThread,      THREAD_PRIORITY_ABOVE_NORMAL,    0x80, "HeroActivationThread"},//  技能活化

	{GetItemData,               THREAD_PRIORITY_BELOW_NORMAL,    0x100, "GetItemData"}, //  物资遍历
	{GetUserBattleData,         THREAD_PRIORITY_BELOW_NORMAL,    0x200, "GetUserBattleData"}, //  物堆遍历

	//{LianZhaoThread,            THREAD_PRIORITY_NORMAL,          0x04, "LianZhaoThread"}, //  连招
	//{BonesTraverseThread,       THREAD_PRIORITY_BELOW_NORMAL,    0x02},//  骨骼遍历
	{RenderThread,              THREAD_PRIORITY_NORMAL,          0x02, "RenderThread"},//  透视 菜单
	{ErgodicThread,             THREAD_PRIORITY_LOWEST,          0x01, "ErgodicThread"} //  后台遍历
};

// 固定8核绑定方案
constexpr ThreadConfig THREAD_CONFIGS8[] = {
	// 函数指针              优先级                     核心掩码
	{Cache_Player_World_Data2,  THREAD_PRIORITY_TIME_CRITICAL,          0x02, "Cache_Player_World_Data2"}, //  缓存数据
	{LocalPlayer_Object_Data2,  THREAD_PRIORITY_TIME_CRITICAL,   0x100, "LocalPlayer_Object_Data2"},//  对象各种数据独占（最高优先级）
	{GetItemData,               THREAD_PRIORITY_BELOW_NORMAL,    0x08, "GetItemData"}, //  物资遍历
	{GetUserBattleData,         THREAD_PRIORITY_BELOW_NORMAL,    0x04, "GetUserBattleData"}, //  物堆遍历
	{ShockKnifeThread,          THREAD_PRIORITY_TIME_CRITICAL,   0x80, "ShockKnifeThread"}, //  振刀
	{LogicKnifeThread,          THREAD_PRIORITY_NORMAL,          0x20, "LogicKnifeThread"}, //  智能放蓄
	//{LianZhaoThread,            THREAD_PRIORITY_NORMAL,          0x04, "LianZhaoThread"}, //  连招
	{HeroActivationThread,      THREAD_PRIORITY_ABOVE_NORMAL,    0x08, "HeroActivationThread"},//  技能活化
	{AimbotThread,              THREAD_PRIORITY_NORMAL,          0x10, "AimbotThread"},//  自瞄 
	//{BonesTraverseThread,       THREAD_PRIORITY_BELOW_NORMAL,    0x02},//  骨骼遍历
	{SmarThread,                THREAD_PRIORITY_TIME_CRITICAL,   0x40, "SmarThread"},//  技能 自动博弈
	{RenderThread,              THREAD_PRIORITY_NORMAL,          0x01, "RenderThread"},//  透视 菜单
	{ErgodicThread,             THREAD_PRIORITY_LOWEST,          0x01, "ErgodicThread"} //  后台遍历
};

void GetCpuCoreInfo() {
	// 获取系统处理器信息
	DWORD len = 0;
	GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);

	// 分配内存以存储处理器信息
	std::vector<BYTE> buffer(len);
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data());

	if (GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len)) {
		std::vector<int> performanceCores;
		std::vector<int> efficiencyCores;

		// EfficiencyClass 越小代表越偏向高性能（P 核）
		BYTE bestClass = 0xFF;
		bool hasClass = false;
		BYTE* walker = buffer.data();
		BYTE* end = buffer.data() + len;
		while (walker < end) {
			auto* entry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(walker);
			if (entry->Relationship == RelationProcessorCore) {
				hasClass = true;
				if (entry->Processor.EfficiencyClass < bestClass) {
					bestClass = entry->Processor.EfficiencyClass;
				}
			}
			if (entry->Size == 0) {
				break;
			}
			walker += entry->Size;
		}

		if (!hasClass) {
			std::cout << "获取处理器信息失败。" << std::endl;
			return;
		}

		// 遍历处理器信息，识别效能核心和效率核心
		walker = buffer.data();
		end = buffer.data() + len;
		const DWORD maxBits = static_cast<DWORD>(sizeof(DWORD_PTR) * 8);
		while (walker < end) {
			auto* entry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(walker);
			if (entry->Relationship == RelationProcessorCore) {
				const auto& processor = entry->Processor;
				const bool isPerformance = (processor.EfficiencyClass == bestClass);
				for (WORD groupIndex = 0; groupIndex < processor.GroupCount; ++groupIndex) {
					const GROUP_AFFINITY& affinity = processor.GroupMask[groupIndex];
					if (affinity.Mask == 0 || affinity.Group != 0) {
						continue;
					}
					for (DWORD bit = 0; bit < maxBits; ++bit) {
						const DWORD_PTR bitMask = (static_cast<DWORD_PTR>(1) << bit);
						if ((affinity.Mask & bitMask) == 0) {
							continue;
						}
						const int logicalIndex = static_cast<int>(bit);
						if (isPerformance) {
							std::cout << "核心 " << logicalIndex << " 是效能核心P" << std::endl;
							performanceCores.push_back(logicalIndex);
						} else {
							std::cout << "核心 " << logicalIndex << " 是效率核心E" << std::endl;
							efficiencyCores.push_back(logicalIndex);
						}
					}
				}
			}
			if (entry->Size == 0) {
				break;
			}
			walker += entry->Size;
		}

		std::cout << "效能核心数量: " << performanceCores.size() << std::endl;
		std::cout << "效率核心数量: " << efficiencyCores.size() << std::endl;

		// 设置所有核心（P+E）的亲和性，不使用CPU 0
		DWORD_PTR dwAffinityMask = 0;
		for (int core : performanceCores) {
			if (core != 0 || !Function::侧键博弈::禁用CPU0) {
				dwAffinityMask |= (static_cast<DWORD_PTR>(1) << core);
			}
		}
		for (int core : efficiencyCores) {
			if (core != 0 || !Function::侧键博弈::禁用CPU0) {
				dwAffinityMask |= (static_cast<DWORD_PTR>(1) << core);
			}
		}

		// 获取当前进程的句柄
		HANDLE hProcess = GetCurrentProcess();
		//std::cout << "test: " << dwAffinityMask << std::endl;
		// 设置处理器相关性
		if (SetProcessAffinityMask(hProcess, dwAffinityMask)) {
			std::cout << "处理器相关性设置成功！" << std::endl;
		}
		else {
			std::cout << "设置处理器相关性失败。" << std::endl;
		}
	}
	else {
		std::cout << "获取处理器信息失败。" << std::endl;
	}
}

// 线程启动
void Start4CoreThreads()
{
	const DWORD cpuCoreCount = GetLogicalProcessorCountAll();
	MyLogTrue("CPU核心数量:", cpuCoreCount);
	const ThreadConfig* configs = THREAD_CONFIGS;
	size_t configCount = sizeof(THREAD_CONFIGS) / sizeof(THREAD_CONFIGS[0]);

	if(Function::侧键博弈::动态线程 == 1){
		MyLogTrue("线程模式2");
		configs = THREAD_CONFIGS;
		configCount = sizeof(THREAD_CONFIGS) / sizeof(THREAD_CONFIGS[0]);
	}
	else if(Function::侧键博弈::动态线程 == 0){
		MyLogTrue("线程模式1 开始自动判断线程方案");
		if (cpuCoreCount >= 12) {
			MyLogTrue("绑定线程方案1");
			configs = THREAD_CONFIGS12;
			configCount = sizeof(THREAD_CONFIGS12) / sizeof(THREAD_CONFIGS12[0]);
		}
		else if (cpuCoreCount >= 10) {
			MyLogTrue("绑定线程方案2");
			configs = THREAD_CONFIGS10;
			configCount = sizeof(THREAD_CONFIGS10) / sizeof(THREAD_CONFIGS10[0]);
		}
		else if (cpuCoreCount >= 8) {
			MyLogTrue("绑定线程方案3");
			configs = THREAD_CONFIGS8;
			configCount = sizeof(THREAD_CONFIGS8) / sizeof(THREAD_CONFIGS8[0]);
		}
		else if (cpuCoreCount >= 4) {
			MyLogTrue("绑定线程方案4");
			configs = THREAD_CONFIGS;
			configCount = sizeof(THREAD_CONFIGS) / sizeof(THREAD_CONFIGS[0]);
		}
		else {
			MyLogTrue("绑定线程方案5");
			configs = THREAD_CONFIGS;
			configCount = sizeof(THREAD_CONFIGS) / sizeof(THREAD_CONFIGS[0]);
		}
	}
	else if (Function::侧键博弈::动态线程 == 2) {
		MyLogTrue("线程模式3 自行测试是否合适自己");
		kEnableDynamicAffinity = true;
	}

	

	const auto preferredCores = kEnableDynamicAffinity ? GetHighPerformanceCoreMasks() : std::vector<DWORD_PTR>{};

	for (size_t i = 0; i < configCount; ++i) {
		const auto& config = configs[i];
		HANDLE hThread = CreateThread(nullptr,
			0,
			config.function,
			nullptr,
			CREATE_SUSPENDED,
			nullptr);

		if (!hThread) {
			continue;
		}

		DisableThreadPowerThrottling(hThread);

		const DWORD_PTR baseMask = config.coreMask ? config.coreMask : 1;
		const DWORD_PTR finalMask = kEnableDynamicAffinity
			? MapToPreferredCoreMask(baseMask, preferredCores)
			: baseMask;

		if (finalMask) {
			SetThreadAffinityMask(hThread, finalMask);
		}

		SetThreadPriority(hThread, config.priority);

		/*std::cout << "[ThreadBind] " << config.name << " -> mask 0x"
			<< std::hex << finalMask << std::dec
			<< " priority " << config.priority
			<< (kEnableDynamicAffinity ? " (dynamic)" : " (fixed)") << std::endl;*/

		g_hThreads.push_back(hThread);
		ResumeThread(hThread);
	}

	GetCpuCoreInfo();
}

void 读取博弈参数() {
	std::ifstream file("博弈配置.ini");
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string token;
			std::string paramName;
			//float paramValue;
			if (std::getline(iss, token, '='))
			{
				paramName = token;
				if (std::getline(iss, token))
				{
					if (strstr(paramName.c_str(), "博弈切刀抓振刀") != NULL) { Function::侧键博弈::博弈切刀抓振刀 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓长闪") != NULL) { Function::侧键博弈::蓄力抓长闪 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓短闪") != NULL) { Function::侧键博弈::蓄力抓短闪 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓白刀") != NULL) { Function::侧键博弈::蓄力抓白刀 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓倒地") != NULL) { Function::侧键博弈::蓄力抓倒地 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓受击") != NULL) { Function::侧键博弈::蓄力抓受击 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓起跳") != NULL) { Function::侧键博弈::蓄力抓起跳 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓下蹲") != NULL) { Function::侧键博弈::蓄力抓下蹲 = stoi(token); }
					if (strstr(paramName.c_str(), "白刀抓长闪") != NULL) { Function::侧键博弈::白刀抓长闪 = stoi(token); }
					if (strstr(paramName.c_str(), "白刀抓短闪") != NULL) { Function::侧键博弈::白刀抓短闪 = stoi(token); }
					if (strstr(paramName.c_str(), "白刀抓蓄力后摇") != NULL) { Function::侧键博弈::白刀抓蓄力后摇 = stoi(token); }
					if (strstr(paramName.c_str(), "白刀抓振") != NULL) { Function::侧键博弈::白刀抓振 = stoi(token); }
					if (strstr(paramName.c_str(), "蓄力抓振延迟") != NULL) { Function::侧键博弈::蓄力抓振延迟 = stoi(token); }
					if (strstr(paramName.c_str(), "白刀蓝顶延迟") != NULL) { Function::侧键博弈::白刀蓝顶延迟 = std::stof(token);}
					if (strstr(paramName.c_str(), "白刀抓短闪延迟") != NULL) { Function::侧键博弈::白刀抓短闪延迟 = std::stoi(token); }

					if (strstr(paramName.c_str(), "白刀抓短闪最远距离") != NULL) { Function::侧键博弈::白刀抓短闪最远距离 = std::stof(token); }
					if (strstr(paramName.c_str(), "白刀抓长闪最远距离") != NULL) { Function::侧键博弈::白刀抓长闪最远距离 = std::stof(token); }

					if (strstr(paramName.c_str(), "动态线程") != NULL) { Function::侧键博弈::动态线程 = stoi(token); }
					if (strstr(paramName.c_str(), "断续切刀时间") != NULL) { Function::侧键博弈::断续切刀时间 = stof(token); }
					if (strstr(paramName.c_str(), "断续切刀最小时间") != NULL) { Function::侧键博弈::断续切刀最小时间 = stof(token); }
					if (strstr(paramName.c_str(), "切拳起身") != NULL) { Function::侧键博弈::切拳起身 = stof(token); }
					if (strstr(paramName.c_str(), "博弈热键") != NULL) { Function::侧键博弈::博弈热键 = stoi(token); }
					if (strstr(paramName.c_str(), "禁用CPU0") != NULL) { Function::侧键博弈::禁用CPU0 = stoi(token); }
					//if (strstr(paramName.c_str(), "钩锁模式") != NULL) { Function::侧键博弈::钩锁模式 = stoi(token); }

					if (strstr(paramName.c_str(), "半自动蓄抓长闪") != NULL) { Function::半自动博弈::蓄力抓长闪 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓短闪") != NULL) { Function::半自动博弈::蓄力抓短闪 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓白刀") != NULL) { Function::半自动博弈::蓄力抓白刀 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓倒地") != NULL) { Function::半自动博弈::蓄力抓倒地 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓受击") != NULL) { Function::半自动博弈::蓄力抓受击 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓起跳") != NULL) { Function::半自动博弈::蓄力抓起跳 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓下蹲") != NULL) { Function::半自动博弈::蓄力抓下蹲 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白抓长闪") != NULL) { Function::半自动博弈::白刀抓长闪 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白抓短闪") != NULL) { Function::半自动博弈::白刀抓短闪 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白抓蓄力后摇") != NULL) { Function::半自动博弈::白刀抓蓄力后摇 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白抓振") != NULL) { Function::半自动博弈::白刀抓振 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄抓振延迟") != NULL) { Function::半自动博弈::蓄力抓振延迟 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白蓝顶延迟") != NULL) {

						Function::半自动博弈::白刀蓝顶延迟 = std::stof(token);
					}
					if (strstr(paramName.c_str(), "半自动白抓短闪最远距离") != NULL) { Function::半自动博弈::白刀抓短闪最远距离 = std::stof(token); }
					if (strstr(paramName.c_str(), "半自动白抓长闪最远距离") != NULL) { Function::半自动博弈::白刀抓长闪最远距离 = std::stof(token); }
					if (strstr(paramName.c_str(), "半自动切刀时间") != NULL) { Function::半自动博弈::断续切刀时间 = std::stof(token); }
					if (strstr(paramName.c_str(), "半自动总开关") != NULL) { Function::半自动博弈::总开关 = std::stoi(token); }
					if (strstr(paramName.c_str(), "半自动自动连招") != NULL) { Function::半自动博弈::自动连招 = std::stoi(token); }
					if (strstr(paramName.c_str(), "半自动自动断蓄") != NULL) { Function::半自动博弈::自动断蓄 = std::stoi(token); }

					if (strstr(paramName.c_str(), "半自动白抓受击") != NULL) { Function::半自动博弈::白刀抓受击 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动白刀抓总开关") != NULL) { Function::半自动博弈::白刀抓总开关 = stoi(token); }
					if (strstr(paramName.c_str(), "半自动蓄力抓总开关") != NULL) { Function::半自动博弈::蓄力抓总开关 = stoi(token); }
				}
			}
		}

		file.close();
		//cout << "活化配置读取完毕" << endl;
	}
}

void 振刀配置初始化() {
	std::ifstream file("模拟振配置.ini");
	if (!file.is_open()) {
		初始化振刀距离参数数据();
		初始化振刀角度参数数据();
		初始化振刀延迟参数数据();
		初始化振刀方式参数();
		初始化间隔配置();
	}
	else {
		// 检查文件中是否包含"链剑"
		std::string content((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
		file.close();
		if (content.find("链剑") == std::string::npos) {
			// 链剑振刀初始化
			链剑参数初始化();
		}
		MyLog("初始化振刀方式参数");
		初始化振刀方式参数();
		/*
		if (content.find("振刀方式") == std::string::npos) {
			LOG("振刀方式初始化");
			初始化振刀方式参数();
		}
		*/
	}
}
void Thread()
{
	读取博弈参数();
	ApplyHighPriorityAndDisableThrottling();
	ApplyHighPerformancePowerPlanAndDisableLock();
	DisableThreadPowerThrottling(GetCurrentThread());
	/*kEnableDynamicAffinity = Function::侧键博弈::动态线程;
	if (kEnableDynamicAffinity) {
		MyLogTrue("动态线程模式");
	}else{
		MyLogTrue("稳定线程模式");
	}*/
	
	std::call_once(g_timePeriodInitFlag, InitTimePeriodOnce);
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	振刀配置初始化();
	//启动线程
	Start4CoreThreads();
	// 主循环等待退出信号
	while (!g_bExit.load(std::memory_order_acquire)) {
		// 使用更高效的等待方式
		if (WaitForSingleObject(g_hExitEvent, 100) == WAIT_OBJECT_0) {
			break;
		}
	}
	// 等待线程退出（最多5秒）
	if (!g_hThreads.empty()) {
		DWORD waitResult = WaitForMultipleObjects(
			static_cast<DWORD>(g_hThreads.size()),
			g_hThreads.data(),
			TRUE,
			5000);
		if (waitResult == WAIT_TIMEOUT) {
			OutputDebugString("警告：部分线程未正常退出");
		}
	}
	// 安全关闭所有句柄
	for (HANDLE hThread : g_hThreads) {
		if (hThread && hThread != INVALID_HANDLE_VALUE) {
			CloseHandle(hThread);
		}
	}
	g_hThreads.clear();  // 清空容器
	// 关闭事件对象
	if (g_hExitEvent) {
		CloseHandle(g_hExitEvent);
		g_hExitEvent = nullptr;
	}
}