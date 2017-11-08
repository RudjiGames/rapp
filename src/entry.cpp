//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <time.h>
#include <inttypes.h>

#define RTM_LIBHANDLER_DEFINE
#include <rbase/inc/libhandler.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/entry_p.h>
#include <rapp/src/cmd.h>
#include <rapp/src/input.h>

#if RAPP_WITH_BGFX
#include <bgfx/bgfx.h>
#endif

namespace rapp
{
	struct CmdContext;

#if RAPP_WITH_BGFX
	static uint32_t s_debug = BGFX_DEBUG_TEXT;
	static uint32_t s_reset = BGFX_RESET_VSYNC;
#else
	static uint32_t s_debug = 0;
	static uint32_t s_reset = 0;
#endif

	static const char* s_keyName[] =
	{
		"None",
		"Esc",
		"Return",
		"Tab",
		"Space",
		"Backspace",
		"Up",
		"Down",
		"Left",
		"Right",
		"Insert",
		"Delete",
		"Home",
		"End",
		"PageUp",
		"PageDown",
		"Print",
		"Plus",
		"Minus",
		"LeftBracket",
		"RightBracket",
		"Semicolon",
		"Quote",
		"Comma",
		"Period",
		"Slash",
		"Backslash",
		"Tilde",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"NumPad0", "NumPad1", "NumPad2", "NumPad3", "NumPad4",
		"NumPad5", "NumPad6", "NumPad7", "NumPad8", "NumPad9",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
		"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
	};
	RTM_STATIC_ASSERT(KeyboardState::Key::Count == RTM_NUM_ELEMENTS(s_keyName) );

	const char* getName(KeyboardState::Key _key)
	{
		RTM_ASSERT(_key < KeyboardState::Key::Count, "Invalid key %d.", _key);
		return s_keyName[_key];
	}

	inline bool toBool(const char* _str)
	{
		char ch = rtm::toLower(_str[0]);
		return ch == 't' ||  ch == '1';
	}

	char keyToAscii(KeyboardState::Key _key, uint8_t _modifiers)
	{
		const bool isAscii = (KeyboardState::Key::Key0 <= _key && _key <= KeyboardState::Key::KeyZ)
						  || (KeyboardState::Key::Esc  <= _key && _key <= KeyboardState::Key::Minus);
		if (!isAscii)
		{
			return '\0';
		}

		const bool isNumber = (KeyboardState::Key::Key0 <= _key && _key <= KeyboardState::Key::Key9);
		if (isNumber)
		{
			return '0' + (char)(_key - KeyboardState::Key::Key0);
		}

		const bool isChar = (KeyboardState::Key::KeyA <= _key && _key <= KeyboardState::Key::KeyZ);
		if (isChar)
		{
			enum { ShiftMask = KeyboardState::Modifier::LShift | KeyboardState::Modifier::RShift };

			const bool shift = !!(_modifiers&ShiftMask);
			return (shift ? 'A' : 'a') + (char)(_key - KeyboardState::Key::KeyA);
		}

		switch (_key)
		{
		case KeyboardState::Key::Esc:       return 0x1b;
		case KeyboardState::Key::Return:    return '\n';
		case KeyboardState::Key::Tab:       return '\t';
		case KeyboardState::Key::Space:     return ' ';
		case KeyboardState::Key::Backspace: return 0x08;
		case KeyboardState::Key::Plus:      return '+';
		case KeyboardState::Key::Minus:     return '-';
		default:             break;
		}

		return '\0';
	}

	bool setOrToggle(uint32_t& _flags, const char* _name, uint32_t _bit, int _first, int _argc, char const* const* _argv)
	{
		if (0 == strcmp(_argv[_first], _name) )
		{
			int arg = _first+1;
			if (_argc > arg)
			{
				_flags &= ~_bit;
				_flags |= toBool(_argv[arg]) ? _bit : 0;
			}
			else
			{
				_flags ^= _bit;
			}

			return true;
		}

		return false;
	}

