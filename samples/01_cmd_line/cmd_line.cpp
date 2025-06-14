//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <cmd_line_pch.h>

struct CmdLineApp : public rapp::App
{
	RAPP_CLASS(CmdLineApp)

	int init(int32_t /*_argc*/, const char* const* /*_argv*/, rtmLibInterface* /*_libInterface = 0*/)
	{
		static const rapp::InputBinding bindings[] =
		{
			{ 0, "exit", 1, { rapp::KeyboardKey::KeyQ, rapp::KeyboardModifier::LCtrl }},
			{ 0, "exit", 1, { rapp::KeyboardKey::KeyQ, rapp::KeyboardModifier::RCtrl }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("bindings", bindings);
		rapp::cmdAdd("exit", cmdExit);

		return 0;
	}

	void suspend() {}
	void resume() {}

	void update(float /*_time*/)
	{
	}

	void draw(float /*_alpha*/)
	{
		appRunOnMainThread(mainThreadFunc, this);
		rtm::Console::rgb(255, 255, 0, "Printing from app thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());
	}

	void shutDown()
	{
		rtm::Console::rgb(255, 255, 0, "Shutting down app\n", (uint32_t)rtm::Thread::getThreadID());
		rapp::inputRemoveBindings("bindings");
	}

	static void mainThreadFunc(void* /*_appClass*/)
	{
		rtm::Console::rgb(0, 255, 0, "Printing from main thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());
	}

	static int cmdExit(App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		_app->quit();
		return 0;
	}
};

RAPP_REGISTER(CmdLineApp, "Command line", "Example of command line application");
