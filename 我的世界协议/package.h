#pragma once
#include "MySocket.h"
class McField;


class Package {
private:
	int id;  // 包id
	unsigned char* data;  // 当前构造状态的包数据
	int size;  // 当前构造状态的包大小或者接收到的包大小（除去包大小字段后）
	int p;  // 读写指针

	// 申请更大的内存
	void AppendNewMem(int s);
	void FrontNewMem(int s);
public:
	Package();

	void AppendField(McField& field);
	void FrontField(McField& field);
	void ReadField(unsigned char* des, int size);
	void SendPack(int id, SOCKET& S);
	// 尝试接收mc的数据包，如果不是mc的数据包就返回false
	bool RecvPack(SOCKET& S);
	// 获取当前包的大小
	int GetSize();
	void ShowInHex();
};