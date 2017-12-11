//--------------------------------------------------------------------------//
/// Copyright (c) 2017 Milos Tosic. All Rights Reserved.                   ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <bgfx_pch.h>
#include <bgfx/bgfx.h>

struct bgfxApp : public rapp::App
{
	RAPP_CLASS(bgfxApp)

	rapp::WindowHandle	m_window;

	virtual ~bgfxApp() {}

	int init(int32_t /*_argc*/, const char* const* /*_argv*/, rtmLibInterface* /*_libInterface = 0*/)
	{
		static const rapp::InputBinding bindings[] =
		{
			{ NULL, "exit", 1, { rapp::KeyboardState::Key::KeyQ,   rapp::KeyboardState::Modifier::LCtrl  }},
			{ NULL, "exit", 1, { rapp::KeyboardState::Key::KeyQ,   rapp::KeyboardState::Modifier::RCtrl  }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("bindings", bindings);
		rapp::cmdAdd("exit", cmdExit, this);

		uint32_t width, height;
		rapp::windowGetDefaultSize(&width, &height);
		m_width		= width;
		m_height	= height;

		m_window = rapp::appGraphicsInit(this, m_width, m_height);
		return 0;
	}

	void suspend() {}
	void resume() {}
	void update(float /*_time*/) {}

	void draw()
	{
		appRunOnMainThread(mainThreadFunc, this);

		rtm::Console::custom(255, 255, 0, 1, "Printing from app thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, (uint16_t)m_width, (uint16_t)m_height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, "rapp/samples/bgfx");
		bgfx::dbgTextPrintf(0, 2, 0x6f, m_description);

		// debug input
		rapp::inputDgbGamePads();
		rapp::inputDgbMouse();
		rapp::inputDgbKeyboard();
		rapp::inputDgbTouch();

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
	}

	void shutDown()
	{
		rtm::Console::custom(255, 255, 0, 1, "Shutting down app\n", (uint32_t)rtm::Thread::getThreadID());
		rapp::appGraphicsShutdown(m_window);
		rapp::inputRemoveBindings("bindings");
	}

	static void mainThreadFunc(void* /*_appClass*/)
	{
		rtm::Console::custom(0, 255, 0, 1, "Printing from main thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());
	}

	static int cmdExit(void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_2(_argv, _argc);
		App* app = (App*)_userData;
		app->quit();
		return 0;
	}
};

RAPP_REGISTER(bgfxApp, "bgfx", "Example of bgfx based application");