	int cmdMouseLock(void* /*_userData*/, int _argc, char const* const* _argv)
	{
		if (_argc > 1)
		{
			inputSetMouseLock(_argc > 1 ? toBool(_argv[1]) : !inputIsMouseLocked() );
			return 0;
		}

		return 1;
	}

#if RAPP_WITH_BGFX
	int cmdGraphics(void* /*_userData*/, int _argc, char const* const* _argv)
	{
		if (_argc > 1)
		{
			if (setOrToggle(s_reset, "vsync",       BGFX_RESET_VSYNC,              1, _argc, _argv)
			||  setOrToggle(s_reset, "maxaniso",    BGFX_RESET_MAXANISOTROPY,      1, _argc, _argv)
			||  setOrToggle(s_reset, "hmd",         BGFX_RESET_HMD,                1, _argc, _argv)
			||  setOrToggle(s_reset, "hmddbg",      BGFX_RESET_HMD_DEBUG,          1, _argc, _argv)
			||  setOrToggle(s_reset, "hmdrecenter", BGFX_RESET_HMD_RECENTER,       1, _argc, _argv)
			||	setOrToggle(s_reset, "msaa4",		BGFX_RESET_MSAA_X4,			   1, _argc, _argv)
			||	setOrToggle(s_reset, "msaa8",		BGFX_RESET_MSAA_X8,			   1, _argc, _argv)
			||  setOrToggle(s_reset, "msaa16",      BGFX_RESET_MSAA_X16,           1, _argc, _argv)
			||  setOrToggle(s_reset, "flush",       BGFX_RESET_FLUSH_AFTER_RENDER, 1, _argc, _argv)
			||  setOrToggle(s_reset, "flip",        BGFX_RESET_FLIP_AFTER_RENDER,  1, _argc, _argv)
			   )
			{
				return 0;
			}
			else if (setOrToggle(s_debug, "stats",     BGFX_DEBUG_STATS,     1, _argc, _argv)
				 ||  setOrToggle(s_debug, "ifh",       BGFX_DEBUG_IFH,       1, _argc, _argv)
				 ||  setOrToggle(s_debug, "text",      BGFX_DEBUG_TEXT,      1, _argc, _argv)
				 ||  setOrToggle(s_debug, "wireframe", BGFX_DEBUG_WIREFRAME, 1, _argc, _argv) )
			{
				bgfx::setDebug(s_debug);
				return 0;
			}
			else if (0 == strcmp(_argv[1], "screenshot") )
			{
				bgfx::FrameBufferHandle fbh = BGFX_INVALID_HANDLE; 

				if (_argc > 2)
				{
					bgfx::requestScreenShot(fbh, _argv[2]);
				}
				else
				{
					time_t tt;
					time(&tt);

					char filePath[256];
					::snprintf(filePath, sizeof(filePath), "temp/screenshot-%" PRId64 "", (uint64_t)tt);
					bgfx::requestScreenShot(fbh, filePath);
				}

				return 0;
			}
			else if (0 == strcmp(_argv[1], "fullscreen") )
			{
				WindowHandle window = { 0 };
				windowToggleFullscreen(window);
				return 0;
			}
		}

		return 1;
	}
#endif

	//static const InputBinding s_bindings[] =
	//{
	//	{ NULL, "exit",                              1, { KeyboardState::Key::KeyQ,   KeyboardState::Modifier::LCtrl  }},
	//	{ NULL, "exit",                              1, { KeyboardState::Key::KeyQ,   KeyboardState::Modifier::RCtrl  }},
	//	{ NULL, "graphics fullscreen",               1, { KeyboardState::Key::KeyF,   KeyboardState::Modifier::LCtrl  }},
	//	{ NULL, "graphics fullscreen",               1, { KeyboardState::Key::KeyF,   KeyboardState::Modifier::RCtrl  }},
	//	{ NULL, "graphics fullscreen",               1, { KeyboardState::Key::F11,    KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics stats",                    1, { KeyboardState::Key::F1,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics stats 0\ngraphics text 0", 1, { KeyboardState::Key::F1,     KeyboardState::Modifier::LShift }},
	//	{ NULL, "graphics wireframe",                1, { KeyboardState::Key::F3,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics hmd",                      1, { KeyboardState::Key::F4,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics hmdrecenter",              1, { KeyboardState::Key::F4,     KeyboardState::Modifier::LShift }},
	//	{ NULL, "graphics hmddbg",                   1, { KeyboardState::Key::F4,     KeyboardState::Modifier::LCtrl  }},
	//	{ NULL, "graphics vsync",                    1, { KeyboardState::Key::F7,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics msaa16",                   1, { KeyboardState::Key::F8,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics flush",                    1, { KeyboardState::Key::F9,     KeyboardState::Modifier::NoMods }},
	//	{ NULL, "graphics screenshot",               1, { KeyboardState::Key::Print,  KeyboardState::Modifier::NoMods }},

