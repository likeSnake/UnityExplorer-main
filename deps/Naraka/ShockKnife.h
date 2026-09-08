#pragma once
#include <stdint.h>
#include "Hook.h"
#include "Data.h"
#include "DhzBoxNet.h"

namespace 振刀逻辑
{
	//振刀
	void BlueToRed();
	//双键振刀
	void DoubileBondShock();
	//跳振
	void JumpShock();
	void cA或长闪A(int type, int spTime);
	//蹲振
	void SquatShock();
	//使用F
	void SkillF();
	//使用V
	void SkillV();
	//内存振
	void MemShock();
	//天海F2振刀
	void TianhaiF2();
	//拼刀
	void CollideKnife();
	//短闪
	void ShortDodge(uint32_t Key);
	//长闪
	void LongDodge(uint32_t Key);
	//中闪
	void ZhongDodge();
	//随机左右短闪
	void ShortDodgeEx();
	//随机前后左右左右短闪
	void ShortDodgeExAll();
	//随机前后短闪
	void ShortDodgeWx();
	//闪振
	void DodgeExBlueToRed(int sleep);
	void 后闪振();
	void 强后闪振();
	//后闪修刀振
	void DodgeExXiuBlueToRed(int sleep);
	//武田随机F
	void WuTianF();
	//腾云式右键抓振刀后摇
	void 腾云式B();
	//C滚轮切刀
	void C滚轮切刀();
	void C滚轮切刀重置蓄力();
	float 重置蓄力断蓄点();
	//跳切刀
	void 跳断蓄();
	//自动断蓄
	void 断蓄();
	//cc断蓄
	void CC();
	void 跳A();
    void 短接长闪();
	// 长闪A
	void 长闪A(int Key);
    //长闪绕背
	void 长闪绕背(uint32_t Key);
	void 月闪切(float ping);
	void 月闪切右A(float ping);
	void 月闪切2();
	void 太双切A();
	void 太双切A太刀();
	void 背包切();
	void 长剑A抓僵直();
	bool 侧键是否按下();

	void 长剑蓄力释放();
	void 链剑蓄力释放();
	void 太刀蓄力释放();
	void 横刀蓄力释放();
	void 双刀蓄力释放();
	void 双戟蓄力释放();
	void 双截棍蓄力释放();
	void 斩马蓄力释放();
	void 斩马柄击释放();
	void 长棍蓄力释放();
	void 长枪蓄力释放();
	void 匕首蓄力释放();
	void 拳刃蓄力释放();

	void 处决后跳A();
	void 处决后双跳A();
	bool 通用CC断蓄力();
	float 取随机Folat数(float min, float max);
	bool 通用CC断蓄进蓄力();
	void 半自动断蓄力();
	bool 长闪蓄力(int 平A方式 = 0);
	bool 自动选择重置蓄力方式(int 平A方式 = 0);
	void 长闪A2(int Key, float mSleep);
	void 平A(int Key);
    void 升龙();
    void 拳刃升龙右A();
	void 单刀拳双切A(bool 左A = true, int SleepTimp = 100);
	void 拳切百裂(int sleep);
	void 拳切3A(int sleep);
	void 钩锁百裂();
	void 钩锁百裂自动钩锁();
	
	void 执行横刀平A僵直抓取();
	void 执行太刀平A僵直抓取();
	void 执行长剑平A僵直抓取();
	void 执行链剑平A僵直抓取();
	void 执行双截棍平A僵直抓取();
	void 执行匕首扇子平A僵直抓取();
	void 执行双刀平A僵直抓取();
	void 执行双戟平A僵直抓取();
	void 执行斩马刀平A僵直抓取();
	void 执行长棍平A僵直抓取();
	void 执行长枪平A僵直抓取();
	void 执行拳刃平A僵直抓取();


	void 双刀太刀切拳百裂();
	void 太刀月影闪切双刀A僵直抓取();
	void 太刀月影闪切长剑A僵直抓取();
	void 太刀月影闪切双戟A僵直抓取();
	void 双刀月影闪切太刀A僵直抓取();
	void 双刀月影闪切横刀A僵直抓取();
	void 横刀拳切百裂();
	void 太刀A4拳切百裂();
	void 通用拳切百裂(bool 直接升龙 = false, int 蓄力方式 = 1, bool 左A抓 = true, int 结束闪避方式 = 0);
	void 长枪基础跳A();
	void 判断断闪蓄();
	void 长剑重刃s闪AB();
	void 通用双刀流切拳蓄力(int type = 0, int 左右刀 = 0, bool 直接升龙 = false, bool 左蓄 = false);
	void 双刀铁马残红切刀蓄(int type = 0, int 左右刀 = 0, bool 直接升龙 = false, bool 左蓄 = false);
	void 双刀BB();
	void 匕首扇子BBCA();
	void 双刀BB右2();
	void 抓壁击(int type);

	void 横刀太刀切刀颠勺();
	void 切刀断闪蓄(int 蓄力方式 = 0); // 0左蓄 1右蓄
	void 横刀切刀颠勺();
	void 双刀流打磐石();
	void 通用C切百裂(int 平A类型 = 0);
	void b1c切Ac切A(int 平A类型 = 0, int 魂玉类型 = 0);
	void 拳刃升龙bbca();
	void 双键闪(uint32_t Key1, uint32_t Key2);

	/*武器博弈*/
	void 太刀博弈();
	void 横刀博弈();
	void 双刀博弈();
	void 匕首博弈();
	void 双戟博弈();
	void 长剑博弈();
	void 链剑博弈();
	void 双截棍博弈();
	void 斩马刀博弈();
	void 长棍博弈();
	void 长枪博弈();
	void 拳刃博弈();
	void 飞刀博弈();
	void 空手博弈();

	/*通用*/
	void 释放受击技能();
	void 执行太刀嫖刀僵直抓取();
	void 执行双刀嫖刀僵直抓取();
	void 随机左右闪();
	void 通用嫖刀抓取();
	bool 出招后血量判断();
	bool 是否特殊状态();
	bool 侧键按下或半自动();
	bool 判断敌方是否受击中();
	void 左右键直接松开();
	void 通用切刀重置蓄力(bool 强制切刀 = false);
	bool 是否单刀武器();
	bool 可C切武器(int 当前武器);
	bool 不可拳切白裂武器(int 当前武器);
	bool 抓闪判断(wstring O_ActionName);
	bool 判断鼠标左键是否按下();
	bool 判断鼠标右键是否按下();
	void 判断鼠标左键状态改变(bool isEnd);
	void 判断鼠标右键状态改变(bool isEnd);

	void 通用钩锁白裂(int 平A类型 = 0, bool 直接升龙 = false);
	void 长剑浮空3A(int 平A类型 = 0);
	bool 抓短闪时间判断();
	bool 蓄力抓短闪判断();
	// 方向0W 1S  2A 3D
	void 长闪蓄(bool 左蓄 = true, int 方向 = 0);

	
}