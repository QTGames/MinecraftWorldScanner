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

// ��ʱʱ������
void MySocket::setTimeout(int in_SRTimeout, int in_connectTimeout) {
	SRTimeout = in_SRTimeout;
	connectTimeout = in_connectTimeout;
}

// ��ȡ�׽���
SOCKET MySocket::getSocket() {
	return sock;
}

// ��������
bool MySocket::doConnect(const char* ip, unsigned short port) {
	// ����Ĭ�ϳ�ʱʱ��
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&SRTimeout, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&SRTimeout, sizeof(int));

	// �������ӵ�ַ
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_addr.s_addr = inet_addr(ip);
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_port = htons(port);

	//���÷�������ʽ����
	unsigned long ul = 1;
	int ret = ioctlsocket(sock, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR) {
		//std::wcout << "ioctlsocket error..." << std::endl;
		return false;
	}

	connect(sock, (const struct sockaddr*)&sockAddr, sizeof(sockAddr)); //��������

	//select ģ�ͣ������ó�ʱ
	struct timeval timeout;
	fd_set r;

	FD_ZERO(&r);
	FD_SET(sock, &r);
	timeout.tv_sec = connectTimeout; //���ӳ�ʱʱ��
	timeout.tv_usec = 0;
	ret = select(0, 0, &r, 0, &timeout);
	if (ret <= 0)
	{
		::closesocket(sock);
		//std::wcout << "Connect TimeOut..." << std::endl;
		return false;
	}

	//һ�������ģʽ�׽ӱȽ��ѿ��ƣ����Ը���ʵ��������� ���������ģʽ
	unsigned long ul1 = 0;
	ret = ioctlsocket(sock, FIONBIO, (unsigned long*)&ul1);
	if (ret == SOCKET_ERROR) {
		//std::wcout << "ioctlsocket error..." << std::endl;
		return false;
	}

	return true;
}