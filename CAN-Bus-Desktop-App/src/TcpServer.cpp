#include <iostream>
#include "TcpServer.h"


bool TcpServer::connect(const unsigned short port)
{
	std::cout << "Listening for TCP client!\n";
	if (listener_.listen(port) != sf::Socket::Status::Done)
	{
		;
	}

	// wait for a connection
	if (listener_.accept(socket_) != sf::Socket::Status::Done)
	{
		;
	}

	std::cout << "New client connected: " << socket_.getRemoteAddress().value() << " with port: " << socket_.getRemotePort() << '\n';
	connected_ = true;
	return true;
}

void TcpServer::disconnect()
{
	socket_.disconnect();
	connected_ = false;
}

bool TcpServer::isConnected() const
{
	return connected_;
}

size_t TcpServer::receive(uint8_t* dst, size_t capacity)
{
	size_t received{};
	if (socket_.receive(dst, capacity, received) != sf::Socket::Status::Done)
	{
		std::cerr << "error receiving data!\n";
		socket_.disconnect();
	}

	return received;
}