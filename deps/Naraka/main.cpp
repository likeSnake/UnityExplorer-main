#include"pak.h"
#include "Thread.h"
#include "libs/VMProtectSDK.h"

#include <iostream>
#include "NetVerHelp/Help头文件/NetworkVerHelp.h"
#include "NetVerHelp/Help头文件/Rc4.h"

#include <windows.h>
#include <winhttp.h>
#include <string>


// 自己加的验证
std::wstring myServer = L"pubg.ling520.top";
//std::wstring myServer = L"127.0.0.1";
std::wstring MyYjwjPath = L"/pubg/userDMAYJWJInit";

//瑞科网络验证：www.rukeyz.com

//平台类型  1:网络验证平台  2:自动发货平台
Model::PlatformType platformtypeid = Model::PlatformType::NetVer;
string encryptKey = "3bb90ff2";//加密的key 注意：如果软件设置了加密通讯那么此值必填(此参数是在软件列表里面进行设置)
string signSalt = "8852acee";//签名盐 注意：如果软件设置了加密通讯那么此值必填(此参数是在软件列表里面进行设置)
Model::EncryptType encrypttypeid = Model::EncryptType::RC4;//请求接口时，业务数据data参数加密码通讯类型(此参数是在软件列表里面进行设置)  0：无加密   3：RC4加密:
string goodscode = "51a3d5e3c61c94ce";//必填，软件或者商品的编码。此值在哪获取：看接口文档有说明
string platformUserCode = "81f88dbb510e18fd48c68203c55efe20a4deb31e";//个人中心里面可以查看得到。代理商的话，那么在：代理管理--》代理商列表，可以查看得到 注意：如果是作者自己就填写自己的platformUserCode,如果是代理商的，得填写代理商的Code

string versionname = "v2.6";//当前软件版本号
string giteeVersionname = "2.3";//当前软件版本号
string maccode = Help::NetworkVerHelp::GetMac();//机器码


//此账号是测试的账号，已过期了。请您在验证平台手动注册一个，或者通过接口注册一个用来测试
string userName = "test2";
string userpwd = "111111";
string newUserPwd = "222222";

//此卡号是测试的卡号，已过期了。您自己请从“卡密登录->>卡密列表,生成一个卡密，然后拿做测试”
string cardnum = "yue-89907a78330f4701";
string TestVer = "测试变量名";//获取软件的变量，如果变量名称为空的话，那么接口返回来的是此软件所有变量，否则就是此变量名所以应的变量值



//心跳Key,每次心跳此值都会变
string heartbeatkey;
//登录成功后的令牌
string token;

double myGlobalTime = 0.0;

#define DISABLE_QUICK_EDIT_MODE 0x01
#define DISABLE_INSERT_MODE 0x02
#define DISABLE_MOUSE_INPUT 0x03 
#define DISABLE_ALL (DISABLE_QUICK_EDIT_MODE | DISABLE_INSERT_MODE | DISABLE_MOUSE_INPUT)

VOID CloseConsoleMode(UINT uTag = DISABLE_QUICK_EDIT_MODE)
{	//通用函数
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	if (uTag & DISABLE_QUICK_EDIT_MODE)
		mode &= ~ENABLE_QUICK_EDIT_MODE;  //移除快速编辑模式
	if (uTag & DISABLE_INSERT_MODE)
		mode &= ~ENABLE_INSERT_MODE;      //移除插入模式
	if (uTag & DISABLE_MOUSE_INPUT)
		mode &= ~ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin, mode);
	return;
}
VOID CloseConsoleQuickEditMode(VOID)
{	//关闭快速编辑模式,这个很好用
	CloseConsoleMode(DISABLE_QUICK_EDIT_MODE);
}
VOID CloseConsoleInsertMode(VOID)
{	//关闭插入模式？我也不是很懂
	CloseConsoleMode(DISABLE_INSERT_MODE);
}
VOID CloseConsoleMouseInput(VOID)
{	//关闭鼠标输入？我也不是很懂
	CloseConsoleMode(DISABLE_MOUSE_INPUT);
}
void DataRefresh()
{
	while (true)
	{
		mem.RefreshConfigSet();
		this_thread::sleep_for(chrono::milliseconds(30));
	}
}


bool GetModuleStatus()
{

    if (mem.Init("NarakaBladepoint.exe", false, false))
    {

		printf("[+] Dma初始化成功\n");
		
		if (mem.GetKeyboard() ->InitKeyboard())
		{
			printf("[+] 注册热键成功 \n");
		}
		else { printf("[+] 注册热键失败 需要重启主机 不是副机 不是副机 不是副机 \n"); }
		/*do {
			Sleep(100);
			Offset::UnityPlayer = mem.GetBaseDaddy("UnityPlayer.dll");

			if (Offset::UnityPlayer) {
				printf("[+] GameAddrBase: %llx\n", Offset::UnityPlayer);
				break;
			}
		} while (!Offset::UnityPlayer);*/
		do {
			Sleep(100);
			Offset::NarakaBladepoint = mem.GetBaseDaddy("NarakaBladepoint.exe");
			if (Offset::NarakaBladepoint) {
				 //printf("[+] NarakaBladepoint: %llx\n", Offset::NarakaBladepoint);
				break;
			}
		} while (!Offset::NarakaBladepoint);
		bool useGameAssemblySuper = false;
		do {
			Sleep(100);
			useGameAssemblySuper = false;
			Offset::GameAssembly = mem.GetBaseDaddy("GameAssembly.dll");
			if (!Offset::GameAssembly) {
				Offset::GameAssembly = mem.GetBaseDaddy("GameAssembly_Super.dll");
				useGameAssemblySuper = (Offset::GameAssembly != 0);
			}

			if (Offset::GameAssembly) {
				Offset::ApplyGameAssemblyOffsets(useGameAssemblySuper);
				if (useGameAssemblySuper) {
					printf("使用第二套方案:GameAssembly_Super.dll\n");
				}
				/*printf("[+] GameAddrBase (%s): %llx\n",
					useGameAssemblySuper ? "GameAssembly_Super.dll" : "GameAssembly.dll",
					Offset::GameAssembly);*/
				break;
			}
		} while (!Offset::GameAssembly);
		if (Offset::GameAssembly)
		{
			/*size_t size = mem.GetBaseSize("GameAssembly.dll");
			printf("[+] ++++++++++++++++开始扫描更新基址++++++++++++++++++ \n");
			auto ptr = mem.FindSignature("48 8B 0D ?? ?? ?? ?? F6 81 37 01 00 00 02 74 0E 83 B9 E0 00 00 00 00 75 05 E8 ?? ?? ?? ?? 48 89 6C 24 78 33 C9 48 89 BC 24 80 00 00 00", Offset::GameAssembly,Offset::GameAssembly + size);
			cout << "世界基址: " << ptr << endl;*/
			/*printf("[+] 鼠标插盒子 鼠标插盒子 鼠标插盒子  \n");*/
			//printf("[+] 数据读取成功执行渲染 \n");
			return true;
		}
		return false;
    }
	else
	{
		printf("[+] Dma初始化失败\n");
		return false;
	}
}
const string INI_FILE = ".\\Key.ini";
// 保存卡密到 INI 文件
void SaveCardToINI(const string& cardnum) {
	// 写入卡密到 INI 文件
	if (WritePrivateProfileString("Key", "CardNumber", cardnum.c_str(), INI_FILE.c_str())) {
		cout << "卡密已保存！" << endl;
	}

}

