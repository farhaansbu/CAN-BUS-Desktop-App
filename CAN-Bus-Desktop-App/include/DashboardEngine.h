#ifndef DASHBOARDENGINE_H
#define DASHBOARDENGINE_H

#include <SFML/Network.hpp>
#include "DashboardUI.h"

class DashboardEngine
{
public:
	DashboardEngine();
	void init();
	void run();
	void parseData();
	void renderingThread();
	void receiveData();

private:
	DashboardUI m_GUI;
	sf::TcpSocket m_socket;
	sf::TcpListener m_listener;
	sf::IpAddress m_ipAddress;
	unsigned short m_port;
	std::array<char, 1024> m_buffer;

};




#endif
