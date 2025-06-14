//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/input.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_WINDOWS

#ifdef RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif // RAPP_WITH_BGFX

#include <windowsx.h>
#include <xinput.h>
#include <objbase.h>

#ifndef XINPUT_GAMEPAD_GUIDE
#	define XINPUT_GAMEPAD_GUIDE 0x400
#endif // XINPUT_GAMEPAD_GUIDE

#ifndef XINPUT_DLL_A
#	define XINPUT_DLL_A "xinput.dll"
#endif // XINPUT_DLL_A

namespace rapp
{
	struct WindowInfo
	{
		App*		m_app;
		HWND		m_window;
		uint32_t	m_flags;
		uint32_t	m_handle;
	};

	int32_t rappMain();

	typedef DWORD (WINAPI* PFN_XINPUT_GET_STATE)(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef void  (WINAPI* PFN_XINPUT_ENABLE)(BOOL enable); // 1.4+

	PFN_XINPUT_GET_STATE XInputGetState;
	PFN_XINPUT_ENABLE    XInputEnableFn;

	struct XInputRemap
	{
		uint16_t			m_bit;
		GamepadButton::Enum	m_button;
	};

	static XInputRemap s_xinputRemap[] =
	{
		{ XINPUT_GAMEPAD_DPAD_UP,        GamepadButton::Up	      },
		{ XINPUT_GAMEPAD_DPAD_DOWN,      GamepadButton::Down      },
		{ XINPUT_GAMEPAD_DPAD_LEFT,      GamepadButton::Left      },
		{ XINPUT_GAMEPAD_DPAD_RIGHT,     GamepadButton::Right     },
		{ XINPUT_GAMEPAD_START,          GamepadButton::Start     },
		{ XINPUT_GAMEPAD_BACK,           GamepadButton::Back      },
		{ XINPUT_GAMEPAD_LEFT_THUMB,     GamepadButton::LThumb    },
		{ XINPUT_GAMEPAD_RIGHT_THUMB,    GamepadButton::RThumb    },
		{ XINPUT_GAMEPAD_LEFT_SHOULDER,  GamepadButton::LShoulder },
		{ XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadButton::RShoulder },
		{ XINPUT_GAMEPAD_GUIDE,          GamepadButton::Guide     },
		{ XINPUT_GAMEPAD_A,              GamepadButton::A         },
		{ XINPUT_GAMEPAD_B,              GamepadButton::B         },
		{ XINPUT_GAMEPAD_X,              GamepadButton::X         },
		{ XINPUT_GAMEPAD_Y,              GamepadButton::Y         },
	};

	struct XInput
	{
		XInput()
			: m_xinputdll(NULL)
		{
			memset(m_connected, 0, sizeof(m_connected) );
			memset(m_state, 0, sizeof(m_state) );

			m_deadzone[GamepadAxis::LeftX ] =
			m_deadzone[GamepadAxis::LeftY ] = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			m_deadzone[GamepadAxis::RightX] =
			m_deadzone[GamepadAxis::RightY] = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			m_deadzone[GamepadAxis::LeftZ ] =
			m_deadzone[GamepadAxis::RightZ] = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

			memset(m_flip, 1, sizeof(m_flip) );
			m_flip[GamepadAxis::LeftY ] =
			m_flip[GamepadAxis::RightY] = -1;
		}

		void init()
		{
			m_xinputdll = LoadLibrary(XINPUT_DLL_A);

			if (NULL != m_xinputdll)
			{
				*(FARPROC*)&XInputGetState = ::GetProcAddress(m_xinputdll, "XInputGetState");
//				*(FARPROC*)&XInputEnableFn   = ::GetProcAddress(m_xinputdll, "XInputEnableFn"  );

				if (NULL == XInputGetState)
				{
					shutdown();
				}
			}
		}

		void shutdown()
		{
			if (NULL != m_xinputdll)
			{
				FreeLibrary(m_xinputdll);
				m_xinputdll = NULL;
			}
		}

		bool filter(GamepadAxis::Enum _axis, int32_t _old, int32_t* _value)
		{
			const int32_t deadzone = m_deadzone[_axis];
			int32_t value = *_value;
			value = value > deadzone || value < -deadzone ? value : 0;
			*_value = value * m_flip[_axis];
			return _old != value;
		}

		void update(EventQueue& _eventQueue)
		{
			if (NULL == m_xinputdll)
			{
				return;
			}

			for (uint32_t ii = 0; ii < RTM_NUM_ELEMENTS(m_state); ++ii)
			{
				GamepadHandle handle = { static_cast<uint16_t>(ii) };
				XINPUT_STATE state;
				DWORD result = XInputGetState(ii, &state);

				bool connected = ERROR_SUCCESS == result;
				if (connected != m_connected[ii])
				{
					_eventQueue.postGamepadEvent(rapp::kDefaultWindowHandle, handle, connected);
				}

				m_connected[ii] = connected;

				if (connected
				&&  m_state[ii].dwPacketNumber != state.dwPacketNumber)
				{
					XINPUT_GAMEPAD& gamepad = m_state[ii].Gamepad;
					const uint16_t changed = gamepad.wButtons ^ state.Gamepad.wButtons;
					const uint16_t current = gamepad.wButtons;
					if (0 != changed)
					{
						for (uint32_t jj = 0; jj < RTM_NUM_ELEMENTS(s_xinputRemap); ++jj)
						{
							uint16_t bit = s_xinputRemap[jj].m_bit;
							if (bit & changed)
							{
								_eventQueue.postGamepadButtonsEvent(rapp::kDefaultWindowHandle, handle, s_xinputRemap[jj].m_button, !!(0 == (current & bit)));
							}
						}

						gamepad.wButtons = state.Gamepad.wButtons;
					}

					if (gamepad.bLeftTrigger != state.Gamepad.bLeftTrigger)
					{
						int32_t value = state.Gamepad.bLeftTrigger;
						if (filter(GamepadAxis::LeftZ, gamepad.bLeftTrigger, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::LeftZ, value);
						}

						gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
					}

					if (gamepad.bRightTrigger != state.Gamepad.bRightTrigger)
					{
						int32_t value = state.Gamepad.bRightTrigger;
						if (filter(GamepadAxis::RightZ, gamepad.bRightTrigger, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::RightZ, value);
						}

						gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
					}

					if (gamepad.sThumbLX != state.Gamepad.sThumbLX)
					{
						int32_t value = state.Gamepad.sThumbLX;
						if (filter(GamepadAxis::LeftX, gamepad.sThumbLX, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::LeftX, value);
						}

						gamepad.sThumbLX = state.Gamepad.sThumbLX;
					}

					if (gamepad.sThumbLY != state.Gamepad.sThumbLY)
					{
						int32_t value = state.Gamepad.sThumbLY;
						if (filter(GamepadAxis::LeftY, gamepad.sThumbLY, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::LeftY, value);
						}

						gamepad.sThumbLY = state.Gamepad.sThumbLY;
					}

					if (gamepad.sThumbRX != state.Gamepad.sThumbRX)
					{
						int32_t value = state.Gamepad.sThumbRX;
						if (filter(GamepadAxis::RightX, gamepad.sThumbRX, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::RightX, value);
						}

						gamepad.sThumbRX = state.Gamepad.sThumbRX;
					}

					if (gamepad.sThumbRY != state.Gamepad.sThumbRY)
					{
						int32_t value = state.Gamepad.sThumbRY;
						if (filter(GamepadAxis::RightY, gamepad.sThumbRY, &value) )
						{
							_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::RightY, value);
						}

						gamepad.sThumbRY = state.Gamepad.sThumbRY;
					}
				}
			}
		}

		HMODULE m_xinputdll;

		int32_t m_deadzone[GamepadAxis::Count];
		int8_t m_flip[GamepadAxis::Count];
		XINPUT_STATE m_state[ENTRY_CONFIG_MAX_GAMEPADS];
		bool m_connected[ENTRY_CONFIG_MAX_GAMEPADS];
	};

