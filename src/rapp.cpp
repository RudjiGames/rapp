//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/src/rapp_timer.h>
#include <rapp/src/cmd.h>
#include <rapp/src/console.h>
#include <rapp/src/entry_p.h>

#if RAPP_WITH_BGFX
#include <bgfx/bgfx.h>
#include <common/imgui/imgui.h>
#include <dear-imgui/imgui_internal.h>
#endif

#define RAPP_CMD_READ(_type, _name)		\
	_type _name;						\
	cc->read(_name)

namespace rapp {

struct Command
{
	enum Enum : uint8_t
	{
		Dbg,

		Init,
		Suspend,
		Resume,
		Update,
		Draw,
		DrawGUI,
		Shutdown,

		Count
	};
};

static rtm::CommandBuffer	s_commChannel;		// rapp_main to app class thread communication

rtm_vector<App*>& appGetRegistered()
{
	static rtm_vector<App*> apps;
	return apps;
}

int32_t rappThreadFunc(void* _userData)
{
	rtm::CommandBuffer* cc = (rtm::CommandBuffer*)_userData;

	while (cc->dataAvailable())
	{
		uint8_t cmd;
		cc->read(cmd);

		switch (cmd)
		{
			case Command::Init:
				{
					RAPP_CMD_READ(App*, app);
					RAPP_CMD_READ(int, argc);
					RAPP_CMD_READ(const char* const*, argv);

					app->init(argc, argv, 0);
				}
				break;

			case Command::Suspend:
				{
					RAPP_CMD_READ(App*, app);
					app->suspend();

				}
				break;

			case Command::Resume:
				{
					RAPP_CMD_READ(App*, app);
					app->resume();
				}
				break;

			case Command::Update:
				{
					RAPP_CMD_READ(App*, app);
					RAPP_CMD_READ(float, time);
					app->update(time);
				}
				break;

			case Command::Draw:
				{
					RAPP_CMD_READ(App*, app);
					app->draw();
				}
				break;

			case Command::DrawGUI:
				{
					RAPP_CMD_READ(App*, app);
					RTM_UNUSED(app);
#if RAPP_WITH_BGFX
					MouseState ms;
					inputGetMouseState(ms);
					imguiBeginFrame(ms.m_absolute[0], ms.m_absolute[1]
						, (ms.m_buttons[MouseState::Button::Left  ] ? IMGUI_MBUT_LEFT   : 0)
						| (ms.m_buttons[MouseState::Button::Right ] ? IMGUI_MBUT_RIGHT  : 0)
						| (ms.m_buttons[MouseState::Button::Middle] ? IMGUI_MBUT_MIDDLE : 0)
						,  ms.m_absolute[2]
						, uint16_t(app->m_width)
						, uint16_t(app->m_height)
						);

					app->drawGUI();
					app->m_console->draw();
					imguiEndFrame();
#endif
				}
				break;

			case Command::Shutdown:
				{
					RAPP_CMD_READ(App*, app);
					app->shutDown();
				}
				break;

		default:
			RTM_ASSERT(false, "Invalid command!");
		};
	}

	return 0;
}

void init(rtmLibInterface* _libInterface)
{
	g_allocator		= _libInterface ? _libInterface->m_memory : 0;
	g_errorHandler	= _libInterface ? _libInterface->m_error : 0;

	inputInit();

	if (appGetRegistered().size() > 1)
		cmdAdd("app", cmdApp, 0, "Application management commands, type 'app help' for more info");

	s_commChannel.init(rappThreadFunc);
}

void shutDown()
{
	inputShutdown();

	s_commChannel.shutDown();
}

void appRegister(App* _app);

App::App(const char* _name, const char* _description)
	: m_name(_name)
	, m_description(_description)
	, m_exitCode(0)
	, m_width(0)
	, m_height(0)
	, m_console(0)
{
	appRegister(this);
}

void appRegister(App* _app)
{
	appGetRegistered().push_back(_app);
}

///
App* appGet(uint32_t _index)
{
	if (_index < appGetCount())
		return appGetRegistered()[_index];
	return 0;
}

///
uint32_t appGetCount()
{
	return (uint32_t)appGetRegistered().size();
}

void appInit(App* _app, int _argc, const char* const* _argv)
{
	s_commChannel.write(Command::Init);
	s_commChannel.write(_app);
	s_commChannel.write(_argc);
	s_commChannel.write(_argv);
}

void appShutDown(App* _app)
{
	s_commChannel.write(Command::Shutdown);
	s_commChannel.write(_app);
}

void appSuspend(App* _app)
{
	s_commChannel.write(Command::Suspend);
	s_commChannel.write(_app);
}

void appResume(App* _app)
{
	s_commChannel.write(Command::Resume);
	s_commChannel.write(_app);
}

void appUpdate(App* _app, float _time)
{
	s_commChannel.write(Command::Update);
	s_commChannel.write(_app);
	s_commChannel.write(_time);
}

void appDraw(App* _app)
{
	s_commChannel.write(Command::Draw);
	s_commChannel.write(_app);
}

void appDrawGUI(App* _app)
{
	s_commChannel.write(Command::DrawGUI);
	s_commChannel.write(_app);
}

WindowHandle appGraphicsInit(App* _app, uint32_t _width, uint32_t _height)
{
	RTM_UNUSED_3(_app, _width, _height);

#if RAPP_WITH_BGFX
	WindowHandle win = rapp::windowCreate(	_app, 0, 0, _width, _height,
											RAPP_WINDOW_FLAG_ASPECT_RATIO	|
											RAPP_WINDOW_FLAG_FRAME			|
											RAPP_WINDOW_FLAG_RENDERING		|
											RAPP_WINDOW_FLAG_MAIN_WINDOW,
											_app->m_name);

	bgfx::init();
	bgfx::reset(_width, _height, BGFX_RESET_VSYNC);

//#if !RTM_RETAIL
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT);
//#endif

	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	imguiCreate();

