#pragma once
#include "MySocket.h"

MySocket::MySocket() {
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

MySocket::~MySocket() {
	closesocket(sock);
	WSACleanup();
}

// 超时时间设置
void MySocket::setTimeout(int in_SRTimeout, int in_connectTimeout) {
	SRTimeout = in_SRTimeout;
	connectTimeout = in_connectTimeout;
}

// 获取套接字
SOCKET MySocket::getSocket() {
	return sock;
}

// 进行连接
bool MySocket::doConnect(const char* ip, unsigned short port) {
	// 设置默认超时时间
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&SRTimeout, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&SRTimeout, sizeof(int));

	// 设置连接地址
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_addr.s_addr = inet_addr(ip);
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_port = htons(port);

	//设置非阻塞方式连接
	unsigned long ul = 1;
	int ret = ioctlsocket(sock, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR) {
		//std::wcout << "ioctlsocket error..." << std::endl;
		return false;
	}

	connect(sock, (const struct sockaddr*)&sockAddr, sizeof(sockAddr)); //立即返回

	//select 模型，即设置超时
	struct timeval timeout;
	fd_set r;

	FD_ZERO(&r);
	FD_SET(sock, &r);
	timeout.tv_sec = connectTimeout; //连接超时时间
	timeout.tv_usec = 0;
	ret = select(0, 0, &r, 0, &timeout);
	if (ret <= 0)
	{
		::closesocket(sock);
		//std::wcout << "Connect TimeOut..." << std::endl;
		return false;
	}

	//一般非阻塞模式套接比较难控制，可以根据实际情况考虑 再设回阻塞模式
	unsigned long ul1 = 0;
	ret = ioctlsocket(sock, FIONBIO, (unsigned long*)&ul1);
	if (ret == SOCKET_ERROR) {
		//std::wcout << "ioctlsocket error..." << std::endl;
		return false;
	}

	return true;
}