	XInput s_xinput;

	enum
	{
		WM_USER_WINDOW_CREATE = WM_USER,
		WM_USER_WINDOW_DESTROY,
		WM_USER_WINDOW_SET_TITLE,
		WM_USER_WINDOW_SET_POS,
		WM_USER_WINDOW_SET_SIZE,
		WM_USER_WINDOW_TOGGLE_FRAME,
		WM_USER_WINDOW_MOUSE_LOCK,
		WM_USER_CALL_FUNC,
		WM_USER_CALL_KEY_DOWN,
		WM_USER_CALL_KEY_UP
	};

	struct TranslateKeyModifiers
	{
		int						m_vk;
		KeyboardModifier::Enum	m_modifier;
	};

	static const TranslateKeyModifiers s_translateKeyModifiers[8] =
	{
		{ VK_LMENU,    KeyboardModifier::LAlt   },
		{ VK_RMENU,    KeyboardModifier::RAlt   },
		{ VK_LCONTROL, KeyboardModifier::LCtrl  },
		{ VK_RCONTROL, KeyboardModifier::RCtrl  },
		{ VK_LSHIFT,   KeyboardModifier::LShift },
		{ VK_RSHIFT,   KeyboardModifier::RShift },
		{ VK_LWIN,     KeyboardModifier::LMeta  },
		{ VK_RWIN,     KeyboardModifier::RMeta  },
	};

	static uint8_t translateKeyModifiers()
	{
		uint8_t modifiers = 0;
		for (uint32_t ii = 0; ii < RTM_NUM_ELEMENTS(s_translateKeyModifiers); ++ii)
		{
			const TranslateKeyModifiers& tkm = s_translateKeyModifiers[ii];
			modifiers |= 0 > GetKeyState(tkm.m_vk) ? tkm.m_modifier : KeyboardModifier::None;
		}
		return modifiers;
	}

	static const TranslateKeyModifiers s_translateKeyModifiersConsole[5] =
	{
		{ RIGHT_ALT_PRESSED,   KeyboardModifier::RAlt   },
		{ LEFT_ALT_PRESSED,    KeyboardModifier::LAlt   },
		{ LEFT_CTRL_PRESSED,   KeyboardModifier::LCtrl  },
		{ RIGHT_CTRL_PRESSED,  KeyboardModifier::RCtrl  },
		{ SHIFT_PRESSED,       KeyboardModifier::LShift },
//		{ SHIFT_PRESSED,       KeyboardModifier::RShift },
//		{ ,                    KeyboardModifier::LMeta  },
//		{ ,                    KeyboardModifier::RMeta  },
	};

	static uint8_t translateKeyModifiersConsole(uint32_t _modifiers)
	{
		uint8_t modifiers = 0;
		for (uint32_t ii=0; ii<RTM_NUM_ELEMENTS(s_translateKeyModifiersConsole); ++ii)
		{
			const TranslateKeyModifiers& tkm = s_translateKeyModifiersConsole[ii];
			modifiers |= _modifiers & tkm.m_vk ? tkm.m_modifier : KeyboardModifier::None;
		}
		return modifiers;
	}

	static uint8_t s_translateKey[256];

	static KeyboardKey::Enum translateKey(WPARAM _wparam)
	{
		return (KeyboardKey::Enum)s_translateKey[_wparam&0xff];
	}

	struct MainThreadEntry
	{
		int m_argc;
		const char* const* m_argv;
		int32_t m_result;

		static int32_t threadFunc(void* _userData);
	};

	struct Msg
	{
		Msg()
			: m_x(0)
			, m_y(0)
			, m_width(0)
			, m_height(0)
			, m_flags(0)
		{
			m_title[0] = '\0';
		}

		App*      m_app;
		int32_t   m_x;
		int32_t   m_y;
		uint32_t  m_width;
		uint32_t  m_height;
		uint32_t  m_flags;
		char      m_title[256];
	};

	static void mouseCapture(HWND _hwnd, bool _capture)
	{
		if (_capture)
		{
			SetCapture(_hwnd);
		}
		else
		{
			ReleaseCapture();
		}
	}

