#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <atomic>
#include <SFML/Network.hpp>

class TcpServer {
public:
	bool connect(const unsigned short port);
	void disconnect();
	bool isConnected() const;
		
	size_t receive(uint8_t* dst, size_t capacity);

private:
	sf::TcpSocket socket_;
	sf::TcpListener listener_;
	std::atomic<bool> connected_{ false };
};




#endif 
