#include "mcdt.h"
#include "package.h"
#include <iostream>



char Tools::dec2Hex(unsigned char data) {
	if (data >= 0 && data <= 9) {
		return data + '0';
	}
	else if (data >= 10 && data <= 15) {
		return 'a' + data - 10;
	}
}


void Tools::dataStream2Hex(unsigned char data[], int size) {
	for (int i = 0; i < size; i++) {
		std::cout << dec2Hex(data[i] / 0x10);
		std::cout << dec2Hex(data[i] % 0x10) << " ";
	}
	std::cout << std::endl;
}



McVarInt::McVarInt(Package& pack) {
	value = 0;  // 往value里面写
	int position = 0;  // 当前在value的位置
	while (true) {
		unsigned char currentByte;
		pack.ReadField(&currentByte, sizeof(unsigned char));
		// 数据部分
		value |= (currentByte & 0x7f) << position;
		// 标志位部分
		if ((currentByte & 0x80) == 0)
			break;
		// 正在写的位置
		position += 7;
		// 如果位置大于int型
		if (position >= (sizeof(int)) * 8) {
			break;
		}
	}
}

McVarInt::McVarInt(SOCKET sock) {
	value = 0;  // 往value里面写
	int position = 0;  // 当前在value的位置
	while (true) {
		unsigned char currentByte;
		int rec = recv(sock, (char*)&currentByte, sizeof(unsigned char), 0);
		//pack.readField(&currentByte, sizeof(unsigned char));
		// 数据部分
		value |= (currentByte & 0x7f) << position;
		// 标志位部分
		if ((currentByte & 0x80) == 0)
			break;
		// 正在写的位置
		position += 7;
		// 如果位置大于int型
		if (position >= sizeof(int) * 8) {
			break;
		}
	}
}

McVarInt::McVarInt(int in_val) {
	value = in_val;
}

void McVarInt::GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len) {
	int t_val = value;  // 用于生成intval格式的中间变量
	unsigned char data[5];  // 不超过5字节
	int count = 0;

	// 生成intval
	while (true) {
		if ((t_val & ~0x7f) == 0) {
			data[count++] = t_val;
			break;
		}
		data[count++] = (t_val & 0x7f) | 0x80;

		// 连符号也跟着右移
		t_val = (unsigned int)t_val >> 7;
	}

	// 获取网络格式的数据
	net_data_len = sizeof(unsigned char) * count;
	m_networkData = new unsigned char[net_data_len];
	memcpy(m_networkData, data, net_data_len);
	this->m_networkData = m_networkData;
}

int McVarInt::getVal() {
	return value;
}

McVarInt::~McVarInt() {
	delete m_networkData;
}


McString::McString(Package& pack) {
	// 先读取前缀varint来获取长度
	McVarInt t_len(pack);
	len = t_len.getVal();
	// 申请内存
	data = new char[len];
	// 再保存字符串内容
	int size = len * sizeof(char);
	pack.ReadField((unsigned char*)data, size);
}

McString::McString(char* in_data, int in_len) {
	len = in_len;
	// 申请内存
	data = new char[len + 1];
	memcpy(data, in_data, in_len * sizeof(char));
	data[len] = '\0';
}

void McString::GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len) {
	// 获取前缀的网络格式
	McVarInt t_len(len);
	unsigned char* varint_data;
	int varint_net_data_len;
	t_len.GetNetworkFormat(varint_data, varint_net_data_len);
	// 为拼接后的申请空间
	int str_size = len * sizeof(char);
	net_data_len = varint_net_data_len + str_size;
	m_networkData = new unsigned char[net_data_len];
	// 进行拼接
	memcpy(m_networkData, varint_data, varint_net_data_len);
	memcpy(m_networkData + varint_net_data_len, data, str_size);
	// 为了回收内存
	this->m_networkData = m_networkData;
}

void McString::show() {
	for (int i = 0; i < len; i++) {
		std::wcout << data[i];
	}
	//setlocale(LC_ALL, "");
	//wprintf(L"%s", data);
}

std::string McString::getVal() {
	char* t = new char[len + 1];
	memcpy(t, data, len);
	t[len] = '\0';
	std::string ret(t);
	delete t;
	return ret;
}

McString::~McString() {
	delete m_networkData;
}


McUnsignedShort::McUnsignedShort(Package& pack) {
	m_networkData = NULL;
	// 再保存字符串内容
	pack.ReadField((unsigned char*)(&data), sizeof(data));
}

McUnsignedShort::McUnsignedShort(unsigned short in_data) {
	m_networkData = NULL;
	data = in_data;
}

void McUnsignedShort::GetNetworkFormat(unsigned char*& m_networkData, int& net_data_len) {
	m_networkData = new unsigned char[sizeof(unsigned short)];
	net_data_len = sizeof(unsigned short);
	memcpy(m_networkData, &data, net_data_len);
	this->m_networkData = m_networkData;
}

unsigned short McUnsignedShort::get() {
	return data;
}

McUnsignedShort::~McUnsignedShort() {
	delete m_networkData;
}