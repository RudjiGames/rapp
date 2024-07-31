//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#define RTM_LIBHANDLER_DEFINE
#include <rbase/inc/libhandler.h>

#include <rapp/src/rapp_timer.h>
#include <rapp/src/cmd.h>
#include <rapp/src/console.h>
#include <rapp/src/entry_p.h>
#include <rapp/src/app_data.h>

#ifdef RAPP_WITH_BGFX
#include <bx/allocator.h>
#include <bgfx/bgfx.h>
#include <dear-imgui/imgui/imgui.h>
#include <dear-imgui/imgui_internal.h>
extern NVGcontext* g_currentContext;
#endif // RAPP_WITH_BGFX

#if RTM_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif // RTM_PLATFORM_EMSCRIPTEN

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
		Frame,
		Shutdown,

		Count
	};
};

static rtm::CommandBuffer	s_commChannel;		// rapp_main to app class thread communication
App*						s_app = 0;

rtm::FixedArray<App*, RAPP_MAX_APPS>& appGetRegistered()
{
	static rtm::FixedArray<App*, RAPP_MAX_APPS> apps;
	return apps;
}

static void drawGUI(App* _app)
{
	RTM_UNUSED(_app);
#ifdef RAPP_WITH_BGFX
	MouseState ms;
	inputGetMouseState(ms);
	imguiBeginFrame(ms.m_absolute[0], ms.m_absolute[1]
		, (ms.m_buttons[MouseState::Button::Left  ] ? IMGUI_MBUT_LEFT   : 0)
		| (ms.m_buttons[MouseState::Button::Right ] ? IMGUI_MBUT_RIGHT  : 0)
		| (ms.m_buttons[MouseState::Button::Middle] ? IMGUI_MBUT_MIDDLE : 0)
		,  ms.m_absolute[2]
		, uint16_t(_app->m_width)
		, uint16_t(_app->m_height)
		);

	nvgBeginFrame(_app->m_data->m_vg, (float)_app->m_width, (float)_app->m_height, 1.0f);

	_app->drawGUI();
	_app->m_data->m_console->draw();

	nvgEndFrame(_app->m_data->m_vg);
	imguiEndFrame();
#endif // RAPP_WITH_BGFX
}

#ifdef RAPP_WITH_BGFX
static uint32_t s_debug = 0;
extern uint32_t g_debug;
#endif // RAPP_WITH_BGFX

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

					rtmLibInterface libInterface;
					libInterface.m_error	= g_errorHandler;
					libInterface.m_memory	= g_allocator;
					app->init(argc, argv, &libInterface);
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
					RAPP_CMD_READ(float, alpha);
#ifdef RAPP_WITH_BGFX
					if (app->m_resetView)
					{
						bgfx::reset(app->m_width, app->m_height);

						app->m_resetView = false;
					}

					// Set view 0 default viewport.
					bgfx::setViewRect(0, 0, 0, (uint16_t)app->m_width, (uint16_t)app->m_height);

					// This dummy draw call is here to make sure that view 0 is cleared
					// if no other draw calls are submitted to view 0.
					bgfx::touch(0);

					g_currentContext = app->m_data->m_vg;

					app->draw(alpha);
#endif // #RAPP_WITH_BGFX
				}
				break;

			case Command::DrawGUI:
				{
					RAPP_CMD_READ(App*, app);
					drawGUI(app);
				}
				break;

			case Command::Frame:
				{
#ifdef RAPP_WITH_BGFX
					g_currentContext = 0;

					bgfx::frame();
					if (s_debug != g_debug)
					{
						bgfx::setDebug(g_debug);
						s_debug = g_debug;
					}
#endif // RAPP_WITH_BGFX
				}
				break;

			case Command::Shutdown:
				{
					RAPP_CMD_READ(App*, app);
					app->shutDown();
#ifdef RAPP_WITH_BGFX
					g_currentContext = 0;
#endif // RAPP_WITH_BGFX
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

#if !RTM_PLATFORM_EMSCRIPTEN
	s_commChannel.init(rappThreadFunc);
#endif // !RTM_PLATFORM_EMSCRIPTEN
}

void shutDown()
{
	inputShutdown();

#if !RTM_PLATFORM_EMSCRIPTEN
	s_commChannel.shutDown();
#endif // !RTM_PLATFORM_EMSCRIPTEN
}

void appRegister(App* _app);

App::App(const char* _name, const char* _description)
	: m_name(_name)
	, m_description(_description)
	, m_exitCode(0)
	, m_width(0)
	, m_height(0)
	, m_frameRate(60)
	, m_data(0)
	, m_resetView(false)
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

void appDraw(App* _app, float _alpha)
{
	s_commChannel.write(Command::Draw);
	s_commChannel.write(_app);
	s_commChannel.write(_alpha);
}

void appDrawGUI(App* _app)
{
	s_commChannel.write(Command::DrawGUI);
	s_commChannel.write(_app);
}

void appFrame()
{
	s_commChannel.write(Command::Frame);
}

