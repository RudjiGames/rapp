//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <job_system_pch.h>

struct JobSystemApp : public rapp::App
{
	RAPP_CLASS(JobSystemApp)

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
		rapp::cmdAdd("exit",         cmdExit);
		rapp::cmdAdd("switch", cmdSwitchMode);

		m_runMode = Mode::Parallel;
		return 0;
	}

	void suspend() {}
	void resume() {}

	static const uint32_t s_width	= 2048;
	static const uint32_t s_height	= 2048;
	static const uint32_t s_tile	= 64;
	static const uint32_t s_tileX	= s_width / s_tile;
	static const uint32_t s_tileY	= s_height / s_tile;

	void update(float /*_time*/)
	{
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

		switch (m_runMode)
		{
			case Serial:
				{
					tileMandelbrot(s_tiles, 0, s_tileX * s_tileY);
				};
				break;

			case Parallel:
				{
					rapp::JobHandle group = rapp::jobCreateGroup(tileMandelbrot, s_tiles, sizeof(Tile), s_tileX * s_tileY, false);
					rapp::jobRun(group);
					rapp::jobWait(group);
					rapp::jobDestroy(group);
				};
				break;
		};

		float endTime = rtm::CPU::time();

		rtm::Console::rgb(	0, 255, 0,
								"%s Mandelbrot set (%d x %d) took: %f ms in %d tiles   - (Ctrl+S to switch execution mode)\n",
								m_runMode == Mode::Serial ? "Serial" : "Parallel",
								s_width, s_height, (endTime-startTime)*1000.0f,
								s_tileX*s_tileY);
	}

	void draw(float /*_alpha*/)
	{
	}

	void shutDown()
	{
		rtm::Console::rgb(255, 255, 0, "Shutting down app\n");
		rapp::inputRemoveBindings("bindings");
	}

	static void tileMandelbrot(void* _userData, uint32_t _start, uint32_t _end)
	{
		for (uint32_t range=_start; range<_end; ++range)
		{
			Tile* tile = &((Tile*)_userData)[range];

			for (uint32_t Y = tile->y; Y < tile->y + tile->h; ++Y)
			for (uint32_t X = tile->x; X < tile->x + tile->w; ++X)
			{
				float x = (float)Y * 2 / tile->tw - 1.5f;
				float y = (float)X * 2 / tile->th - 1;

				int iteration = 0;
				while (++iteration < 12)
				{
					const float nx = x * x - y * y;
					const float ny = 2.0f * x * y;
					x = nx;
					y = ny;
				}
			}
		}
	}

	static int cmdExit(App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		_app->quit();
		return 0;
	}

	static int cmdSwitchMode(App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		JobSystemApp* app = (JobSystemApp*)_app;
		app->m_runMode = (app->m_runMode == Mode::Parallel) ? Mode::Serial : Mode::Parallel;
		return 0;
	}
};

RAPP_REGISTER(JobSystemApp, "Job system", "Example of using the job system");