	struct Context
	{
		Context()
			: m_mz(0)
			, m_consoleMouseButtonState(0)
			, m_frame(true)
			, m_mouseLock(NULL)
			, m_init(false)
			, m_exit(false)
		{
			memset(s_translateKey, 0, sizeof(s_translateKey) );
			s_translateKey[VK_ESCAPE]			= KeyboardKey::Esc;
			s_translateKey[VK_RETURN]			= KeyboardKey::Return;
			s_translateKey[VK_TAB]				= KeyboardKey::Tab;
			s_translateKey[VK_BACK]				= KeyboardKey::Backspace;
			s_translateKey[VK_SPACE]			= KeyboardKey::Space;
			s_translateKey[VK_UP]				= KeyboardKey::Up;
			s_translateKey[VK_DOWN]				= KeyboardKey::Down;
			s_translateKey[VK_LEFT]				= KeyboardKey::Left;
			s_translateKey[VK_RIGHT]			= KeyboardKey::Right;
			s_translateKey[VK_INSERT]			= KeyboardKey::Insert;
			s_translateKey[VK_DELETE]			= KeyboardKey::Delete;
			s_translateKey[VK_HOME]				= KeyboardKey::Home;
			s_translateKey[VK_END]				= KeyboardKey::End;
			s_translateKey[VK_PRIOR]			= KeyboardKey::PageUp;
			s_translateKey[VK_NEXT]				= KeyboardKey::PageDown;
			s_translateKey[VK_SNAPSHOT]			= KeyboardKey::Print;
			s_translateKey[VK_OEM_PLUS]			= KeyboardKey::Plus;
			s_translateKey[VK_OEM_MINUS]		= KeyboardKey::Minus;
			s_translateKey[VK_OEM_NEC_EQUAL]	= KeyboardKey::Equal;
			s_translateKey[VK_OEM_4]			= KeyboardKey::LeftBracket;
			s_translateKey[VK_OEM_6]			= KeyboardKey::RightBracket;
			s_translateKey[VK_OEM_1]			= KeyboardKey::Semicolon;
			s_translateKey[VK_OEM_7]			= KeyboardKey::Quote;
			s_translateKey[VK_OEM_COMMA]		= KeyboardKey::Comma;
			s_translateKey[VK_OEM_PERIOD]		= KeyboardKey::Period;
			s_translateKey[VK_OEM_2]			= KeyboardKey::Slash;
			s_translateKey[VK_OEM_5]			= KeyboardKey::Backslash;
			s_translateKey[VK_OEM_3]			= KeyboardKey::Tilde;
			s_translateKey[VK_F1]				= KeyboardKey::F1;
			s_translateKey[VK_F2]				= KeyboardKey::F2;
			s_translateKey[VK_F3]				= KeyboardKey::F3;
			s_translateKey[VK_F4]				= KeyboardKey::F4;
			s_translateKey[VK_F5]				= KeyboardKey::F5;
			s_translateKey[VK_F6]				= KeyboardKey::F6;
			s_translateKey[VK_F7]				= KeyboardKey::F7;
			s_translateKey[VK_F8]				= KeyboardKey::F8;
			s_translateKey[VK_F9]				= KeyboardKey::F9;
			s_translateKey[VK_F10]				= KeyboardKey::F10;
			s_translateKey[VK_F11]				= KeyboardKey::F11;
			s_translateKey[VK_F12]				= KeyboardKey::F12;
			s_translateKey[VK_NUMPAD0]			= KeyboardKey::NumPad0;
			s_translateKey[VK_NUMPAD1]			= KeyboardKey::NumPad1;
			s_translateKey[VK_NUMPAD2]			= KeyboardKey::NumPad2;
			s_translateKey[VK_NUMPAD3]			= KeyboardKey::NumPad3;
			s_translateKey[VK_NUMPAD4]			= KeyboardKey::NumPad4;
			s_translateKey[VK_NUMPAD5]			= KeyboardKey::NumPad5;
			s_translateKey[VK_NUMPAD6]			= KeyboardKey::NumPad6;
			s_translateKey[VK_NUMPAD7]			= KeyboardKey::NumPad7;
			s_translateKey[VK_NUMPAD8]			= KeyboardKey::NumPad8;
			s_translateKey[VK_NUMPAD9]			= KeyboardKey::NumPad9;
			s_translateKey[uint8_t('0')]		= KeyboardKey::Key0;
			s_translateKey[uint8_t('1')]		= KeyboardKey::Key1;
			s_translateKey[uint8_t('2')]		= KeyboardKey::Key2;
			s_translateKey[uint8_t('3')]		= KeyboardKey::Key3;
			s_translateKey[uint8_t('4')]		= KeyboardKey::Key4;
			s_translateKey[uint8_t('5')]		= KeyboardKey::Key5;
			s_translateKey[uint8_t('6')]		= KeyboardKey::Key6;
			s_translateKey[uint8_t('7')]		= KeyboardKey::Key7;
			s_translateKey[uint8_t('8')]		= KeyboardKey::Key8;
			s_translateKey[uint8_t('9')]		= KeyboardKey::Key9;
			s_translateKey[uint8_t('A')]		= KeyboardKey::KeyA;
			s_translateKey[uint8_t('B')]		= KeyboardKey::KeyB;
			s_translateKey[uint8_t('C')]		= KeyboardKey::KeyC;
			s_translateKey[uint8_t('D')]		= KeyboardKey::KeyD;
			s_translateKey[uint8_t('E')]		= KeyboardKey::KeyE;
			s_translateKey[uint8_t('F')]		= KeyboardKey::KeyF;
			s_translateKey[uint8_t('G')]		= KeyboardKey::KeyG;
			s_translateKey[uint8_t('H')]		= KeyboardKey::KeyH;
			s_translateKey[uint8_t('I')]		= KeyboardKey::KeyI;
			s_translateKey[uint8_t('J')]		= KeyboardKey::KeyJ;
			s_translateKey[uint8_t('K')]		= KeyboardKey::KeyK;
			s_translateKey[uint8_t('L')]		= KeyboardKey::KeyL;
			s_translateKey[uint8_t('M')]		= KeyboardKey::KeyM;
			s_translateKey[uint8_t('N')]		= KeyboardKey::KeyN;
			s_translateKey[uint8_t('O')]		= KeyboardKey::KeyO;
			s_translateKey[uint8_t('P')]		= KeyboardKey::KeyP;
			s_translateKey[uint8_t('Q')]		= KeyboardKey::KeyQ;
			s_translateKey[uint8_t('R')]		= KeyboardKey::KeyR;
			s_translateKey[uint8_t('S')]		= KeyboardKey::KeyS;
			s_translateKey[uint8_t('T')]		= KeyboardKey::KeyT;
			s_translateKey[uint8_t('U')]		= KeyboardKey::KeyU;
			s_translateKey[uint8_t('V')]		= KeyboardKey::KeyV;
			s_translateKey[uint8_t('W')]		= KeyboardKey::KeyW;
			s_translateKey[uint8_t('X')]		= KeyboardKey::KeyX;
			s_translateKey[uint8_t('Y')]		= KeyboardKey::KeyY;
			s_translateKey[uint8_t('Z')]		= KeyboardKey::KeyZ;
		}

