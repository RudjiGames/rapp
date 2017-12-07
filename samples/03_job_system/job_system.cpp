//--------------------------------------------------------------------------//
/// Copyright (c) 2017 Milos Tosic. All Rights Reserved.                   ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <job_system_pch.h>

#include <atomic>
static std::atomic<int> s_numTiles = 0;

struct JobSystemApp : public rapp::App
{
	RAPP_CLASS(JobSystemApp)

	virtual ~JobSystemApp() {}

	enum Mode
	{
		Serial,
		Parallel
	};

	Mode	m_runMode;

	struct Tile
	{
		uint32_t x, y, w, h, tw, th;
	};

	int init(int32_t /*_argc*/, const char* const* /*_argv*/, rtmLibInterface* /*_libInterface = 0*/)
	{
		static const rapp::InputBinding bindings[] =
		{
			{ NULL, "switch", 1, { rapp::KeyboardState::Key::KeyS,   rapp::KeyboardState::Modifier::LCtrl  }},
			{ NULL, "exit",   1, { rapp::KeyboardState::Key::KeyQ,   rapp::KeyboardState::Modifier::LCtrl  }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("bindings", bindings);
		rapp::cmdAdd("exit",         cmdExit, this);
		rapp::cmdAdd("switch", cmdSwitchMode, this);

		m_runMode = Mode::Parallel;
		return 0;
	}

	void suspend() {}
	void resume() {}

	void update(float /*_time*/)
	{
		const uint32_t s_width   = 2048;
		const uint32_t s_height  = 2048;
		const uint32_t s_tile    = 64;
		const uint32_t s_tileX   = s_width  / s_tile;
		const uint32_t s_tileY   = s_height / s_tile;

		static Tile s_tiles[s_tileX * s_tileY];
		for (uint32_t y=0; y<s_tileY; ++y)
		for (uint32_t x=0; x<s_tileX; ++x)
		{
			s_tiles[(y*s_tileX) + x].x  = x*s_tile;
			s_tiles[(y*s_tileX) + x].y  = y*s_tile;
			s_tiles[(y*s_tileX) + x].w  = s_tile;
			s_tiles[(y*s_tileX) + x].h  = s_tile;
			s_tiles[(y*s_tileX) + x].tw = s_width;
			s_tiles[(y*s_tileX) + x].th = s_height;
		}

		float startTime = rtm::CPU::time();

		s_numTiles = 0;

		switch (m_runMode)
		{
			case Serial:
				{
					for (uint32_t i=0; i<s_tileX*s_tileY; ++i)
						tileMandelbrot(&s_tiles[i]);
				};
				break;

			case Parallel:
				{
					rapp::JobHandle group = rapp::jobCreateGroup(tileMandelbrot, s_tiles, sizeof(Tile), s_tileX*s_tileY);
					rapp::jobRun(group);
					rapp::jobWait(group);
				};
				break;
		};

		float endTime = rtm::CPU::time();

		rtm::Console::custom(	0, 255, 0,
								m_runMode == Mode::Serial ? 0 : 1,
								"%s Mandelbrot set (%d x %d) took: %f ms in %d tiles   - (Ctrl+S to switch execution mode)\n",
								m_runMode == Mode::Serial ? "Serial" : "Parallel",
								s_width, s_height, (endTime-startTime)*1000.0f,
								s_tileX*s_tileY);
	}

	void draw()
	{
	}

	void shutDown()
	{
		rtm::Console::custom(255, 255, 0, 1, "Shutting down app\n", (uint32_t)rtm::Thread::getThreadID());
		rapp::inputRemoveBindings("bindings");
	}

	static void tileMandelbrot(void* _userData)
	{
		Tile* tile = (Tile*)_userData;
		++s_numTiles;

		for (uint32_t Y=tile->y; Y<tile->y+tile->h; ++Y)
		for (uint32_t X=tile->x; X<tile->x+tile->w; ++X)
		{
			float x = (float)Y * 2 / tile->tw - 1.5f;
			float y = (float)X * 2 / tile->th - 1;

			int iteration = 0;
			while (++iteration < 12)
			{
				const float nx = x*x - y*y;
				const float ny = 2.0f * x*y;
				x = nx;
				y = ny;
			}
		}
	}

	static int cmdExit(void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_2(_argv, _argc);
		App* app = (App*)_userData;
		app->quit();
		return 0;
	}

	static int cmdSwitchMode(void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_2(_argv, _argc);
		JobSystemApp* app = (JobSystemApp*)_userData;
		app->m_runMode = (app->m_runMode == Mode::Parallel) ? Mode::Serial : Mode::Parallel;
		return 0;
	}
};

RAPP_REGISTER(JobSystemApp, "Job system", "Example of using the job system");
