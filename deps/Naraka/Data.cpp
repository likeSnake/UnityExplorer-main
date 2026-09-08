#include "Data.h"
DoubleBuffer data_buffers;
std::mutex g_InteractiveMutex;

int 跳出方式 = 0;
int 平A方式 = 0;
long 双刀上次出蓄力时间;
long 长剑上次出蓄力时间;
long 上次Shiftsh时间;
long 敌人上次闪避时间;
long 我上次闪避时间;
bool 活化闪避 = false;

bool 鼠标左键是否按下 = false;
bool 鼠标右键是否按下 = false;
bool 是否半自动模式 = false;
int 上次武器按键 = KEY_1_EXCLAMATION_MARK;
bool 博弈键按下 = false;
bool 调试模式 = true;

LocalPlayer_Data1 LocalPlayer_Data;
PlayerData FatPlayerData;
namespace Function
{
	namespace ESP
	{
		//  快捷键绑定
		uint32_t 显示或隐藏界面按键;

		bool 菜单 = true;
		bool 菜单_2 = true;
		bool 方框 = false;
        bool 骨骼 = false;
		bool 信息 = true;
		bool 血条 = true;
		bool 预警 = true;
		bool 射线 = true;
		bool 手持 = true;
		bool 名字 = true;
		bool 灵魂 = true;

		bool 近战_白 = false;
		bool 近战_蓝 = false;
		bool 近战_紫 = true;
		bool 近战_金 = true;
		bool 远程_白 = false;
		bool 远程_蓝 = false;
		bool 远程_紫 = true;
		bool 远程_金 = true;
		bool 道具 = false;
		bool 扩容 = false;
		bool 智能扩容 = true;
		bool 金魂 = false;
		bool 夺魂 = true;
		bool 属性 = false;
		bool 盒子 = false;

		bool 护甲Lv1 = false;
		bool 护甲Lv2 = false;
		bool 护甲Lv3 = true;
		bool 护甲Lv4 = true;
		bool 护甲Lv5 = true;
		bool 换甲护甲 = true;

		bool 果实 = false;
		bool 萤火虫 = true;
		bool 金堆 = true;
		bool 蓝堆 = false;
		bool 绿堆 = true;
		bool 任务 = true;
		bool 秘籍 = false;
		bool 所有对象 = false;

		int 血条样式 = 0;
		int 预警样式 = 0;
		ImColor 人机颜色 = ImColor(60, 179, 113);
		ImColor 玩家颜色 = ImColor(255, 0, 0);
		ImColor 信息颜色 = ImColor(255, 0, 0);
		ImColor 名字颜色 = ImColor(60, 179, 0);
		float daoJuTextSize = 15;
		float userTextSize = 15;
	}
	namespace Kmbox
	{
		namespace Net
		{
			char IP[0XFF], Port[0XFF], mac[0XFF];
			int NetState = 5;
		}
		namespace B_Pro
		{
			bool BproState = false;
			char PorttextBuffer[0XFF] = "\0";
			char SpeedBuffer[0XFF] = "115200";
		}
		namespace DhzBox_Lite
		{
			bool DproState = false;
			char IP[0XFF] = "192.168.8.88";
			char Port[0XFF] = "8888";
			char RANDOM[0XFF] = "88";
		}
	}
	namespace Shock
	{
		bool F_ShockKnife = true;		//振刀功能开关
		bool M_ShockKnife = false;				//内存振开关
		bool Self_play = false;	//振刀判断到需要振刀时，是否执行振刀
		bool Flag_CollideKnife = true;	//振刀判断到需要拼刀时，是否执行拼刀
		bool Flag_ShortDodge = true;	//振刀判断到需要闪避时，是否执行闪避
		bool 扬鞭劲振刀 = false;
		bool 横栏振刀 = false;
		bool 横刀左蓄振刀 = false;
		bool 只振视野范围内 = false;

		int 振刀按键 = KEY_G;
	}
	namespace 侧键博弈 {

		 bool 蓄力抓振 = true;
		 bool 博弈切刀抓振刀 = true;
		 bool 蓄力抓长闪 = false;
		 bool 蓄力抓短闪 = true;
		 bool 蓄力抓白刀 = true;
		 bool 蓄力抓倒地 = true;
		 bool 蓄力抓受击 = true;
		 bool 蓄力抓起跳 = false;
		 bool 蓄力抓下蹲 = false;
		 bool 自动连招 = true;
		 bool 禁用CPU0 = true;

