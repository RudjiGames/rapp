/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <rapp_pch.h>
#include <rapp/inc/rapp.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_EMSCRIPTEN

#ifdef RAPP_WITH_BGFX
#include <bx/bx.h>
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
	for(;;) {			                                                                                  \
		EMSCRIPTEN_RESULT __result__ = _call;                                                             \
		_check(EMSCRIPTEN_RESULT_SUCCESS == __result__, #_call " FAILED 0x%08x\n", (uint32_t)__result__); \
		RTM_UNUSED(__result__);                                                                           \
	break; }

#define EMSCRIPTEN_CHECK(_call) _EMSCRIPTEN_CHECK(RTM_ASSERT, _call)

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
			s_translateKey[DOM_PK_ESCAPE]			= KeyboardKey::Esc;
			s_translateKey[DOM_PK_0]				= KeyboardKey::Key0;
			s_translateKey[DOM_PK_1]				= KeyboardKey::Key1;
			s_translateKey[DOM_PK_2]				= KeyboardKey::Key2;
			s_translateKey[DOM_PK_3]				= KeyboardKey::Key3;
			s_translateKey[DOM_PK_4]				= KeyboardKey::Key4;
			s_translateKey[DOM_PK_5]				= KeyboardKey::Key5;
			s_translateKey[DOM_PK_6]				= KeyboardKey::Key6;
			s_translateKey[DOM_PK_7]				= KeyboardKey::Key7;
			s_translateKey[DOM_PK_8]				= KeyboardKey::Key8;
			s_translateKey[DOM_PK_9]				= KeyboardKey::Key9;
			s_translateKey[DOM_PK_MINUS]			= KeyboardKey::Minus;
			s_translateKey[DOM_PK_EQUAL]			= KeyboardKey::Equal;
			s_translateKey[DOM_PK_BACKSPACE]		= KeyboardKey::Backspace;
			s_translateKey[DOM_PK_TAB]				= KeyboardKey::Tab;
			s_translateKey[DOM_PK_Q]				= KeyboardKey::KeyQ;
			s_translateKey[DOM_PK_W]				= KeyboardKey::KeyW;
			s_translateKey[DOM_PK_E]				= KeyboardKey::KeyE;
			s_translateKey[DOM_PK_R]				= KeyboardKey::KeyR;
			s_translateKey[DOM_PK_T]				= KeyboardKey::KeyT;
			s_translateKey[DOM_PK_Y]				= KeyboardKey::KeyY;
			s_translateKey[DOM_PK_U]				= KeyboardKey::KeyU;
			s_translateKey[DOM_PK_I]				= KeyboardKey::KeyI;
			s_translateKey[DOM_PK_O]				= KeyboardKey::KeyO;
			s_translateKey[DOM_PK_P]				= KeyboardKey::KeyP;
			s_translateKey[DOM_PK_BRACKET_LEFT]		= KeyboardKey::LeftBracket;
			s_translateKey[DOM_PK_BRACKET_RIGHT]	= KeyboardKey::RightBracket;
			s_translateKey[DOM_PK_ENTER]			= KeyboardKey::Return;
			s_translateKey[DOM_PK_A]				= KeyboardKey::KeyA;
			s_translateKey[DOM_PK_S]				= KeyboardKey::KeyS;
			s_translateKey[DOM_PK_D]				= KeyboardKey::KeyD;
			s_translateKey[DOM_PK_F]				= KeyboardKey::KeyF;
			s_translateKey[DOM_PK_G]				= KeyboardKey::KeyG;
			s_translateKey[DOM_PK_H]				= KeyboardKey::KeyH;
			s_translateKey[DOM_PK_J]				= KeyboardKey::KeyJ;
			s_translateKey[DOM_PK_K]				= KeyboardKey::KeyK;
			s_translateKey[DOM_PK_L]				= KeyboardKey::KeyL;
			s_translateKey[DOM_PK_SEMICOLON]		= KeyboardKey::Semicolon;
			s_translateKey[DOM_PK_QUOTE]			= KeyboardKey::Quote;
			s_translateKey[DOM_PK_BACKQUOTE]		= KeyboardKey::Tilde;
			s_translateKey[DOM_PK_BACKSLASH]		= KeyboardKey::Backslash;
			s_translateKey[DOM_PK_Z]				= KeyboardKey::KeyZ;
			s_translateKey[DOM_PK_X]				= KeyboardKey::KeyX;
			s_translateKey[DOM_PK_C]				= KeyboardKey::KeyC;
			s_translateKey[DOM_PK_V]				= KeyboardKey::KeyV;
			s_translateKey[DOM_PK_B]				= KeyboardKey::KeyB;
			s_translateKey[DOM_PK_N]				= KeyboardKey::KeyN;
			s_translateKey[DOM_PK_M]				= KeyboardKey::KeyM;
			s_translateKey[DOM_PK_COMMA]			= KeyboardKey::Comma;
			s_translateKey[DOM_PK_PERIOD]			= KeyboardKey::Period;
			s_translateKey[DOM_PK_SLASH]			= KeyboardKey::Slash;
			s_translateKey[DOM_PK_SPACE]			= KeyboardKey::Space;
			s_translateKey[DOM_PK_F1]				= KeyboardKey::F1;
			s_translateKey[DOM_PK_F2]				= KeyboardKey::F2;
			s_translateKey[DOM_PK_F3]				= KeyboardKey::F3;
			s_translateKey[DOM_PK_F4]				= KeyboardKey::F4;
			s_translateKey[DOM_PK_F5]				= KeyboardKey::F5;
			s_translateKey[DOM_PK_F6]				= KeyboardKey::F6;
			s_translateKey[DOM_PK_F7]				= KeyboardKey::F7;
			s_translateKey[DOM_PK_F8]				= KeyboardKey::F8;
			s_translateKey[DOM_PK_F9]				= KeyboardKey::F9;
			s_translateKey[DOM_PK_F10]				= KeyboardKey::F10;
			s_translateKey[DOM_PK_NUMPAD_7]			= KeyboardKey::NumPad7;
			s_translateKey[DOM_PK_NUMPAD_8]			= KeyboardKey::NumPad8;
			s_translateKey[DOM_PK_NUMPAD_9]			= KeyboardKey::NumPad9;
			s_translateKey[DOM_PK_NUMPAD_SUBTRACT]	= KeyboardKey::Minus;
			s_translateKey[DOM_PK_NUMPAD_4]			= KeyboardKey::NumPad4;
			s_translateKey[DOM_PK_NUMPAD_5]			= KeyboardKey::NumPad5;
			s_translateKey[DOM_PK_NUMPAD_6]			= KeyboardKey::NumPad6;
			s_translateKey[DOM_PK_NUMPAD_ADD]		= KeyboardKey::Plus;     
			s_translateKey[DOM_PK_NUMPAD_1]			= KeyboardKey::NumPad1;
			s_translateKey[DOM_PK_NUMPAD_2]			= KeyboardKey::NumPad2;
			s_translateKey[DOM_PK_NUMPAD_3]			= KeyboardKey::NumPad3;
			s_translateKey[DOM_PK_NUMPAD_0]			= KeyboardKey::NumPad0;
			s_translateKey[DOM_PK_INTL_BACKSLASH]	= KeyboardKey::Backslash;
			s_translateKey[DOM_PK_F11]				= KeyboardKey::F11;
			s_translateKey[DOM_PK_F12]				= KeyboardKey::F12;
			s_translateKey[DOM_PK_NUMPAD_EQUAL]		= KeyboardKey::Equal;   
			s_translateKey[DOM_PK_NUMPAD_COMMA]		= KeyboardKey::Comma;


			memset(s_translateKeyHigh, 0, sizeof(s_translateKeyHigh));
			s_translateKeyHigh[DOM_PK_NUMPAD_ENTER & 0xff]	= KeyboardKey::Return;
			s_translateKeyHigh[DOM_PK_HOME & 0xff]			= KeyboardKey::Home;
			s_translateKeyHigh[DOM_PK_ARROW_UP & 0xff]		= KeyboardKey::Up;
			s_translateKeyHigh[DOM_PK_PAGE_UP & 0xff]		= KeyboardKey::PageUp;
			s_translateKeyHigh[DOM_PK_ARROW_LEFT & 0xff]	= KeyboardKey::Left;
			s_translateKeyHigh[DOM_PK_ARROW_RIGHT & 0xff]	= KeyboardKey::Right;
			s_translateKeyHigh[DOM_PK_ARROW_DOWN & 0xff]	= KeyboardKey::Down;
			s_translateKeyHigh[DOM_PK_PAGE_DOWN & 0xff]		= KeyboardKey::PageDown;
			s_translateKeyHigh[DOM_PK_INSERT & 0xff]		= KeyboardKey::Insert;
			s_translateKeyHigh[DOM_PK_DELETE & 0xff]		= KeyboardKey::Delete;

			for (char ch = 'a'; ch <= 'z'; ++ch)
			{
				s_translateKey[uint8_t(ch)]       =
				s_translateKey[uint8_t(ch - ' ')] = KeyboardKey::KeyA + (ch - 'a');
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

			EMSCRIPTEN_CHECK(emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, resizeCb) );

			//EmscriptenFullscreenStrategy fullscreenStrategy = {};
			//fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			//fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
			//fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			//fullscreenStrategy.canvasResizedCallback = canvasResizeCb;
			//fullscreenStrategy.canvasResizedCallbackUserData = this;
			//EMSCRIPTEN_CHECK(emscripten_request_fullscreen_strategy(s_canvasID, false, &fullscreenStrategy) );

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
		RTM_UNUSED(_userData);

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
					MouseButton mb = _event->button == 2
						? MouseButton::RightButton  : ( (_event->button == 1)
						? MouseButton::Middle : MouseButton::Left)
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
		RTM_UNUSED(_userData);

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
				mask |= KeyboardModifier::LShift;
			else
				mask |= KeyboardModifier::RShift;
		}

		if (_event->altKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardModifier::LAlt;
			else
				mask |= KeyboardModifier::RAlt;
		}

		if (_event->ctrlKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardModifier::LCtrl;
			else
				mask |= KeyboardModifier::RCtrl;
		}

		if (_event->metaKey)
		{
			if (_event->location == DOM_KEY_LOCATION_LEFT)
				mask |= KeyboardModifier::LMeta;
			else
				mask |= KeyboardModifier::RMeta;
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

	KeyboardKey handleKeyEvent(const EmscriptenKeyboardEvent* _event, uint8_t* _modifiers, uint8_t* _pressedChar)
	{
		*(unsigned long*)_pressedChar = _event->keyCode;
		int dom_pk_code = emscripten_compute_dom_pk_code(_event->code);

		*_modifiers = translateModifiers(_event);

		// if this is a unhandled key just return None
		if (dom_pk_code < 256)
			return KeyboardKey(s_translateKey[dom_pk_code]);

		return KeyboardKey(s_translateKeyHigh[dom_pk_code & 0xff]);
	}

	EM_BOOL Context::keyCb(int32_t _eventType, const EmscriptenKeyboardEvent* _event, void* _userData)
	{
		RTM_UNUSED(_userData);

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
			KeyboardKey key = handleKeyEvent(_event, &modifiers, &pressedChar[0]);

			// Returning true means that we take care of the key (instead of the default behavior)
			switch (_eventType)
			{
				case EMSCRIPTEN_EVENT_KEYPRESS:
				case EMSCRIPTEN_EVENT_KEYDOWN:
					if (key == KeyboardKey::KeyQ && (modifiers & KeyboardModifier::RMeta) )
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
		RTM_UNUSED_3(_eventType, _event, _userData);
		int width, height;
		emscripten_get_canvas_element_size(s_canvasID, &width, &height);
		s_ctx.m_eventQueue.postSizeEvent(rapp::kDefaultWindowHandle, width, height);
		return false;
	}

	EM_BOOL Context::canvasResizeCb(int32_t _eventType, const void* _reserved, void* _userData)
	{
		RTM_UNUSED_3(_eventType, _reserved, _userData);
		int width, height;
		emscripten_get_canvas_element_size(s_canvasID, &width, &height);
		s_ctx.m_eventQueue.postSizeEvent(rapp::kDefaultWindowHandle, width, height);
		return false;
	}

	EM_BOOL Context::focusCb(int32_t _eventType, const EmscriptenFocusEvent* _event, void* _userData)
	{
		RTM_UNUSED_2(_event, _userData);

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

	static GamepadButton::Enum s_buttonRemap[] =
	{
		GamepadButton::A,
		GamepadButton::B,
		GamepadButton::X,
		GamepadButton::Y,
		GamepadButton::LShoulder,
		GamepadButton::RShoulder,
		GamepadButton::None, //TL
		GamepadButton::None,//TR
		GamepadButton::Back,
		GamepadButton::Start,
		GamepadButton::LThumb,
		GamepadButton::RThumb,
		GamepadButton::Up,
		GamepadButton::Down,
		GamepadButton::Left,
		GamepadButton::Right
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
							s_ctx.m_eventQueue.postGamepadButtonsEvent(rapp::kDefaultWindowHandle, handle, s_buttonRemap[j], ge.analogButton[j]);
					}
				}
				prevState[g] = ge;
			}
		}
	}

	EM_BOOL Context::gamepadCb(int _eventType, const EmscriptenGamepadEvent* _gamepadEvent, void* _userData)
	{
		RTM_UNUSED_2(_eventType, _userData);
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
		RTM_UNUSED_4(_app, _x, _y, _width);
		RTM_UNUSED_3(_height, _flags, _title);
		WindowHandle handle = { kDefaultWindowHandle.idx };

		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void setWindowPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		RTM_UNUSED_3(_handle, _x, _y);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		RTM_UNUSED_3(_handle, _width, _height);
		//if (_handle.idx == rapp::kDefaultWindowHandle.idx)
		//	emscripten_set_canvas_element_size(s_canvasID, _width, _height);  css_element?
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		RTM_UNUSED(_handle);
		emscripten_set_window_title(_title);
	}

	void setWindowFlags(WindowHandle _handle, uint32_t _flags, bool _enabled)
	{
		RTM_UNUSED_3(_handle, _flags, _enabled);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);

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
		RTM_UNUSED_2(_handle, _lock);
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

	void inputEmitKeyPress(KeyboardKey::Enum _key, uint8_t _modifiers)
	{
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, true);
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, false);
	}
}

int main(int _argc, const char* const* _argv)
{
	return rapp::s_ctx.run(_argc, _argv);
}

#endif // RTM_PLATFORM_EMSCRIPTEN
