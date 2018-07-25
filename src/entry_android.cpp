//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/inc/rapp.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_ANDROID

#if RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif // RAPP_WITH_BGFX

#include <stdio.h>

#include <android/input.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/window.h>
#include <android_native_app_glue.h>

extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <android_native_app_glue.c>
#pragma GCC diagnostic pop
} // extern "C"

namespace rapp
{
	inline void androidSetWindow(::ANativeWindow* _window)
	{
		RTM_UNUSED(_window);
#if RAPP_WITH_BGFX
		bgfx::PlatformData pd;
		pd.ndt          = NULL;
		pd.nwh          = _window;
		pd.context      = NULL;
		pd.backBuffer   = NULL;
		pd.backBufferDS = NULL;
		bgfx::setPlatformData(pd);
#endif // RAPP_WITH_BGFX
	}

	struct GamepadRemap
	{
		uint16_t				m_keyCode;
		GamepadState::Buttons	m_key;
	};

	static GamepadRemap s_gamepadRemap[] =
	{
		{ AKEYCODE_DPAD_UP,       GamepadState::Buttons::Up        },
		{ AKEYCODE_DPAD_DOWN,     GamepadState::Buttons::Down      },
		{ AKEYCODE_DPAD_LEFT,     GamepadState::Buttons::Left      },
		{ AKEYCODE_DPAD_RIGHT,    GamepadState::Buttons::Right     },
		{ AKEYCODE_BUTTON_START,  GamepadState::Buttons::Start     },
		{ AKEYCODE_BACK,          GamepadState::Buttons::Back      },
		{ AKEYCODE_BUTTON_THUMBL, GamepadState::Buttons::LThumb    },
		{ AKEYCODE_BUTTON_THUMBR, GamepadState::Buttons::RThumb    },
		{ AKEYCODE_BUTTON_L1,     GamepadState::Buttons::LShoulder },
		{ AKEYCODE_BUTTON_R1,     GamepadState::Buttons::RShoulder },
		{ AKEYCODE_GUIDE,         GamepadState::Buttons::Guide     },
		{ AKEYCODE_BUTTON_A,      GamepadState::Buttons::A         },
		{ AKEYCODE_BUTTON_B,      GamepadState::Buttons::B         },
		{ AKEYCODE_BUTTON_X,      GamepadState::Buttons::X         },
		{ AKEYCODE_BUTTON_Y,      GamepadState::Buttons::Y         },
	};

	struct GamepadAxisRemap
	{
		int32_t m_event;
		GamepadAxis::Enum m_axis;
		bool m_convert;
	};

	static GamepadAxisRemap s_translateAxis[] =
	{
		{ AMOTION_EVENT_AXIS_X,        GamepadAxis::LeftX,  false },
		{ AMOTION_EVENT_AXIS_Y,        GamepadAxis::LeftY,  false },
		{ AMOTION_EVENT_AXIS_LTRIGGER, GamepadAxis::LeftZ,  false },
		{ AMOTION_EVENT_AXIS_Z,        GamepadAxis::RightX, true  },
		{ AMOTION_EVENT_AXIS_RZ,       GamepadAxis::RightY, false },
		{ AMOTION_EVENT_AXIS_RTRIGGER, GamepadAxis::RightZ, false },
	};

	enum Command : uint8_t
	{
		None,
		RunFunc,
		Quit
	};

	static rtm::SpScQueue<>		s_channel(1024);

	#define RAPP_CMD_READ(_type, _name)		\
		_type _name;						\
		while (!s_channel.read(&_name))

	#define RAPP_CMD_WRITE(_val)			\
		while (!s_channel.write(_val));

	struct MainThreadEntry
	{
		int m_argc;
		char** m_argv;

		static int32_t threadFunc(void* _userData);
	};

	struct Context
	{
		Context()
			: m_window(NULL)
			, m_count(0)
		{
			memset(m_value, 0, sizeof(m_value) );

			// Deadzone values from xinput.h
			m_deadzone[GamepadAxis::LeftX ] =
			m_deadzone[GamepadAxis::LeftY ] = 7849;
			m_deadzone[GamepadAxis::RightX] =
			m_deadzone[GamepadAxis::RightY] = 8689;
			m_deadzone[GamepadAxis::LeftZ ] =
			m_deadzone[GamepadAxis::RightZ] = 30;
		}

		void run(android_app* _app)
		{
			m_app = _app;
			m_app->userData = (void*)this;
			m_app->onAppCmd = onAppCmdCB;
			m_app->onInputEvent = onInputEventCB;
			ANativeActivity_setWindowFlags(m_app->activity, 0
				| AWINDOW_FLAG_FULLSCREEN
				| AWINDOW_FLAG_KEEP_SCREEN_ON
				, 0
				);

			const char* argv[1] = { "android.so" };
			m_mte.m_argc = 1;
			m_mte.m_argv = const_cast<char**>(argv);
			
			while (0 == m_app->destroyRequested)
			{
				uintptr_t cmd = 0;
				if (s_channel.read(&cmd))
				{
					switch (cmd)
					{
						case Command::RunFunc:
							{
								RAPP_CMD_READ(rapp::ThreadFn,	fn);
								RAPP_CMD_READ(void*,				userData);

								fn(userData);
							}
							break;

						case Command::Quit:
							{
								RAPP_CMD_READ(rapp::App*, app);
								app->quit();
							}
							break;

					default:
						RTM_ASSERT(false, "Invalid command!");
					};
				}

				int32_t num;
				android_poll_source* source;
				/*int32_t id =*/ ALooper_pollAll(-1, NULL, &num, (void**)&source);

				if (NULL != source)
				{
					source->process(m_app, source);
				}
			}

			m_thread.stop();
		}

