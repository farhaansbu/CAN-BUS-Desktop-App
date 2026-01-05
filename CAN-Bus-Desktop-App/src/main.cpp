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

    // Initialization
    engine.init();

    // Run networking/engine loop in the background
    std::thread engineThread(&DashboardEngine::run, &engine);

    // Run the GUI on the main thread 
    engine.renderingThread();

    engineThread.join();

    return 0;
}
