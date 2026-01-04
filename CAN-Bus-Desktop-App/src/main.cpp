#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include "DashboardEngine.h"

#include <iostream>
#include <thread>


int a{ 5 };

int main()
{
	std::cout << "Initializing Engine\n";
	DashboardEngine engine;

	std::cout << "Creating rendering thread!\n";
	std::thread renderThread(&DashboardEngine::renderingThread, &engine);
	renderThread.detach();

	engine.init();	
	engine.run();

	return 0;
}