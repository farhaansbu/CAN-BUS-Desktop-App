#include <iostream>
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "DashboardUI.h"


#define CAN_TABLE_COLUMNS 5

namespace CAN_Table_Column
{
	enum CAN_Table_Indices
	{
		Number,
		Timestamp,
		ID,
		Length,
		Data
	};
}


void DashboardUI::init()
{
	m_window.create(sf::VideoMode({ 1920, 1080 }), "CAN Bus Dashboard");
	m_window.setFramerateLimit(60);
	ImGui::SFML::Init(m_window);

	std::string fontPath = std::string(ASSETS_DIR) + "/Fonts/robotoMono-regular.ttf";

	std::cout << "Loading first font!\n";
	m_ImGuiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f, nullptr);
	ImGui::SFML::UpdateFontTexture();
	ImGui::GetIO().FontDefault = m_ImGuiFont;

	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	SetupImGuiStyle();

}


void DashboardUI::sGUI()
{
	init();

	sf::Texture tach;
	tach.loadFromFile(std::string(ASSETS_DIR) + "/Tachometer2.png");
	sf::Sprite tachometer{ tach };

	sf::Texture tach_arrow;
	tach_arrow.loadFromFile(std::string(ASSETS_DIR) + "/Tachometer_arrow2.png");
	sf::Sprite tachometer_arrow{ tach_arrow };

	sf::Font font;
	if (!font.openFromFile(std::string(ASSETS_DIR) + "/Fonts/FearRobot.ttf"))
	{
		std::cerr << "Failed to load front from file!\n";
	}

	tachometer.setPosition({ 200, 300 });
	tachometer_arrow.setOrigin({ 24, 24 });
	tachometer_arrow.setPosition({ tachometer.getPosition().x + tachometer.getLocalBounds().size.x / 2,
								tachometer.getPosition().y + tachometer.getLocalBounds().size.y / 2 });

	sf::Text speed{ font };
	speed.setString("45 MPH");
	speed.setCharacterSize(75);
	speed.setPosition({ tachometer.getPosition().x + tachometer.getLocalBounds().size.x / 2 - speed.getLocalBounds().size.x / 2, 100 });


	while (m_window.isOpen())
	{

		ImGui::SFML::Update(m_window, m_deltaClock.restart());
	
		

		// Event handling
		while (const auto event = m_window.pollEvent())
		{
			ImGui::SFML::ProcessEvent(m_window, *event);
			if (event->is<sf::Event::Closed>())
			{
				m_window.close();
			}
		}

		//angle += 0.2f;
		tachometer_arrow.setRotation(sf::degrees(angle));

		// Draw SFML stuff
		m_window.clear(sf::Color{ 43, 47, 59 });
		m_window.draw(tachometer);
		m_window.draw(tachometer_arrow);
		m_window.draw(speed);

		// Draw ImGUI stuff
		sImGui();
		ImGui::SFML::Render(m_window);

		// Display
		m_window.display();
	}

	// Cleanup
	ImGui::SFML::Shutdown();
}

void DashboardUI::sImGui()
{
	ImGui::SetNextWindowPos(ImVec2(960.0f, 75.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(900.0f, 900.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Diagnostics", 0, ImGuiWindowFlags_NoCollapse);
	if (ImGui::BeginTabBar("Items"))
	{
		if (ImGui::BeginTabItem("Information"))
		{
			ImGui::Text("Vehicle Information");
			ImGui::Separator();

			// Collapsible: General Info
			if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("VIN: %s", "1HGCM82633A123456");
				ImGui::Text("Make: %s", "Honda");
				ImGui::Text("Model: %s", "Civic");
				ImGui::Text("Year: %d", 2010);
				ImGui::Text("Odometer: %.1f miles", 123456.7f);
			}

			// Collapsible: Live Data
			if (ImGui::CollapsingHeader("Live Data"))
			{
				ImGui::Text("Engine RPM: %d", 3200);
				ImGui::Text("Vehicle Speed: %d MPH", 45);
				ImGui::Text("Throttle Position: %.1f %%", 32.4f);
				ImGui::Text("Coolant Temp: %.1f *C", 90.0f);
				ImGui::Text("Battery Voltage: %.2f V", 13.8f);
			}

			// Collapsible: Sensor Status (table layout)
			if (ImGui::CollapsingHeader("Sensors"))
			{
				if (ImGui::BeginTable("sensors", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Sensor");
					ImGui::TableSetupColumn("Status");
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("O2 Sensor");
					ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0, 1, 0, 1), "OK");

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("ABS");
					ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1, 0, 0, 1), "Fault");

					ImGui::EndTable();
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("CAN Frames"))
		{
			if (ImGui::BeginTable("CAN Data", CAN_TABLE_COLUMNS, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("#");
				ImGui::TableSetupColumn("Timestamp");
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Length");
				ImGui::TableSetupColumn("Data");
				ImGui::TableHeadersRow();

				for (int i = 0; i < 10; ++i)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(CAN_Table_Column::Number);
					ImGui::Text("%d", i);
					ImGui::TableSetColumnIndex(CAN_Table_Column::Timestamp); 
					ImGui::Text("0.00213");
					ImGui::TableSetColumnIndex(CAN_Table_Column::ID);
					ImGui::Text("0x153");
					ImGui::TableSetColumnIndex(CAN_Table_Column::Length);
					ImGui::Text("%d", 8);
					ImGui::TableSetColumnIndex(CAN_Table_Column::Data);
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "OK");
				}

				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();

}


void DashboardUI::setAngle(float a)
{
	angle = a;
}

void DashboardUI::SetupImGuiStyle()
{
	// Fork of Comfortable Dark Cyan style from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(20.0f, 20.0f);
	style.WindowRounding = 11.5f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 20.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 17.39999961853027f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 3.400000095367432f);
	style.FrameRounding = 5.599999904632568f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.899999618530273f, 13.39999961853027f);
	style.ItemInnerSpacing = ImVec2(7.099999904632568f, 1.799999952316284f);
	style.CellPadding = ImVec2(10.10000038146973f, 9.199999809265137f);
	style.IndentSpacing = 0.0f;
	style.ColumnsMinSpacing = 8.699999809265137f;
	style.ScrollbarSize = 13.80000019073486f;
	style.ScrollbarRounding = 15.89999961853027f;
	style.GrabMinSize = 9.199999809265137f;
	style.GrabRounding = 20.0f;
	style.TabRounding = 9.800000190734863f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09411764889955521f, 0.1019607856869698f, 0.1176470592617989f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1137254908680916f, 0.125490203499794f, 0.1529411822557449f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.1702002137899399f, 0.5488495826721191f, 0.7081544995307922f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1803921610116959f, 0.1882352977991104f, 0.196078434586525f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1529411822557449f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1411764770746231f, 0.1647058874368668f, 0.2078431397676468f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.105882354080677f, 0.105882354080677f, 0.105882354080677f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1294117718935013f, 0.1490196138620377f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.3728348612785339f, 0.2819908261299133f, 0.8111587762832642f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2108338326215744f, 0.15402752161026f, 0.4849785566329956f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.1394784450531006f, 0.1007939130067825f, 0.3261802792549133f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.125490203499794f, 0.2745098173618317f, 0.572549045085907f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.8068669438362122f, 0.5714160203933716f, 0.1904621571302414f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.05260733887553215f, 0.8755365014076233f, 0.7805831432342529f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.3728348612785339f, 0.2819908261299133f, 0.8111587762832642f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2666666805744171f, 0.2901960909366608f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
}

