#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include <iostream>

#include "GetServerStatue.h"
#include "package.h"
#include "mcdt.h"
#include "Locks.h"


std::string str(const int& num) {
	int len = 0;
	//如果是0特殊处理
	if (num == 0) len = 1;
	else {
		int t = abs(num);
		while (t != 0) {
			t /= 10;
			len++;
		}
	}
	if (num < 0) len++; //考虑负数

	//申请内存
	char res[10] = { 0 };
	res[len] = '\0';
	if (num < 0) res[0] = '-';
	//载入结果数组
	if (num == 0) res[0] = '0';
	else {
		int t = abs(num);
		while (t != 0) {
			res[len - 1] = t % 10 + '0';
			t /= 10;
			len--;
		}
	}
	return std::string(res);
}

std::string getTime() {
	time_t timep;
	tm* p;
	std::string timestr;
	time(&timep);
	p = localtime(&timep);
	timestr += str(1900 + p->tm_year);
	timestr += "/";
	timestr += str(1 + p->tm_mon);
	timestr += "/";
	timestr += str(p->tm_mday);
	timestr += " ";
	timestr += str(p->tm_hour);
	timestr += ":";
	timestr += str(p->tm_min);
	timestr += ":";
	timestr += str(p->tm_sec);

	return timestr;
}

bool strPicker(std::string aim, std::string head, std::string tail, std::string& res) {
	// 夹在head和tail中间
	bool flag = false;
	bool first_o = true;  // 优化输出
	int p = 0;
	while (1) {
		// 得到起始下标
		int beg = aim.find(head, p);
		if (beg == aim.npos) {
			break;  // 找不到头直接退出
		}
		beg += head.length();
		// 找到尾部的下一个下标
		int end = aim.find(tail, beg);
		if (end == aim.npos) {
			break;  // 找不到尾直接退出
		}
		if (!first_o) {
			res += ", "; // 如果前面有东西了，就加个分隔符
		}
		else {
			first_o = false;
		}
		res += aim.substr(beg, end - beg);  // 找到的结果叠加
		flag = true;
		p = end;
	}
	return flag;
}

bool getVersionField(std::string str, std::string& res) {
	std::string head = "\"version\":{\"name\":\"";
	std::string tail = "\",";
	return strPicker(str, head, tail, res);
}

bool getPlayerStateField(std::string str, std::string& max, std::string& online) {
	std::string max_head = "\"max\":";
	std::string max_tail = ",\"online\":";
	bool max_success = strPicker(str, max_head, max_tail, max);
	// online的尾巴有两种情况，一种是花括号 还有一种是逗号
	std::string online_head = "\"online\":";
	std::string online_tail_1 = ",";
	std::string online_tail_2 = "}";
	std::string online_res_1, online_res_2;
	bool online_res1_success = strPicker(str, online_head, online_tail_1, online_res_1);
	bool online_res2_success = strPicker(str, online_head, online_tail_2, online_res_2);
	if (online_res_1.length() > online_res_2.length()) {	// 选最短的那个就行（临时解决）
		online = online_res_2;
	}
	else {
		online = online_res_1;
	}

	return max_success && (online_res1_success || online_res2_success);
}

