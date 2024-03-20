#pragma once
#include "MySocket.h"
#include <string>
class Package;

class Tools {
private:
	static char dec2Hex(unsigned char data);
public:
	// 字节数据流以16进制的形式显示出来
	static void dataStream2Hex(unsigned char data[], int len);
};


class McField {
protected:
	unsigned char* m_networkData;
public:
	// 获取网络中的信息格式
	virtual void GetNetworkFormat(unsigned char*& networkData, int& net_data_len) = 0;
};


class McVarInt : public McField {
private:
	int value;
public:
	// 数据流中读取
	McVarInt(Package& pack);
	McVarInt(SOCKET sock);
	~McVarInt();

	// 直接用整数初始化
	McVarInt(int in_val);

	// 获取网络中的信息格式
	virtual void GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len);

	// 获取值
	int getVal();
};


class McString : public McField {
private:
	char* data;  // 字符串本身
	int len;  // 字符长度（字节）

public:
	// 数据流中读取
	McString(Package& pack);

	// 通过字符串创建
	McString(char* in_data, int in_len);
	
	~McString();

	// 获取网络中的信息格式
	virtual void GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len);

	// 输出
	void show();
	
	// 获取值
	std::string getVal();
};


class McUnsignedShort : public McField {
private:
	unsigned short data;
public:
	// 数据流中读取
	McUnsignedShort(Package& pack);

	~McUnsignedShort();

	// 通过数值创建
	McUnsignedShort(unsigned short in_data);

	// 获取网络中的信息格式
	virtual void GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len);

	// 输出
	unsigned short get();
};