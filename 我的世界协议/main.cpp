#define _CRT_SECURE_NO_WARNINGS
#include <thread>
#include <vector>
#include <iostream>

#include "package.h"
#include "mcdt.h"
#include "MySocket.h"
#include "Locks.h"
#include "GetServerStatue.h"


void Portthread(std::string ip, unsigned short server_port, bool ifMonitor, int & portthreadAmount, int& world_amount) {
	GetServerStatue process(ip.c_str(), server_port);
	if (process.run()) {
		world_amount++;  // run的返回值为真说明世界存在
	}
	else if (ifMonitor) {
		std::cout << "<Monitor> " << ip << " lost!" << std::endl;
	}

	tcount.lock();
	portthreadAmount--;  // 线程数-1
	tcount.unlock();
}

struct IpConfig {
	std::string ip;
	int start;
	int end;
	int maxThreadPerIP;
};
void Ipthread(IpConfig ipConfig, bool ifMonitor, int timeGap) {
	int portThreadAmount = 0;
	int world_amount = 0;
	std::vector<std::thread*> threadPool;
	for (int i = ipConfig.start; i <= ipConfig.end; i++) {
		// 单ip下最大线程数限制
		if (portThreadAmount >= ipConfig.maxThreadPerIP) {
			
			Sleep(100);
			i--;
			continue;
		}
		// 输出状态
		if (GetServerStatue::ifDebugMode) {
			if (i % 1000 == 0) {
				m.lock();
				std::cout << "<debug>" << ipConfig.ip << "\t Scanning Port:[" << i << std::endl;
				m.unlock();
			}
		}
		// 线程数+1
		tcount.lock();
		portThreadAmount++;  // 线程数+1，必须加在这里，不能加在getStatuePortthread开头
		tcount.unlock();
		std::thread *t = new std::thread(Portthread, ipConfig.ip, i, ifMonitor, std::ref(portThreadAmount), std::ref(world_amount));
		threadPool.push_back(t);
		Sleep(10);

		// ifMonitor模式从头开始
		if (i == ipConfig.end && ifMonitor) {
			i = ipConfig.start - 1;
			while (!threadPool.empty()) {
				std::thread* t = threadPool.back();
				t->join();
				delete t;
				threadPool.pop_back();
			}
			Sleep(timeGap * 1000);
		}

	}

	// 等待这个ip下的所有port线程载入完毕后再逐一等待与释放port thread（由于单ip最大线程数限制，载入需要等待很长时间）
	while (!threadPool.empty()) {
		std::thread* t = threadPool.back();
		t->join();
		delete t;
		threadPool.pop_back();
	}
	m.lock();
	std::cout << "<info> " << ipConfig.ip << "\t world_amount:" << world_amount << std::endl;
	m.unlock();
}


int main() {
	std::wcout << "QiTiStudio Bear Scanner V2.9\n\n";

	std::wcout << "Input [ifMonitor & ifDebugMode & ifShowNoPlayerWorld]:" << std::endl;
	bool ifDebugMode = false, ifShowNoPlayerWorld = true, ifMonitor = false;
	std::cin >> ifMonitor >> ifDebugMode >> ifShowNoPlayerWorld;
	GetServerStatue::ifDebugMode = ifDebugMode;
	GetServerStatue::ifShowNoPlayerWorld = ifShowNoPlayerWorld;
	int timeGap = 0;
	if (ifMonitor) {
		std::cout << "Input [timeGap](S):" << std::endl;
		std::cin >> timeGap;
	}


	std::wcout << "Input [ip, start, end, maxthread]s end by '#':" << std::endl;
	std::vector<IpConfig> ipConfigVector;
	while (1) {
		IpConfig config;
		std::cin >> config.ip;
		if (config.ip == "#") {
			break;
		}
		std::cin >> config.start >> config.end >> config.maxThreadPerIP;
		ipConfigVector.push_back(config);
	}

	while (1) {
		std::vector<IpConfig>::iterator it = ipConfigVector.begin();
		std::vector<std::thread*> threadPool;
		for (; it != ipConfigVector.end(); ++it)
		{
			std::thread* ip_t = new std::thread(Ipthread, (*it), ifMonitor, timeGap);
			threadPool.push_back(ip_t);
		}
		std::wcout << "<info> ip load finished! amount: " << ipConfigVector.size() << std::endl << std::endl;

		// 等待所有ip线程退出
		while (!threadPool.empty()) {
			std::thread* t = threadPool.back();
			//std::cout << "<debug> join() ip thread -> " << t << std::endl;  // 如果想输出是哪个ip卡住了建议创建结构体保存ip和对应的线程地址
			t->join();
			delete t;
			//std::cout << "<debug> delete ip thread -> " << t << std::endl;
			threadPool.pop_back();
			std::cout << "<debug> rest ip thread amount:" << threadPool.size() << std::endl;
		}

		std::wcout << "<info> all ip scan finished!" << std::endl;
		std::wcout << "======================================================================================" << std::endl;
	}

	return 0;
}