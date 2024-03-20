#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class MySocket {
private:
	SOCKET sock;
	WSADATA wsaData;
	sockaddr_in sockAddr;
	int SRTimeout = 5000;  // 收发超时时间
	int connectTimeout = 10;  // connect超时时间

public:
	MySocket();
	~MySocket();
	// 超时时间设置
	void setTimeout(int in_SRTimeout, int in_connectTimeout);
	// 获取套接字
	SOCKET getSocket();
	// 进行连接
	bool doConnect(const char* ip, unsigned short port);

};