	//	RAPP_INPUT_BINDING_END
	//};

	int main(int _argc, char** _argv)
	{
		cmdInit();
		cmdAdd("mouselock", cmdMouseLock);
#if RAPP_WITH_BGFX
		cmdAdd("graphics",  cmdGraphics);
#endif

		WindowHandle defaultWindow = { 0 };

		const char* executable = rtm::pathGetFileName(_argv[0]);
		windowSetTitle(defaultWindow, executable ? executable : "rapp");

		int32_t result = rapp::rapp_main(_argc, _argv);

		cmdShutdown();
	
		return result;
	}
	
	bool processEvents(App* _app)
	{
		uint32_t debug = s_debug;
		uint32_t reset = s_reset;

		WindowHandle handle = { UINT16_MAX };

		const Event* ev;
		do
		{
			struct SE { const Event* m_ev; SE() : m_ev(poll() ) {} ~SE() { if (NULL != m_ev) { release(m_ev); } } } scopeEvent;
			ev = scopeEvent.m_ev;

			if (NULL != ev)
			{
				switch (ev->m_type)
				{
				case Event::Axis:
					{
						const AxisEvent* axis = static_cast<const AxisEvent*>(ev);
						inputSetGamepadAxis(axis->m_gamepad, axis->m_axis, axis->m_value);
					}
					break;

				case Event::Char:
					{
						const CharEvent* chev = static_cast<const CharEvent*>(ev);
						inputChar(chev->m_len, chev->m_char);
					}
					break;

				case Event::Exit:
					return true;

				case Event::Gamepad:
					{
						const GamepadEvent* gev = static_cast<const GamepadEvent*>(ev);
						inputSetGamepadConnected(gev->m_gamepad, gev->m_connected);
						RAPP_DBG("gamepad %d, %d", gev->m_gamepad.idx, gev->m_connected);
					}
					break;

				case Event::GamepadButton:
					{
						const GamepadButtonEvent* gev = static_cast<const GamepadButtonEvent*>(ev);
						inputSetGamepadButtonState(gev->m_gamepad, gev->m_button, gev->m_pressed);
					}
					break;

				case Event::Mouse:
					{
						const MouseEvent* mouse = static_cast<const MouseEvent*>(ev);
						handle = mouse->m_handle;

						if (mouse->m_move)
						{
							inputSetMousePos(mouse->m_mx, mouse->m_my, mouse->m_mz);
						}
						else
						{
							inputSetMouseButtonState(mouse->m_button, mouse->m_down);
						}
					}
					break;

				case Event::Key:
					{
						const KeyEvent* key = static_cast<const KeyEvent*>(ev);
						handle = key->m_handle;
						inputSetKeyState(key->m_key, key->m_modifiers, key->m_down);
					}
					break;

				case Event::Size:
					{
						const SizeEvent* size = static_cast<const SizeEvent*>(ev);
						handle  = size->m_handle;
						_app->m_width  = size->m_width;
						_app->m_height = size->m_height;
						reset  = !s_reset; // force reset
					}
					break;

				case Event::Window:
					break;

				default:
					break;
				}
			}

			inputProcess();

			inputResetMouseMovement();
			inputResetGamepadAxisMovement();

		} while (NULL != ev);

		if ((handle.idx == 0) && (reset != s_reset))
		{
			reset = s_reset;
#if RAPP_WITH_BGFX
			bgfx::reset(_app->m_width, _app->m_height, reset);
#endif
			inputSetMouseResolution((uint16_t)_app->m_width, (uint16_t)_app->m_height);
		}

		s_debug = debug;

		return _app->m_exitCode != -1;
	}

} // namespace rapp