string LoadCardFromINI() {
	char cardnum[256];
	// 从 INI 文件读取卡密
	if (GetPrivateProfileString("Key", "CardNumber", "", cardnum, sizeof(cardnum), INI_FILE.c_str())) {
		return string(cardnum);
	}
	return "";
}

std::string Utf8ToGbk(const char* utf8) {
	// 先把 UTF-8 转成 Unicode
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	std::wstring wstr(wideLen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], wideLen);

	// 再把 Unicode 转成 GBK
	int gbkLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string gbkStr(gbkLen, '\0');
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &gbkStr[0], gbkLen, NULL, NULL);

	return gbkStr;
}

#pragma region 第一步，先初始化软件信息
//第一步，先初始化软件信息
bool IniSoftInfo()
{
	bool IniResult = false;
	//构建初化软件入参
	Model::In_IniSoftInfoArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.versionname = versionname;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GetIniSoftInfo(args, platformtypeid, encryptKey, signSalt, encrypttypeid, goodscode, platformUserCode);
	if (result.code == 0)
	{
		switch (platformtypeid)
		{
		case Model::NetVer://网络验证平台
			//iniSoftInfoData:初始化完后的具体结果
			if (Help::iniSoftInfoData.inisoftkey != "")
			{
				if (Help::iniSoftInfoData.requestflag != args.requestflag)
				{
					IniResult = false;
					Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\n";
					std::cout << Msg + "\n";
					return IniResult;
				}

				Msg = Msg + "初始化软件成功\n";
				//Msg = Msg + "接口返回的数据：\n";
				//Msg = Msg + "编码：" + to_string(result.code) + "\n";
				//Msg = Msg + "信息：" + result.msg + "\n";
				//Msg = Msg + "data数据：" + result.data + "\n";
				Help::Rc4 rc4;
				std::string decrypted = Utf8ToGbk(rc4.Decrypt(result.data.c_str(), encryptKey.c_str()));

				cJSON* root = cJSON_Parse(decrypted.c_str());
				if (!root) {
					cout << "解析 JSON 失败！" << endl;
					return -1;
				}

				// 获取 softinfo 对象
				cJSON* softinfo = cJSON_GetObjectItem(root, "softinfo");
				if (softinfo) {
					// 获取 notice 字段
					cJSON* notice = cJSON_GetObjectItem(softinfo, "notice");
					if (notice ) {
						//Msg = Msg + "\n**************公告******************\n";
						//Msg = Msg + notice->valuestring + "\n";
						//cout << "notice: " << notice->valuestring << endl;
					}
				}

				// 释放内存
				cJSON_Delete(root);

				IniResult = true;
				//Msg = Msg + decrypted;
				if (Help::iniSoftInfoData.softInfo.newversionnum != "")
				{
					if (Help::iniSoftInfoData.softInfo.newversionnum != versionname)
					{
						//Msg = Msg + "\n发现新版本：" + Help::iniSoftInfoData.softInfo.newversionnum + "\n";
						//Msg = Msg + "" + Help::iniSoftInfoData.softInfo.networkdiskurl + "\n";
						//Msg = Msg + "提取码:" + Help::iniSoftInfoData.softInfo.diskpwd + "\n";

						if (Help::iniSoftInfoData.softInfo.isforceupd) {
							// 强制更新
							//Msg = Msg + "当前版本为强制更新\n";
							IniResult = false;
						}
						else {
							//Msg = Msg + "不强制更新，用户自由选择更新\n";
						}
					}
				}

			}
			break;
		}




	}
	else
	{
		Msg = Msg + "初始化软件失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//软件初始化错误信息【软件】
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用


		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//1003:作者软件或商品销售后台到期
		//1011:平台用户Code-platformUserCode,不存在
		//1010:此软件未分配给代理商
		//2001:版本号-versionname，不能为空


		//软件初始化错误信息【商品】
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//1011:平台用户Code-platformUserCode,不存在
	}

	std::cout << Msg + "\n";
	return IniResult;
}
#pragma endregion

#pragma region 心跳示例
//心跳示例
string HeartBeat(string cardnumorusername)
{

	//心跳那一块，你得加容错机制，心跳正常的操作如下：
	//错误失败次数 = 0;
	//while (true) {
	//	心跳结果 = 请求瑞科心跳接口(心跳参数);
	//	if (心跳结果.code != 0) {
	//		if (心跳结果.code == -999) {
	//			错误失败次数 = 错误失败次数 + 1;
	//		}
	//		else {
	//			强制关闭软件
	//		}
	//	}
	//	else {
	//		错误失败次数 = 0;
	//	}
	//	if (错误失败次数 >= 12) {
	//		//跳出循环啥不做处理
	//		//至余为什么连续失败12直接跳出不做任何处理的原因：
	//		//1、确实是本当前电脑网络有问题
	//		//2、有可能服务器当前有问题
	//		//以上两种情况，不能中断你用户的操作，如果用户确实到期了 
	//		//下次登录的时候反正肯定也是登录不上去的
	//		break;
	//	}
	//	延时(5分钟)
	//}

	string Msg = "";
	Msg = Msg + "开始心跳：\n";
	//注意：心跳是放在死循环里面，此只是做演示，所以没有做死循环了(注意：心跳间隔时间最低不能低于十秒钟一次，不然会引发心跳频繁)
	 //构建心跳入参
	Model::In_HeartBeatArgs HeartBeatArgs;
	HeartBeatArgs.maccode = maccode;//必填
	HeartBeatArgs.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	HeartBeatArgs.requestflag = to_string(HeartBeatArgs.timestamp);//必填
	HeartBeatArgs.cardnumorusername = cardnumorusername;//必填
	HeartBeatArgs.token = token;//必填
	HeartBeatArgs.heartbeatkey = heartbeatkey;//必填(每次把最新的心跳Key赋值)
	//请求接口，获取结果
	Model::Result	result = Help::NetworkVerHelp::GetHeartBeat(HeartBeatArgs);
	Msg = Msg + "心跳返回结果：\n";
	Msg = Msg + "编码：" + to_string(result.code) + "\n";
	Msg = Msg + "信息：" + result.msg + "\n";
	if (result.code == 0)
	{
		//心跳具体数据
		Model::Out_heartBeat heartBeatResult = Help::NetworkVerHelp::ConvertHeartBeat(result.data);
		//这一块相当重要：一定要判断这个“心跳Key(heartbeatkey)”和"请求标识(requestflag)"，防止别人修改你请求的地址，给你返回一个假消息回来
		if (heartBeatResult.heartbeatkey != "" && heartBeatResult.requestflag == HeartBeatArgs.requestflag)
		{
			heartbeatkey = heartBeatResult.heartbeatkey;
			Msg = Msg + "最新的心跳Key：" + heartBeatResult.heartbeatkey + "\n";
			Msg = Msg + "到期时间：" + heartBeatResult.endtime + "\n";
			Msg = Msg + "剩余点数：" + to_string(heartBeatResult.surpluspointvalue) + "\n\n";
		}
		else
		{

			Msg = Msg + "本程序被不法分子修改了接口返回值，您可以强制关闭软件\n\n";
		}
	}
	else
	{
		Msg = Msg + "心跳失败\n\n";
		//心跳错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1021:token失效或不正确
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1023:签名-sign,不正确
		//1019:data业务参数不需要加密
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1017:data业务参数Json不正确
		//1020:token不能为空
		//1054:平台类型-platformtypeid,错误

		//1033:心跳Key-heartBeatkey,不能为空
		//1034:心跳Key不正确
		//1012:心跳请求频繁

		//6005:卡密被禁用
		//4005:账号被禁用
		//4003:账号到期
		//6003:卡密到期
		//4004:账号点数不足
		//6004:卡密点数不足

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//4007:账号-username,不能为空
	}

	return Msg;
}
#pragma endregion