		void onAppCmd(int32_t _cmd)
		{
			switch (_cmd)
			{
				case APP_CMD_INPUT_CHANGED:
					// Command from main thread: the AInputQueue has changed.  Upon processing
					// this command, android_app->inputQueue will be updated to the new queue
					// (or NULL).
					break;

				case APP_CMD_INIT_WINDOW:
					// Command from main thread: a new ANativeWindow is ready for use.  Upon
					// receiving this command, android_app->window will contain the new window
					// surface.
					if (m_window == NULL)
					{
						m_window = m_app->window;
#if RAPP_WITH_BGFX
						androidSetWindow(m_window);
#endif // RAPP_WITH_BGFX

						int32_t width  = ANativeWindow_getWidth(m_window);
						int32_t height = ANativeWindow_getHeight(m_window);

						RAPP_DBG("ANativeWindow width %d, height %d", width, height);
						WindowHandle defaultWindow = { 0 };
						m_eventQueue.postSizeEvent(defaultWindow, width, height);

						m_thread.start(MainThreadEntry::threadFunc, &m_mte);
					}
					break;

				case APP_CMD_TERM_WINDOW:
					// Command from main thread: the existing ANativeWindow needs to be
					// terminated.  Upon receiving this command, android_app->window still
					// contains the existing window; after calling android_app_exec_cmd
					// it will be set to NULL.
					break;

				case APP_CMD_WINDOW_RESIZED:
					// Command from main thread: the current ANativeWindow has been resized.
					// Please redraw with its new size.
					break;

				case APP_CMD_WINDOW_REDRAW_NEEDED:
					// Command from main thread: the system needs that the current ANativeWindow
					// be redrawn.  You should redraw the window before handing this to
					// android_app_exec_cmd() in order to avoid transient drawing glitches.
					break;

				case APP_CMD_CONTENT_RECT_CHANGED:
					// Command from main thread: the content area of the window has changed,
					// such as from the soft input window being shown or hidden.  You can
					// find the new content rect in android_app::contentRect.
					break;

				case APP_CMD_GAINED_FOCUS:
					// Command from main thread: the app's activity window has gained
					// input focus.
					break;

				case APP_CMD_LOST_FOCUS:
					// Command from main thread: the app's activity window has lost
					// input focus.
					break;

				case APP_CMD_CONFIG_CHANGED:
					// Command from main thread: the current device configuration has changed.
					break;

				case APP_CMD_LOW_MEMORY:
					// Command from main thread: the system is running low on memory.
					// Try to reduce your memory use.
					break;

				case APP_CMD_START:
					// Command from main thread: the app's activity has been started.
					break;

				case APP_CMD_RESUME:
					// Command from main thread: the app's activity has been resumed.
					break;

				case APP_CMD_SAVE_STATE:
					// Command from main thread: the app should generate a new saved state
					// for itself, to restore from later if needed.  If you have saved state,
					// allocate it with malloc and place it in android_app.savedState with
					// the size in android_app.savedStateSize.  The will be freed for you
					// later.
					break;

				case APP_CMD_PAUSE:
					// Command from main thread: the app's activity has been paused.
					break;

				case APP_CMD_STOP:
					// Command from main thread: the app's activity has been stopped.
					break;

				case APP_CMD_DESTROY:
					// Command from main thread: the app's activity is being destroyed,
					// and waiting for the app thread to clean up and exit before proceeding.
					m_eventQueue.postExitEvent();
					break;
			}
		}

		bool filter(GamepadAxis::Enum _axis, int32_t* _value)
		{
			const int32_t old = m_value[_axis];
			const int32_t deadzone = m_deadzone[_axis];
			int32_t value = *_value;
			value = value > deadzone || value < -deadzone ? value : 0;
			m_value[_axis] = value;
			*_value = value;
			return old != value;
		}