		 bool 白刀抓长闪 = true;
		 bool 白刀抓短闪 = true;
		 bool 白刀抓蓄力后摇 = true;
		 bool 白刀抓振 = true;
		 bool 白刀蓝顶 = true;

		 bool 切拳起身 = true;

		 int 动态线程 = 0;

		 int 蓄力抓振延迟 = 230;
		 int 白刀抓短闪延迟 = 200;
		 int 白刀抓蓄力后摇延迟 = 100;
		 float 白刀蓝顶延迟 = 0.1;
		 float 白刀抓短闪最远距离 = 5;
		 float 白刀抓长闪最远距离 = 5;

		 int 钩锁模式 = 1;
		 float 断续切刀时间 = 1.0;
		 float 断续切刀最小时间 = 0.50;

		 int 博弈热键 = VK_XBUTTON2;

		 float 博弈距离范围 = 6.0;
		 float 博弈对蓄力距离 = 5;

		 float 动态断蓄时间点 = 0.5;
	}

	namespace 半自动博弈 {

		bool 总开关 = true;
		bool 蓄力抓总开关 = true;
		bool 蓄力抓振 = true;
		bool 蓄力抓长闪 = false;
		bool 蓄力抓短闪 = true;
		bool 蓄力抓白刀 = true;
		bool 蓄力抓倒地 = true;
		bool 蓄力抓受击 = true;
		bool 蓄力抓起跳 = false;
		bool 蓄力抓下蹲 = false;
		bool 自动连招 = true;
		bool 自动断蓄 = false;

		bool 白刀抓总开关 = true;
		bool 白刀抓长闪 = true;
		bool 白刀抓短闪 = true;
		bool 白刀抓蓄力后摇 = true;
		bool 白刀抓振 = true;
		bool 白刀蓝顶 = true;
		bool 白刀抓受击 = true;

		int 动态线程 = false;

		int 蓄力抓振延迟 = 230;
		int 白刀抓蓄力后摇延迟 = 100;
		float 白刀蓝顶延迟 = 0.1;
		float 白刀抓短闪最远距离 = 5;
		float 白刀抓长闪最远距离 = 5;

		int 钩锁模式 = 1;
		float 断续切刀时间 = 1.2;

		int 博弈热键 = VK_XBUTTON2;
	}

	namespace LogicKnife
	{
		bool BlueAttackShock = true;	//出蓄抓振刀功能开关
		bool BlueAttackWrite = true;    //出蓄抓白刀功能开关
		bool BlueAttackdodge = true;    //出蓄抓闪功能开关
		bool BlueZhuaJiangZhi = true;    //出蓄抓站立僵直
		bool BlueAttacklanding = true;    //出蓄蓝顶功能开关
		bool BlueAttackDuanXu = true;    //自动断蓄功能开关
		bool AutoCombo = true;			//自动连招功能开关
		bool BlueAttackDuanXu_fast = true;	//自动连招功能开关
		bool auto_pickup_weapon = false;	//自动毛刀
		bool NO_Shock_3A = false;	//不振3A

		float 博弈距离范围外 = 7;
		float 博弈距离范围内 = 7;
		float 博弈对蓄力距离 = 5;

		bool 嫖刀开关 = false;
	}
	namespace HeroActivation
	{
		bool IsDodge = true;		//闪避功能开关
		bool 躲避火男F = true;
		bool 躲避火男冲拳 = true;
		bool 躲避妖刀大招 = true;
		bool 躲避顾清寒V1V2 = true;
		bool 躲避武田F = true;
		bool 躲避胡为 = true;
		bool 躲避宁红叶F = true;
		bool 躲避狐狸大招 = true;
		bool 躲避三娘V2 = true;
		bool 躲避哈迪大招 = true;
		bool 躲避岳山F = true;
		bool 躲避魏轻F1 = true;
		bool 躲避魏轻V = true;
		bool 躲避迦南V2 = true;
		bool 躲避迦南F2 = true;
		bool 躲避刘炼V = true;
		bool 躲避张起灵 = true;
		bool 躲避季盈盈V蓄力 = true;
		bool 躲避无尘V2 = true;
		
		bool 技能活化 = false;
		bool 武田自动毛刀 = true;
		bool 和尚自动F2 = true;
		bool 季盈盈自动放蓄 = true;
		bool 顾清寒自动放蓄 = true;

		namespace 特殊处理
		{
			bool 永远躲避顾倾寒V1 = true;
		}

	}
	