#pragma region 退出登录示例
//退出登录示例
void LoginOut(string cardnumorusername)
{
	std::cout << "退出登录：""\n";
	//构建退出登录入参
	Model::In_LoginOutArgs LoginOutArgs;
	LoginOutArgs.maccode = maccode;//必填
	LoginOutArgs.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	LoginOutArgs.cardnumorusername = cardnumorusername;//必填
	LoginOutArgs.token = token;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeLoginOut(LoginOutArgs);
	std::cout << "退出登录返回结果：""\n";
	std::cout << "编码：" + to_string(result.code) + "\n";
	std::cout << "信息：" + result.msg + "\n";
	std::cout << "data数据：" + result.data + "\n\n";

	//退出登录错误信息
	//1001:未知错误
	//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
	//1007:提交的参数Json不正确
	//1036:软件初始化的Key-inisoftkey,不能为空
	//1037:软件初始化的Key-inisoftkey,不存在

	//1002:系统出错
	//1015:data业务参数不能为空
	//1018:data业务参数需要加密
	//1022:签名-sign,不能为空
	//1021:token失效或不正确
	//1032:提交的加密类型参数，与后台设置加密类型不一致
	//1023:签名-sign,不正确
	//1019:data业务参数不需要加密
	//1030:没有此加密类型
	//1031:加密类型不对或加密不对
	//1017:data业务参数Json不正确
	//1020:token不能为空
	//1054:平台类型-platformtypeid,错误


	//1006：data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
	//6001:卡密-cardNum,不能为空
	//4007:账号-username,不能为空
}
#pragma endregion