		void winSetHwnd(::HWND _window)
		{
			RTM_UNUSED(_window);
#ifdef RAPP_WITH_BGFX
			bgfx::PlatformData pd;
			memset(&pd, 0, sizeof(pd));
			pd.nwh = _window;
			bgfx::setPlatformData(pd);
#endif // RAPP_WITH_BGFX
		}

		int32_t run(int _argc, const char* const* _argv)
		{
			CoInitialize(0);
			SetDllDirectory(".");

			s_xinput.init();

			HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);

			WNDCLASSEX wnd;
			memset(&wnd, 0, sizeof(wnd) );
			wnd.cbSize			= sizeof(wnd);
			wnd.style			= CS_HREDRAW | CS_VREDRAW;
			wnd.lpfnWndProc		= wndProc;
			wnd.hInstance		= instance;
			wnd.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
			wnd.hCursor			= LoadCursor(NULL, IDC_ARROW);
			wnd.lpszClassName	= "rapp";
			wnd.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);
			wnd.cbWndExtra		= 32;
			RegisterClassExA(&wnd);

			m_hwndRapp = CreateWindowEx(0, "rapp"
				, "rapp"
				, 0
				, 0
				, 0
				, 0
				, 0
				, HWND_MESSAGE
				, NULL
				, NULL
				, NULL
				);

