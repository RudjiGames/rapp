//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rprof_pch.h>
#include <math.h>
#include <rapp/inc/widgets/keyboard.h>

struct rprofApp : public rapp::App
{
	RAPP_CLASS(rprofApp)

	rapp::WindowHandle	m_window;
	float				m_time;

	int init(int32_t _argc, const char* const* _argv, rtmLibInterface* _libInterface)
	{
		RTM_UNUSED_3(_argc, _argv, _libInterface);

		m_time = 0.0f;

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

		rapp::appGraphicsInit(this, m_width, m_height, RAPP_WINDOW_FLAG_DPI_AWARE);

#ifdef RAPP_WITH_RPROF
		rprofInit();
#endif // RAPP_WITH_RPROF

		return 0;
	}

	void suspend() {}
	void resume() {}
	void update(float _time)
	{
#ifdef RAPP_WITH_RPROF
		RPROF_SCOPE("Update");
#endif // RAPP_WITH_RPROF

		m_time += _time;
	}

	void draw(float /*_alpha*/)
	{
#ifdef RAPP_WITH_RPROF
		rprofBeginFrame();
		RPROF_SCOPE("Draw");
#endif // RAPP_WITH_RPROF

		appRunOnMainThread(mainThreadFunc, this);

		//rtm::Console::rgb(255, 255, 0, "Printing from app thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x17, "rapp/samples/bgfx");
		bgfx::dbgTextPrintf(0, 2, 0x37, m_description);

		// debug input
		rapp::inputDgbGamePads();
		rapp::inputDgbMouse();
		rapp::inputDgbKeyboard();
		rapp::inputDgbTouch();
	}

	void drawGUI()
	{
#ifdef RAPP_WITH_RPROF
		ProfilerFrame frame;
		rprofGetFrame(&frame);
		rprofDrawFrame(&frame);
#endif // RAPP_WITH_RPROF

		ImGui::SetNextWindowPos(ImVec2(10.0f, 666.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("ImGui dialog");
		ImGui::Separator();
		ImGui::Button("Button", ImVec2(180.0f,23.0f));
		ImGui::End();

		rapp::rappVirtualKeyboard();

		rapp::MouseState ms;
		rapp::inputGetMouseState(ms);
	}
	
	void shutDown()
	{
#ifdef RAPP_WITH_RPROF
		rprofShutDown();
#endif // RAPP_WITH_RPROF
		//rtm::Console::rgb(255, 255, 0, "Shutting down app\n");
		rapp::appGraphicsShutdown(this, m_window);
		rapp::inputRemoveBindings("bindings");
	}

	static void mainThreadFunc(void* /*_appClass*/)
	{
		//rtm::Console::rgb(0, 255, 0, "Printing from main thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());
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

RAPP_REGISTER(rprofApp, "rprof", "Example of an app with rprof profiler built in");
