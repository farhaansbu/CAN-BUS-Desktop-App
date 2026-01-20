#include "App.h"


int main()
{
	//std::cout << "Initializing Engine\n";
	//DashboardEngine engine;

	//std::cout << "Creating rendering thread!\n";
	//std::thread renderThread(&DashboardEngine::renderingThread, &engine);
	//renderThread.detach();

	//engine.init();	
	//engine.run();

	//return 0;

    // Replace with your port
    const unsigned short port = 3333;

    App app(port);
    app.run();
    
    return 0;
}