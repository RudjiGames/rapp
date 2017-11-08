//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/src/rapp_timer.h>
#include <rapp/src/cmd.h>
#include <rapp/src/entry_p.h>

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
		Shutdown,
		RunFunc,

		Count
	};
};

static rtm::CommandBuffer	s_commChannel;		// rapp_main to app class thread communication

rtm_vector<App*>& getApps()
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
{
	appRegister(this);
}

void appRegister(App* _app)
{
	getApps().push_back(_app);
}

///
App* appGet(uint32_t _index)
{
	if (_index < appGetCount())
		return getApps()[_index];
	return 0;
}

///
uint32_t appGetCount()
{
	return (uint32_t)getApps().size();
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
		s_commChannel.frame();
	}

	appShutDown(_app);

	return 0;
}

int rapp_main(int _argc, char** _argv)
{
	rtmLibInterface* errorHandler = 0;
	//&RAPP_INSTANCE(CmdLineApp)
	rapp::init(errorHandler);
	int ret = rapp::appRun(rapp::getApps()[0], _argc, _argv);
	rapp::shutDown();

	return ret;
}

} // namespace rapp
