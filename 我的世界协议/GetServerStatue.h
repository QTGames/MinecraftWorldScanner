#pragma once
#include "MySocket.h"

class GetServerStatue {
private:
	char* s_add;
	unsigned short server_port;
	MySocket S;
public:
	static bool ifDebugMode;  // �Ƿ���ʾԭʼjson��Ϣ���������Ƿ������
	static bool ifShowNoPlayerWorld; // �Ƿ���ʾû����ҵ�����
	GetServerStatue(const char* in_s_add, unsigned short in_server_port);
	~GetServerStatue();

	bool run();

	// ģ��1.18.2�ͻ��˽�������
	void handShake();

	// �����������ݰ�
	void sendResquest();

	// ���շ��������͵İ�
	bool recvResponse();
};