//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <multi_app_pch.h>

struct app1 : public rapp::App
{
	RAPP_CLASS(app1)

	rapp::WindowHandle	m_window;

	int init(int32_t /*_argc*/, const char* const* /*_argv*/, rtmLibInterface* /*_libInterface = 0*/)
	{
		static const rapp::InputBinding bindings[] =
		{
			{ 0, "exit", 1, { rapp::KeyboardKey::KeyQ,  rapp::KeyboardModifier::LCtrl }},
			{ 0, "exit", 1, { rapp::KeyboardKey::KeyQ,  rapp::KeyboardModifier::RCtrl }},
			{ 0, "hide", 1, { rapp::KeyboardKey::Tilde, rapp::KeyboardModifier::None  }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("bindings", bindings);
		rapp::cmdAdd("exit", cmdExit,			0, "quits application");
		rapp::cmdAdd("hide", cmdHideConsole,	0, "hides console");

		uint32_t width, height;
		rapp::windowGetDefaultSize(&width, &height);
		m_width		= width;
		m_height	= height;

		m_window = rapp::appGraphicsInit(this, m_width, m_height, RAPP_WINDOW_FLAG_DPI_AWARE);
		return 0;
	}

	void suspend() {}
	void resume() {}
	void update(float /*_time*/) {}

	void draw(float /*_alpha*/)
	{
		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x17, "rapp/samples/multi_app");
		bgfx::dbgTextPrintf(0, 2, 0x37, m_description);

		// debug input
		rapp::inputDgbGamePads();
		rapp::inputDgbMouse();
		rapp::inputDgbKeyboard();
		rapp::inputDgbTouch();
	}

	void drawGUI()
	{
		ImGui::SetNextWindowPos(ImVec2(10.0f, 666.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("ImGui dialog");
		ImGui::Separator();
		ImGui::Button("Button", ImVec2(180.0f,23.0f));
		ImGui::End();
	}
	
	void shutDown()
	{
		rtm::Console::rgb(255, 255, 0, "Shutting down app\n", (uint32_t)rtm::threadGetID());
		rapp::appGraphicsShutdown(this, m_window);
		rapp::inputRemoveBindings("bindings");
	}

	static int cmdExit(rapp::App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		_app->quit();
		return 0;
	}

	static int cmdHideConsole(rapp::App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		rapp::cmdConsoleToggle(_app);
		return 0;
	}
};

RAPP_REGISTER(app1, "app1", "Application 1");