#pragma region 卡密登录示例
//卡密登录示例
bool LoginByCard(string _CardNum, bool IsLoginOut)
{
	bool IsLoginOk = false;
	string Msg = "";
	//构建登录入参
	Model::In_CardLoginArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnum = _CardNum;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GetLoginByCard(args);
	Msg = Msg + "----------卡密登录操作----------\n";
	if (result.code == 0)
	{
		//登录成功后的具体数据
		Model::Out_Login LoginData = Help::NetworkVerHelp::ConvertLoginData(result.data);
		if (LoginData.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			IsLoginOk = false;
			return IsLoginOk;
		}
		Msg = Msg + "登录成功\n";
		//Msg = Msg + "接口返回的数据：\n";
		//Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		//Msg = Msg + "data数据：" + result.data + "\n";
		token = LoginData.token;
		heartbeatkey = LoginData.heartbeatkey;
		//Msg = Msg + "token令牌：" + LoginData.token + "\n";
		//Msg = Msg + "登录接口返回的心跳Key：" + LoginData.heartbeatkey + "\n";

		HeartBeat(_CardNum);//心跳示例

		if (IsLoginOut)
		{
			IsLoginOk = false;
			LoginOut(_CardNum);
		}
		else
		{
			IsLoginOk = true;
		}
	}
	else
	{

		Msg = Msg + "登录失败" + to_string(result.code) + "\n";
		//Msg = Msg + "接口返回的数据：\n";
		//Msg = Msg + "编码：" + to_string(result.code) + "\n";
		//Msg = Msg + "信息：" + result.msg + "\n";
		//Msg = Msg + "data数据：" + result.data + "\n";

		//卡密登录错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//1035:机器码 - maccode,长度不能大于100
		//6001:卡密-cardNum,不能为空
		//1027:此软件不是卡密登录类型
		//1003:作者软件或商品销售后台到期
		//6002:卡密-cardNum,不存在或已被删除
		//6003:卡密到期
		//6004:卡密点数不足
		//6005:卡密被禁用
		//6006:卡密已在线，禁止重复登录
		//6007:卡密尚未开通
		//1013:非绑定电脑上登陆
		//1014:超过最大登录数量
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
	return IsLoginOk;
}
#pragma endregion

#pragma region 卡密详情示例
//卡密详情示例
void CardDetail(string _CardNum)
{
	string Msg = "";
	//构建卡密详情入参
	Model::In_cardDetailArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnum = _CardNum;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeCardDetail(args);
	Msg = Msg + "----------卡密详情操作----------\n";
	if (result.code == 0)
	{
		//卡密详情具体数据
		Model::Out_cardDetail cardDetail = Help::NetworkVerHelp::ConvertCardDetail(result.data);
		if (cardDetail.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}
		Msg = Msg + "卡密详情成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";

		Msg = Msg + "到期时间：" + cardDetail.endtime + "\n";
		Msg = Msg + "剩余点数：" + to_string(cardDetail.surpluspointvalue) + "\n";

	}
	else
	{

		Msg = Msg + "卡密详情失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";

		//卡密详情错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//1027:此软件不是卡密登录类型
		//1003:作者软件或商品销售后台到期
		//6002:卡密-cardNum,不存在或已被删除
		//1065:查询速度过快
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 注册账号示例
//注册账号示例
void AccountRegister(string _username, string _userpwd)
{
	//构建注册账号入参
	Model::In_AccountRegisterArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.username = _username;//必填
	args.userpwd = _userpwd;//必填
	args.qq = "";//可选
	args.wx = "";//可选
	args.alipay = "";//可选
	args.tel = "";//可选
	args.email = "";//可选

	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeAccountRegister(args);
	Msg = Msg + "----------注册账号操作----------\n";
	if (result.code == 0)
	{
		//注册账号具体数据
		Model::Out_DataBaseArgs out_DataBaseArgs = Help::NetworkVerHelp::ConvertOutDataBaseArgs(result.data);
		if (out_DataBaseArgs.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}

		Msg = Msg + "注册账号成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
	}
	else
	{
		Msg = Msg + "注册账号失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";

		//账号注册错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1011:平台用户Code-platformUserCode,不存在
		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//10026:此软件不是账号登录类型
		//1003:作者软件或商品销售后台到期
		//1062:注册的账号或卡密过快，请稍后重试
		//3001:账号-username,密码-pwd,不能为空
		//3002:账号-username,长度不能大于20
		//3003:密码-userpwd,长度不能大于20
		//3007:账号-username,只能字母或数字
		//3005:已超过最大注册数量
		//1010:此软件未分配给代理商
		//3006：账号-username,已存在，请更换一个

	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;

}
#pragma endregion

#pragma region 账号密码登录示例
//账号密码登录示例
bool LoginAccount(string _username, string _userpwd, bool IsLoginOut)
{
	bool IsLoginOk = false;
	//构建账号登录入参
	Model::In_AccountLoginArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.username = _username;//必填
	args.userpwd = _userpwd;//必填

	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeAccountLogin(args);
	Msg = Msg + "----------账号密码登录操作----------\n";
	if (result.code == 0)
	{
		//账号登录具体数据
		Model::Out_Login LoginData = Help::NetworkVerHelp::ConvertLoginData(result.data);
		if (LoginData.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			IsLoginOk = false;
			return IsLoginOk;
		}

		Msg = Msg + "登录成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		token = LoginData.token;
		heartbeatkey = LoginData.heartbeatkey;
		Msg = Msg + "token令牌：" + LoginData.token + "\n";
		Msg = Msg + "登录接口返回的心跳Key：" + LoginData.heartbeatkey + "\n";

		Msg = Msg + HeartBeat(_username); //心跳示例

		if (IsLoginOut)
		{
			IsLoginOk = false;
			LoginOut(_username);
		}
	}
	else
	{
		Msg = Msg + "登录失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//账号登录错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：软件版本号-versionname,机器码-MacCode,时间戳-timestamp,不能为空
		//1035:机器码-maccode,长度不能大于100
		//4001:账号-username,密码-userpwd,不能为空
		//1026:此软件不是账号登录类型
		//1003:作者软件或商品销售后台到期
		//4002:账号或密码错误或被删除
		//4009:账号尚未开通
		//4003:账号到期
		//4004:账号点数不足
		//4005:账号被禁用
		//4002:账号或密码错误或被删除
		//4006:账号已在线，禁止重复登录
		//1013:非绑定电脑上登陆
		//1014:超过最大登录数量
	}

	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
	return IsLoginOk;
}
#pragma endregion

#pragma region 修改用户密码示例
//修改用户密码示例
void updPwd(string _username, string _userpwd, string _newpwd)
{
	//构建修改账号密码入参
	Model::In_UpdPwdArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.username = _username;//必填
	args.userpwd = _userpwd;//必填
	args.newpwd = _newpwd;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeUpdPwd(args);
	Msg = Msg + "----------修改用户密码操作----------\n";
	if (result.code == 0)
	{
		//修改账号密码具体数据
		Model::Out_DataBaseArgs out_DataBaseArgs = Help::NetworkVerHelp::ConvertOutDataBaseArgs(result.data);
		if (out_DataBaseArgs.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}

		Msg = Msg + "修改用户密码成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
	}
	else
	{
		Msg = Msg + "修改用户密码失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//修改用户密码错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//5001:账号-username,密码-userpwd,新密码-newpwd,不能为空
		//4002:账号或密码错误或被删除
		//4005:账号被禁用
	}
	Msg = Msg + "--------------------\n\n\n";

	std::cout << Msg;
}
#pragma endregion

#pragma region 账号详情示例
//账号详情示例
void AccountDetail(string _userName, string _userPwd)
{
	string Msg = "";
	//构建账号详情入参
	Model::In_accountDetailArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.username = _userName;//必填
	args.userpwd = _userPwd;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeAccountDetail(args);
	Msg = Msg + "----------账号详情操作----------\n";
	if (result.code == 0)
	{
		//账号详情具体数据
		Model::Out_accountDetail accountDetail = Help::NetworkVerHelp::ConvertAccountDetail(result.data);
		if (accountDetail.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}

		Msg = Msg + "账号详情成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		Msg = Msg + "到期时间：" + accountDetail.endtime + "\n";
		Msg = Msg + "剩余点数：" + to_string(accountDetail.surpluspointvalue) + "\n";

	}
	else
	{

		Msg = Msg + "账号详情失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//账号详情错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//4001:账号-username,密码-userpwd,不能为空
		//1026:此软件不是账号登录类型
		//1003:作者软件或商品销售后台到期
		//4002:账号或密码错误或被删除
		//1065:查询速度过快
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 扣点示例
//扣点示例
void BucklePoint(string cardnumorusername, int bucklevalue, string _token)
{
	//注意：此软件消耗类型是：点数，才可以调用此接口进行扣点
   //构建扣点入参
	Model::In_BucklePointArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnumorusername = cardnumorusername;//必填
	args.bucklevalue = bucklevalue;//必填
	args.token = _token;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GeBucklePoint(args);
	Msg = Msg + "----------扣点操作----------\n";
	if (result.code == 0)
	{
		//扣点具体数据
		Model::Out_bucklePoint bucklePoint = Help::NetworkVerHelp::ConvertBucklePoint(result.data);
		if (bucklePoint.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}
		Msg = Msg + "扣点成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		Msg = Msg + "剩余点数：" + to_string(bucklePoint.surpluspointvalue) + "\n";
	}
	else
	{
		Msg = Msg + "扣点失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//扣点错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1021:token失效或不正确
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1023:签名-sign,不正确
		//1019:data业务参数不需要加密
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1017:data业务参数Json不正确
		//1020:token不能为空
		//1054:平台类型-platformtypeid,错误


		//6005:卡密被禁用
		//4005:账号被禁用
		//4003:账号到期
		//6003:卡密到期
		//4004:账号点数不足
		//6004:卡密点数不足

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//4007:账号-username,不能为空
		//1024:此软件不是消耗点数类型
		//6002:卡密-cardNum,不存在或已被删除
		//6004:卡密点数不足
		//4008:账号-username,不存在或已被删除
		//4004:账号点数不足
	}

	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 获取远程变量
//获取远程变量
void GetremoteVar(string cardnumorusername, string _token, string _testVer)
{
	//构建获取软件变量入参
	Model::In_getRemoteVarArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnumorusername = cardnumorusername;//必填
	args.varname = _testVer;//必填，如果此值为空，那么接口返回来的是此软件所有的变量，否则是此变量名所对应的变量值
	args.token = _token;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::getRemoteVar(args);
	Msg = Msg + "----------获取软件变量操作----------\n";
	if (result.code == 0)
	{
		//变量具体数据
		Model::Out_getRemoteVar RemoteVar = Help::NetworkVerHelp::ConvertRemoteVar(result.data);
		if (RemoteVar.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}

		Msg = Msg + "获取软件变量成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "变量值：" + RemoteVar.varValue + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
	}
	else
	{
		Msg = Msg + "获取软件变量失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//获取软件变量错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1021:token失效或不正确
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1023:签名-sign,不正确
		//1019:data业务参数不需要加密
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1017:data业务参数Json不正确
		//1020:token不能为空
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006：data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//4007:账号-username,不能为空
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 获取远程算法
//获取远程算法
void Getcalculate(string calculateid, string _token, string requestargs)
{
	//构建获取远程算法入参
	Model::In_getCalculateArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.calculateid = calculateid;//必填
	args.requestargs = requestargs;//必填，此参数的格式：args1=value1，如果有多个参数则是：args1=value1&args2=value2
	args.token = _token;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::getCalculate(args);
	Msg = Msg + "----------获取远程算法操作----------\n";
	if (result.code == 0)
	{
		//远程算法具体数据
		Model::Out_getCalculate CalculateInfo = Help::NetworkVerHelp::ConvertCalculate(result.data);


		Msg = Msg + "获取远程算法成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "变量值：" + CalculateInfo.calculateresult + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
	}
	else
	{
		Msg = Msg + "获取远程算法失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 解绑机器码(如果软件设置解绑扣除相应的时间或点数，那么解绑成功后会自动扣除)
//解绑机器码(如果软件设置解绑扣除相应的时间或点数，那么解绑成功后会自动扣除)
void unbundMac(string cardnumorusername, string _token)
{
	//构建解绑机器码入参
	Model::In_unbundMacArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnumorusername = cardnumorusername;//必填

	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::unbundMac(args);
	Msg = Msg + "----------解绑机器码操作----------\n";
	if (result.code == 0)
	{
		//解绑机器码具体数据
		Model::Out_unbundMacArgs unbundMacData = Help::NetworkVerHelp::ConvertUnbundMac(result.data);
		if (unbundMacData.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}
		Msg = Msg + "解绑机器码成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		Msg = Msg + "到期时间：" + unbundMacData.endtime + "\n";
		Msg = Msg + "剩余点数：" + to_string(unbundMacData.surpluspointvalue) + "\n";

	}
	else
	{
		Msg = Msg + "解绑机器码失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//解绑机器码错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1021:token失效或不正确
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1023:签名-sign,不正确
		//1019:data业务参数不需要加密
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1017:data业务参数Json不正确
		//1020:token不能为空
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006：data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//4007:账号-username,不能为空

		//1009:商品Code-goodscode,不存在
		//6002:卡密-cardNum,不存在或已被删除
		//7004:在线支付中，禁止解绑
		//7001:剩余时间不够扣除解绑的数值
		//7002:剩余点数不够扣除解绑的数值
		//4008:账号-username,不存在或已被删除
		//7003:账号尚未开通，禁止解绑
	}

	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 修改卡密/账号备注示例
//修改卡密/账号备注示例
void UpdRemark(string cardnumorusername, string remarks, string _token)
{
	//构建入参
	Model::In_updRemarkArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnumorusername = cardnumorusername;//必填
	args.remarks = remarks;//必填
	args.token = _token;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GetUpdRemark(args);
	Msg = Msg + "----------修改卡密/账号备注操作----------\n";
	if (result.code == 0)
	{
		Msg = Msg + "修改成功\n";
	}
	else
	{
		Msg = Msg + "修改失败\n";
		//修改卡密/账号备注错误信息
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1021:token失效或不正确
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1023:签名-sign,不正确
		//1019:data业务参数不需要加密
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1017:data业务参数Json不正确
		//1020:token不能为空
		//1054:平台类型-platformtypeid,错误
		//1003:作者软件或商品销售后台到期
		//1068:已离线状态


		//6005:卡密被禁用
		//4005:账号被禁用
		//4003:账号到期
		//6003:卡密到期

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//6001:卡密-cardNum,不能为空
		//4007:账号-username,不能为空
		//6002:卡密-cardNum,不存在或已被删除
		//4008:账号-username,不存在或已被删除
		//1070:修改卡密或账号的备注长度不能超过500
	}

	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 充值卡充值示例
//充值卡充值示例
void RechCardOrAccount(string cardnumorusername, string rechcardnum)
{
	std::cout << "充值卡充值：""\n";
	//构建入参
	Model::In_RechCardRenewCardOrAccountArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.cardnumorusername = cardnumorusername;//必填
	args.rechcardnum = rechcardnum;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GetRechCardRenew(args);
	std::cout << "充值卡充值返回结果：""\n";
	std::cout << "编码：" + to_string(result.code) + "\n";
	std::cout << "信息：" + result.msg + "\n";
	std::cout << "data数据：" + result.data + "\n\n";

	//充值卡充值卡密或账号
	//1001:未知错误
	//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
	//1007:提交的参数Json不正确
	//1036:软件初始化的Key-inisoftkey,不能为空
	//1037:软件初始化的Key-inisoftkey,不存在

	//1002:系统出错
	//1015:data业务参数不能为空
	//1009:商品Code-goodscode,不存在
	//1032:提交的加密类型参数，与后台设置加密类型不一致
	//1018:data业务参数需要加密
	//1022:签名-sign,不能为空
	//1023:签名-sign,不正确
	//1030:没有此加密类型
	//1031:加密类型不对或加密不对
	//1016:data业务参数加密不正确
	//1019:data业务参数不需要加密
	//1017:data业务参数Json不正确
	//1053:此商品已被禁用
	//1054:平台类型-platformtypeid,错误

	//1066:请求标识不能为空
	//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
	//1071:被充值的卡密或账号：不能为空
	//1072:充值卡: 不能为空
	//1073:充值卡: 不存在
	//1074:此充值卡已被使用，不能再次充值
	//1075:当前软件下不存在此充值卡
	//1076:被充值的卡密或账号不正确
	//1078:充值卡与被充值卡密或账号消耗类型不一致
}
#pragma endregion

#pragma region 充值卡详情示例
//充值卡详情示例
void RechCardNumDetail(string rechcardnum)
{
	//构建入参
	Model::In_rechCardDetailArgs args;
	args.maccode = maccode;//必填
	args.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	args.requestflag = to_string(args.timestamp);//必填
	args.rechcardnum = rechcardnum;//必填
	string Msg = "";
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::GetRechCardDetail(args);
	Msg = Msg + "----------获取充值卡详情操作----------\n";
	if (result.code == 0)
	{
		//具体数据
		Model::Out_rechCardNumDetail RemoteVar = Help::NetworkVerHelp::ConvertRechCardDetail(result.data);
		if (RemoteVar.requestflag != args.requestflag)
		{
			Msg = Msg + "接口返回的数据已被“破解者”截持，您可以强制关闭软件或者不做任何处理\r\n";
			Msg = Msg + "--------------------\n\n\n";
			std::cout << Msg;
			return;
		}

		Msg = Msg + "获取软件变量成功\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "所属软件：" + RemoteVar.softname + "\n";
		Msg = Msg + "可使用值名称：" + RemoteVar.consumevaluename + "\n";
		Msg = Msg + "消耗类型名称：" + RemoteVar.consumetypename + "\n";
		Msg = Msg + "使用状态名称：" + RemoteVar.rechcardstatename + "\n";
	}
	else
	{
		Msg = Msg + "获取软件变量失败\n";
		Msg = Msg + "接口返回的数据：\n";
		Msg = Msg + "编码：" + to_string(result.code) + "\n";
		Msg = Msg + "信息：" + result.msg + "\n";
		Msg = Msg + "data数据：" + result.data + "\n";
		//充值卡详情
		//1001:未知错误
		//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
		//1007:提交的参数Json不正确
		//1036:软件初始化的Key-inisoftkey,不能为空
		//1037:软件初始化的Key-inisoftkey,不存在

		//1002:系统出错
		//1015:data业务参数不能为空
		//1009:商品Code-goodscode,不存在
		//1032:提交的加密类型参数，与后台设置加密类型不一致
		//1018:data业务参数需要加密
		//1022:签名-sign,不能为空
		//1023:签名-sign,不正确
		//1030:没有此加密类型
		//1031:加密类型不对或加密不对
		//1016:data业务参数加密不正确
		//1019:data业务参数不需要加密
		//1017:data业务参数Json不正确
		//1053:此商品已被禁用
		//1054:平台类型-platformtypeid,错误

		//1066:请求标识不能为空
		//1006:data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
		//1072:充值卡: 不能为空
		//1073:充值卡: 不存在
		//1075:当前软件下不存在此充值卡
	}
	Msg = Msg + "--------------------\n\n\n";
	std::cout << Msg;
}
#pragma endregion

#pragma region 禁用还是删除：单码或账号示例
//禁用还是删除：单码或账号示例
void DisableCardOrAccount(string cardnumorusername)
{
	std::cout << "禁用还是删除：单码或账号：""\n";
	//构建禁用还是删除：单码或账号入参
	Model::In_DisableCardOrAccountArgs DisableCardOrAccountArgs;
	DisableCardOrAccountArgs.maccode = maccode;//必填
	DisableCardOrAccountArgs.timestamp = Help::NetworkVerHelp::GetTimeStamp();//必填
	DisableCardOrAccountArgs.cardnumorusername = cardnumorusername;//必填
	DisableCardOrAccountArgs.token = token;//必填
	DisableCardOrAccountArgs.disablecardoraccounttype = Model::DisableCardOrAccountType::Disable;//必填
	//请求接口，获取结果
	Model::Result result = Help::NetworkVerHelp::DisableCardOrAccount(DisableCardOrAccountArgs);
	std::cout << "禁用还是删除：单码或账号返回结果：""\n";
	std::cout << "编码：" + to_string(result.code) + "\n";
	std::cout << "信息：" + result.msg + "\n";
	std::cout << "data数据：" + result.data + "\n\n";

	//禁用还是删除：单码或账号错误信息
	//1001:未知错误
	//1005:平台ID-platformtypeid,业务ID-businessID,加密类型-encrypttypeid,平台用户编码-platformusercode,商品编码-goodscode,时间戳-timestamp,不能为空
	//1007:提交的参数Json不正确
	//1036:软件初始化的Key-inisoftkey,不能为空
	//1037:软件初始化的Key-inisoftkey,不存在

	//1002:系统出错
	//1015:data业务参数不能为空
	//1018:data业务参数需要加密
	//1022:签名-sign,不能为空
	//1021:token失效或不正确
	//1032:提交的加密类型参数，与后台设置加密类型不一致
	//1023:签名-sign,不正确
	//1019:data业务参数不需要加密
	//1030:没有此加密类型
	//1031:加密类型不对或加密不对
	//1017:data业务参数Json不正确
	//1020:token不能为空
	//1054:平台类型-platformtypeid,错误


	//1006：data里面的参数：机器码-MacCode,时间戳-timestamp,不能为空
	//6001:卡密-cardNum,不能为空
	//4007:账号-username,不能为空
}
#pragma endregion

string GetExePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string fullPath(buffer);
	size_t pos = fullPath.find_last_of("\\/");
	return fullPath.substr(0, pos); // 去掉 exe 文件名，只保留目录
}

// 从文件读取卡密
string LoadLocalCard(string cardFile) {
	ifstream fin(cardFile);
	string card;
	if (fin.is_open()) {
		getline(fin, card);
		fin.close();
	}
	return card;
}

// 保存卡密到文件
void SaveLocalCard(const string& card, string cardFile) {
	ofstream fout(cardFile, ios::trunc); // 覆盖写入
	if (fout.is_open()) {
		fout << card;
		fout.close();
	}
}

#pragma comment(lib, "winhttp.lib")

std::string SendPostRequest(const std::wstring& server, int port, const std::wstring& path, const std::string& jsonData)
{
	std::string response;
	HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
	BOOL bResults = FALSE;    //  提前声明
	DWORD dwSize = 0;         //  提前声明
	LPCWSTR headers = L"Content-Type: application/json\r\n";

	// 1. 打开 Session
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		std::cerr << "WinHttpOpen 失败: " << GetLastError() << std::endl;
		return "";
	}

	// 2. 连接服务器
	hConnect = WinHttpConnect(hSession, server.c_str(), port, 0);
	if (!hConnect) {
		std::cerr << "WinHttpConnect 失败: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hSession);
		return "";
	}

	// 3. 创建请求
	hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		(port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0);
	if (!hRequest) {
		std::cerr << "WinHttpOpenRequest 失败: " << GetLastError() << std::endl;
		goto cleanup;
	}

	// 4. 设置请求头
	
	if (!WinHttpAddRequestHeaders(hRequest, headers, -1, WINHTTP_ADDREQ_FLAG_ADD)) {
		std::cerr << "WinHttpAddRequestHeaders 失败: " << GetLastError() << std::endl;
		goto cleanup;
	}

	// 5. 发送请求
	bResults = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		(LPVOID)jsonData.c_str(),
		jsonData.length(),
		jsonData.length(), 0);
	if (!bResults) {
		//std::cerr << "WinHttpSendRequest 失败: " << GetLastError() << std::endl;
		goto cleanup;
	}

	// 6. 接收响应
	bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (!bResults) {
		std::cerr << "WinHttpReceiveResponse 失败: " << GetLastError() << std::endl;
		goto cleanup;
	}

	// 7. 读取响应数据
	do {
		DWORD dwDownloaded = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			break;

		if (dwSize == 0)
			break;

		std::string buffer(dwSize, 0);
		if (WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded))
			response.append(buffer, 0, dwDownloaded);
		else
			break;

	} while (dwSize > 0);

cleanup:
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return response;
}

std::string httpGet(const std::wstring& url)
{
	std::wstring host, path;
	INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;

	// 拆分URL
	if (url.rfind(L"https://", 0) == 0) {
		size_t pos = url.find(L'/', 8);
		host = url.substr(8, pos - 8);
		path = url.substr(pos);
	}
	else {
		return "Invalid URL";
	}

	HINTERNET hSession = WinHttpOpen(L"MyApp/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return "Open failed";

	HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
	if (!hConnect) { WinHttpCloseHandle(hSession); return "Connect failed"; }

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE);

	std::string response;
	if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(hRequest, NULL)) {

		DWORD dwSize = 0;
		do {
			WinHttpQueryDataAvailable(hRequest, &dwSize);
			if (!dwSize) break;

			std::string buffer(dwSize, 0);
			DWORD dwDownloaded = 0;
			WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded);
			response.append(buffer.c_str(), dwDownloaded);
		} while (dwSize > 0);
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return Utf8ToGbk(response.c_str());
}

#include <random>
std::wstring makeRandomUrl(std::wstring base) {

	// 生成随机整数
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned long long> dis(100000, 999999999);

	unsigned long long rnd = dis(gen);

	// 拼接成 URL
	std::wstring url = base + L"?random=" + std::to_wstring(rnd);
	return url;
}

// 比较版本号（返回 -1 小于, 0 等于, 1 大于）
// 版本号比较函数
int compareVersion(const string& v1, const string& v2) {
	istringstream s1(v1), s2(v2);
	string token1, token2;
	while (true) {
		bool b1 = static_cast<bool>(getline(s1, token1, '.'));
		bool b2 = static_cast<bool>(getline(s2, token2, '.'));
		int num1 = b1 ? stoi(token1) : 0;
		int num2 = b2 ? stoi(token2) : 0;

		if (num1 < num2) return -1;
		if (num1 > num2) return 1;
		if (!b1 && !b2) break;
	}
	return 0;
}

// 用于设置控制台文本颜色
void SetColor(WORD color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int main()
{
	// 设置控制台代码页为 UTF - 8
	//SetConsoleOutputCP(CP_UTF8);

	VMProtectBegin("main");
	//CloseConsoleQuickEditMode();
	//CloseConsoleInsertMode();
	//CloseConsoleMouseInput();
    system("color 0A");
	bool 验证开关 = true;
	int port = 8090;
	// JSON 数据
	/*std::string json = R"({
        "maccode": "ABC123XYZ",
        "firstTime": "",
        "lastTime": "",
        "versionName": "1.0.5",
        "startTemp": 0
    })";*/

	std::string json = R"({
    "maccode": ")" + maccode + R"(",
    "firstTime": "",
    "lastTime": "",
    "versionName": ")" + giteeVersionname + R"(",
    "startTemp": 1
})";

	std::string resp = SendPostRequest(myServer, port, MyYjwjPath, json);

	// 公告
	std::wstring url = makeRandomUrl(L"https://gitee.com/likesnake/yjwjDma/raw/master/ts_pro_all");
	std::string jsonGitee = httpGet(url);
	std::cout << "\n" << jsonGitee << "\n" << std::endl;

	// 检测新版本
	std::wstring urlVersion = makeRandomUrl(L"https://gitee.com/likesnake/yjwjDma/raw/master/ts_pro_version");
	std::string jsonVersion = httpGet(urlVersion);

	try {

		// 解析 JSON
		cJSON* root = cJSON_Parse(jsonVersion.c_str());
		if (!root) {
			cout << "解析 JSON 失败！" << endl;
			return -1;
		}

		cJSON* versions = cJSON_GetObjectItem(root, "updates");
		if (!versions ) {
			cout << "JSON 格式错误：未找到 updates 数组" << endl;
			cJSON_Delete(root);
			return -1;
		}

		string latestVersion;
		string latestContent;
		string isForcedUpdating;

		int size = cJSON_GetArraySize(versions);
		for (int i = 0; i < size; ++i) {
			cJSON* item = cJSON_GetArrayItem(versions, i);
			if (!item) continue;

			cJSON* version = cJSON_GetObjectItem(item, "version");
			cJSON* content = cJSON_GetObjectItem(item, "content");
			cJSON* jsUpdating = cJSON_GetObjectItem(item, "isForcedUpdating");

			if (version) {
				string v = version->valuestring;
				string c = content->valuestring;

				if (latestVersion.empty() || compareVersion(v, latestVersion) > 0) {
					latestVersion = v;
					latestContent = c;
					isForcedUpdating = jsUpdating->valuestring;
				}
			}
		}

		if (compareVersion(giteeVersionname, latestVersion) < 0) {
			SetColor(9);  // 设置字体
			cout << "发现新版本！" << endl;
			SetColor(9);  // 设置字体
			cout << "\n" << "当前版本: " << giteeVersionname << "  最新版本: " << latestVersion << endl;
			cout << "\n" << "更新内容:" << endl;

			SetColor(11);  // 设置黄色字体，突出显示更新内容
			cout << latestContent << endl;

			SetColor(10);  // 恢复

			if (isForcedUpdating == "1") {
				// 强制更新
				while (true)
				{
					Sleep(1000);
				}
				return 0;
			}
		}
		else {
			cout << "当前已是最新版本:" << giteeVersionname << endl;
		}
		cout << "" << endl;
		// 释放 JSON 内存
		cJSON_Delete(root);

	}
	catch (const exception& e) {
		cerr << "❌ 解析 JSON 出错: " << e.what() << endl;
	}




	if (!resp.empty()) {
		if (resp != "success") {
			验证开关 = false;
		}
		//std::cout << "服务器返回: " << resp << std::endl;
	}
	else {
		//std::cout << "请求失败或无返回内容。" << std::endl;
	}
	验证开关 = false;
	
	if (!验证开关)
	{
		if (GetModuleStatus()) {
			Thread();
		}
		return  getchar();
	}
	else
	{
		if (IniSoftInfo())
		{
			////订单查询
			//string orderid = "a01d6322ffad357b1de";
			//SearchOrder(orderid);

			//操作的业务类型ID
			Model::BusinessType businessType = Model::BusinessType::cardLogin;
			switch (businessType)
			{
			case Model::BusinessType::cardLogin://卡密登录示例
			{
				string inputCardnum;

				string exeDir = GetExePath();
				string cardFile = exeDir + "\\cardnum.txt";

				// 尝试读取本地卡密
				inputCardnum = LoadLocalCard(cardFile);

				bool loginOk = false;

				int failCount = 0;           // 错误计数
				const int maxFail = 3;       // 最多允许错误 3 次

				// 如果本地没有卡密或验证失败，就进入循环输入
				while (!loginOk && failCount < maxFail) {
					cout << "请输入或粘贴卡密: ";

					if (!inputCardnum.empty()) {
						// 把本地卡密直接写到控制台
						cout << inputCardnum;
					}

					string userInput;
					getline(cin, userInput);

					// 如果用户直接回车，说明接受默认卡密
					if (userInput.empty()) {
						userInput = inputCardnum;
					}

					if (LoginByCard(userInput, false)) {
						loginOk = true;
						SaveLocalCard(userInput, cardFile); // 验证成功后再保存
						cout << "卡密验证成功！" << endl;

						if (GetModuleStatus()) {
							Thread();
						}
					}
					else {
						failCount++;
						if (failCount >= maxFail) {
							cout << "连续错误 " << maxFail << " 次，程序已退出。" << endl;
							return 0; // 直接关闭程序
						}
						cout << "卡密验证失败，请重新输入。(再输错 "
							<< (maxFail - failCount) << " 次后自动退出)" << endl;
						inputCardnum.clear();
					}
				}

				/*string inputCardnum;
				cout << "请输入或粘贴卡密: ";
				getline(cin, inputCardnum); // 推荐用 getline，可以避免空格或换行问题
				if (LoginByCard(inputCardnum, false)) {
					//执行代码
					if (GetModuleStatus())
					{

						Thread();

					}
				}*/
				break;
			}
			case Model::BusinessType::cardDetail://卡密详情示例
				CardDetail(cardnum);
				break;
			case Model::BusinessType::accountRegister://账号注册示例
				AccountRegister(userName, userpwd);
				break;
			case Model::BusinessType::accountLogin://账号登录示例
				LoginAccount(userName, userpwd, true);
				break;
			case Model::BusinessType::updPwd://修改用户密码示例
				updPwd(userName, userpwd, newUserPwd);
				break;
			case Model::BusinessType::accountDetail://账号详情示例
				AccountDetail(userName, userpwd);
				break;
			case Model::BusinessType::bucklePoint://扣点示例
				//先登录
				if (LoginByCard(cardnum, false))
				{
					//然后再扣点,注意，如果指定的扣点数为0，那么就会默认扣除后台设置的数值
					BucklePoint(cardnum, 1, token);
				}
				break;
			case Model::BusinessType::getremoteVar://获取软件变量示例
				//先登录
				if (LoginByCard(cardnum, false))
				{
					GetremoteVar(cardnum, token, TestVer);
				}
				////先登录
				//if (LoginAccount(userName, userpwd, false))
				//{
				//	GetremoteVar(userName, token);
				//}
				break;
			case Model::BusinessType::remoteCalculate://获取远程算法示例
				//先登录
				if (LoginByCard(cardnum, false))
				{
					Getcalculate("e4a2132e56d79e5b", token, "args1=测试值1&args2=测试值2");
				}
				////先登录
				//if (LoginAccount(userName, userpwd, false))
				//{
				//	Getcalculate("e4a2132e56d79e5b", token, "args1=测试值1&args2=测试值2");
				//}
				break;
			case Model::BusinessType::unbundMac://解绑机器码示例
				//先登录
				if (LoginByCard(cardnum, false))
				{
					//然后解除绑定机器码(卡密示例)
					BucklePoint(cardnum, 1, token);
				}
				////先登录
				//if (LoginAccount(userName, userpwd, false))
				//{
				//	//然后解除绑定机器码(账号示例)
				//	BucklePoint(cardnum, 1, token);
				//}
				break;
			case Model::BusinessType::updRemark://修改卡密/账号备注示例
				//先登录
				if (LoginByCard(cardnum, false))
				{
					UpdRemark(cardnum, "此卡密或账号修改的备注值。可以通过“卡密登录接口”，“卡密详情接口”，“账号登录接口”，“账号详情接口”这四个接口来进行获取卡密/账号的备注", token);
				}
				////先登录
				//if (LoginAccount(userName, userpwd, false))
				//{
				//	UpdRemark(cardnum, "此卡密或账号修改的备注值。可以通过“卡密登录接口”，“卡密详情接口”，“账号登录接口”，“账号详情接口”这四个接口来进行获取卡密/账号的备注", token);
				//}
				break;
			case Model::BusinessType::rechCardRenewCardOrAccount://充值卡充值
				RechCardOrAccount("6cb8ce8c93e5803b$", cardnum);//注意：写死的这个充值卡已使用完了，请自行从后台生成一个新的充值卡做测试
				break;
			case Model::BusinessType::rechCardNumDetail://充值卡详情
				RechCardNumDetail("6cb8ce8c93e5803b$");
				break;
			case Model::BusinessType::disableCardOrAccount://禁用还是删除：卡密或账号
				//此功能描述：
				//1、 此功能主要是为了防破解使用，比如：当你软件在运行的过程中，如果捕获到有破解迹象特征【防破解的代码是需要你自己写的】，那么就可以调用此子程序，把当前登录的卡密或者账号给禁用掉或者删除掉
				//2、 此功能只能登录成功后才能调用此子程序
				//3、 此功能调用完后，会立即强制关闭软件的！
				//先登录
				if (LoginByCard(cardnum, false))
				{
					DisableCardOrAccount(cardnum);
				}
				////先登录
				//if (LoginAccount(userName, userpwd, false))
				//{
				//	DisableCardOrAccount(cardnum);
				//}
				break;
			}

			//DisableCardOrAccount(string cardnumorusername)
		}

		return  getchar();
	}
	

	VMProtectEnd();
}
