#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class MySocket {
private:
	SOCKET sock;
	WSADATA wsaData;
	sockaddr_in sockAddr;
	int SRTimeout = 5000;  // �շ���ʱʱ��
	int connectTimeout = 10;  // connect��ʱʱ��

public:
	MySocket();
	~MySocket();
	// ��ʱʱ������
	void setTimeout(int in_SRTimeout, int in_connectTimeout);
	// ��ȡ�׽���
	SOCKET getSocket();
	// ��������
	bool doConnect(const char* ip, unsigned short port);

};