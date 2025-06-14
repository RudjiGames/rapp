//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_PS4

#include <mouse.h>
#include <pad.h>
#include <libime/ime_types.h>
#include <libime/ime_keycode.h>
#include <libime/libime_api.h>
#include <libsysmodule.h>

extern "C" size_t sceLibcHeapSize = 256 * 1024 * 1024;

static rapp::EventQueue g_eventQueue;

namespace rapp
{
	struct Context;
	Context& getContext();

		struct GamePadRemap
		{
			uint32_t				m_bit;
			GamepadButtons	m_button;
		};

		static GamePadRemap s_gamepadRemap[] =
		{
			{ SCE_PAD_BUTTON_L3,			GamepadButtons::LThumb    },
			{ SCE_PAD_BUTTON_R3,			GamepadButtons::RThumb    },
			{ SCE_PAD_BUTTON_OPTIONS,		GamepadButtons::Start     },
			{ SCE_PAD_BUTTON_UP,			GamepadButtons::Up        },
			{ SCE_PAD_BUTTON_RIGHT,			GamepadButtons::Right     },
			{ SCE_PAD_BUTTON_DOWN,			GamepadButtons::Down      },
			{ SCE_PAD_BUTTON_LEFT,			GamepadButtons::Left      },
			{ SCE_PAD_BUTTON_L2,			GamepadState::None      },
			{ SCE_PAD_BUTTON_R2,			GamepadState::None      },
			{ SCE_PAD_BUTTON_L1,			GamepadButtons::LShoulder },
			{ SCE_PAD_BUTTON_R1,			GamepadButtons::RShoulder },
			{ SCE_PAD_BUTTON_TRIANGLE,		GamepadButtons::Y         },
			{ SCE_PAD_BUTTON_CIRCLE,		GamepadButtons::B         },
			{ SCE_PAD_BUTTON_CROSS,			GamepadButtons::A         },
			{ SCE_PAD_BUTTON_SQUARE,		GamepadButtons::X         },
			{ SCE_PAD_BUTTON_TOUCH_PAD,		GamepadButtons::Guide      },
		};

	struct MainThreadEntry
	{
		int m_argc;
		const char* const* m_argv;
		int32_t m_result;

		static int32_t threadFunc(void* _userData);
	};