	namespace Flag
	{
		bool IsMyManage = false;					//自身是否可以振刀的FLAG
		bool IsBlueToRed = false;					//自身是否可以续转
		bool dodgeBlueToRed = false;					//自身是否可以闪振							
		bool ISAtoG = true;							//判断招式是否可以G震
		bool HeroTianHaiF2 = false;				//天海是否可以使用F2
		bool 数据遍历 = false;
		bool 全局数据 = false;
		bool 人物数据 = false;
		bool 对象数据 = false;
		bool 堆堆数据 = false;
		bool 物品数据 = false;
		bool 显示未知物品ID = false;
		bool 显示未知堆ID = false;
		bool 开发日志 = false;
		bool 性能测试开关 = false;

		float 蓝顶延迟 = 0;
	}
	namespace AiMBot
	{
		bool AiMBotState = true;
		char AimKey[0XFF] = "17";
		float smoothnessX = 1.42;
		float smoothnessY = 1.0;
		float AimRange = 200;
		float maxSpeed = 25.;
		float smoothingFactor = 0.4; 
		float deadZone = 0.25;
		float lockError = 1.0f;
		float Dt = 1.1f;
		int 总平滑度 = 1;
		int 自瞄热键 = 1;

	}

}

namespace 振刀参数
{
	namespace 振刀方式
	{

		长剑振刀方式 长剑;
		链剑振刀方式 链剑;
		太刀振刀方式 太刀;
		阔刀振刀方式 阔刀;
		长枪振刀方式 长枪;
		匕首振刀方式 匕首;
		双截棍振刀方式 双截棍;
		双刀振刀方式 双刀;
		长棍振刀方式 长棍;
		斩马刀振刀方式 斩马刀;
		双戟振刀方式 双戟;
		扇子振刀方式 扇子;
		横刀振刀方式 横刀;
		拳刃振刀方式 拳刃;
		飞刀振刀方式 飞刀;
	}

	namespace 闪避
	{

		长剑闪避延迟 长剑;
		链剑闪避延迟 链剑;
		太刀闪避延迟 太刀;
		阔刀闪避延迟 阔刀;
		长枪闪避延迟 长枪;
		匕首闪避延迟 匕首;
		双截棍闪避延迟 双截棍;
		双刀闪避延迟 双刀;
		长棍闪避延迟 长棍;
		斩马刀闪避延迟 斩马刀;
		双戟闪避延迟 双戟;
		扇子闪避延迟 扇子;
		横刀闪避延迟 横刀;
		拳刃闪避延迟 拳刃;
		飞刀闪避延迟 飞刀;
	}

	namespace 距离
	{

		长剑1 长剑;
		链剑振刀方式2 链剑;
		太刀1 太刀;
		阔刀1 阔刀;
		长枪1 长枪;
		匕首1 匕首;
		双截棍1 双截棍;
		双刀1 双刀;
		长棍1 长棍;
		斩马刀1 斩马刀;
		双戟1 双戟;
		扇子1 扇子;
		横刀1 横刀;
		拳刃1 拳刃;
		飞刀1 飞刀;
	}

	namespace 角度
	{
		长剑2 长剑;
		链剑振刀方式2 链剑;
		太刀2 太刀;
		阔刀2 阔刀;
		长枪2 长枪;
		匕首2 匕首;
		双截棍2 双截棍;
		双刀2 双刀;
		长棍2 长棍;
		斩马刀2 斩马刀;
		双戟2 双戟;
		扇子2 扇子;
		横刀2 横刀;
		拳刃2 拳刃;
		飞刀2 飞刀;
	}

	namespace 延迟
	{
		长剑3 长剑;
		链剑振刀方式2 链剑;
		太刀3 太刀;
		阔刀3 阔刀;
		长枪3 长枪;
		匕首3 匕首;
		双截棍3 双截棍;
		双刀3 双刀;
		长棍3 长棍;
		斩马刀3 斩马刀;
		双戟3 双戟;
		扇子3 扇子;
		横刀3 横刀;
		拳刃3 拳刃;
		飞刀3 飞刀;
	}

	namespace 特殊处理
	{
		bool 阔刀右蓄是否短闪振 = true;
		bool 双截棍左蓄是否短闪振 = true;
		bool 斩马刀左蓄是否长闪振 = true;
		bool 扇子左蓄是否中闪振 = true;
		bool 扇子右蓄是否中闪振 = true;
		bool 横刀左蓄是否只短闪 = true;
        bool 拳刃右蓄是否短闪振 = true;
		bool 长枪左蓄是否短闪振 = true;
		bool 是否长闪振 = false;
		bool 胡为开大振刀 = true;
	}
}