		int32_t onInputEvent(AInputEvent* _event)
		{
			WindowHandle  defaultWindow = { 0 };
			GamepadHandle handle        = { 0 };
			const int32_t type       = AInputEvent_getType(_event);
			const int32_t source     = AInputEvent_getSource(_event);
			const int32_t actionBits = AMotionEvent_getAction(_event);

			switch (type)
			{
			case AINPUT_EVENT_TYPE_MOTION:
				{
					if (0 != (source & (AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) ) )
					{
						for (uint32_t ii = 0; ii < RTM_NUM_ELEMENTS(s_translateAxis); ++ii)
						{
							const float fval = AMotionEvent_getAxisValue(_event, s_translateAxis[ii].m_event, 0);
							int32_t value = int32_t( (s_translateAxis[ii].m_convert ? fval * 2.0f + 1.0f : fval) * INT16_MAX);
							GamepadAxis::Enum axis = s_translateAxis[ii].m_axis;
							if (filter(axis, &value) )
							{
								m_eventQueue.postAxisEvent(defaultWindow, handle, axis, value);
							}
						}

						return 1;
					}
					else
					{
						float mx = AMotionEvent_getX(_event, 0);
						float my = AMotionEvent_getY(_event, 0);
						int32_t count = AMotionEvent_getPointerCount(_event);

						int32_t action = (actionBits & AMOTION_EVENT_ACTION_MASK);
						int32_t index  = (actionBits & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

						count = m_count;

						switch (action)
						{
						case AMOTION_EVENT_ACTION_DOWN:
						case AMOTION_EVENT_ACTION_POINTER_DOWN:
							m_count++;
							break;

						case AMOTION_EVENT_ACTION_UP:
						case AMOTION_EVENT_ACTION_POINTER_UP:
							m_count--;
							break;

						default:
							break;
						}

						if (count != m_count)
						{
							//m_eventQueue.postMouseEvent(defaultWindow
							//	, (int32_t)mx
							//	, (int32_t)my
							//	, 0
							//	, 1 == count ? MouseState::Button::Left : MouseState::Button::Right
							//	, false
							//	);

							if (0 != m_count)
							{
								//m_eventQueue.postMouseEvent(defaultWindow
								//	, (int32_t)mx
								//	, (int32_t)my
								//	, 0
								//	, 1 == m_count ? MouseState::Button::Left : MouseState::Button::Right
								//	, true
								//	);
							}
						}

						switch (action)
						{
						case AMOTION_EVENT_ACTION_MOVE:
							if (0 == index)
							{
								//m_eventQueue.postMouseEvent(defaultWindow
								//	, (int32_t)mx
								//	, (int32_t)my
								//	, 0
								//	);
							}
							break;

						default:
							break;
						}
					}
				}
				break;

			case AINPUT_EVENT_TYPE_KEY:
				{
					int32_t keyCode = AKeyEvent_getKeyCode(_event);

					if (0 != (source & (AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) ) )
					{
						for (uint32_t jj = 0; jj < RTM_NUM_ELEMENTS(s_gamepadRemap); ++jj)
						{
							if (keyCode == s_gamepadRemap[jj].m_keyCode)
							{
								//m_eventQueue.postKeyEvent(defaultWindow, s_gamepadRemap[jj].m_key, 0, actionBits == AKEY_EVENT_ACTION_DOWN);
								break;
							}
						}
					}

					return 1;
				}
				break;

			default:
				RAPP_DBG("type %d", type);
				break;
			}

			return 0;
		}

		static void onAppCmdCB(struct android_app* _app, int32_t _cmd)
		{
			Context* self = (Context*)_app->userData;
			self->onAppCmd(_cmd);
		}

		static int32_t onInputEventCB(struct android_app* _app, AInputEvent* _event)
		{
			Context* self = (Context*)_app->userData;
			return self->onInputEvent(_event);
		}

		MainThreadEntry m_mte;
		rtm::Thread m_thread;

		EventQueue m_eventQueue;

		ANativeWindow* m_window;
		android_app* m_app;

		int32_t m_count;
		int32_t m_value[GamepadAxis::Count];
		int32_t m_deadzone[GamepadAxis::Count];
	};

	static Context s_ctx;

	const Event* poll()
	{
		return s_ctx.m_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		return s_ctx.m_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		s_ctx.m_eventQueue.release(_event);
	}

	void appRunOnMainThread(ThreadFn _fn, void* _userData)
	{
		RAPP_CMD_WRITE(Command::RunFunc);
		RAPP_CMD_WRITE(_fn);
		RAPP_CMD_WRITE(_userData);
	}

	void windowGetDefaultSize(uint32_t* _width, uint32_t* _height)
	{
		RTM_ASSERT(_width, "");
		RTM_ASSERT(_height, "");
		*_width		= 1920;
		*_height	= 1080;
	}

	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags, const char* _title)
	{
		RTM_UNUSED_6(_x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		RTM_UNUSED_3(_handle, _x, _y);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		RTM_UNUSED_3(_handle, _width, _height);
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		RTM_UNUSED_2(_handle, _title);
	}

	void windowToggleFrame(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		RTM_UNUSED_2(_handle, _lock);
	}

	int32_t MainThreadEntry::threadFunc(void* _userData)
	{
		int32_t result = chdir("/sdcard/rapp/");
		RTM_ASSERT(0 == result, "Failed to chdir to dir. android.permission.WRITE_EXTERNAL_STORAGE?", errno);

		MainThreadEntry* self = (MainThreadEntry*)_userData;
		result = main(self->m_argc, self->m_argv);
//		PostMessage(s_ctx.m_hwnd, WM_QUIT, 0, 0);
		return result;
	}

} // namespace rapp

extern "C" void android_main(android_app* _app)
{
	using namespace rapp;
	s_ctx.run(_app);
}

#endif // RTM_PLATFORM_ANDROID