WindowHandle appGraphicsInit(App* _app, uint32_t _width, uint32_t _height)
{
	RTM_UNUSED_3(_app, _width, _height);
#ifdef RAPP_WITH_BGFX
	WindowHandle win = rapp::windowCreate(	_app, 0, 0, _width, _height,
											RAPP_WINDOW_FLAG_ASPECT_RATIO	|
											RAPP_WINDOW_FLAG_FRAME			|
											RAPP_WINDOW_FLAG_RENDERING		|
											RAPP_WINDOW_FLAG_MAIN_WINDOW,
											_app->m_name);

	bgfx::Init init;
	init.type     = bgfx::RendererType::Count;
	init.vendorId = BGFX_PCI_ID_NONE;
	init.platformData.nwh  = rapp::windowGetNativeHandle(win);
	init.platformData.ndt  = rapp::windowGetNativeDisplayHandle();
	init.resolution.width  = _width;
	init.resolution.height = _height;
	init.resolution.reset  = BGFX_RESET_VSYNC;
	bgfx::init(init);

#if !RTM_RETAIL
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT);
#endif

	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0x17132Eff, 1.0f, 0);

	imguiCreate();

	_app->m_data			= new AppData;
	_app->m_data->m_console = new Console(_app);
	_app->m_data->m_vg		= nvgCreate(1, 0);

	bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

	return win;
#else
	return {0};
#endif
}

void appGraphicsShutdown(App* _app, WindowHandle _mainWindow)
{
	RTM_UNUSED_2(_app, _mainWindow);

#ifdef RAPP_WITH_BGFX
	nvgDelete(_app->m_data->m_vg);
	_app->m_data->m_vg = 0;

	delete _app->m_data->m_console;
	_app->m_data->m_console = 0;

	delete _app->m_data;
	_app->m_data = 0;

	imguiDestroy();

	bgfx::frame();
	bgfx::shutdown();
	rapp::windowDestroy(_mainWindow);
#endif
}

void* appGetNanoVGctx(App* _app)
{
	RTM_UNUSED(_app);
#ifdef RAPP_WITH_BGFX
	return _app->m_data->m_vg;
#else
	return 0;
#endif
}

App* g_next_app = 0;

void appSwitch(App* _app)
{
	g_next_app = _app;
}

bool processEvents(App* _app);

#if RTM_PLATFORM_EMSCRIPTEN
static const char* s_canvasID = "#canvas";

extern void emscriptenUpdateGamepads();
static void updateApp()
{
	static bool first_frame = true;
	static FrameStep fs(s_app->m_frameRate);

	if (processEvents(s_app))
	{
		if (s_app->m_frameRate != fs.frameRate())
		fs.setFrameRate(s_app->m_frameRate);

		while (fs.update())
		{
			float time = fs.step();
			s_app->update(time);
		}

		if (s_debug != g_debug)
		{
			bgfx::setDebug(g_debug);
			s_debug = g_debug;
		}

		emscriptenUpdateGamepads();

#ifdef RAPP_WITH_BGFX
		double dwidth, dheight;
		emscripten_get_element_css_size(s_canvasID, &dwidth, &dheight);
		uint32_t width	= (uint32_t)dwidth;
		uint32_t height	= (uint32_t)dheight;

		if (s_app->m_resetView)
		{
			s_app->m_width = width;
			s_app->m_height = height;
			s_app->m_resetView = false;
			bgfx::reset(s_app->m_width, s_app->m_height);
		}

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, (uint16_t)s_app->m_width, (uint16_t)s_app->m_height);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x17132Eff, 1.0f, 0);

		if (first_frame)
		{
			// reset view in first frame to match canvas size
			bgfx::reset(s_app->m_width, s_app->m_height);
			first_frame = false;
		}

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		s_app->draw(fs.alpha());

		if (s_app->m_width && s_app->m_height)
			drawGUI(s_app);

		bgfx::frame();
#endif // RAPP_WITH_BGFX

		if (g_next_app)
		{
			s_app->shutDown();
			s_app = g_next_app;

			s_app->init(0, 0);//_argc, _argv);
			g_next_app = 0;
		}
	}
}
#endif // RTM_PLATFORM_EMSCRIPTEN

void appSetUpdateFrameRate(App* _app, int32_t fps)
{
	_app->m_frameRate = fps;
}

int appRun(App* _app, int _argc, const char* const* _argv)
{
#if RTM_PLATFORM_EMSCRIPTEN
	s_app = _app;
	_app->init(_argc, _argv);
	emscripten_set_main_loop(&updateApp, -1, 1);
#else // RTM_PLATFORM_EMSCRIPTEN
	appInit(_app, _argc, _argv);

	FrameStep fs;
	while (processEvents(_app))
	{
		if (_app->m_frameRate != fs.frameRate())
			fs.setFrameRate(_app->m_frameRate);

		while (fs.update())
		{
			float time = fs.step();
			appUpdate(_app, time);
		}

		appDraw(_app, fs.alpha());

		if (_app->m_width && _app->m_height)
			appDrawGUI(_app);

		appFrame();

		if (g_next_app)
		{
			appShutDown(_app);

			_app = g_next_app;
			g_next_app = 0;

			appInit(_app, _argc, _argv);
		}

		s_commChannel.frame();
	}
#endif // RTM_PLATFORM_EMSCRIPTEN

	appShutDown(_app);

	return 0;
}

int rapp_main(int _argc, const char* const* _argv)
{
	rtmLibInterface libInterface;
	libInterface.m_error	= rtm::rbaseGetErrorHandler();
	libInterface.m_memory	= rtm::rbaseGetMemoryManager();

	rapp::init(&libInterface);

	int ret = rapp::appRun(rapp::appGetRegistered()[0], _argc, _argv);
	rapp::shutDown();

	return ret;
}

} // namespace rapp
