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

extern "C" void entry_emscripten_yield()
{
//	emscripten_sleep(0);
}

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

	struct Context
	{
		Context()
			: m_scrollf(0.0f)
			, m_mx(0)
			, m_my(0)
			, m_scroll(0)
		{
			memset(s_translateKey, 0, sizeof(s_translateKey));
			s_translateKey[27]             = KeyboardState::Key::Esc;
			s_translateKey[uint8_t('\n')]  =
			s_translateKey[uint8_t('\r')]  = KeyboardState::Key::Return;
			s_translateKey[uint8_t('\t')]  = KeyboardState::Key::Tab;
			s_translateKey[127]            = KeyboardState::Key::Backspace;
			s_translateKey[uint8_t(' ')]   = KeyboardState::Key::Space;
			s_translateKey[38]             = KeyboardState::Key::Up;
			s_translateKey[40]             = KeyboardState::Key::Down;
			s_translateKey[37]             = KeyboardState::Key::Left;
			s_translateKey[39]             = KeyboardState::Key::Right;

			s_translateKey[uint8_t('+')]   =
			s_translateKey[uint8_t('=')]   = KeyboardState::Key::Plus;
			s_translateKey[uint8_t('_')]   =
			s_translateKey[uint8_t('-')]   = KeyboardState::Key::Minus;

			s_translateKey[uint8_t(':')]   =
			s_translateKey[uint8_t(';')]   = KeyboardState::Key::Semicolon;
			s_translateKey[uint8_t('"')]   =
			s_translateKey[uint8_t('\'')]  = KeyboardState::Key::Quote;

			s_translateKey[uint8_t('{')]   =
			s_translateKey[uint8_t('[')]   = KeyboardState::Key::LeftBracket;
			s_translateKey[uint8_t('}')]   =
			s_translateKey[uint8_t(']')]   = KeyboardState::Key::RightBracket;

			s_translateKey[uint8_t('<')]   =
			s_translateKey[uint8_t(',')]   = KeyboardState::Key::Comma;
			s_translateKey[uint8_t('>')]   =
			s_translateKey[uint8_t('.')]   = KeyboardState::Key::Period;
			s_translateKey[uint8_t('?')]   =
			s_translateKey[uint8_t('/')]   = KeyboardState::Key::Slash;
			s_translateKey[uint8_t('|')]   =
			s_translateKey[uint8_t('\\')]  = KeyboardState::Key::Backslash;

			s_translateKey[uint8_t('~')]   =
			s_translateKey[uint8_t('`')]   = KeyboardState::Key::Tilde;

			s_translateKey[uint8_t('0')]   = KeyboardState::Key::Key0;
			s_translateKey[uint8_t('1')]   = KeyboardState::Key::Key1;
			s_translateKey[uint8_t('2')]   = KeyboardState::Key::Key2;
			s_translateKey[uint8_t('3')]   = KeyboardState::Key::Key3;
			s_translateKey[uint8_t('4')]   = KeyboardState::Key::Key4;
			s_translateKey[uint8_t('5')]   = KeyboardState::Key::Key5;
			s_translateKey[uint8_t('6')]   = KeyboardState::Key::Key6;
			s_translateKey[uint8_t('7')]   = KeyboardState::Key::Key7;
			s_translateKey[uint8_t('8')]   = KeyboardState::Key::Key8;
			s_translateKey[uint8_t('9')]   = KeyboardState::Key::Key9;

			for (char ch = 'a'; ch <= 'z'; ++ch)
			{
				s_translateKey[uint8_t(ch)]       =
				s_translateKey[uint8_t(ch - ' ')] = KeyboardState::Key::KeyA + (ch - 'a');
			}
		}

		int32_t run(int _argc, const char* const* _argv)
		{
			static const char* canvas = "#canvas";

			EMSCRIPTEN_CHECK(emscripten_set_mousedown_callback(canvas, this, true, mouseCb) );
			EMSCRIPTEN_CHECK(emscripten_set_mouseup_callback(canvas, this, true, mouseCb) );
			EMSCRIPTEN_CHECK(emscripten_set_mousemove_callback(canvas, this, true, mouseCb) );

			EMSCRIPTEN_CHECK(emscripten_set_wheel_callback(canvas, this, true, wheelCb) );

			EMSCRIPTEN_CHECK(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );
			EMSCRIPTEN_CHECK(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );
			EMSCRIPTEN_CHECK(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, keyCb) );

			EMSCRIPTEN_CHECK(emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, resizeCb) );

			EmscriptenFullscreenStrategy fullscreenStrategy = {};
			fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
			fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
			fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
			fullscreenStrategy.canvasResizedCallback = canvasResizeCb;
			fullscreenStrategy.canvasResizedCallbackUserData = this;

			//EMSCRIPTEN_CHECK(emscripten_request_fullscreen_strategy(canvas, false, &fullscreenStrategy) );

			EMSCRIPTEN_CHECK(emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );
			EMSCRIPTEN_CHECK(emscripten_set_focusin_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );
			EMSCRIPTEN_CHECK(emscripten_set_focusout_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, focusCb) );

			int32_t result = main(_argc, _argv);

			return result;
		}

		static EM_BOOL mouseCb(int eventType, const EmscriptenMouseEvent* event, void* userData);
		static EM_BOOL wheelCb(int eventType, const EmscriptenWheelEvent* event, void* userData);
		static EM_BOOL keyCb(int eventType, const EmscriptenKeyboardEvent* event, void* userData);
		static EM_BOOL resizeCb(int eventType, const EmscriptenUiEvent* event, void* userData);
		static EM_BOOL canvasResizeCb(int eventType, const void* reserved, void* userData);
		static EM_BOOL focusCb(int eventType, const EmscriptenFocusEvent* event, void* userData);

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
			mask |= KeyboardState::Modifier::LShift | KeyboardState::Modifier::RShift;
		}

		if (_event->altKey)
		{
			mask |= KeyboardState::Modifier::LAlt | KeyboardState::Modifier::RAlt;
		}

		if (_event->ctrlKey)
		{
			mask |= KeyboardState::Modifier::LCtrl | KeyboardState::Modifier::RCtrl;
		}

		if (_event->metaKey)
		{
			mask |= KeyboardState::Modifier::LMeta | KeyboardState::Modifier::RMeta;
		}

		return mask;
	}

	KeyboardState::Key handleKeyEvent(const EmscriptenKeyboardEvent* _event, uint8_t* _specialKeys, uint8_t* _pressedChar)
	{
		*_pressedChar = uint8_t(_event->keyCode);

		int32_t keyCode = int32_t(_event->keyCode);
		*_specialKeys = translateModifiers(_event);

		if (_event->charCode == 0)
		{
			switch (keyCode)
			{
				case 112: return KeyboardState::Key::F1;
				case 113: return KeyboardState::Key::F2;
				case 114: return KeyboardState::Key::F3;
				case 115: return KeyboardState::Key::F4;
				case 116: return KeyboardState::Key::F5;
				case 117: return KeyboardState::Key::F6;
				case 118: return KeyboardState::Key::F7;
				case 119: return KeyboardState::Key::F8;
				case 120: return KeyboardState::Key::F9;
				case 121: return KeyboardState::Key::F10;
				case 122: return KeyboardState::Key::F11;
				case 123: return KeyboardState::Key::F12;

				case  37: return KeyboardState::Key::Left;
				case  39: return KeyboardState::Key::Right;
				case  38: return KeyboardState::Key::Up;
				case  40: return KeyboardState::Key::Down;
			}
		}

		// if this is a unhandled key just return None
		if (keyCode < 256)
		{
			return KeyboardState::Key(s_translateKey[keyCode]);
		}

		return KeyboardState::Key::None;
	}

	EM_BOOL Context::keyCb(int32_t _eventType, const EmscriptenKeyboardEvent* _event, void* _userData)
	{
		BX_UNUSED(_userData);

		if (_event)
		{
			uint8_t modifiers = 0;
			uint8_t pressedChar[4];
			KeyboardState::Key key = handleKeyEvent(_event, &modifiers, &pressedChar[0]);

			// Returning true means that we take care of the key (instead of the default behavior)
			if (key != KeyboardState::Key::None)
			{
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
							enum { ShiftMask = KeyboardState::Modifier::LShift|KeyboardState::Modifier::RShift };
							s_ctx.m_eventQueue.postCharEvent(kDefaultWindowHandle, 1, pressedChar);
							s_ctx.m_eventQueue.postKeyEvent(kDefaultWindowHandle, key, modifiers, true);
							return true;
						}
						break;

					case EMSCRIPTEN_EVENT_KEYUP:
						s_ctx.m_eventQueue.postKeyEvent(kDefaultWindowHandle, key, modifiers, false);
						return true;
				}
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
		return false;
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
		*_width		= 1920;
		*_height	= 1080;
	}

	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags, const char* _title)
	{
		BX_UNUSED(_app, _x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };

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
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		BX_UNUSED(_handle, _title);
	}

	void setWindowFlags(WindowHandle _handle, uint32_t _flags, bool _enabled)
	{
		BX_UNUSED(_handle, _flags, _enabled);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		BX_UNUSED(_handle);
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		BX_UNUSED(_handle, _lock);
	}
	 
	void* windowGetNativeHandle(WindowHandle _handle)
	{
		if (kDefaultWindowHandle.idx == _handle.idx)
		{
			return (void*)"#canvas";
		}

		return NULL;
	}

	void* getNativeDisplayHandle()
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
