#include <iostream>
#include <string>
#include "DashboardEngine.h"

#define SERVER 

DashboardEngine::DashboardEngine()
	: m_ipAddress({ 192, 168, 82, 122 }), m_port{ 3333 }
{}

void DashboardEngine::init()
{

#if defined SERVER
	 // bind listener to a port
	std::cout << "Listening for TCP client!\n";
	if (m_listener.listen(1025) != sf::Socket::Status::Done)
	{
		;
	}

	// wait for a connection
	if (m_listener.accept(m_socket) != sf::Socket::Status::Done)
	{
		;
	}
	std::cout << "new client connected: " << m_socket.getRemoteAddress().value() << " with port: " << m_socket.getRemotePort() << 
       std::endl;

#else

	std::cout << "Connecting to TCP server!\n";
	sf::Socket::Status status = m_socket.connect(m_ipAddress, m_port);

	if (status != sf::Socket::Status::Done)
	{
		// error...
		std::cerr << "Error connecting to TCP server!\n";
	}

#endif
}
void DashboardEngine::run()
{

#ifdef SERVER
   while (true)
   {
   	// Receive a message from the client
   	std::array <uint8_t, 1024> buffer;
   	buffer.fill(0);

   	std::size_t received = 0;
   	if (m_socket.receive(buffer.data(), buffer.size(), received) != sf::Socket::Status::Done)
   	{
   		std::cout << "Error receiving data!\n";
   		break;
   	}

   	std::cout << "The client said: " << buffer.data() << '\n';

   	// Send an answer
   	std::array<uint8_t, 1024> message(buffer);

   	if (m_socket.send(message.data(), received) != sf::Socket::Status::Done)
   	{
   		std::cout << "Error sending data!\n";
   		break;
   	}
   }

#else
	while (true)
	{
		m_buffer.fill(0);

		std::cout << "Enter data to send via TCP: ";
		std::cin.getline(m_buffer.data(), m_buffer.size());

		// TCP socket:
		if (m_socket.send(m_buffer.data(), m_buffer.size()) != sf::Socket::Status::Done)
		{
			// error...
			std::cout << "Error sending data!\n";
		}

		std::size_t received;
		if (m_socket.receive(m_buffer.data(), m_buffer.size(), received) != sf::Socket::Status::Done)
		{
			// error...
			std::cerr << "Error receiving data!\n";
		}

		std::cout << "\nThe server said: " << m_buffer.data() << std::endl;
		std::cout << "Received " << received << " bytes" << std::endl;
	}

#endif

}
void DashboardEngine::parseData()
{

}
void DashboardEngine::renderingThread()
{
	m_GUI.sGUI();
}

void DashboardEngine::receiveData()
{

}