	struct InputPS4
	{
		SceUserServiceUserId			m_userID;
		SceUserServiceLoginUserIdList	m_userIdList;
		uint8_t							m_prevLeftTrigger[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		uint8_t							m_prevRightTrigger[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		uint32_t						m_prevPadButtons[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		uint32_t						m_currPadButtons[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		bool							m_connected[SCE_USER_SERVICE_MAX_LOGIN_USERS];

		ScePadData						m_padState[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		ScePadControllerInformation		m_padInfo[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		int								m_padHandles[SCE_USER_SERVICE_MAX_LOGIN_USERS];
		float							m_leftStick[SCE_USER_SERVICE_MAX_LOGIN_USERS][2];
		float							m_rightStick[SCE_USER_SERVICE_MAX_LOGIN_USERS][2];
		int8_t							m_flip[GamepadAxis::Count];

		SceMouseData					m_mouseData[8];
		uint32_t						m_mouseButtons;
		int								m_mouseX;
		int								m_mouseY;
		int								m_mouseZ;
		int								m_tilt;
		int32_t							m_mouseHandle;
		int								m_mouseHistory;

		static uint8_t					s_translateKey[256];

		InputPS4()
		{
			m_mouseButtons	= 0;
			m_mouseX		= 0;
			m_mouseY		= 0;
			m_mouseZ		= 0;

			for (int i=0; i<GamepadAxis::Count; ++i)
				m_flip[i] = 1;

			m_flip[GamepadAxis::LeftY ] = -1;
			m_flip[GamepadAxis::RightY] = -1;

			memset(m_padState, 0, sizeof(m_padState));
			memset(m_prevLeftTrigger,  0, sizeof(m_prevLeftTrigger));
			memset(m_prevRightTrigger, 0, sizeof(m_prevRightTrigger));

			memset(s_translateKey, 0, sizeof(s_translateKey) );
			s_translateKey[SCE_IME_KEYCODE_ESCAPE]       = KeyboardKey::Esc;
			s_translateKey[SCE_IME_KEYCODE_RETURN]       = KeyboardKey::Return;
			s_translateKey[SCE_IME_KEYCODE_TAB]          = KeyboardKey::Tab;
			s_translateKey[SCE_IME_KEYCODE_BACKSPACE]    = KeyboardKey::Backspace;
			s_translateKey[SCE_IME_KEYCODE_SPACEBAR]     = KeyboardKey::Space;
			s_translateKey[SCE_IME_KEYCODE_UPARROW]      = KeyboardKey::Up;
			s_translateKey[SCE_IME_KEYCODE_DOWNARROW]    = KeyboardKey::Down;
			s_translateKey[SCE_IME_KEYCODE_LEFTARROW]    = KeyboardKey::Left;
			s_translateKey[SCE_IME_KEYCODE_RIGHTARROW]   = KeyboardKey::Right;
			s_translateKey[SCE_IME_KEYCODE_INSERT]       = KeyboardKey::Insert;
			s_translateKey[SCE_IME_KEYCODE_DELETE]       = KeyboardKey::Delete;
			s_translateKey[SCE_IME_KEYCODE_HOME]         = KeyboardKey::Home;
			s_translateKey[SCE_IME_KEYCODE_END]          = KeyboardKey::End;
			s_translateKey[SCE_IME_KEYCODE_PRIOR]        = KeyboardKey::PageUp;
			s_translateKey[SCE_IME_KEYCODE_PAGEUP]       = KeyboardKey::PageUp;
			s_translateKey[SCE_IME_KEYCODE_PAGEDOWN]     = KeyboardKey::PageDown;
			s_translateKey[SCE_IME_KEYCODE_EQUAL]        = KeyboardKey::Plus;
			s_translateKey[SCE_IME_KEYCODE_MINUS]        = KeyboardKey::Minus;
			s_translateKey[SCE_IME_KEYCODE_LEFTBRACKET]  = KeyboardKey::LeftBracket;
			s_translateKey[SCE_IME_KEYCODE_RIGHTBRACKET] = KeyboardKey::RightBracket;
			s_translateKey[SCE_IME_KEYCODE_SEMICOLON]    = KeyboardKey::Semicolon;
			s_translateKey[SCE_IME_KEYCODE_SINGLEQUOTE]  = KeyboardKey::Quote;
			s_translateKey[SCE_IME_KEYCODE_COMMA]        = KeyboardKey::Comma;
			s_translateKey[SCE_IME_KEYCODE_PERIOD]       = KeyboardKey::Period;
			s_translateKey[SCE_IME_KEYCODE_SLASH]        = KeyboardKey::Slash;
			s_translateKey[SCE_IME_KEYCODE_BACKSLASH]    = KeyboardKey::Backslash;
			s_translateKey[SCE_IME_KEYCODE_BACKQUOTE]    = KeyboardKey::Tilde;
			s_translateKey[SCE_IME_KEYCODE_F1]           = KeyboardKey::F1;
			s_translateKey[SCE_IME_KEYCODE_F2]           = KeyboardKey::F2;
			s_translateKey[SCE_IME_KEYCODE_F3]           = KeyboardKey::F3;
			s_translateKey[SCE_IME_KEYCODE_F4]           = KeyboardKey::F4;
			s_translateKey[SCE_IME_KEYCODE_F5]           = KeyboardKey::F5;
			s_translateKey[SCE_IME_KEYCODE_F6]           = KeyboardKey::F6;
			s_translateKey[SCE_IME_KEYCODE_F7]           = KeyboardKey::F7;
			s_translateKey[SCE_IME_KEYCODE_F8]           = KeyboardKey::F8;
			s_translateKey[SCE_IME_KEYCODE_F9]           = KeyboardKey::F9;
			s_translateKey[SCE_IME_KEYCODE_F10]          = KeyboardKey::F10;
			s_translateKey[SCE_IME_KEYCODE_F11]          = KeyboardKey::F11;
			s_translateKey[SCE_IME_KEYCODE_F12]          = KeyboardKey::F12;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_0]     = KeyboardKey::NumPad0;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_1]     = KeyboardKey::NumPad1;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_2]     = KeyboardKey::NumPad2;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_3]     = KeyboardKey::NumPad3;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_4]     = KeyboardKey::NumPad4;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_5]     = KeyboardKey::NumPad5;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_6]     = KeyboardKey::NumPad6;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_7]     = KeyboardKey::NumPad7;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_8]     = KeyboardKey::NumPad8;
			s_translateKey[SCE_IME_KEYCODE_KEYPAD_9]     = KeyboardKey::NumPad9;
			s_translateKey[SCE_IME_KEYCODE_0]            = KeyboardKey::Key0;
			s_translateKey[SCE_IME_KEYCODE_1]            = KeyboardKey::Key1;
			s_translateKey[SCE_IME_KEYCODE_2]            = KeyboardKey::Key2;
			s_translateKey[SCE_IME_KEYCODE_3]            = KeyboardKey::Key3;
			s_translateKey[SCE_IME_KEYCODE_4]            = KeyboardKey::Key4;
			s_translateKey[SCE_IME_KEYCODE_5]            = KeyboardKey::Key5;
			s_translateKey[SCE_IME_KEYCODE_6]            = KeyboardKey::Key6;
			s_translateKey[SCE_IME_KEYCODE_7]            = KeyboardKey::Key7;
			s_translateKey[SCE_IME_KEYCODE_8]            = KeyboardKey::Key8;
			s_translateKey[SCE_IME_KEYCODE_9]            = KeyboardKey::Key9;
			s_translateKey[SCE_IME_KEYCODE_A]            = KeyboardKey::KeyA;
			s_translateKey[SCE_IME_KEYCODE_B]            = KeyboardKey::KeyB;
			s_translateKey[SCE_IME_KEYCODE_C]            = KeyboardKey::KeyC;
			s_translateKey[SCE_IME_KEYCODE_D]            = KeyboardKey::KeyD;
			s_translateKey[SCE_IME_KEYCODE_E]            = KeyboardKey::KeyE;
			s_translateKey[SCE_IME_KEYCODE_F]            = KeyboardKey::KeyF;
			s_translateKey[SCE_IME_KEYCODE_G]            = KeyboardKey::KeyG;
			s_translateKey[SCE_IME_KEYCODE_H]            = KeyboardKey::KeyH;
			s_translateKey[SCE_IME_KEYCODE_I]            = KeyboardKey::KeyI;
			s_translateKey[SCE_IME_KEYCODE_J]            = KeyboardKey::KeyJ;
			s_translateKey[SCE_IME_KEYCODE_K]            = KeyboardKey::KeyK;
			s_translateKey[SCE_IME_KEYCODE_L]            = KeyboardKey::KeyL;
			s_translateKey[SCE_IME_KEYCODE_M]            = KeyboardKey::KeyM;
			s_translateKey[SCE_IME_KEYCODE_N]            = KeyboardKey::KeyN;
			s_translateKey[SCE_IME_KEYCODE_O]            = KeyboardKey::KeyO;
			s_translateKey[SCE_IME_KEYCODE_P]            = KeyboardKey::KeyP;
			s_translateKey[SCE_IME_KEYCODE_Q]            = KeyboardKey::KeyQ;
			s_translateKey[SCE_IME_KEYCODE_R]            = KeyboardKey::KeyR;
			s_translateKey[SCE_IME_KEYCODE_S]            = KeyboardKey::KeyS;
			s_translateKey[SCE_IME_KEYCODE_T]            = KeyboardKey::KeyT;
			s_translateKey[SCE_IME_KEYCODE_U]            = KeyboardKey::KeyU;
			s_translateKey[SCE_IME_KEYCODE_V]            = KeyboardKey::KeyV;
			s_translateKey[SCE_IME_KEYCODE_W]            = KeyboardKey::KeyW;
			s_translateKey[SCE_IME_KEYCODE_X]            = KeyboardKey::KeyX;
			s_translateKey[SCE_IME_KEYCODE_Y]            = KeyboardKey::KeyY;
			s_translateKey[SCE_IME_KEYCODE_Z]            = KeyboardKey::KeyZ;
		}

		void init()
		{
			// Init users
			sceUserServiceInitialize(NULL);
			sceUserServiceGetInitialUser(&m_userID);
			sceUserServiceGetLoginUserIdList(&m_userIdList);

			// Init game pad(s)
			int32_t ret = scePadInit();
			if(ret != SCE_OK)
				rtm::Console::error("scePadInit() failed: 0x%08x\n", ret);

			for(uint32_t i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; ++i)
			{
				m_padHandles[i] = scePadOpen(m_userIdList.userId[i], SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
				if(m_padHandles[i] < 0)
					rtm::Console::error("scePadOpen() userId[%d] failed: 0x%08x\n", i, m_padHandles[i]);
				else
					rtm::Console::info("scePadOpen() success for userId[%d]\n", i);

				m_prevPadButtons[i] = 0;
				m_currPadButtons[i] = 0;
				m_connected[i]		= false;

				ScePadControllerInformation info;
				scePadGetControllerInformation(m_padHandles[i], &info);
			}


			// Init mouse
			ret = sceSysmoduleLoadModule( SCE_SYSMODULE_MOUSE );
			if (ret == SCE_OK)
			{
				ret = sceMouseInit();
				SceMouseOpenParam param;
				param.behaviorFlag	= 0; // SCE_MOUSE_OPEN_PARAM_MERGED
				m_mouseHandle		= sceMouseOpen(m_userID, SCE_MOUSE_PORT_TYPE_STANDARD, 0, &param);
			}

			// Init keyboard
			ret = sceSysmoduleLoadModule( SCE_SYSMODULE_LIBIME );

			SceImeKeyboardParam kparam;
			kparam.option	= 0;
			kparam.arg		= (void*)this;
			kparam.handler	= keyboardEvent;

			memset(kparam.reserved1, 0x00, sizeof(kparam.reserved1));
			memset(kparam.reserved2, 0x00, sizeof(kparam.reserved2));

			if (sceImeKeyboardOpen(m_userID, &kparam ) != SCE_OK)
				rtm::Console::error("sceImeKeyboardOpen() error\n");

			rtm::Console::info("sceImeKeyboardOpen() success\n");

			SceImeKeyboardResourceIdArray resourceIdArray;

			if (( ret = sceImeKeyboardGetResourceId(m_userID, &resourceIdArray ))!= SCE_OK )
			{
				for (int idx = 0; idx < SCE_IME_KEYBOARD_MAX_NUMBER; ++idx)
				{
					if (resourceIdArray.resourceId[idx] != SCE_IME_KEYBOARD_RESOURCE_ID_INVALID)
					{
						SceImeKeyboardInfo info;
						if ((ret = sceImeKeyboardGetInfo(resourceIdArray.resourceId[idx], &info)) != SCE_OK)
							rtm::Console::error("sceImeKeyboardGetInfo() error\n");
						else
							rtm::Console::info("sceImeKeyboardGetInfo() success\n");
					}
				}
			}
		}

		void shutdown()
		{
			sceSysmoduleUnloadModule( SCE_SYSMODULE_MOUSE );
			sceSysmoduleUnloadModule( SCE_SYSMODULE_LIBIME );
		
			for(uint32_t i=0; i<SCE_USER_SERVICE_MAX_LOGIN_USERS; ++i)
			{
				if (m_padHandles[i] > 0)
				{
					int ret = scePadClose(m_padHandles[i]);
					if (ret != SCE_OK)
						rtm::Console::error("scePadClose() failed: 0x%08x\n", m_padHandles[i]);
				}
			}
		}

		static uint8_t getModifiers(uint32_t _keyStatus)
		{
			uint8_t modifiers = 0;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_L_CTRL)		modifiers |= KeyboardModifier::LCtrl;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_L_SHIFT)	modifiers |= KeyboardModifier::LShift;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_L_ALT)		modifiers |= KeyboardModifier::LAlt;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_L_GUI)		modifiers |= KeyboardModifier::LMeta;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_R_CTRL)		modifiers |= KeyboardModifier::RCtrl;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_R_SHIFT)	modifiers |= KeyboardModifier::RShift;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_R_ALT)		modifiers |= KeyboardModifier::RAlt;
			if (_keyStatus & SCE_IME_KEYCODE_STATE_MODIFIER_R_GUI)		modifiers |= KeyboardModifier::RMeta;
			return modifiers;
		}

		static uint8_t s_modifiers;

		static void keyboardEvent(void* _arg, const SceImeEvent* _event)
		{
			//InputPS4* input = (InputPS4*)_arg;
			WindowHandle handle = { UINT16_MAX };

			switch (_event->id)
			{
			case SCE_IME_KEYBOARD_EVENT_OPEN:
					break;

			case SCE_IME_KEYBOARD_EVENT_KEYCODE_DOWN:
					{
						rapp::KeyboardKey key = (rapp::KeyboardKey)s_translateKey[_event->param.keycode.keycode];
						s_modifiers = getModifiers(_event->param.keycode.status);
						g_eventQueue.postKeyEvent(handle, key, s_modifiers, true);
					}
					break;

			case SCE_IME_KEYBOARD_EVENT_KEYCODE_UP:
					{
						rapp::KeyboardKey key = (rapp::KeyboardKey)s_translateKey[_event->param.keycode.keycode];
						s_modifiers = getModifiers(_event->param.keycode.status);
						g_eventQueue.postKeyEvent(handle, key, s_modifiers, false);
					}
					break;

			case SCE_IME_KEYBOARD_EVENT_KEYCODE_REPEAT:
					{
						rapp::KeyboardKey key = (rapp::KeyboardKey)s_translateKey[_event->param.keycode.keycode];
						s_modifiers = getModifiers(_event->param.keycode.status);
						g_eventQueue.postKeyEvent(handle, key, s_modifiers, true);
					}
					break;

			case SCE_IME_KEYBOARD_EVENT_CONNECTION:
					rtm::Console::info("Keyboard connected!\n");
					break;

			case SCE_IME_KEYBOARD_EVENT_DISCONNECTION:
					rtm::Console::info("Keyboard disconnected!\n");
					break;

			case SCE_IME_KEYBOARD_EVENT_ABORT:
					rtm::Console::info("Keyboard manager was aborted\n" );
					break;

			default:
					rtm::Console::info("Invalid keybaord event: id=%d\n", _event->id);
					break;
			}
		}

		void update(EventQueue& _eventQueue)
		{
			WindowHandle windowHandle = { UINT16_MAX };

			for (int i=0; i<SCE_USER_SERVICE_MAX_LOGIN_USERS; ++i)
			{
				GamepadHandle handle = { static_cast<uint16_t>(i) };

				// Update game pad

				if (SCE_OK != scePadReadState(m_padHandles[i], &m_padState[i]))
					continue;

				ScePadData& data = m_padState[i];
				if (data.connected != m_connected[i])
				{
					_eventQueue.postGamepadEvent(windowHandle, handle, data.connected);
					if (data.connected)
						scePadGetControllerInformation(m_padHandles[i], &m_padInfo[i]);
				}

				m_connected[i] = data.connected;

				if (!m_connected[i])
					continue;

				ScePadControllerInformation& info = m_padInfo[i];

				float deadZoneL = (float)info.stickInfo.deadZoneLeft / 255.0f;
				float deadZoneR = (float)info.stickInfo.deadZoneRight / 255.0f;

				float lX = float((int32_t)data.leftStick.x  * 2 -255) / 255.0f;
				float lY = float((int32_t)data.leftStick.y  * 2 -255) / 255.0f;
				float rX = float((int32_t)data.rightStick.x * 2 -255) / 255.0f;
				float rY = float((int32_t)data.rightStick.y * 2 -255) / 255.0f;
	
				float leftStickX  = ((fabsf(lX) < deadZoneL) ? 0.0f : lX);
				float leftStickY  = ((fabsf(lY) < deadZoneL) ? 0.0f : lY);
				float rightStickX = ((fabsf(rX) < deadZoneR) ? 0.0f : rX);
				float rightStickY = ((fabsf(rY) < deadZoneR) ? 0.0f : rY);

				m_prevPadButtons[i] = m_currPadButtons[i];
				m_currPadButtons[i] = data.buttons;

				uint32_t buttonChange = m_prevPadButtons[i] ^ m_currPadButtons[i];

				if (buttonChange)
				{
					for (uint32_t j=0; j<RTM_NUM_ELEMENTS(s_gamepadRemap); ++j)
					{
						uint16_t bit = s_gamepadRemap[j].m_bit;
						if (bit & buttonChange)
							_eventQueue.postGamepadButtonsEvent(windowHandle, handle, s_gamepadRemap[j].m_button, !!(0 == (buttonChange & bit)));
					}
				}

				if (m_prevLeftTrigger[i] != data.analogButtons.l2)
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::LeftZ, data.analogButtons.l2 * m_flip[GamepadAxis::LeftZ]);
					m_prevLeftTrigger[i] = data.analogButtons.l2;
					rtm::Console::info("LZ %d\n", data.analogButtons.l2);
				}

				if (m_prevRightTrigger[i] != data.analogButtons.r2)
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::RightZ, data.analogButtons.r2 * m_flip[GamepadAxis::RightZ]);
					m_prevRightTrigger[i] = data.analogButtons.r2;
					rtm::Console::info("RZ %d\n", data.analogButtons.r2);
				}

				if (leftStickX != m_leftStick[i][0])
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::LeftX, (int32_t)(leftStickX * 32767.0f) * m_flip[GamepadAxis::LeftX]);
					m_leftStick[i][0] = leftStickX;
				}

				if (leftStickY != m_leftStick[i][1])
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::LeftY, (int32_t)(leftStickY * 32767.0f) * m_flip[GamepadAxis::LeftY]);
					m_leftStick[i][1] = leftStickY;
				}

				if (rightStickX != m_rightStick[i][0])
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::RightX, (int32_t)(rightStickX * 32767.0f) * m_flip[GamepadAxis::RightX]);
					m_rightStick[i][0] = rightStickX;
				}

				if (rightStickY != m_rightStick[i][1])
				{
					_eventQueue.postAxisEvent(windowHandle, handle, GamepadAxis::RightY, (int32_t)(rightStickY * 32767.0f) * m_flip[GamepadAxis::RightY]);
					m_rightStick[i][1] = rightStickY;
				}
			}

			// Update keyboard
			sceImeUpdate( keyboardEvent );

			// Update mouse
			m_mouseHistory = sceMouseRead(m_mouseHandle, m_mouseData, RTM_NUM_ELEMENTS(m_mouseData));
			if (m_mouseHistory >= SCE_OK)
			{
				WindowHandle handle = { UINT16_MAX };

				for(int i=0;i<m_mouseHistory;i++)
				{
					uint32_t oldMouseButtons = m_mouseButtons;
					m_mouseButtons = m_mouseData[i].buttons;
					oldMouseButtons ^= m_mouseButtons;

					m_mouseX += m_mouseData[i].xAxis;
					m_mouseY += m_mouseData[i].yAxis;
					m_mouseZ += m_mouseData[i].wheel;
					m_tilt   += m_mouseData[i].tilt;

					if (oldMouseButtons & SCE_MOUSE_BUTTON_PRIMARY)
						g_eventQueue.postMouseEvent(handle, m_mouseX, m_mouseY, m_mouseZ, MouseButton::Left, s_modifiers, m_mouseData[i].buttons & SCE_MOUSE_BUTTON_PRIMARY, false);

					if (oldMouseButtons & SCE_MOUSE_BUTTON_SECONDARY)
						g_eventQueue.postMouseEvent(handle, m_mouseX, m_mouseY, m_mouseZ, MouseButton::RightButton, s_modifiers, m_mouseData[i].buttons & SCE_MOUSE_BUTTON_SECONDARY, false);

					if (oldMouseButtons & SCE_MOUSE_BUTTON_OPTIONAL)
						g_eventQueue.postMouseEvent(handle, m_mouseX, m_mouseY, m_mouseZ, MouseButton::Middle, s_modifiers, m_mouseData[i].buttons & SCE_MOUSE_BUTTON_OPTIONAL, false);
				}
			}
		}
	};

	InputPS4 s_input;
	uint8_t InputPS4::s_translateKey[256];
	uint8_t InputPS4::s_modifiers = 0;

	#define RAPP_CMD_READ(_type, _name)		\
		_type _name;						\
		while (!m_channel.read(&_name))

	#define RAPP_CMD_WRITE(_val)			\
		while (!getContext().m_channel.write(_val));

		struct Context
	{
		enum Command : uint8_t
		{
			None,
			RunFunc,
			Quit
		};

		Context()
			: m_channel(1024)
		{
		}

		void processMessages()
		{
			uintptr_t cmd = 0;
			if (m_channel.read(&cmd))
			{
				switch (cmd)
				{
					case Command::RunFunc:
						{
							RAPP_CMD_READ(ThreadFn, fn);
							RAPP_CMD_READ(void*,         userData);

							fn(userData);
						}
						break;

					case Command::Quit:
						{
							RAPP_CMD_READ(App*, app);
							app->quit();
						}
						break;

				default:
					RTM_ASSERT(false, "Invalid command!");
				};
			}
		}

		int32_t run(int _argc, const char* const*  _argv)
		{
			s_input.init();

			MainThreadEntry mte;
			mte.m_argc = _argc;
			mte.m_argv = _argv;

			rtm::Thread thread;
			thread.start(mte.threadFunc, &mte);

			while (!m_exit)
			{
				s_input.update(m_eventQueue);
				processMessages();
			}

			thread.stop();

			s_input.shutdown();

			return mte.m_result;
		}

		EventQueue			m_eventQueue;
		rtm::Mutex			m_lock;
		rtm::SpScQueue<>	m_channel;

		bool m_exit;
	};

	Context& getContext()
	{
		static Context s_ctx;
		return s_ctx;
	}

	int32_t MainThreadEntry::threadFunc(void* _userData)
	{
		MainThreadEntry* self = (MainThreadEntry*)_userData;
		self->m_result = main(self->m_argc, self->m_argv);
		getContext().m_exit = true;
		return self->m_result;
	}

	const Event* poll()
	{
		return g_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		return g_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		g_eventQueue.release(_event);
	}
	
	void appRunOnMainThread(ThreadFn _fn, void* _userData)
	{
		RAPP_CMD_WRITE(Context::Command::RunFunc);
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
		RTM_UNUSED(_app);
		RTM_UNUSED_6(_x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void* windowGetNativeHandle(WindowHandle _handle)
	{
		RTM_UNUSED_1(_handle);
	}

	void* windowGetNativeDisplayHandle()
	{
		return NULL;
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

	void inputEmitKeyPress(KeyboardKey::Enum _key, uint8_t _modifiers)
	{
		g_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, true);
		g_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, false);
	}
}

int main(int _argc, const char* const* _argv)
{
	return rapp::getContext().run(_argc, _argv);
}

#endif // RTM_PLATFORM_PS4
