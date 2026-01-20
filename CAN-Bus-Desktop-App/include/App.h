#ifndef APP_H
#define APP_H


#include <atomic>
#include <thread>
#include "CanFrameParser.h"
#include "ConcurrentQueue.h"
#include "TcpServer.h"
#include "TelemetryModel.h"
#include "DashboardUI.h"

class App {
public:

	explicit App(unsigned short port = 0);
	~App();
	void run();

private:
	void networkLoop();

	std::atomic<bool> running_{ false };
	std::thread net_thread_;
	
	
	ConcurrentQueue<CanFrame> frame_queue_;
	TelemetryModel model_;
	DashboardUI ui_;
	TcpServer server_;
	unsigned short port_{ 0 };
	CanFrameParser parser_;

};


#endif