#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include <iostream>

#include "GetServerStatue.h"
#include "package.h"
#include "mcdt.h"
#include "Locks.h"


std::string str(const int& num) {
	int len = 0;
	//�����0���⴦��
	if (num == 0) len = 1;
	else {
		int t = abs(num);
		while (t != 0) {
			t /= 10;
			len++;
		}
	}
	if (num < 0) len++; //���Ǹ���

	//�����ڴ�
	char res[10] = { 0 };
	res[len] = '\0';
	if (num < 0) res[0] = '-';
	//����������
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
	// ����head��tail�м�
	bool flag = false;
	bool first_o = true;  // �Ż����
	int p = 0;
	while (1) {
		// �õ���ʼ�±�
		int beg = aim.find(head, p);
		if (beg == aim.npos) {
			break;  // �Ҳ���ͷֱ���˳�
		}
		beg += head.length();
		// �ҵ�β������һ���±�
		int end = aim.find(tail, beg);
		if (end == aim.npos) {
			break;  // �Ҳ���βֱ���˳�
		}
		if (!first_o) {
			res += ", "; // ���ǰ���ж����ˣ��ͼӸ��ָ���
		}
		else {
			first_o = false;
		}
		res += aim.substr(beg, end - beg);  // �ҵ��Ľ������
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
	// online��β�������������һ���ǻ����� ����һ���Ƕ���
	std::string online_head = "\"online\":";
	std::string online_tail_1 = ",";
	std::string online_tail_2 = "}";
	std::string online_res_1, online_res_2;
	bool online_res1_success = strPicker(str, online_head, online_tail_1, online_res_1);
	bool online_res2_success = strPicker(str, online_head, online_tail_2, online_res_2);
	if (online_res_1.length() > online_res_2.length()) {	// ѡ��̵��Ǹ����У���ʱ�����
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
		// ��ʼ��
		server_port = in_server_port;
		s_add = new char[strlen(in_s_add) + 1];
		strcpy(s_add, in_s_add);
		// ���ó�ʱʱ��
		S.setTimeout(20000, 2);  // �շ�10s��ʱ���Ƿ���mc��������������2s��ʱ���˿��Ƿ񿪷ţ�
	}
GetServerStatue::~GetServerStatue() {
		delete s_add;
	}

bool GetServerStatue::run() {
	// ��������
	if (!S.doConnect(s_add, server_port)) {
		// std::wcout << s_add << ":" << server_port << " Closed Prot\n";
		return false;	// ���socket���ӽ���ʧ��
	}
	// ������
	handShake();  // ǿ�Ʒ������ֺ������
	sendResquest();
	Sleep(15 * 1000);  // �ȴ���������ȫ�����ݷ�����Ϻ��ٶ�ȡ�����������Ǳ�֤ɨ�赽���з������Ĺؼ�
	return recvResponse();  // ���������Ƿ��������������Ӧ
}

// ģ��1.18.2�ͻ��˽�������
void GetServerStatue::handShake() {
	Package pack;
	// �汾��
	McVarInt version(340);  // 758
	pack.AppendField(version);
	// ��������ַ
	McString server_address((char*)s_add, strlen(s_add));
	pack.AppendField(server_address);
	// �������˿�
	McUnsignedShort s_port(server_port);
	pack.AppendField(s_port);
	// 1 for status, 2 for login
	McVarInt next_state(1);
	pack.AppendField(next_state);
	// ���Ͱ�
	SOCKET socket = S.getSocket();
	pack.SendPack(0, socket);
	// ���������
	//std::wcout << "handshake pack -> ";
	//pack.showInHex();
}

// �����������ݰ�
void GetServerStatue::sendResquest() {
	// dataΪ��
	// ֱ�ӷ���
	Package pack;
	SOCKET socket = S.getSocket();
	pack.SendPack(0, socket);
	// ���������
	//std::wcout << "request pack -> ";
	//pack.showInHex();
}

// ���շ��������͵İ�
bool GetServerStatue::recvResponse() {
	// ��ִ�е�����˵���ǿ��ŵĶ˿ڣ���ifdebugmodeģʽ��һ�����������
	Package pack;
	SOCKET socket = S.getSocket();
	// ��ȡ���������ȡʧ�ܾ�ֱ���˳�
	if (!pack.RecvPack(socket)) {
		// std::std::wcout << "getted_VarInt = " << pack.getSize() << " | ";
		if (ifDebugMode) {
			m.lock();
			std::wcout << "<debug> " << s_add << ":" << server_port << " Send an unknown pack or didn't send a pack back! (recvPackError)\n";
			m.unlock();
		}
		return false;  // ��������mc�İ����߷���������û�ظ�
	}

	// ���ַ������������
	McString response(pack);
	if (ifDebugMode) {
		m.lock();
		std::wcout << "<debug> JSON from " << s_add << ":" << server_port << std::endl;
		std::wcout << "========================================================================" << std::endl;
		response.show(); // ������յ�json��
		std::wcout << "\n========================================================================" << std::endl;
		m.unlock();
		return false;  // debugģʽ����Ϊ�������緵�صİ�������Ч��
	}

	// ��ʾ���
	std::string version, max, online, players;
	if (getVersionField(response.getVal(), version) && getPlayerStateField(response.getVal(), max, online)) {
		// ��ʾɨ����
		bool have_player = getPlayersField(response.getVal(), players);
		if (have_player || ifShowNoPlayerWorld) { // �Ƶ�����ȥ�ˣ�����Ҳ����
			if (
				ifDebugMode ||  // ����ǵ���ģʽ�����������
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
				players.find("?") == players.npos) {  // 1.12.2��mod������ʾ �����ַ���/?����ʾ
				// printf("%s:%d\t playerSta: [%s|%s]\t version: %s\t\t players: %s\n", s_add, server_port, online.c_str(), max.c_str(), version.c_str(), players.c_str());
				m.lock();
				std::cout << std::setw(10) << std::setfill(' ') << getTime() << "\t " << s_add << ":" << server_port << "\t "
					<< std::setw(15) << std::setfill(' ') << "[" + max + "|" + online << "] "
					<< std::setw(20) << std::setfill(' ') << version
					<< "\t players: " << players << "\n";
				m.unlock();
			}
		}
		return true;  // ˵���Ǹ���Ծ������
	}
	else {
		// ����mc��������û�а汾��Ϣ��������Ϣ
		if (ifDebugMode) {
			std::wcout << "getted_VarInt = " << pack.GetSize() << " | ";
			std::wcout << s_add << ":" << server_port << " Not Minecraft Server! (noVersionInfo)\n";
		}
		return false;
	}
}

bool GetServerStatue::ifDebugMode = false;
bool GetServerStatue::ifShowNoPlayerWorld = true;