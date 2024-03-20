#pragma once
#include "MySocket.h"

class GetServerStatue {
private:
	char* s_add;
	unsigned short server_port;
	MySocket S;
public:
	static bool ifDebugMode;  // 是否显示原始json信息无论世界是否有玩家
	static bool ifShowNoPlayerWorld; // 是否显示没有玩家的世界
	GetServerStatue(const char* in_s_add, unsigned short in_server_port);
	~GetServerStatue();

	bool run();

	// 模拟1.18.2客户端进行握手
	void handShake();

	// 发送请求数据包
	void sendResquest();

	// 接收服务器发送的包
	bool recvResponse();
};