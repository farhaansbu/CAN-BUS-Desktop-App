#ifndef DASHBOARDUI_H
#define DASHBOARDUI_H

#include <SFML/Graphics.hpp>
#include "imgui.h"

class DashboardUI
{
public:
	void init();			// Initializes SFML window and ImGui
	void sGUI();
	void setAngle(float a);

private:
	void sImGui();			// Contains all of the Dear ImGui elements
	void SetupImGuiStyle();	// Sets up ImGui style
	sf::RenderWindow m_window;
	sf::Clock m_deltaClock;
	float angle{ 0.0f };
	ImFont* m_ImGuiFont;
};

#endif