bool getPlayersField(std::string str, std::string& res) {
	std::string t;
	std::string head = "\"players\":";
	std::string tail = "\"version\":";
	strPicker(str, head, tail, t);
	head = "\"name\":\"";
	tail = "\"}";
	strPicker(t, head, tail, res);
	return (res.length() != 0);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GetServerStatue::GetServerStatue(const char* in_s_add, unsigned short in_server_port) {
		// 初始化
		server_port = in_server_port;
		s_add = new char[strlen(in_s_add) + 1];
		strcpy(s_add, in_s_add);
		// 设置超时时间
		S.setTimeout(20000, 2);  // 收发10s超时（是否是mc服务器），连接2s超时（端口是否开放）
	}
GetServerStatue::~GetServerStatue() {
		delete s_add;
	}

bool GetServerStatue::run() {
	// 进行连接
	if (!S.doConnect(s_add, server_port)) {
		// std::wcout << s_add << ":" << server_port << " Closed Prot\n";
		return false;	// 如果socket连接建立失败
	}
	// 三部曲
	handShake();  // 强制发送握手和请求包
	sendResquest();
	Sleep(15 * 1000);  // 等待服务器将全部数据发送完毕后再读取缓冲区，这是保证扫描到所有服务器的关键
	return recvResponse();  // 看服务器是否给出我期望的响应
}

// 模拟1.18.2客户端进行握手
void GetServerStatue::handShake() {
	Package pack;
	// 版本号
	McVarInt version(340);  // 758
	pack.AppendField(version);
	// 服务器地址
	McString server_address((char*)s_add, strlen(s_add));
	pack.AppendField(server_address);
	// 服务器端口
	McUnsignedShort s_port(server_port);
	pack.AppendField(s_port);
	// 1 for status, 2 for login
	McVarInt next_state(1);
	pack.AppendField(next_state);
	// 发送包
	SOCKET socket = S.getSocket();
	pack.SendPack(0, socket);
	// 输出包内容
	//std::wcout << "handshake pack -> ";
	//pack.showInHex();
}

// 发送请求数据包
void GetServerStatue::sendResquest() {
	// data为空
	// 直接发包
	Package pack;
	SOCKET socket = S.getSocket();
	pack.SendPack(0, socket);
	// 输出包内容
	//std::wcout << "request pack -> ";
	//pack.showInHex();
}

// 接收服务器发送的包
bool GetServerStatue::recvResponse() {
	// 能执行到这里说明是开放的端口，在ifdebugmode模式下一定会输出东西
	Package pack;
	SOCKET socket = S.getSocket();
	// 收取包，如果收取失败就直接退出
	if (!pack.RecvPack(socket)) {
		// std::std::wcout << "getted_VarInt = " << pack.getSize() << " | ";
		if (ifDebugMode) {
			m.lock();
			std::wcout << "<debug> " << s_add << ":" << server_port << " Send an unknown pack or didn't send a pack back! (recvPackError)\n";
			m.unlock();
		}
		return false;  // 根本不是mc的包或者服务器根本没回复
	}

	// 按字符串解读包内容
	McString response(pack);
	if (ifDebugMode) {
		m.lock();
		std::wcout << "<debug> JSON from " << s_add << ":" << server_port << std::endl;
		std::wcout << "========================================================================" << std::endl;
		response.show(); // 输出接收的json串
		std::wcout << "\n========================================================================" << std::endl;
		m.unlock();
		return false;  // debug模式下认为所有世界返回的包都是无效包
	}

	// 显示输出
	std::string version, max, online, players;
	if (getVersionField(response.getVal(), version) && getPlayerStateField(response.getVal(), max, online)) {
		// 显示扫描结果
		bool have_player = getPlayersField(response.getVal(), players);
		if (have_player || ifShowNoPlayerWorld) { // 移到上面去了，有玩家才输出
			if (
				ifDebugMode ||  // 如果是调试模式则无条件输出
				version.find("1.12") == version.npos &&
				version.find("1.7") == version.npos &&
				version.find(":") == version.npos &&
				version.find("\\") == version.npos &&
				version.find("?") == version.npos &&
				version.find("/") == version.npos &&
				players.find(":") == players.npos &&
				players.find("\\") == players.npos &&
				players.find(".") == players.npos &&
				players.find("/") == players.npos &&
				players.find("?") == players.npos) {  // 1.12.2有mod，不显示 特殊字符：/?不显示
				// printf("%s:%d\t playerSta: [%s|%s]\t version: %s\t\t players: %s\n", s_add, server_port, online.c_str(), max.c_str(), version.c_str(), players.c_str());
				m.lock();
				std::cout << std::setw(10) << std::setfill(' ') << getTime() << "\t " << s_add << ":" << server_port << "\t "
					<< std::setw(15) << std::setfill(' ') << "[" + max + "|" + online << "] "
					<< std::setw(20) << std::setfill(' ') << version
					<< "\t players: " << players << "\n";
				m.unlock();
			}
		}
		return true;  // 说明是个活跃的世界
	}
	else {
		// 不是mc服务器：没有版本信息或人数信息
		if (ifDebugMode) {
			std::wcout << "getted_VarInt = " << pack.GetSize() << " | ";
			std::wcout << s_add << ":" << server_port << " Not Minecraft Server! (noVersionInfo)\n";
		}
		return false;
	}
}

bool GetServerStatue::ifDebugMode = false;
bool GetServerStatue::ifShowNoPlayerWorld = true;