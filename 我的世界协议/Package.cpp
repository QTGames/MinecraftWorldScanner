#include <iostream>

#include "package.h"
#include "mcdt.h"

void Package::AppendNewMem(int s) {
	unsigned char* t = new unsigned char[s + size];
	memcpy(t, data, size);
	delete data;
	data = t;
	// 更新size
	size += s;
}

void Package::FrontNewMem(int s) {
	unsigned char* t = new unsigned char[s + size];
	memcpy(t+s, data, size);
	delete data;
	data = t;
	// 更新size
	size += s;
}

Package::Package() {
	// 初始化基本数值
	id = -1;
	size = 0;
	p = 0;
	data = NULL;
}

void Package::AppendField(McField &field) {
	unsigned char* src;
	int e_size;
	field.GetNetworkFormat(src, e_size);
	// 直接申请内存
	AppendNewMem(e_size);
	// 将字符串本身拷贝到数据流
	memcpy(data + p, src, e_size);
	p += e_size;
}

void Package::FrontField(McField& field) {
	unsigned char* src;
	int e_size;
	field.GetNetworkFormat(src, e_size);
	// 直接申请内存
	FrontNewMem(e_size);
	// 将字符串本身拷贝到数据流
	memcpy(data, src, e_size);
	p += e_size;
}

void Package::ReadField(unsigned char* des, int o_size) {
	memcpy(des, data + p, o_size);
	p += o_size;
}

void Package::SendPack(int id, SOCKET &S) {
	/* 构造包格式 */
	// 生成包序号（id）字段
	McVarInt i(id);
	// 在前面追加字段
	FrontField(i);
	// 生成整个包的长度
	McVarInt sum_len(size);
	FrontField(sum_len);
	
	// 执行发送
	send(S, (char*)data, size, 0);
}

bool Package::RecvPack(SOCKET& S) {
	/* 先拆解包结构 */

	/* 读取第一个varint获得id+data的长度 */
	McVarInt p_len(S);
	size = p_len.getVal();
	// 不是mc的服务器：包的大小超过1M
	if (size < 0 || size > 1024*1024*1) {
		return false;
	}

	/* 继续读取全部数据到data中 */
	data = new unsigned char[size];
	int recv_len = recv(S, (char *)data, size, 0);
	// 不是mc的服务器：包的接收结果不符合预期
	if (recv_len != size) {
		return false;
	}

	/* 获得id */
	McVarInt id_len(*this);
	id = id_len.getVal();
	// 不是mc的服务器：id为负数
	if (id < 0) {
		return false;
	}

	// 成功
	return true;
}

int Package::GetSize() {
	return size;
}

void Package::ShowInHex() {
	Tools::dataStream2Hex(data, size);
}