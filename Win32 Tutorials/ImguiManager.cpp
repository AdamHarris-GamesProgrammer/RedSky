#include "ImguiManager.h"
#include "imgui/imgui.h"

ImguiManager::ImguiManager()
{
	//create context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

ImguiManager::~ImguiManager()
{
	//destroy context
	ImGui::DestroyContext();
}