	ImGui::GetIO().KeyMap[ImGuiKey_Tab]			= KeyboardState::Key::Tab;
	ImGui::GetIO().KeyMap[ImGuiKey_LeftArrow]	= KeyboardState::Key::Left;
	ImGui::GetIO().KeyMap[ImGuiKey_RightArrow]	= KeyboardState::Key::Right;
	ImGui::GetIO().KeyMap[ImGuiKey_UpArrow]		= KeyboardState::Key::Up;
	ImGui::GetIO().KeyMap[ImGuiKey_DownArrow]	= KeyboardState::Key::Down;
	ImGui::GetIO().KeyMap[ImGuiKey_Home]		= KeyboardState::Key::Home;
	ImGui::GetIO().KeyMap[ImGuiKey_End]			= KeyboardState::Key::End;
	ImGui::GetIO().KeyMap[ImGuiKey_Delete]		= KeyboardState::Key::Delete;
	ImGui::GetIO().KeyMap[ImGuiKey_Backspace]	= KeyboardState::Key::Backspace;
	ImGui::GetIO().KeyMap[ImGuiKey_Enter]		= KeyboardState::Key::Return;
	ImGui::GetIO().KeyMap[ImGuiKey_Escape]		= KeyboardState::Key::Esc;
	ImGui::GetIO().KeyMap[ImGuiKey_A]			= KeyboardState::Key::KeyA;
	ImGui::GetIO().KeyMap[ImGuiKey_C]			= KeyboardState::Key::KeyC;
	ImGui::GetIO().KeyMap[ImGuiKey_V]			= KeyboardState::Key::KeyV;
	ImGui::GetIO().KeyMap[ImGuiKey_X]			= KeyboardState::Key::KeyX;
	ImGui::GetIO().KeyMap[ImGuiKey_Y]			= KeyboardState::Key::KeyY;
	ImGui::GetIO().KeyMap[ImGuiKey_Z]			= KeyboardState::Key::KeyZ;

	_app->m_console = new Console(_app);

	return win;
#else
	return {0};
#endif
}

void appGraphicsShutdown(App* _app, WindowHandle _mainWindow)
{
	RTM_UNUSED_2(_app, _mainWindow);

#if RAPP_WITH_BGFX

	delete _app->m_console;

	ImGui::ClearActiveID();
	imguiDestroy();

	bgfx::shutdown();
	rapp::windowDestroy(_mainWindow);
#endif
}

App* g_next_app = 0;

void appSwitch(App* _app)
{
	rtm::interlockedSet((intptr_t*)&g_next_app, (intptr_t)_app);
}

bool processEvents(App* _app);

///
int appRun(App* _app, int _argc, const char* const* _argv)
{
	appInit(_app, _argc, _argv);

	FrameStep fs;
	while (processEvents(_app))
	{
		while (fs.update())
		{
			float time = fs.step();
			appUpdate(_app, time);
		}

		appDraw(_app);

		if (_app->m_width && _app->m_height)
			appDrawGUI(_app);

		if (g_next_app)
		{
			appShutDown(_app);
			_app = g_next_app;

			appInit(_app, _argc, _argv);
			g_next_app = 0;
		}

		s_commChannel.frame();
	}

	appShutDown(_app);

	return 0;
}

int rapp_main(int _argc, const char* const* _argv)
{
	rtmLibInterface* errorHandler = 0;
	//&RAPP_INSTANCE(CmdLineApp)
	rapp::init(errorHandler);
	int ret = rapp::appRun(rapp::appGetRegistered()[0], _argc, _argv);
	rapp::shutDown();

	return ret;
}

} // namespace rapp
