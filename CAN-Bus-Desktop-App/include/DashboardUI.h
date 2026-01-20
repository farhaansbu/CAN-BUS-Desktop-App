#ifndef DASHBOARDUI_H
#define DASHBOARDUI_H

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "TelemetryModel.h"

class App;

class DashboardUI {
	friend class App;
public:
	void init();			// Initializes SFML window and ImGui
	void render(const TelemetryModel& model);

private:
	void render_imgui(const TelemetryModel& model);			// Contains all of the Dear ImGui elements
	void SetupImGuiStyle();	// Sets up ImGui 
	sf::RenderWindow window_;
	sf::Clock deltaClock_;
	ImFont* imGuiFont_;
};

#endif