/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <rapp_pch.h>
#include <rapp/inc/rapp.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_EMSCRIPTEN

#include <bx/bx.h>

#if RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

extern "C" void entry_emscripten_yield()
{
//	emscripten_sleep(0);
}

static const char* s_canvasID = "#canvas";

#define _EMSCRIPTEN_CHECK(_check, _call)                                                                  \
	BX_MACRO_BLOCK_BEGIN                                                                                  \
		EMSCRIPTEN_RESULT __result__ = _call;                                                             \
		_check(EMSCRIPTEN_RESULT_SUCCESS == __result__, #_call " FAILED 0x%08x\n", (uint32_t)__result__); \
		BX_UNUSED(__result__);                                                                            \
	BX_MACRO_BLOCK_END

#define EMSCRIPTEN_CHECK(_call) _EMSCRIPTEN_CHECK(BX_ASSERT, _call)

namespace rapp
{
	static uint8_t s_translateKey[256];
	static uint8_t s_translateKeyHigh[128];

	struct Context
	{
		Context()
			: m_scrollf(0.0f)
			, m_mx(0)
			, m_my(0)
			, m_scroll(0)
		{
			memset(s_translateKey, 0, sizeof(s_translateKey));
			s_translateKey[DOM_PK_ESCAPE]			= KeyboardState::Key::Esc;
			s_translateKey[DOM_PK_0]				= KeyboardState::Key::Key0;
			s_translateKey[DOM_PK_1]				= KeyboardState::Key::Key1;
			s_translateKey[DOM_PK_2]				= KeyboardState::Key::Key2;
			s_translateKey[DOM_PK_3]				= KeyboardState::Key::Key3;
			s_translateKey[DOM_PK_4]				= KeyboardState::Key::Key4;
			s_translateKey[DOM_PK_5]				= KeyboardState::Key::Key5;
			s_translateKey[DOM_PK_6]				= KeyboardState::Key::Key6;
			s_translateKey[DOM_PK_7]				= KeyboardState::Key::Key7;
			s_translateKey[DOM_PK_8]				= KeyboardState::Key::Key8;
			s_translateKey[DOM_PK_9]				= KeyboardState::Key::Key9;
			s_translateKey[DOM_PK_MINUS]			= KeyboardState::Key::Minus;
			s_translateKey[DOM_PK_EQUAL]			= KeyboardState::Key::Equal;
			s_translateKey[DOM_PK_BACKSPACE]		= KeyboardState::Key::Backspace;
			s_translateKey[DOM_PK_TAB]				= KeyboardState::Key::Tab;
			s_translateKey[DOM_PK_Q]				= KeyboardState::Key::KeyQ;
			s_translateKey[DOM_PK_W]				= KeyboardState::Key::KeyW;
			s_translateKey[DOM_PK_E]				= KeyboardState::Key::KeyE;
			s_translateKey[DOM_PK_R]				= KeyboardState::Key::KeyR;
			s_translateKey[DOM_PK_T]				= KeyboardState::Key::KeyT;
			s_translateKey[DOM_PK_Y]				= KeyboardState::Key::KeyY;
			s_translateKey[DOM_PK_U]				= KeyboardState::Key::KeyU;
			s_translateKey[DOM_PK_I]				= KeyboardState::Key::KeyI;
			s_translateKey[DOM_PK_O]				= KeyboardState::Key::KeyO;
			s_translateKey[DOM_PK_P]				= KeyboardState::Key::KeyP;
			s_translateKey[DOM_PK_BRACKET_LEFT]		= KeyboardState::Key::LeftBracket;
			s_translateKey[DOM_PK_BRACKET_RIGHT]	= KeyboardState::Key::RightBracket;
			s_translateKey[DOM_PK_ENTER]			= KeyboardState::Key::Return;
			s_translateKey[DOM_PK_A]				= KeyboardState::Key::KeyA;
			s_translateKey[DOM_PK_S]				= KeyboardState::Key::KeyS;
			s_translateKey[DOM_PK_D]				= KeyboardState::Key::KeyD;
			s_translateKey[DOM_PK_F]				= KeyboardState::Key::KeyF;
			s_translateKey[DOM_PK_G]				= KeyboardState::Key::KeyG;
			s_translateKey[DOM_PK_H]				= KeyboardState::Key::KeyH;
			s_translateKey[DOM_PK_J]				= KeyboardState::Key::KeyJ;
			s_translateKey[DOM_PK_K]				= KeyboardState::Key::KeyK;
			s_translateKey[DOM_PK_L]				= KeyboardState::Key::KeyL;
			s_translateKey[DOM_PK_SEMICOLON]		= KeyboardState::Key::Semicolon;
			s_translateKey[DOM_PK_QUOTE]			= KeyboardState::Key::Quote;
			s_translateKey[DOM_PK_BACKQUOTE]		= KeyboardState::Key::Tilde;
			s_translateKey[DOM_PK_BACKSLASH]		= KeyboardState::Key::Backslash;
			s_translateKey[DOM_PK_Z]				= KeyboardState::Key::KeyZ;
			s_translateKey[DOM_PK_X]				= KeyboardState::Key::KeyX;
			s_translateKey[DOM_PK_C]				= KeyboardState::Key::KeyC;
			s_translateKey[DOM_PK_V]				= KeyboardState::Key::KeyV;
			s_translateKey[DOM_PK_B]				= KeyboardState::Key::KeyB;
			s_translateKey[DOM_PK_N]				= KeyboardState::Key::KeyN;
			s_translateKey[DOM_PK_M]				= KeyboardState::Key::KeyM;
			s_translateKey[DOM_PK_COMMA]			= KeyboardState::Key::Comma;
			s_translateKey[DOM_PK_PERIOD]			= KeyboardState::Key::Period;
			s_translateKey[DOM_PK_SLASH]			= KeyboardState::Key::Slash;
			s_translateKey[DOM_PK_SPACE]			= KeyboardState::Key::Space;
			s_translateKey[DOM_PK_F1]				= KeyboardState::Key::F1;
			s_translateKey[DOM_PK_F2]				= KeyboardState::Key::F2;
			s_translateKey[DOM_PK_F3]				= KeyboardState::Key::F3;
			s_translateKey[DOM_PK_F4]				= KeyboardState::Key::F4;
			s_translateKey[DOM_PK_F5]				= KeyboardState::Key::F5;
			s_translateKey[DOM_PK_F6]				= KeyboardState::Key::F6;
			s_translateKey[DOM_PK_F7]				= KeyboardState::Key::F7;
			s_translateKey[DOM_PK_F8]				= KeyboardState::Key::F8;
			s_translateKey[DOM_PK_F9]				= KeyboardState::Key::F9;
			s_translateKey[DOM_PK_F10]				= KeyboardState::Key::F10;
			s_translateKey[DOM_PK_NUMPAD_7]			= KeyboardState::Key::NumPad7;
			s_translateKey[DOM_PK_NUMPAD_8]			= KeyboardState::Key::NumPad8;
			s_translateKey[DOM_PK_NUMPAD_9]			= KeyboardState::Key::NumPad9;
			s_translateKey[DOM_PK_NUMPAD_SUBTRACT]	= KeyboardState::Key::Minus;
			s_translateKey[DOM_PK_NUMPAD_4]			= KeyboardState::Key::NumPad4;
			s_translateKey[DOM_PK_NUMPAD_5]			= KeyboardState::Key::NumPad5;
			s_translateKey[DOM_PK_NUMPAD_6]			= KeyboardState::Key::NumPad6;
			s_translateKey[DOM_PK_NUMPAD_ADD]		= KeyboardState::Key::Plus;     
			s_translateKey[DOM_PK_NUMPAD_1]			= KeyboardState::Key::NumPad1;
			s_translateKey[DOM_PK_NUMPAD_2]			= KeyboardState::Key::NumPad2;
			s_translateKey[DOM_PK_NUMPAD_3]			= KeyboardState::Key::NumPad3;
			s_translateKey[DOM_PK_NUMPAD_0]			= KeyboardState::Key::NumPad0;
			s_translateKey[DOM_PK_INTL_BACKSLASH]	= KeyboardState::Key::Backslash;
			s_translateKey[DOM_PK_F11]				= KeyboardState::Key::F11;
			s_translateKey[DOM_PK_F12]				= KeyboardState::Key::F12;
			s_translateKey[DOM_PK_NUMPAD_EQUAL]		= KeyboardState::Key::Equal;   
			s_translateKey[DOM_PK_NUMPAD_COMMA]		= KeyboardState::Key::Comma;


			memset(s_translateKeyHigh, 0, sizeof(s_translateKeyHigh));
			s_translateKeyHigh[DOM_PK_NUMPAD_ENTER & 0xff]	= KeyboardState::Key::Return;
			s_translateKeyHigh[DOM_PK_HOME & 0xff]			= KeyboardState::Key::Home;
			s_translateKeyHigh[DOM_PK_ARROW_UP & 0xff]		= KeyboardState::Key::Up;
			s_translateKeyHigh[DOM_PK_PAGE_UP & 0xff]		= KeyboardState::Key::PageUp;
			s_translateKeyHigh[DOM_PK_ARROW_LEFT & 0xff]	= KeyboardState::Key::Left;
			s_translateKeyHigh[DOM_PK_ARROW_RIGHT & 0xff]	= KeyboardState::Key::Right;
			s_translateKeyHigh[DOM_PK_ARROW_DOWN & 0xff]	= KeyboardState::Key::Down;
			s_translateKeyHigh[DOM_PK_PAGE_DOWN & 0xff]		= KeyboardState::Key::PageDown;
			s_translateKeyHigh[DOM_PK_INSERT & 0xff]		= KeyboardState::Key::Insert;
			s_translateKeyHigh[DOM_PK_DELETE & 0xff]		= KeyboardState::Key::Delete;

			for (char ch = 'a'; ch <= 'z'; ++ch)
			{
				s_translateKey[uint8_t(ch)]       =
				s_translateKey[uint8_t(ch - ' ')] = KeyboardState::Key::KeyA + (ch - 'a');
			}
		}

		int32_t run(int _argc, const char* const* _argv)
		{
			EMSCRIPTEN_CHECK(emscripten_set_mousedown_callback(s_canvasID, this, true, mouseCb) );
			EMSCRIPTEN_CHECK(emscripten_set_mouseup_callback(s_canvasID, this, true, mouseCb) );
			EMSCRIPTEN_CHECK(emscripten_set_mousemove_callback(s_canvasID, this, true, mouseCb) );

			EMSCRIPTEN_CHECK(emscripten_set_wheel_callback(s_canvasID, this, true, wheelCb) );

			EMSCRIPTEN_CHECK(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );
			EMSCRIPTEN_CHECK(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );
			EMSCRIPTEN_CHECK(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );

			EMSCRIPTEN_CHECK(emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, resizeCb) );

			//EmscriptenFullscreenStrategy fullscreenStrategy = {};
			//fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			//fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
			//fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			//fullscreenStrategy.canvasResizedCallback = canvasResizeCb;
			//fullscreenStrategy.canvasResizedCallbackUserData = this;
			//EMSCRIPTEN_CHECK(emscripten_request_fullscreen_strategy(s_canvasID, false, &fullscreenStrategy) );

			//EmscriptenFullscreenStrategy fullscreenStrategy = {};
			//fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			//fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
			//fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			//fullscreenStrategy.canvasResizedCallback = canvasResizeCb;
			//fullscreenStrategy.canvasResizedCallbackUserData = this;   // pointer to user data
			//EMSCRIPTEN_CHECK(emscripten_enter_soft_fullscreen(s_canvasID, &fullscreenStrategy));

			EMSCRIPTEN_CHECK(emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );
			EMSCRIPTEN_CHECK(emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );
			EMSCRIPTEN_CHECK(emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );

			EMSCRIPTEN_CHECK(emscripten_set_gamepadconnected_callback(this, true, gamepadCb) );
			EMSCRIPTEN_CHECK(emscripten_set_gamepaddisconnected_callback(this, true, gamepadCb) );

			int32_t result = main(_argc, _argv);

			return result;
		}

		static EM_BOOL mouseCb(int _eventType, const EmscriptenMouseEvent* event, void* userData);
		static EM_BOOL wheelCb(int _eventType, const EmscriptenWheelEvent* event, void* userData);
		static EM_BOOL keyCb(int _eventType, const EmscriptenKeyboardEvent* event, void* userData);
		static EM_BOOL resizeCb(int _eventType, const EmscriptenUiEvent* event, void* userData);
		static EM_BOOL canvasResizeCb(int _eventType, const void* reserved, void* userData);
		static EM_BOOL focusCb(int _eventType, const EmscriptenFocusEvent* event, void* userData);
		static EM_BOOL gamepadCb(int _eventType, const EmscriptenGamepadEvent *_gamepadEvent, void *userData);


		EventQueue m_eventQueue;

		float   m_scrollf;
		int32_t m_mx;
		int32_t m_my;
		int32_t m_scroll;
	};

	static Context s_ctx;

	EM_BOOL Context::mouseCb(int32_t _eventType, const EmscriptenMouseEvent* _event, void* _userData)
	{
		BX_UNUSED(_userData);

		if (_event)
		{
			switch (_eventType)
			{
				case EMSCRIPTEN_EVENT_MOUSEMOVE:
					s_ctx.m_mx = _event->targetX;
					s_ctx.m_my = _event->targetY;
					s_ctx.m_eventQueue.postMouseEvent(rapp::kDefaultWindowHandle, s_ctx.m_mx, s_ctx.m_my, s_ctx.m_scroll, 0);
					return true;

				case EMSCRIPTEN_EVENT_MOUSEDOWN:
				case EMSCRIPTEN_EVENT_MOUSEUP:
				case EMSCRIPTEN_EVENT_DBLCLICK:
					s_ctx.m_mx = _event->targetX;
					s_ctx.m_my = _event->targetY;
					MouseState::Button mb = _event->button == 2
						? MouseState::Button::Right  : ( (_event->button == 1)
						? MouseState::Button::Middle : MouseState::Button::Left)
						;
					s_ctx.m_eventQueue.postMouseEvent(
						  rapp::kDefaultWindowHandle
						, s_ctx.m_mx
						, s_ctx.m_my
						, s_ctx.m_scroll
						, mb
						, 0
						, (_eventType != EMSCRIPTEN_EVENT_MOUSEUP)
						, false
						);
					return true;
			}
		}

		return false;
	}

	EM_BOOL Context::wheelCb(int32_t _eventType, const EmscriptenWheelEvent* _event, void* _userData)
	{
		BX_UNUSED(_userData);

		if (_event)
		{
			switch (_eventType)
			{
				case EMSCRIPTEN_EVENT_WHEEL:
				{
					s_ctx.m_scrollf += _event->deltaY;

					s_ctx.m_scroll = (int32_t)s_ctx.m_scrollf;
					s_ctx.m_eventQueue.postMouseEvent(kDefaultWindowHandle, s_ctx.m_mx, s_ctx.m_my, s_ctx.m_scroll, 0);
					return true;
				}
			}
		}

		return false;
	}

	uint8_t translateModifiers(const EmscriptenKeyboardEvent* _event)
	{
		uint8_t mask = 0;

		if (_event->shiftKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardState::Modifier::LShift;
			else
				mask |= KeyboardState::Modifier::RShift;
		}

		if (_event->altKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardState::Modifier::LAlt;
			else
				mask |= KeyboardState::Modifier::RAlt;
		}

		if (_event->ctrlKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardState::Modifier::LCtrl;
			else
				mask |= KeyboardState::Modifier::RCtrl;
		}

		if (_event->metaKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardState::Modifier::LMeta;
			else
				mask |= KeyboardState::Modifier::RMeta;
		}

		return mask;
	}

	static int number_of_characters_in_utf8_string(const char *str)
	{
		if (!str) return 0;
		int num_chars = 0;
		while(*str)
		{
			if ((*str++ & 0xC0) != 0x80) ++num_chars; // Skip all continuation bytes
		}
		return num_chars;
	}

	//static int emscripten_key_event_is_printable_character(const EmscriptenKeyboardEvent *keyEvent)
	//{
	//	// Not sure if this is correct, but heuristically looks good. Improvements on corner cases welcome.
	//	return number_of_characters_in_utf8_string(keyEvent->key) == 1;
	//}

	KeyboardState::Key handleKeyEvent(const EmscriptenKeyboardEvent* _event, uint8_t* _modifiers, uint8_t* _pressedChar)
	{
		*(unsigned long*)_pressedChar = _event->keyCode;
		int dom_pk_code = emscripten_compute_dom_pk_code(_event->code);

		*_modifiers = translateModifiers(_event);

		// if this is a unhandled key just return None
		if (dom_pk_code < 256)
			return KeyboardState::Key(s_translateKey[dom_pk_code]);

		return KeyboardState::Key(s_translateKeyHigh[dom_pk_code & 0xff]);
	}

	EM_BOOL Context::keyCb(int32_t _eventType, const EmscriptenKeyboardEvent* _event, void* _userData)
	{
		BX_UNUSED(_userData);

		bool ret =  _event->keyCode == DOM_VK_BACK_SPACE // Don't navigate away from this test page on backspace.
				||  _event->keyCode == DOM_VK_TAB // Don't change keyboard focus to different browser UI elements while testing.
				|| (_event->keyCode >= DOM_VK_F1 && _event->keyCode <= DOM_VK_F24) // Don't F5 refresh the test page to reload.
				||  _event->ctrlKey // Don't trigger e.g. Ctrl-B to open bookmarks
				||  _event->altKey // Don't trigger any alt-X based shortcuts either (Alt-F4 is not overrideable though)
				||  _eventType == EMSCRIPTEN_EVENT_KEYPRESS || _eventType == EMSCRIPTEN_EVENT_KEYUP; // Don't perform any default actions on these.

		if (_event)
		{
			uint8_t modifiers = 0;
			uint8_t pressedChar[4];
			KeyboardState::Key key = handleKeyEvent(_event, &modifiers, &pressedChar[0]);

			// Returning true means that we take care of the key (instead of the default behavior)
			switch (_eventType)
			{
				case EMSCRIPTEN_EVENT_KEYPRESS:
				case EMSCRIPTEN_EVENT_KEYDOWN:
					if (key == KeyboardState::Key::KeyQ && (modifiers & KeyboardState::Modifier::RMeta) )
					{
						s_ctx.m_eventQueue.postExitEvent();
					}
					else
					{
						if (_eventType == EMSCRIPTEN_EVENT_KEYPRESS)
						{
//							if (emscripten_key_event_is_printable_character(_event))
								s_ctx.m_eventQueue.postCharEvent(kDefaultWindowHandle, number_of_characters_in_utf8_string((char*)pressedChar), pressedChar);

							return ret;
						}

						s_ctx.m_eventQueue.postKeyEvent(kDefaultWindowHandle, key, modifiers, true);
						return ret;
					}
					break;

				case EMSCRIPTEN_EVENT_KEYUP:
					s_ctx.m_eventQueue.postKeyEvent(kDefaultWindowHandle, key, modifiers, false);
					return ret;
			}
		}
		return false;
	}

	EM_BOOL Context::resizeCb(int32_t _eventType, const EmscriptenUiEvent* _event, void* _userData)
	{
		BX_UNUSED(_eventType, _event, _userData);
		return false;
	}

	EM_BOOL Context::canvasResizeCb(int32_t _eventType, const void* _reserved, void* _userData)
	{
		BX_UNUSED(_eventType, _reserved, _userData);
		int width, height;
		emscripten_get_canvas_element_size(s_canvasID, &width, &height);
		s_ctx.m_eventQueue.postSizeEvent(rapp::kDefaultWindowHandle, width, height);
		return true;
	}

	EM_BOOL Context::focusCb(int32_t _eventType, const EmscriptenFocusEvent* _event, void* _userData)
	{
		BX_UNUSED(_event, _userData);

		if (_event)
		{
			switch (_eventType)
			{
				case EMSCRIPTEN_EVENT_BLUR:
					s_ctx.m_eventQueue.postSuspendEvent(kDefaultWindowHandle, SuspendEvent::Enum::DidSuspend);
					return true;

				case EMSCRIPTEN_EVENT_FOCUS:
					s_ctx.m_eventQueue.postSuspendEvent(kDefaultWindowHandle, SuspendEvent::Enum::DidResume);
					return true;

				case EMSCRIPTEN_EVENT_FOCUSIN:
					s_ctx.m_eventQueue.postSuspendEvent(kDefaultWindowHandle, SuspendEvent::Enum::WillResume);
					return true;

				case EMSCRIPTEN_EVENT_FOCUSOUT:
					s_ctx.m_eventQueue.postSuspendEvent(kDefaultWindowHandle, SuspendEvent::Enum::WillSuspend);
					return true;
			}
		}

		return false;
	}

	static GamepadState::Buttons s_buttonRemap[] =
	{
		GamepadState::A,
		GamepadState::B,
		GamepadState::X,
		GamepadState::Y,
		GamepadState::LShoulder,
		GamepadState::RShoulder,
		GamepadState::None, //TL
		GamepadState::None,//TR
		GamepadState::Back,
		GamepadState::Start,
		GamepadState::LThumb,
		GamepadState::RThumb,
		GamepadState::Up,
		GamepadState::Down,
		GamepadState::Left,
		GamepadState::Right
	};

	static GamepadAxis::Enum s_axisRemap[] =
	{
		GamepadAxis::LeftX,
		GamepadAxis::LeftY,
		GamepadAxis::RightX,
		GamepadAxis::RightY
	};

	void emscriptenUpdateGamepads()
	{
		static EmscriptenGamepadEvent prevState[32];
		static int prevNumGamepads = 0;

		int numGamepads = 0;
		if (EMSCRIPTEN_RESULT_SUCCESS == emscripten_sample_gamepad_data())
		{
			numGamepads = emscripten_get_num_gamepads();
			if (numGamepads != prevNumGamepads)
				prevNumGamepads = numGamepads;
		}

		for(int i=0; i<numGamepads && i<32; ++i)
		{
			EmscriptenGamepadEvent ge;
			int ret = emscripten_get_gamepad_status(i, &ge);
			if (ret == EMSCRIPTEN_RESULT_SUCCESS)
			{
				GamepadHandle handle = { static_cast<uint16_t>(i) };
				int g = ge.index;

				for(int j=0; (j<ge.numAxes) && (j<4); ++j)
				{
					if (ge.axis[j] != prevState[g].axis[j])
						s_ctx.m_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, s_axisRemap[j], ge.axis[j] * 32768.0f);
				}

				for(int j=0; j<ge.numButtons; ++j)
				{
					if ((ge.analogButton[j] != prevState[g].analogButton[j]) || (ge.digitalButton[j] != prevState[g].digitalButton[j]))
					{
						if (j == 6) // L trigger
							s_ctx.m_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::LeftZ, 255.0f * ge.analogButton[j]);
						else
						if (j == 7) // R trigger
							s_ctx.m_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, handle, GamepadAxis::RightZ, 255.0f * ge.analogButton[j]);
						else
							s_ctx.m_eventQueue.postGamepadButtonEvent(rapp::kDefaultWindowHandle, handle, s_buttonRemap[j], ge.analogButton[j]);
					}
				}
				prevState[g] = ge;
			}
		}
	}

	EM_BOOL Context::gamepadCb(int _eventType, const EmscriptenGamepadEvent* _gamepadEvent, void* _userData)
	{
		BX_UNUSED(_eventType, _userData);
		if (_gamepadEvent->connected)
		{
			GamepadHandle handle = { static_cast<uint16_t>(_gamepadEvent->index) };
			s_ctx.m_eventQueue.postGamepadEvent(rapp::kDefaultWindowHandle, handle, _gamepadEvent->connected != 0);
		}
		return false;
	}

	const Event* poll()
	{
		entry_emscripten_yield();
		return s_ctx.m_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		entry_emscripten_yield();
		return s_ctx.m_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		s_ctx.m_eventQueue.release(_event);
	}

	void appRunOnMainThread(ThreadFn _fn, void* _userData)
	{
		_fn(_userData);
	}

	void windowGetDefaultSize(uint32_t* _width, uint32_t* _height)
	{
		RTM_ASSERT(_width, "");
		RTM_ASSERT(_height, "");
		double width, height;
		emscripten_get_element_css_size(s_canvasID, &width, &height);
		*_width		= (int)width;
		*_height	= (int)height;
	}

	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags, const char* _title)
	{
		BX_UNUSED(_app, _x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { kDefaultWindowHandle.idx };

		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		BX_UNUSED(_handle);
	}

	void setWindowPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		BX_UNUSED(_handle, _x, _y);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		BX_UNUSED(_handle, _width, _height);
		//if (_handle.idx == rapp::kDefaultWindowHandle.idx)
		//	emscripten_set_canvas_element_size(s_canvasID, _width, _height);  css_element?
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		BX_UNUSED(_handle);
		emscripten_set_window_title(_title);
	}

	void setWindowFlags(WindowHandle _handle, uint32_t _flags, bool _enabled)
	{
		BX_UNUSED(_handle, _flags, _enabled);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		BX_UNUSED(_handle);

		//EmscriptenFullscreenChangeEvent state;
		//emscripten_get_fullscreen_status(&state);

		//if (!state.isFullscreen)
		//{
		//	EmscriptenFullscreenStrategy fullscreenStrategy = {};
		//	fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
		//	fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
		//	fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
		//	fullscreenStrategy.canvasResizedCallback = canvasResizeCb;
		//	fullscreenStrategy.canvasResizedCallbackUserData = this;

		//	EMSCRIPTEN_CHECK(emscripten_request_fullscreen_strategy(s_canvasID, false, &fullscreenStrategy) );
		//}
		//else
		//	emscripten_exit_fullscreen();
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		BX_UNUSED(_handle, _lock);
	}
	 
	void* windowGetNativeHandle(WindowHandle _handle)
	{
		if (kDefaultWindowHandle.idx == _handle.idx)
			return (void*)s_canvasID;

		return NULL;
	}

	void* windowGetNativeDisplayHandle()
	{
		return NULL;
	}
}

int main(int _argc, const char* const* _argv)
{
	using namespace entry;
	return rapp::s_ctx.run(_argc, _argv);
}

#endif // RTM_PLATFORM_EMSCRIPTEN
