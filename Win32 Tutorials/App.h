#pragma once
#include "Window.h"
#include "RedSkyTimer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "Model.h"
#include "TestPlane.h"
#include "ScriptCommander.h"
#include "TestCube.h"
#include "Stencil.h"
#include "FrameCommander.h"#
#include "Material.h"

class App
{
public:
	App(const std::string& commandLine = "");
	~App();
	int Go();
private:
	void DoFrame();

	void SpawnBackgroundControlWindow() noexcept;
	void ShowImguiDemoWindow();

	void PollInput(float dt);

private:
	std::string commandLine;
	ImguiManager imgui;
	Window wnd;
	RedSkyTimer timer;
	ScriptCommander scriptCommander;
	float simSpeed = 1.0f;

	bool showDemoWindow = false;

	Camera cam;

	PointLight light;

	FrameCommander fc;

	TestCube cube{ wnd.Gfx(), 4.0f };
	TestCube cube2{ wnd.Gfx(), 4.0f };

	DirectX::XMFLOAT4 bgColour = { 0.1f,0.1f,0.2f, 1.0f };

	std::unique_ptr<Mesh> pLoaded;

	//Model sponza{ wnd.Gfx(), "Models\\Sponza\\sponza.obj", 1.0f / 20.0f };
	//TestPlane bluePlane{ wnd.Gfx(), 6.0f, {0.3f, 0.3f, 1.0f, 0.0f} };
	//TestPlane redPlane{ wnd.Gfx(), 6.0f, {1.0f, 0.3f,0.3f,0.0f} };

	Model goblin{ wnd.Gfx(), "Models\\gobber\\GoblinX.obj", 6.0f };
	/*Model wall{ wnd.Gfx(), "Models\\brick_wall\\brick_wall.obj", 6.0f };
	TestPlane tp{ wnd.Gfx(), 6.0f };*/
	//Model nano{ wnd.Gfx(), "Models\\nanoTextured\\nanosuit.obj", 2.0f };

};