namespace 活化参数
{
	namespace 距离
	{
		float 火男冲拳 = 3.5f;		//0-5M
		float 火男顶膝 = 7.5f;		//0-5M
		float 妖刀大招 = 25.f;		//0-35M
		float 顾倾寒大招 = 25.f;	//0-35M
		float 武田F2 = 5.f;			//0-6M
		float 武田F3 = 3.5f;			//0-4M
		float 胡为F1 = 5.5f;			//0-7M
		float 胡为F2 = 8.f;			//0-10m
		float 胡为跃击 = 10.f;		//0-12M
		float 胡为V1V2 = 10.f;		//0-15M
		float 宁红叶F1 = 8.f;		//0-10M
		float 宁红叶F2 = 8.f;		//0-10M
		float 宁红叶F3 = 8.f;		//0-10M
		float 狐狸V1 = 12.f;		//0-20M
		float 狐狸V2 = 7.5f;		//0-20M
		float 火男F = 5.f;			//0-5M
		float 岳山F = 7.5f;			//0-5M
		float 三娘V2 = 35.f;		//0-40M
		float 哈迪大招1段 = 10.f;	//0-12M
		float 哈迪大招2段 = 15.f;	//0-12M
		float 哈迪大招3段 = 20.f;	//0-15M
		float 魏轻F1 = 6.f;		//0-8M
		float 魏轻V1 = 12.f;		//0-20M
		float 魏轻V2 = 4.f;		//0-6M
		float 迦南V2 = 20.f;		
		float 迦南F2 = 20.f;		
		float 刘炼V1 = 20.f;
		float 刘炼V2 = 12.f;
		float 张起灵F2 = 10.f;
		float 张起灵V1 = 15.f;
		float 张起灵V12段 = 10.f;
		float 张起灵V2 = 20.f;
		float 季盈盈V1蓄力 = 10.5f;
		float 季盈盈V2蓄力 = 10.5f;
		float 无尘V2 = 15.f;
		float 希拉F = 14.5f;
		float 彩戏师F = 13.5f;		
	}

	namespace 角度
	{
		float 火男冲拳 = 25.f;
		float 妖刀大招3M = 90.f;
		float 妖刀大招3M外 = 35.f;
		float 顾倾寒大招3M = 45.f;
		float 顾倾寒大招3M外 = 5.f;
		float 武田F21M = 180.f;
		float 武田F21M外 = 8.f;
		float 武田F31M = 180.f;
		float 武田F31M外 = 8.f;
		float 胡为F2 = 45.f;
		float 胡为跃击 = 45.f;
		float 胡为V1V2 = 45.f;
		float 宁红叶F1 = 90.f;
		float 宁红叶F2 = 90.f;
		float 宁红叶F3 = 90.f;
		float 狐狸大招4M = 90.f;
		float 狐狸大招8M = 45.f;
		float 狐狸大招12M = 25.f;
		float 狐狸大招12M外 = 15.f;
		float 火男F = 90.f;
		float 岳山F = 90.f;
		float 三娘V24M = 45.f;
		float 三娘V210M = 25.f;
		float 三娘V210M外 = 12.f;
		float 哈迪大招 = 120.f;
		float 魏轻F1 = 90.f;
		float 魏轻V1 = 90.f;
		float 魏轻V2 = 90.f;
		float 迦南V2 = 180.f;	
		float 迦南F2 = 90.f;
		float 刘炼V1 = 120.f;
		float 刘炼V2 = 120.f;
		float 张起灵F2 = 90.f;
		float 张起灵V1 = 90.f;
		float 张起灵V12段 = 90.f;
		float 张起灵V2 = 90.f;
		float 季盈盈V1蓄力 = 90.f;
		float 季盈盈V2蓄力 = 90.f;
		float 无尘V2 = 90.f;
		float 希拉F = 90.f;
		float 彩戏师F = 90.f;
	}

	int 出蓄抓振延迟 = 0;
	int 蓝顶延迟 = 101;
}

namespace 调试
{
	逻辑执行1 逻辑执行;
	bool 输出逻辑执行 = false;
	bool 输出逻辑执行Flag = false;		//这个不用加在菜单，随动的。
	bool 输出调试执行 = false;
}

namespace 连招方式 {
	int 长剑 = 0;
	int 链剑 = 0;
	int 太刀 = 0;
	int 阔刀 = 0;
	int 长枪 = 0;
	int 匕首 = 0;
	int 双截棍 = 0;
	int 双刀 = 0;
	int 长棍 = 0;
	int 斩马刀 = 0;
	int 双戟 = 0;
	int 扇子 = 0;
	int 横刀 = 0;
	int 拳刃 = 0;
	int 飞刀 = 0;
}