			DWORD saveOldConsoleMode;
			GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &saveOldConsoleMode);
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS);

			m_width     = 0;
			m_height    = 0;
			m_oldWidth  = 0;
			m_oldHeight = 0;

			MainThreadEntry mte;
			mte.m_argc = _argc;
			mte.m_argv = _argv;

			rtm::Thread thread;
			thread.start(mte.threadFunc, &mte);

			m_init = true;

			MSG msg;
			msg.message = WM_NULL;

			while (!m_exit)
			{
				s_xinput.update(m_eventQueue);
				WaitForInputIdle(GetCurrentProcess(), 16);

				while (0 != PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				HANDLE consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
				DWORD numEvents = 0;
				GetNumberOfConsoleInputEvents(consoleHandle, &numEvents);
				if (numEvents)
				{
					RTM_ASSERT(numEvents <= 64, "");
					INPUT_RECORD eventBuffer[64];

					DWORD numEventsRead = 0;
					ReadConsoleInput(consoleHandle, eventBuffer, numEvents, &numEventsRead);

					for (DWORD i=0; i<numEventsRead; ++i)
					{
						if (eventBuffer[i].EventType == FOCUS_EVENT)
						{
							if (!eventBuffer[i].Event.FocusEvent.bSetFocus)
							{
								m_consoleMouseButtonState = 0;
							}
						}

						if (eventBuffer[i].EventType == KEY_EVENT)
						{
							KEY_EVENT_RECORD& ker = eventBuffer[i].Event.KeyEvent;
							uint8_t modifiers = translateKeyModifiersConsole(ker.dwControlKeyState);
							KeyboardKey::Enum key = translateKey(ker.wVirtualKeyCode);
							WindowHandle handle = handleFromHwnd(m_hwndRapp);
							m_eventQueue.postKeyEvent(handle, key, modifiers, ker.bKeyDown);
						}

						if (eventBuffer[i].EventType == MOUSE_EVENT)
						{
							MOUSE_EVENT_RECORD& mer = eventBuffer[i].Event.MouseEvent;

							uint8_t modifiers = translateKeyModifiersConsole(mer.dwControlKeyState);
							uint32_t oldMouseButtons = m_consoleMouseButtonState;
							m_consoleMouseButtonState = mer.dwButtonState;
							oldMouseButtons ^= m_consoleMouseButtonState;

							int32_t mx = mer.dwMousePosition.X;
							int32_t my = mer.dwMousePosition.Y;

							switch (mer.dwEventFlags)
							{
							case MOUSE_MOVED:
								m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, modifiers);
								break;

							case DOUBLE_CLICK:
								if (FROM_LEFT_1ST_BUTTON_PRESSED & m_consoleMouseButtonState)
									m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Left, modifiers, FROM_LEFT_1ST_BUTTON_PRESSED & m_consoleMouseButtonState, true);
								if (FROM_LEFT_2ND_BUTTON_PRESSED & m_consoleMouseButtonState)
									m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Middle, modifiers, FROM_LEFT_2ND_BUTTON_PRESSED & m_consoleMouseButtonState, true);
								if (RIGHTMOST_BUTTON_PRESSED & m_consoleMouseButtonState)
									m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Right, modifiers, RIGHTMOST_BUTTON_PRESSED & m_consoleMouseButtonState, true);
								break;

							case MOUSE_WHEELED:
								{
									int32_t z = static_cast<int32_t>(mer.dwButtonState);
									z >>= 16;
									m_mz += z;
									m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, modifiers);
								}
								break;
							};

							if (FROM_LEFT_1ST_BUTTON_PRESSED & oldMouseButtons)
								m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Left, modifiers, FROM_LEFT_1ST_BUTTON_PRESSED & m_consoleMouseButtonState, false);

							if (FROM_LEFT_2ND_BUTTON_PRESSED & oldMouseButtons)
								m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Middle, modifiers, FROM_LEFT_2ND_BUTTON_PRESSED & m_consoleMouseButtonState, false);

							if (RIGHTMOST_BUTTON_PRESSED & oldMouseButtons)
								m_eventQueue.postMouseEvent(handleFromHwnd(m_hwndRapp), mx, my, m_mz, MouseButton::Right, modifiers, RIGHTMOST_BUTTON_PRESSED & m_consoleMouseButtonState, false);

						}
					}
				}
			}

			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), saveOldConsoleMode);

			thread.stop();

			DestroyWindow(m_hwndRapp);

			s_xinput.shutdown();

			CoUninitialize();

			return mte.m_result;
		}

		inline WindowHandle handleFromHwnd(HWND _hwnd)
		{
			for (uint32_t i=0; i<m_windows.size(); ++i)
			{
				WindowInfo& info = *m_windows.getDataIndexedPtr(i);
				if (info.m_window == _hwnd)
					return { info.m_handle };
			}

			return { UINT32_MAX };
		}

		inline App* appFromHwnd(HWND _hwnd)
		{
			for (uint32_t i=0; i<m_windows.size(); ++i)
			{
				WindowInfo& info = *m_windows.getDataIndexedPtr(i);
				if (info.m_window == _hwnd)
					return info.m_app;
			}

			return 0;
		}

		inline uint32_t winArrayIndex(uint32_t _handle)
		{
			return rtm::Handle<>::getIndex(_handle);
		}

		LRESULT process(HWND _hwnd, UINT _id, WPARAM _wparam, LPARAM _lparam)
		{
			if (m_init)
			{
				switch (_id)
				{
				case WM_USER_WINDOW_CREATE:
					{
						Msg* msg = (Msg*)_lparam;
						HWND hwnd = CreateWindowA("rapp"
							, msg->m_title
							, WS_OVERLAPPEDWINDOW|WS_VISIBLE
							, msg->m_x
							, msg->m_y
							, msg->m_width
							, msg->m_height
							, m_hwndRapp
							, NULL
							, (HINSTANCE)GetModuleHandle(NULL)
							, 0
							);

						clear(hwnd);

						WindowHandle handle = { (uint32_t)_wparam };
						WindowInfo& info = *m_windows.getDataPtr(handle.idx);
					
						info.m_handle = _wparam & 0xffffffff;
						if (msg->m_flags & RAPP_WINDOW_FLAG_RENDERING)
						{
							winSetHwnd(hwnd);
							info.m_app = msg->m_app;
						}
						else
							info.m_app = 0;

						adjust(hwnd, msg->m_width, msg->m_height, true);
						clear(hwnd);

						info.m_window	= hwnd;
						info.m_flags	= msg->m_flags;
						m_eventQueue.postSizeEvent(handle, msg->m_width, msg->m_height);
						m_eventQueue.postWindowEvent(handle, hwnd);

						delete msg;
					}
					break;

				case WM_USER_WINDOW_DESTROY:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						WindowHandle handle = { (uint32_t)_wparam };
						WindowInfo& info = *m_windows.getDataPtr(handle.idx);
						PostMessageA(info.m_window, WM_CLOSE, 0, 0);
						m_eventQueue.postWindowEvent(handle);
						DestroyWindow(info.m_window);
						if (info.m_flags & RAPP_WINDOW_FLAG_MAIN_WINDOW)
						{
							App* app = info.m_app;
							if (app)
								app->quit();
						}

						m_windows.free((uint32_t)_wparam);
					}
					break;

				case WM_USER_WINDOW_SET_TITLE:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						uint32_t index = winArrayIndex((uint32_t)_wparam);
						Msg* msg = (Msg*)_lparam;
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);
						SetWindowTextA(info.m_window, msg->m_title);
						delete msg;
					}
					break;

				case WM_USER_WINDOW_SET_POS:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						Msg* msg = (Msg*)_lparam;
						uint32_t index = winArrayIndex((uint32_t)_wparam);
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);
						SetWindowPos(info.m_window, 0, msg->m_x, msg->m_y, 0, 0
							, SWP_NOACTIVATE
							| SWP_NOOWNERZORDER
							| SWP_NOSIZE
							);
						delete msg;
					}
					break;

				case WM_USER_WINDOW_SET_SIZE:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						uint32_t index = winArrayIndex((uint32_t)_wparam);
						uint32_t width  = GET_X_LPARAM(_lparam);
						uint32_t height = GET_Y_LPARAM(_lparam);
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);
						adjust(info.m_window, width, height, true);
					}
					break;

				case WM_USER_WINDOW_TOGGLE_FRAME:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						if (m_frame)
						{
							m_oldWidth  = m_width;
							m_oldHeight = m_height;
						}
						uint32_t index = winArrayIndex((uint32_t)_wparam);
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);
						adjust(info.m_window, m_oldWidth, m_oldHeight, !m_frame);
					}
					break;

				case WM_USER_WINDOW_MOUSE_LOCK:
					{
						rtm::ScopedMutexLocker scope(m_lock);

						uint32_t index = winArrayIndex((uint32_t)_wparam);
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);
						windowSetMouseLock(info.m_window, !!_lparam);
					}
					break;

				case WM_USER_CALL_FUNC:
					{
						ThreadFn fn = (ThreadFn)_wparam;
						void* userData = (void*)_lparam;
						fn(userData);
					}
					break;

				case WM_USER_CALL_KEY_DOWN:
					{
						WindowHandle handle = handleFromHwnd(_hwnd);
						uint8_t modifiers = (uint8_t)_wparam;
						KeyboardKey::Enum key = (KeyboardKey::Enum)_lparam;

						m_eventQueue.postKeyEvent(handle, key, modifiers, true);
					}
					break;

				case WM_USER_CALL_KEY_UP:
					{
						WindowHandle handle = handleFromHwnd(_hwnd);
						uint8_t modifiers = (uint8_t)_wparam;
						KeyboardKey::Enum key = (KeyboardKey::Enum)_lparam;

						m_eventQueue.postKeyEvent(handle, key, modifiers, false);
					}
					break;

				case WM_DESTROY:
					break;

				case WM_QUIT:
				case WM_CLOSE:
					{
						if (_hwnd == m_hwndRapp)
						{
							m_exit = true;
							m_eventQueue.postExitEvent();
						}
						else
						{
							windowDestroy(handleFromHwnd(_hwnd));
						}

						// Don't process message. Window will be destroyed later.
						return 0;
					}

				case WM_SIZING:
					{
						WindowHandle handle = handleFromHwnd(_hwnd);
						uint32_t index = winArrayIndex((uint32_t)handle.idx);
						WindowInfo& info = *m_windows.getDataIndexedPtr(index);

						if (isValid(handle) && (RAPP_WINDOW_FLAG_ASPECT_RATIO & info.m_flags))
						{
							RECT& rect = *(RECT*)_lparam;
							uint32_t width  = rect.right  - rect.left - m_frameWidth;
							uint32_t height = rect.bottom - rect.top  - m_frameHeight;

							// Recalculate size according to aspect ratio
							switch (_wparam)
							{
							case WMSZ_LEFT:
							case WMSZ_RIGHT:
								{
									uint32_t dX, dY;
									windowGetDefaultSize(&dX, &dY);
									float aspectRatio = 1.0f/m_aspectRatio;
									width  = rtm::uint32_max(dX/4, width);
									height = uint32_t(float(width)*aspectRatio);
								}
								break;

							default:
								{
									uint32_t dX, dY;
									windowGetDefaultSize(&dX, &dY);
									float aspectRatio = m_aspectRatio;
									height = rtm::uint32_max(dX/4, height);
									width  = uint32_t(float(height)*aspectRatio);
								}
								break;
							}

							// Recalculate position using different anchor points
							switch(_wparam)
							{
							case WMSZ_LEFT:
							case WMSZ_TOPLEFT:
							case WMSZ_BOTTOMLEFT:
								rect.left   = rect.right - width  - m_frameWidth;
								rect.bottom = rect.top   + height + m_frameHeight;
								break;

							default:
								rect.right  = rect.left + width  + m_frameWidth;
								rect.bottom = rect.top  + height + m_frameHeight;
								break;
							}

							m_eventQueue.postSizeEvent(handleFromHwnd(_hwnd), width, height);
						}
					}
					return 0;

				case WM_SIZE:
					{
						WindowHandle handle = handleFromHwnd(_hwnd);
						if (isValid(handle) )
						{
							uint32_t width  = GET_X_LPARAM(_lparam);
							uint32_t height = GET_Y_LPARAM(_lparam);

							m_width  = width;
							m_height = height;
							m_eventQueue.postSizeEvent(handle, m_width, m_height);
						}
					}
					break;

				case WM_SYSCOMMAND:
					switch (_wparam)
					{
					case SC_MINIMIZE:
					case SC_RESTORE:
						{
							HWND parent = GetWindow(_hwnd, GW_OWNER);
							if (NULL != parent)
							{
								PostMessage(parent, _id, _wparam, _lparam);
							}
						}
					}
					break;

				case WM_MOUSEMOVE:
					{
						int32_t mx = GET_X_LPARAM(_lparam);
						int32_t my = GET_Y_LPARAM(_lparam);

						if (_hwnd == m_mouseLock)
						{
							mx -= m_mx;
							my -= m_my;

							if (0 == mx
							&&  0 == my)
							{
								break;
							}

							setMousePos(_hwnd, m_mx, m_my);
						}

						m_eventQueue.postMouseEvent(handleFromHwnd(_hwnd), mx, my, m_mz, translateKeyModifiers());
					}
					break;

				case WM_MOUSEWHEEL:
					{
						POINT pt = { GET_X_LPARAM(_lparam), GET_Y_LPARAM(_lparam) };
						ScreenToClient(_hwnd, &pt);
						int32_t mx = pt.x;
						int32_t my = pt.y;
						m_mz += GET_WHEEL_DELTA_WPARAM(_wparam)/WHEEL_DELTA;
						m_eventQueue.postMouseEvent(handleFromHwnd(_hwnd), mx, my, m_mz, translateKeyModifiers());
					}
					break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
					{
						mouseCapture(_hwnd, _id == WM_LBUTTONDOWN);
						int32_t mx = GET_X_LPARAM(_lparam);
						int32_t my = GET_Y_LPARAM(_lparam);
						m_eventQueue.postMouseEvent(handleFromHwnd(_hwnd), mx, my, m_mz, MouseButton::Left, translateKeyModifiers(), _id == WM_LBUTTONDOWN, _id == WM_LBUTTONDBLCLK);
					}
					break;

				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
				case WM_MBUTTONDBLCLK:
					{
						mouseCapture(_hwnd, _id == WM_MBUTTONDOWN);
						int32_t mx = GET_X_LPARAM(_lparam);
						int32_t my = GET_Y_LPARAM(_lparam);
						m_eventQueue.postMouseEvent(handleFromHwnd(_hwnd), mx, my, m_mz, MouseButton::Middle, translateKeyModifiers(), _id == WM_MBUTTONDOWN, _id == WM_MBUTTONDBLCLK);
					}
					break;

				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				case WM_RBUTTONDBLCLK:
					{
						mouseCapture(_hwnd, _id == WM_RBUTTONDOWN);
						int32_t mx = GET_X_LPARAM(_lparam);
						int32_t my = GET_Y_LPARAM(_lparam);
						m_eventQueue.postMouseEvent(handleFromHwnd(_hwnd), mx, my, m_mz, MouseButton::Right, translateKeyModifiers(),_id == WM_RBUTTONDOWN, _id == WM_RBUTTONDBLCLK);
					}
					break;

				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYUP:
					{
						uint8_t modifiers = translateKeyModifiers();
						KeyboardKey::Enum key = translateKey(_wparam);

						WindowHandle handle = handleFromHwnd(_hwnd);

						if (KeyboardKey::Print == key
						&&  0x3 == ( (uint32_t)(_lparam)>>30) )
						{
							// VK_SNAPSHOT doesn't generate keydown event. Fire on down event when previous
							// key state bit is set to 1 and transition state bit is set to 1.
							//
							// http://msdn.microsoft.com/en-us/library/windows/desktop/ms646280%28v=vs.85%29.aspx
							m_eventQueue.postKeyEvent(handle, key, modifiers, true);
						}
						m_eventQueue.postKeyEvent(handle, key, modifiers, _id == WM_KEYDOWN || _id == WM_SYSKEYDOWN);

#ifdef RAPP_WITH_BGFX
						if (ImGui::GetCurrentContext())
						{
							ImGui::GetIO().KeyCtrl	= modifiers & (KeyboardModifier::RCtrl  | KeyboardModifier::LCtrl);
							ImGui::GetIO().KeyShift	= modifiers & (KeyboardModifier::RShift | KeyboardModifier::LShift);
							ImGui::GetIO().KeyAlt	= modifiers & (KeyboardModifier::RAlt   | KeyboardModifier::LAlt);
						}
#endif
					}
					break;

				case WM_CHAR:
					{
						uint8_t utf8[4] = {};
						uint8_t len = (uint8_t)WideCharToMultiByte(CP_UTF8
											, 0
											, (LPCWSTR)&_wparam
											, 1
											, (LPSTR)utf8
											, RTM_NUM_ELEMENTS(utf8)
											, NULL
											, NULL
											);
						if (0 != len)
						{
							WindowHandle handle = handleFromHwnd(_hwnd);
							m_eventQueue.postCharEvent(handle, len, utf8);
						}
					}
					break;

				//case WM_DROPFILES:
				//	{
				//		HDROP drop = (HDROP)_wparam;
				//		char tmp[bx::kMaxFilePath];
				//		WCHAR utf16[bx::kMaxFilePath];
				//		uint32_t result = DragQueryFileW(drop, 0, utf16, bx::kMaxFilePath);
				//		BX_UNUSED(result);
				//		WideCharToMultiByte(CP_UTF8, 0, utf16, -1, tmp, bx::kMaxFilePath, NULL, NULL);
				//		WindowHandle handle = findHandle(_hwnd);
				//		m_eventQueue.postDropFileEvent(handle, tmp);
				//	}
				//	break;

				default:
					break;
				}
			}

			return DefWindowProc(_hwnd, _id, _wparam, _lparam);
		}

		void clear(HWND _hwnd)
		{
			RECT rect;
			GetWindowRect(_hwnd, &rect);
			HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0) );
			HDC hdc = GetDC(_hwnd);
			SelectObject(hdc, brush);
			FillRect(hdc, &rect, brush);
		}

		void adjust(HWND _hwnd, uint32_t _width, uint32_t _height, bool _windowFrame)
		{
			m_width = _width;
			m_height = _height;
			m_aspectRatio = float(_width)/float(_height);

			ShowWindow(_hwnd, SW_SHOWNORMAL);
			RECT rect;
			RECT newrect = {0, 0, (LONG)_width, (LONG)_height};
			DWORD style = WS_POPUP|WS_SYSMENU;

			if (m_frame)
			{
				GetWindowRect(_hwnd, &m_rect);
				m_style = GetWindowLong(_hwnd, GWL_STYLE);
			}

			if (_windowFrame)
			{
				rect = m_rect;
				style = m_style;
			}
			else
			{
#if defined(__MINGW32__)
				rect  = m_rect;
				style = m_style;
#else
				HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO mi;
				mi.cbSize = sizeof(mi);
				GetMonitorInfo(monitor, &mi);
				newrect = mi.rcMonitor;
				rect = mi.rcMonitor;
#endif // !defined(__MINGW__)
			}

			SetWindowLong(_hwnd, GWL_STYLE, style);
			uint32_t prewidth = newrect.right - newrect.left;
			uint32_t preheight = newrect.bottom - newrect.top;
			AdjustWindowRect(&newrect, style, FALSE);
			m_frameWidth = (newrect.right - newrect.left) - prewidth;
			m_frameHeight = (newrect.bottom - newrect.top) - preheight;
			UpdateWindow(_hwnd);

			if (rect.left == -32000
			||  rect.top  == -32000)
			{
				rect.left = 0;
				rect.top  = 0;
			}

			int32_t left = rect.left;
			int32_t top = rect.top;
			int32_t width = (newrect.right-newrect.left);
			int32_t height = (newrect.bottom-newrect.top);

			if (!_windowFrame)
			{
				uint32_t dX, dY;
				windowGetDefaultSize(&dX, &dY);
				float aspectRatio = 1.0f/m_aspectRatio;
				width = rtm::uint32_max(dX/4, width);
				height = uint32_t(float(width)*aspectRatio);

				left = newrect.left+(newrect.right-newrect.left-width)/2;
				top = newrect.top+(newrect.bottom-newrect.top-height)/2;
			}

			HWND parent = GetWindow(_hwnd, GW_OWNER);
			if (NULL != parent)
			{
				if (_windowFrame)
				{
					SetWindowPos(parent
						, HWND_TOP
						, -32000
						, -32000
						, 0
						, 0
						, SWP_SHOWWINDOW
						);
				}
				else
				{
					SetWindowPos(parent
						, HWND_TOP
						, newrect.left
						, newrect.top
						, newrect.right-newrect.left
						, newrect.bottom-newrect.top
						, SWP_SHOWWINDOW
						);
				}
			}

			SetWindowPos(_hwnd
				, HWND_TOP
				, left
				, top
				, width
				, height
				, SWP_SHOWWINDOW
				);

			ShowWindow(_hwnd, SW_RESTORE);

			m_frame = _windowFrame;
		}

		void setMousePos(HWND _hwnd, int32_t _mx, int32_t _my)
		{
			POINT pt = { _mx, _my };
			ClientToScreen(_hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
		}

		void windowSetMouseLock(HWND _hwnd, bool _lock)
		{
			if (_hwnd != m_mouseLock)
			{
				if (_lock)
				{
					m_mx = m_width/2;
					m_my = m_height/2;
					ShowCursor(false);
					setMousePos(_hwnd, m_mx, m_my);
				}
				else
				{
					setMousePos(_hwnd, m_mx, m_my);
					ShowCursor(true);
				}

				m_mouseLock = _hwnd;
			}
		}

		static LRESULT CALLBACK wndProc(HWND _hwnd, UINT _id, WPARAM _wparam, LPARAM _lparam);

		EventQueue m_eventQueue;
		rtm::Mutex m_lock;

		HWND m_hwndRapp;
		rtm::Data<WindowInfo, RAPP_MAX_WINDOWS, rtm::Storage::Dense>	m_windows;

		RECT m_rect;
		DWORD m_style;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_oldWidth;
		uint32_t m_oldHeight;
		uint32_t m_frameWidth;
		uint32_t m_frameHeight;
		float m_aspectRatio;

		int32_t m_mx;
		int32_t m_my;
		int32_t m_mz;

		uint32_t m_consoleMouseButtonState;

		bool m_frame;
		HWND m_mouseLock;
		bool m_init;
		bool m_exit;
	};

	static Context s_ctx;

	LRESULT CALLBACK Context::wndProc(HWND _hwnd, UINT _id, WPARAM _wparam, LPARAM _lparam)
	{
		return s_ctx.process(_hwnd, _id, _wparam, _lparam);
	}

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
		PostMessage(s_ctx.m_hwndRapp, WM_USER_CALL_FUNC, (WPARAM)_fn, (LPARAM)_userData);
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
		s_ctx.m_lock.lock();
		WindowHandle handle = { s_ctx.m_windows.allocate() };

		if (s_ctx.m_windows.isValid(handle.idx))
		{
			Msg* msg = new Msg;
			msg->m_app    = _app;
			msg->m_x      = _x;
			msg->m_y      = _y;
			msg->m_width  = _width;
			msg->m_height = _height;
			msg->m_flags  = _flags;

			if (_title)
				rtm::strlCpy(msg->m_title, 256, _title);
			else
				msg->m_title[0] = '\0';

			SendMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_CREATE, handle.idx, (LPARAM)msg);
		}
		s_ctx.m_lock.unlock();

		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		rtm::ScopedMutexLocker scope(s_ctx.m_lock);
		if (s_ctx.m_windows.isValid(_handle.idx))
		{
			PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_DESTROY, _handle.idx, 0);
		}
	}

	void* windowGetNativeHandle(WindowHandle _handle)
	{
		rtm::ScopedMutexLocker scope(s_ctx.m_lock);
		if (s_ctx.m_windows.isValid(_handle.idx))
			return (void*)s_ctx.m_windows.getData(_handle.idx).m_window;
		return (void*)0;
	}

	void* windowGetNativeDisplayHandle()
	{
		return NULL;
	}

	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		Msg* msg = new Msg;
		msg->m_x = _x;
		msg->m_y = _y;
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_SET_POS, _handle.idx, (LPARAM)msg);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_SET_SIZE, _handle.idx, (_height<<16) | (_width&0xffff) );
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		if (_handle.idx == rapp::kDefaultWindowHandle.idx)
		{
			SetWindowTextA(s_ctx.m_hwndRapp, _title);
			return;
		}

		Msg* msg = new Msg;
		rtm::strlCpy(msg->m_title, 256, _title);
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_SET_TITLE, _handle.idx, (LPARAM)msg);
	}

	void windowToggleFrame(WindowHandle _handle)
	{
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_TOGGLE_FRAME, _handle.idx, 0);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_TOGGLE_FRAME, _handle.idx, 0);
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		PostMessage(s_ctx.m_hwndRapp, WM_USER_WINDOW_MOUSE_LOCK, _handle.idx, _lock);
	}

	void inputEmitKeyPress(KeyboardKey::Enum _key, uint8_t _modifiers)
	{
		PostMessage(s_ctx.m_hwndRapp, WM_USER_CALL_KEY_DOWN, (WPARAM)_modifiers, (LPARAM)_key);
		PostMessage(s_ctx.m_hwndRapp, WM_USER_CALL_KEY_UP,   (WPARAM)_modifiers, (LPARAM)_key);

		if ((_key >= KeyboardKey::KeyA) && (_key <= KeyboardKey::KeyZ))
			PostMessage(s_ctx.m_hwndRapp, WM_CHAR, (WPARAM)('A' + _key - KeyboardKey::KeyA), (LPARAM)0);

//		if ((_key >= KeyboardKey::Key0) && (_key <= KeyboardKey::Key9))
//		 Plus, Minus, Equal, LeftBracket, RightBracket, Semicolon, Quote, Comma, Period, Slash, Backslash, Tilde,
//			PostMessage(s_ctx.m_hwndRapp, WM_CHAR, (WPARAM)('0' + _key - KeyboardKey::Key0), (LPARAM)0);
	}

	int32_t MainThreadEntry::threadFunc(void* _userData)
	{
		MainThreadEntry* self = (MainThreadEntry*)_userData;
		self->m_result = main(self->m_argc, self->m_argv);
		PostMessage(s_ctx.m_hwndRapp, WM_QUIT, 0, 0);
		return self->m_result;
	}

} // namespace rapp

int main(int _argc, const char* const* _argv)
{
	return rapp::s_ctx.run(_argc, _argv);
}

#endif // RTM_PLATFORM_WINDOWS
